#include<linux/module.h>
#include<linux/types.h>
#include<linux/kdev_t.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/cdev.h>
#include<linux/kernel.h>
#include<asm/uaccess.h>
#include<linux/errno.h>
#include<linux/wait.h>
#include<linux/semaphore.h>
#include<linux/sched.h>
#include<linux/sched/signal.h>
#include<linux/poll.h>

#define AVAIL ((dev->rpos-dev->wpos-1+dev->size)%dev->size)

MODULE_AUTHOR("lhc");
MODULE_LICENSE("Dual BSD/GPL");

struct scullpipe_dev{
    char *data;
    int rpos;
    int wpos;
    int size;
    struct cdev cdev;
};

int scullpipe_open(struct inode *,struct file *);
int scullpipe_release(struct inode*,struct file *);
ssize_t scullpipe_read(struct file *,char *,size_t,loff_t *);
ssize_t scullpipe_write(struct file *,const char *,size_t,loff_t *);
unsigned int scullpipe_poll(struct file *,poll_table *);

kuid_t uid;
int ucount=0;
// atomic_t avail=ATOMIC_INIT(1);
spinlock_t ulock;
struct semaphore rsem;
struct semaphore wsem;

wait_queue_head_t inq;
wait_queue_head_t outq;

dev_t devno;

struct scullpipe_dev scull_dev={
    .data=NULL,
    .rpos=0,
    .wpos=0,
    .size=100
};

struct file_operations my_fops={
    .owner=THIS_MODULE,
    .read=scullpipe_read,
    .write=scullpipe_write,
    .poll=scullpipe_poll,
    
    .open=scullpipe_open,
    .release=scullpipe_release
};