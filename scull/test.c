#include<fcntl.h>
#include<stdio.h>

int main(int argc,char *argv[])
{
    int fd=open("/dev/scullc0",O_RDWR);
    int file=open(argv[1],O_RDWR);
    
    close(fd);
    close(file);
    return 0;
}