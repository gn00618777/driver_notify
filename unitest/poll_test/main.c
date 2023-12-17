#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#define TEST_NG		(-1)
#define TEST_OK		(0)
#define DEVICE_NAME	"/dev/my_poll_device"

int main()
{
	int fd = -1, ret = 0;
	struct pollfd fds[1];

	fd = open(DEVICE_NAME, O_RDONLY);
	if(fd < 0) {
		perror("Open:");
		return TEST_NG;
	}

	fds[0].fd = fd;
	fds[0].events = POLLIN;

	while(1) {
		ret = poll(fds, 1, -1);  // Infinite timeout
		if(ret < 0) {
			perror("Poll");
			continue;
		} else {
			if(fds[0].revents & POLLIN) {
				char buffer[1024] = {0};
				int bytes_read = read(fd, buffer, sizeof(buffer));
				if(bytes_read < 0) {
					perror("Read");
					close(fd);
					break;
				} else if(bytes_read > 0) {
					printf("%d data read\n", bytes_read);	
				}
			}
		}
	}

	return TEST_OK;
}
