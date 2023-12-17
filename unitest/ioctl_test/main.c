#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define MY_IOC_MAGIC 'k'
#define IOCTL_SET_BUFFER_LEN _IO(MY_IOC_MAGIC, 1)

#define TEST_OK		(0)
#define TEST_NG		(-1)

#define DEVICE_NAME	"/dev/my_poll_device"

int main()
{
	int fd = open(DEVICE_NAME, O_RDWR);

	if(fd < 0) {
		perror("Open:");
		return TEST_NG;
	}

	// Perform IOCTL command
	if(ioctl(fd, IOCTL_SET_BUFFER_LEN) < 0) {
		perror("IOCTL:");
		close(fd);
		return TEST_NG;
	}

	close(fd);

	return TEST_OK;
}
