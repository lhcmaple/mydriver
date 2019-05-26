#include"scullc.h"

dev_t devno;
struct scullc_dev scull_dev={
    .data=NULL,
    .size=0
};

struct file_operations my_fops={
    .owner=THIS_MODULE,
    .read=scullc_read,
    .write=scullc_write,
    .open=scullc_open,
    .release=scullc_release
};

int scullc_trim(struct scullc_dev *dev)
{
    printk(KERN_DEBUG"starting to trim\n");
    kfree(dev->data);
    dev->data=NULL;
    dev->size=0;
    printk(KERN_DEBUG"success to trim\n");
    return 0;
}

ssize_t scullc_read(struct file *filp,char *buf,size_t count,loff_t *f_pos)
{
    printk(KERN_DEBUG"starting to read\n");
    struct scullc_dev *dev=filp->private_data;
    if(dev->size<=*f_pos)
    {
        return 0;
    }
    count=dev->size-*f_pos;
    if(raw_copy_to_user(buf,dev->data+*f_pos,count))
    {
        return 0;
    }
    *f_pos+=count;
    printk(KERN_DEBUG"success to read (%d,%d,%d):\n",dev->size,count,*f_pos);
    return count;
}

ssize_t scullc_write(struct file *filp, const char *buf, size_t count,loff_t *f_pos)
{
    printk(KERN_DEBUG"starting to write\n");
    struct scullc_dev *dev=filp->private_data;
    if(dev->size<*f_pos)
    {
        return -EFAULT;
    }
    raw_copy_from_user(dev->data+*f_pos,buf,count);
    *f_pos+=count;
    dev->size=dev->size<*f_pos?*f_pos:dev->size;
    printk(KERN_DEBUG"success to write (%d,%d,%d):\n",dev->size,count,*f_pos);
    return count;
}

int scullc_open(struct inode *inode,struct file *filp)
{
    printk(KERN_DEBUG"starting to open\n");
    struct scullc_dev *dev;
    dev=container_of(inode->i_cdev,struct scullc_dev,cdev);
    filp->private_data=dev;
    if((filp->f_flags&O_ACCMODE)==O_WRONLY)
    {
        scullc_trim(dev);
        dev->data=kmalloc(1000,GFP_KERNEL);
        dev->size=0;
    }
    printk(KERN_DEBUG"success to open\n");
    return 0;
}

int scullc_release (struct inode *inode, struct file *filp)
{
    printk(KERN_DEBUG"starting to release\n");
    // scullc_trim((struct scullc_dev *)filp->private_data);
    printk(KERN_DEBUG"start to release\n");
    return 0;
}

static int scullc_init(void)
{
    int result;
    printk(KERN_DEBUG"starting to init\n");
    result=alloc_chrdev_region(&devno,0,4,"scullc");
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
    scull_dev.data=kmalloc(1000,GFP_KERNEL);
    scull_dev.size=0;
    printk(KERN_DEBUG"success to init\n");
    return 0;
}

static void scullc_cleanup(void)
{
    printk(KERN_DEBUG"starting to cleanup\n");
    scullc_trim(&scull_dev);
    cdev_del(&scull_dev.cdev);
    unregister_chrdev_region(devno,4);
    printk(KERN_DEBUG"success to cleanup\n");
}

module_init(scullc_init);
module_exit(scullc_cleanup);

// #include"scullc.h"

// dev_t devno;
// struct scullc_dev scull_dev={
//     .next=NULL,
//     .quantum=4000,
//     .qset=1000,
//     .size=0
// };

// struct file_operations my_fops={
//     .owner=THIS_MODULE,
//     .read=scullc_read,
//     .write=scullc_write,
//     .open=scullc_open,
//     .release=scullc_release
// };

// struct scullc_qset *scullc_follow(struct scullc_dev *dev, int n)
// {
//     printk(KERN_DEBUG"starting to follow\n");
//     if(dev->next==NULL)
//     {
//         dev->next=kmalloc(sizeof(struct scullc_qset),GFP_KERNEL);
//         memset(dev->next,0,sizeof(struct scullc_qset));
//     }
//     struct scullc_qset *dptr=dev->next;
//     while(n--)
//     {
//         if(dptr->next==NULL)
//         {
//             dptr->next=kmalloc(sizeof(struct scullc_qset),GFP_KERNEL);
//             memset(dptr->next,0,sizeof(struct scullc_qset));
//         }
//         dptr=dptr->next;
//     }
//     printk(KERN_DEBUG"success to follow\n");
//     return dptr;
// }

// int scullc_trim(struct scullc_dev *dev)
// {
//     printk(KERN_DEBUG"starting to trim\n");
//     struct scullc_qset *dptr,*next;
//     int i,qset=dev->qset;
//     for(dptr=dev->next;dptr;dptr=next)
//     {
//         for(i=0;i<qset;++i)
//         {
//             kfree((dptr->data)[i]);
//         }
//         kfree(dptr->data);
//         next=dptr->next;
//         kfree(dptr);
//     }
//     dev->size=0;
//     dev->next=NULL;
//     printk(KERN_DEBUG"success to trim\n");
//     return 0;
// }

