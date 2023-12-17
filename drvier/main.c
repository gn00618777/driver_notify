#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/fs.h>

#define CLASS_NAME			"my_poll_test_class"
#define DEVICE_NAME			"my_poll_device"
#define BUFFER_SIZE			(128)

#define MY_IOC_MAGIC 'k'
#define IOCTL_SET_BUFFER_LEN _IO(MY_IOC_MAGIC, 1)

struct DRIVER_CONTEXT {
	struct cdev c_dev;
	struct class *dev_class;
	char buffer[BUFFER_SIZE];
	wait_queue_head_t read_queue;
};

static int dev_major = 0;
static int dev_minor = 0;
static int buffer_len = 0;
static struct DRIVER_CONTEXT g_my_ctx;

static int my_driver_open(struct inode *inode, struct file *file) {
	printk("Character device opened\n");
	return 0;
}

static int my_driver_release(struct inode *inode, struct file *file) {
	printk("Character device released\n");
	return 0;
}

static ssize_t my_driver_read(struct file *file, char __user *user_buffer, size_t count, loff_t *ppos) {
	ssize_t ret = 0;
	struct DRIVER_CONTEXT *p_ctx = (struct DRIVER_CONTEXT *)&g_my_ctx;

	if(count > BUFFER_SIZE)
		count = BUFFER_SIZE;

	// If no data available, wait for notification.
	//TODO do we need?
	wait_event_interruptible(p_ctx->read_queue, buffer_len > 0);

	if(buffer_len > 0) {
		// Copy data to user space
		if(copy_to_user(user_buffer, p_ctx->buffer, count)) {
			ret = -EFAULT;
		} else {
			buffer_len = 0; // Reset buffer after reading
			ret = count;
		}
	}

    printk("Read %zd bytes from the device\n", ret);
    return ret;
}

static unsigned int my_driver_poll(struct file *file, poll_table *wait) {
	unsigned int mask = 0;
	struct DRIVER_CONTEXT *p_ctx = (struct DRIVER_CONTEXT *)&g_my_ctx;
	printk("%s\n", __func__);

	//add waitqueue to poll table
	poll_wait(file, &p_ctx->read_queue, wait);

	if(buffer_len > 0)
		mask |= POLLIN;

	return mask;
}

static void notify_buffer_len_changed(void)
{
	struct DRIVER_CONTEXT *p_ctx = (struct DRIVER_CONTEXT *)&g_my_ctx;

	buffer_len = 128;

	wake_up_interruptible(&p_ctx->read_queue);

	return;
}

static long my_driver_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
		case IOCTL_SET_BUFFER_LEN:
			printk("Get ioctl cmd\n");
			notify_buffer_len_changed();
		break;
		default:
			return -ENOTTY; // Not implemented
	}

	return 0;
}

static const struct file_operations char_device_fops = {
    .owner = THIS_MODULE,
    .open = my_driver_open,
    .release = my_driver_release,
    .read = my_driver_read,
    .poll = my_driver_poll,
    .unlocked_ioctl = my_driver_ioctl,
};

static void my_driver_context_init(struct DRIVER_CONTEXT *p_ctx)
{
	memset(p_ctx->buffer, 0, BUFFER_SIZE);
	p_ctx->dev_class = NULL;
	init_waitqueue_head(&p_ctx->read_queue);

	return;
}

static int __init my_driver_init(void)
{
	int result;
	struct device *class_dev = NULL;
	struct DRIVER_CONTEXT *p_ctx = (struct DRIVER_CONTEXT *)&g_my_ctx;

	my_driver_context_init(p_ctx);

	result = alloc_chrdev_region(&((p_ctx->c_dev).dev), 0, 1, DEVICE_NAME);
	if(result < 0) {
		printk("Failed to allocate character device number\n");
		return result;
	}

	dev_major = MAJOR(p_ctx->c_dev.dev);

	cdev_init(&p_ctx->c_dev, &char_device_fops);

	result = cdev_add(&p_ctx->c_dev, MKDEV(dev_major, dev_minor), 1);
	if(result < 0) {
		printk("Failed to add character device\n");
		unregister_chrdev_region(MKDEV(dev_major, dev_minor), 1);
			return result;
	}

	/*register class*/
        p_ctx->dev_class = class_create(THIS_MODULE, CLASS_NAME);
        if(IS_ERR(p_ctx->dev_class))
                goto error;

        class_dev = device_create(p_ctx->dev_class, NULL, MKDEV(dev_major, dev_minor), NULL, DEVICE_NAME);

	printk("Character device registered: %s\n", DEVICE_NAME);

	return 0;

error:
	cdev_del(&p_ctx->c_dev);
	unregister_chrdev_region((p_ctx->c_dev).dev, 1);

	return 0;
}
void my_driver_exit(void)
{
	struct DRIVER_CONTEXT *p_ctx = (struct DRIVER_CONTEXT *)&g_my_ctx;

	device_destroy(p_ctx->dev_class, MKDEV(dev_major, dev_minor));
        class_destroy(p_ctx->dev_class);

	cdev_del(&p_ctx->c_dev);
	unregister_chrdev_region((p_ctx->c_dev).dev, 1);
	printk("exit driver\n");
}

module_init(my_driver_init);
module_exit(my_driver_exit);
MODULE_AUTHOR("Bernie Chen <s990560020@gmail.com>");
MODULE_DESCRIPTION("For poll test");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
