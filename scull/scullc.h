#include<linux/module.h>
#include<linux/types.h>
#include<linux/kdev_t.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/cdev.h>
#include<linux/kernel.h>
#include<asm/uaccess.h>
#include<linux/errno.h>

MODULE_AUTHOR("lhc");
MODULE_LICENSE("Dual BSD/GPL");

struct dataset{
    char *item;
    struct dataset *next;
};

struct scullc_dev{
    struct dataset *data;
    int size;
    int itemsize;
    struct cdev cdev;
};

// struct scullc_dev{
//     struct scullc_qset *next;
//     int quantum;              /* the current allocation size */
//     int qset;                 /* the current array size */
//     size_t size;              /* 32-bit will suffice */
//     struct semaphore sem;     /* Mutual exclusion */
//     struct cdev cdev;
// };

// struct scullc_qset{
//     char **data;
//     struct scullc_qset *next;
// };

int scullc_open(struct inode *,struct file *);
int scullc_release(struct inode*,struct file *);
ssize_t scullc_read(struct file *,char *,size_t,loff_t *);
ssize_t scullc_write(struct file *,const char *,size_t,loff_t *);
int scullc_trim(struct scullc_dev *);
struct scullc_qset *scullc_follow(struct scullc_dev *dev,int n);