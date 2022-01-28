#include <stdio.h>
#include <string.h>
#include <linux/gpio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int main() {
    int t;
    int ret;
    int fd = open("/dev/espclock", 0);

    printf("fd %d\n", fd);
    int i = 0;
    while (i++ < 1000) {
       t=1;
       ret = read(fd, &t, sizeof(int));
       printf("read %d\n", t);
       //event available
       if ( ret == sizeof(int) ) {
       }

    }
    close(fd);

}
