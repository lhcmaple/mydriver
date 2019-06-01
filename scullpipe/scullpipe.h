#include<linux/module.h>
#include<linux/types.h>
#include<linux/kdev_t.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/cdev.h>
#include<linux/kernel.h>
#include<asm/uaccess.h>
#include<linux/errno.h>
#include<linux/spinlock.h>
#include<linux/wait.h>

MODULE_AUTHOR("lhc");
MODULE_LICENSE("Dual BSD/GPL");

struct scullpipe_dev{
    char *data;
    int rpos;
    int wpos;
    int avail;
    int size;
    struct cdev cdev;
};

DEFINE_SPINLOCK(rlock);
DEFINE_SPINLOCK(wlock);

wait_queue_head_t my_queue;
int cond=0;

int scullpipe_open(struct inode *,struct file *);
int scullpipe_release(struct inode*,struct file *);
ssize_t scullpipe_read(struct file *,char *,size_t,loff_t *);
ssize_t scullpipe_write(struct file *,const char *,size_t,loff_t *);