#include<linux/ioport.h>
#include<linux/slab.h>

MODULE_AUTHOR("lhc");
MODULE_LICENSE("Dual BSD/GPL");



static int short_init(void)
{
    struct resource *rs;
    rs=request_region(0,24,"short");
    if(rs==NULL)
        return -1;
    
}

static int short_cleanup(void)
{
    release_region(0,24);
    
}

module_init(&short_init);
module_exit(&short_cleanup);
