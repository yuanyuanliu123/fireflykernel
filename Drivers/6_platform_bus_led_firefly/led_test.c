#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, const char *argv[]) {
	int fd; 
	char status;
	int ret;

	if(argc != 3) {
		printf("para must rathor than three, Usage: ./%s <dev> <on/off>", argv[0]);
		return -1;
	}

	fd = open(argv[1], O_RDWR);
	if(fd < 0) {
		printf("can't open file %s\n", argv[1]);
		return -1;
	}

	if(strcmp("on", argv[2]) == 0) {
		printf("I'll open led\n");
		status = 1;
	}
	else {
		printf("I'll close led\n");
		status = 0;
	}
	ret = write(fd, &status, 1);

	close(fd);
	
	return 0;
}
