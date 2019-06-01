#include"scullpipe.h"
d
dev_t devno;
struct scullpipe_dev scull_dev={
.data=NULL,
.rpos=0,
.wpos=0,
.avail=99,
.size=100,
};

struct file_operations my_fops={
.owner=THIS_MODULE,
.read=scullpipe_read,
.write=scullpipe_write,
.open=scullpipe_open,
.release=scullpipe_release
};

ssize_t scullpipe_read(struct file *filp,char *buf,size_t count,loff_t *f_pos)
{
struct scullpipe_dev *dev=filp->private_data;
int retval;
printk(KERN_DEBUG"starting to read%d,%d,%d,%dn",current->pid,dev->rpos,dev->wpos,dev->avail);
// wait_event_interruptible(my_queue,cond!=0);
// cond=0;
// spin_lock(&rlock);
if(dev->size-dev->avail-1==0)
{
// spin_unlock(&rlock);
return 0;
}
if(count>dev->size-dev->avail-1)
count=dev->size-dev->avail-1;
retval=count;
dev->avail+=count;
if(dev->rpos<dev->wpos)
{
raw_copy_to_user(buf,dev->data+dev->rpos,count);
dev->rpos+=count;
}
else
{
if(dev->size-dev->rpos<=count)
{
count-=dev->size-dev->rpos;
raw_copy_to_user(buf,dev->data+dev->rpos,dev->size-dev->rpos);
if(count)
{
raw_copy_to_user(buf+dev->size-dev->rpos,dev->data,count);
dev->rpos=count;
}
else
dev->rpos=0;
}
else
{
raw_copy_to_user(buf,dev->data+dev->rpos,count);
dev->rpos+=count;
}
}
// spin_unlock(&rlock);
printk(KERN_DEBUG"%sn",dev->data);
printk(KERN_DEBUG"success to read%d,%d,%d,%dn",current->pid,dev->rpos,dev->wpos,dev->avail);
return retval;
}

ssize_t scullpipe_write(struct file *filp, const char *buf, size_t count,loff_t *f_pos)
{
struct scullpipe_dev *dev=filp->private_data;
int retval;
printk(KERN_DEBUG"starting to write%d,%d,%d,%dn",current->pid,dev->rpos,dev->wpos,dev->avail);
// cond=1;
// wake_up_interruptible(&my_queue);
// spin_lock(&wlock);
if(dev->avail==0)
{
// spin_unlock(&wlock);
return 0;
}
if(dev->avail<count)
count=dev->avail;
retval=count;
dev->avail-=count;
if(dev->wpos>=dev->rpos)
{
if(dev->size-dev->wpos<=count)
{
count-=dev->size-dev->wpos;
raw_copy_from_user(dev->data+dev->wpos,buf,dev->size-dev->wpos);
if(count)
{
raw_copy_from_user(dev->data,buf+dev->size-dev->wpos,count);
dev->wpos=count;
}
else
dev->wpos=0;
}
else
{
raw_copy_from_user(dev->data+dev->wpos,buf,count);
dev->wpos+=count;
}
}
else
{
raw_copy_from_user(dev->data+dev->wpos,buf,count);
dev->wpos+=count;
}
// spin_unlock(&wlock);
printk(KERN_DEBUG"%sn",dev->data);
printk(KERN_DEBUG"success to write%d,%d,%d,%dn",current->pid,dev->rpos,dev->wpos,dev->avail);
return retval;
}

int scullpipe_open(struct inode *inode,struct file *filp)
{
struct scullpipe_dev *dev;
printk(KERN_DEBUG"starting to openn");
dev=container_of(inode->i_cdev,struct scullpipe_dev,cdev);
filp->private_data=dev;
printk(KERN_DEBUG"success to openn");
return 0;
}

int scullpipe_release (struct inode *inode, struct file *filp)
{
printk(KERN_DEBUG"starting to releasen");
printk(KERN_DEBUG"success to releasen");
return 0;
}

static int __init scullpipe_init(void)
{
int result,i;
printk(KERN_DEBUG"starting to initn");
result=alloc_chrdev_region(&devno,0,4,"scullpipe");
if(result)
{
printk(KERN_WARNING"failed to alloc the dev_tn");
return result;
}
printk(KERN_DEBUG"success to alloc the dev_tn");
cdev_init(&scull_dev.cdev,&my_fops);
scull_dev.cdev.owner=THIS_MODULE;
result=cdev_add(&scull_dev.cdev,devno,4);
if(result)
{
printk(KERN_WARNING"failed to adding scull_devn");
return result;
}
scull_dev.data=kmalloc(scull_dev.size+1,GFP_KERNEL);
if(scull_dev.data==NULL)
return -EFAULT;
for(i=0;i<100;++i)
scull_dev.data[i]='0';
scull_dev.data[100]=0;
// init_waitqueue_head(&my_queue);
printk(KERN_DEBUG"success to initn");
return 0;
}

static void __exit scullpipe_cleanup(void)
{
printk(KERN_DEBUG"starting to cleanupn");
kfree(scull_dev.data);
cdev_del(&scull_dev.cdev);
unregister_chrdev_region(devno,4);
printk(KERN_DEBUG"success to cleanupn");
}

module_init(scullpipe_init);
module_exit(scullpipe_cleanup);
