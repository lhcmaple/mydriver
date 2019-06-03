#include"scullpipe.h"

ssize_t scullpipe_read(struct file *filp,char *buf,size_t count,loff_t *f_pos)
{
    struct scullpipe_dev *dev=filp->private_data;
    int retval;
    if(down_interruptible(&rsem))
        return -ERESTARTSYS;
    while(dev->size-AVAIL-1==0)
    {
        up(&rsem);
        DEFINE_WAIT(wait);
        prepare_to_wait(&inq,&wait,TASK_INTERRUPTIBLE);
        if(dev->size-AVAIL-1==0)
            schedule();
        finish_wait(&inq,&wait);
        if(signal_pending(current))
            return -ERESTARTSYS;
        if(down_interruptible(&rsem))
            return -ERESTARTSYS;
    }
    printk(KERN_DEBUG"starting to read%d,%d,%d,%d\n",current->pid,dev->rpos,dev->wpos,AVAIL);
    if(count>dev->size-AVAIL-1)
        count=dev->size-AVAIL-1;
    retval=count;
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
    printk(KERN_DEBUG"%s\n",dev->data);
    printk(KERN_DEBUG"success to read%d,%d,%d,%d\n",current->pid,dev->rpos,dev->wpos,AVAIL);
    up(&rsem);
    wake_up_interruptible(&outq);
    return retval;
}

ssize_t scullpipe_write(struct file *filp, const char *buf, size_t count,loff_t *f_pos)
{
    struct scullpipe_dev *dev=filp->private_data;
    int retval;
    if(down_interruptible(&wsem))
        return -ERESTARTSYS;
    while(AVAIL==0)
    {
        up(&wsem);
        DEFINE_WAIT(wait);
        prepare_to_wait(&outq,&wait,TASK_INTERRUPTIBLE);
        if(AVAIL==0)
            schedule();
        finish_wait(&outq,&wait);
        if(signal_pending(current))
            return -ERESTARTSYS;
        if(down_interruptible(&wsem))
            return -ERESTARTSYS;
    }
    printk(KERN_DEBUG"starting to write%d,%d,%d,%d\n",current->pid,dev->rpos,dev->wpos,AVAIL);
    if(AVAIL<count)
        count=AVAIL;
    retval=count;
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
    printk(KERN_DEBUG"%s\n",dev->data);
    printk(KERN_DEBUG"success to write%d,%d,%d,%d\n",current->pid,dev->rpos,dev->wpos,AVAIL);
    up(&wsem);
    wake_up_interruptible(&inq);
    return retval;
}

unsigned int scullpipe_poll(struct file *filp,poll_table *wait)
{
    struct scullpipe_dev *dev=filp->private_data;
    unsigned int mask=0;
    down_interruptible(&rsem);
    down_interruptible(&wsem);
    poll_wait(filp,&inq,wait);
    poll_wait(filp,&outq,wait);
    if(dev->rpos!=dev->wpos)
        mask|=POLLIN|POLLRDNORM;
    if(AVAIL!=0)
        mask|=POLLOUT|POLLWRNORM;
    up(&wsem);
    up(&rsem);
    return mask;
}

int scullpipe_open(struct inode *inode,struct file *filp)
{
    struct scullpipe_dev *dev;
    printk(KERN_DEBUG"starting to open\n");
    // if(!atomic_dec_and_test(&avail))
    // {
    //     atomic_inc(&avail);
    //     printk(KERN_DEBUG"failed to open\n");
    //     return -EBUSY;
    // }
    spin_lock(&ulock);
    if(ucount)
    {
        if(uid.val!=current->cred->euid.val&&uid.val!=current->cred->uid.val&&!capable(CAP_DAC_OVERRIDE))
        {
            printk(KERN_DEBUG"failed to open\n");
            spin_unlock(&ulock);
            return -EBUSY;
        }
    }
    else
    {
        uid=current->cred->euid;
    }
    ucount++;
    spin_unlock(&ulock);
    dev=container_of(inode->i_cdev,struct scullpipe_dev,cdev);
    filp->private_data=dev;
    printk(KERN_DEBUG"success to open\n");
    return 0;
}

int scullpipe_release (struct inode *inode, struct file *filp)
{
    printk(KERN_DEBUG"starting to release\n");
    spin_lock(&ulock);
    ucount--;
    spin_unlock(&ulock);
    // atomic_inc(&avail);
    printk(KERN_DEBUG"success to release\n");
    return 0;
}

static int __init scullpipe_init(void)
{
    int result,i;
    printk(KERN_DEBUG"starting to init\n");
    result=alloc_chrdev_region(&devno,0,4,"scullpipe");
    if(result)
    {
        printk(KERN_WARNING"failed to alloc the dev_t\n");
        return result;
    }
    printk(KERN_DEBUG"success to alloc the dev_t\n");
    cdev_init(&scull_dev.cdev,&my_fops);
    scull_dev.cdev.owner=THIS_MODULE;
    result=cdev_add(&scull_dev.cdev,devno,4);
    if(result)
    {
        printk(KERN_WARNING"failed to adding scull_dev\n");
        return result;
    }
    scull_dev.data=kmalloc(scull_dev.size+1,GFP_KERNEL);
    if(scull_dev.data==NULL)
        return -EFAULT;
    for(i=0;i<100;++i)
        scull_dev.data[i]='0';
    scull_dev.data[100]=0;
    init_waitqueue_head(&inq);
    init_waitqueue_head(&outq);
    spin_lock_init(&ulock);
    sema_init(&rsem,1);
    sema_init(&wsem,1);
    printk(KERN_DEBUG"success to init\n");
    return 0;
}

static void __exit scullpipe_cleanup(void)
{
    printk(KERN_DEBUG"starting to cleanup\n");
    kfree(scull_dev.data);
    cdev_del(&scull_dev.cdev);
    unregister_chrdev_region(devno,4);
    printk(KERN_DEBUG"success to cleanup\n");
}

module_init(scullpipe_init);
module_exit(scullpipe_cleanup);