// ssize_t scullc_read(struct file *filp,char *buf,size_t count,loff_t *f_pos)
// {
//     printk(KERN_DEBUG"starting to read\n");
//     struct scullc_dev *dev=filp->private_data;
//     struct scullc_qset *dptr;
//     int quantum=dev->quantum,qset=dev->qset;
//     int itemsize=quantum*qset;
//     int item,s_pos,q_pos,rest;
//     ssize_t retval=0;
//     if(*f_pos>=dev->size)
//     {
//         return retval;
//     }
//     if(*f_pos+count>dev->size)
//     {
//         count=dev->size-*f_pos;
//     }
//     item=*f_pos/itemsize;
//     rest=*f_pos%itemsize;
//     s_pos=rest/quantum;
//     q_pos=rest%quantum;
//     dptr=scullc_follow(dev,item);
//     printk(KERN_DEBUG"flag1\n");
//     if(dptr->data==NULL||dptr->data[s_pos]==NULL)
//     {
//         return retval;
//     }
//     printk(KERN_DEBUG"flag2\n");
//     if(count>quantum-q_pos)
//     {
//         count=quantum-q_pos;
//     }
//     printk(KERN_DEBUG"flag3\n");
//     if(raw_copy_to_user(buf,dptr->data[q_pos],count))
//     {
//         return 0;
//     }
//     printk(KERN_DEBUG"flag4\n");
//     *f_pos+=count;
//     retval=count;
//     printk(KERN_DEBUG"success to read (size %d):\n",dev->size);
//     return retval;
// }

// ssize_t scullc_write(struct file *filp, const char *buf, size_t count,loff_t *f_pos)
// {
//     printk(KERN_DEBUG"starting to write\n");
//     struct scullc_dev *dev=filp->private_data;
//     struct scullc_qset *dptr;
//     int quantum=dev->quantum,qset=dev->qset;
//     int itemsize=quantum*qset;
//     int item,s_pos,q_pos,rest;
//     ssize_t retval=0;
//     item=*f_pos/itemsize;
//     rest=*f_pos%itemsize;
//     s_pos=rest/quantum;
//     q_pos=rest%quantum;
//     dptr=scullc_follow(dev,item);
//     if(dptr->data==NULL)
//     {
//         dptr->data=kmalloc(dev->qset*sizeof(char *),GFP_KERNEL);
//     }
//     if(dptr->data[s_pos]==NULL)
//     {
//         dptr->data[s_pos]=kmalloc(dev->quantum,GFP_KERNEL);
//     }
//     if(count>quantum-q_pos)
//     {
//         count=quantum-q_pos;
//     }
//     raw_copy_from_user(dptr->data[s_pos]+q_pos,buf,count);
//     *f_pos+=count;
//     retval=count;
//     dev->size=dev->size<*f_pos?dev->size:*f_pos;
//     printk(KERN_DEBUG"success to write (size %d,%d,%d):\n",dev->size,count,*f_pos);
//     return retval;
// }

// int scullc_open(struct inode *inode,struct file *filp)
// {
//     printk(KERN_DEBUG"starting to open\n");
//     struct scullc_dev *dev;
//     dev=container_of(inode->i_cdev,struct scullc_dev,cdev);
//     filp->private_data=dev;
//     if((filp->f_flags&O_ACCMODE)==O_WRONLY)
//     {
//         scullc_trim(dev);
//     }
//     printk(KERN_DEBUG"success to open\n");
//     return 0;
// }

// int scullc_release (struct inode *inode, struct file *filp)
// {
//     printk(KERN_DEBUG"starting to release\n");
//     // scullc_trim((struct scullc_dev *)filp->private_data);
//     printk(KERN_DEBUG"start to release\n");
//     return 0;
// }

// static int scullc_init(void)
// {
//     int result;
//     printk(KERN_DEBUG"starting to init\n");
//     result=alloc_chrdev_region(&devno,0,4,"scullc");
//     if(result)
//     {
//         printk(KERN_WARNING"failed to alloc the dev_t\n");
//         return result;
//     }
//     printk(KERN_DEBUG"success to alloc the dev_t\n");

//     cdev_init(&scull_dev.cdev,&my_fops);
//     scull_dev.cdev.owner=THIS_MODULE;
//     result=cdev_add(&scull_dev.cdev,devno,4);
//     if(result)
//     {
//         printk(KERN_WARNING"failed to adding scull_dev\n");
//         return result;
//     }
//     printk(KERN_DEBUG"success to init\n");
//     return 0;
// }

// static void scullc_cleanup(void)
// {
//     printk(KERN_DEBUG"starting to cleanup\n");
//     scullc_trim(&scull_dev);
//     cdev_del(&scull_dev.cdev);
//     unregister_chrdev_region(devno,4);
//     printk(KERN_DEBUG"success to cleanup\n");
// }

// module_init(scullc_init);
// module_exit(scullc_cleanup);
