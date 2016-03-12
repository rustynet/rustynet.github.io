 /*
	FreeBSD ZCS Device Switcher
	(c) ze_d0g [CiN] 2k3
 */

#include <sys/types.h>
#include <stdio.h>
#include <dev/ppbus/ppi.h>
#include <dev/ppbus/ppbconf.h>
#include <fcntl.h>

main(int argc, char **argv) {
 u_int8_t value=0;
 int fd=open("/dev/ppi0",O_RDWR);

 if(argc<2) { 
	printf ("\tuze : 'zcs ON' or 'zcs OFF'\n");
	exit(0);
 }

 if (argv[1][1] == 78) value=1;

 ioctl(fd,PPISDATA,&value);
 
 close(fd);
}
