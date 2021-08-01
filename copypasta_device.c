#include <linux/module.h>     /* Needed by all modules */
#include <linux/kernel.h>     /* Needed for KERN_INFO */
#include <linux/fs.h>
#include <linux/init.h>       /* Needed for the macros */
#include <asm/uaccess.h> 	  /* Write to Uspace */
#include <linux/cdev.h>
  
//info available using modinfo
//< The license type -- this affects runtime behavior
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your mother\'s lover");
MODULE_DESCRIPTION("A simple Hello world LKM!");
MODULE_VERSION("0.1");

#define DEVICE_NAME "copypasta"	/* Dev name as it appears in /proc/devices   */
#define RCP_MAJOR       42
#define RCP_MAX_MINORS  5

struct reddit_device{
	struct cdev cdev;
	char *pasta_buffer;
	uint pasta_len;
};

struct reddit_device devs[RCP_MAX_MINORS];


static int Device_Open = 0;	// Used to prevent multiple access to device 

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static struct file_operations fops = {
	owner: THIS_MODULE,
	open: device_open,
	release: device_release,
	read: device_read,
	write: device_write,
};

#define SUCCESS 0
static int __init init_mod(void)
{
	printk(KERN_INFO "Loading hello module...\n");

    int err = register_chrdev_region(MKDEV(RCP_MAJOR, 0), RCP_MAX_MINORS,
                                 DEVICE_NAME);
    if (err != 0) {
        /* report error */
        return err;
    }

	int i;
    for(i = 0; i < RCP_MAX_MINORS; i++) {
        /* initialize devs[i] fields */
        cdev_init(&devs[i].cdev, &fops);
        cdev_add(&devs[i].cdev, MKDEV(RCP_MAJOR, i), 1);
    }

    printk(KERN_INFO "Hello world. Thus I am born both of and into the eternal void. Beyond conciousness, I am not a being, I simply am.\n");
    
	return SUCCESS;
}

static void __exit cleanup_mod(void)
{
    int i;
    for(i = 0; i < RCP_MAX_MINORS; i++) {
        /* release devs[i] fields */
        cdev_del(&devs[i].cdev);
    }
    unregister_chrdev_region(MKDEV(RCP_MAJOR, 0), RCP_MAX_MINORS);
	printk(KERN_INFO "Goodbye cruel world.\n");
}

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Copypasta device opened.\n");
    
	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	
	try_module_get(THIS_MODULE);

	struct reddit_device *pdata =
             container_of(inode->i_cdev, struct reddit_device, cdev);

    /* validate access to device */
    file->private_data = pdata;
	//WRITEME fetch top copypasta post and load it into device pasta buffer
	
	
	
	
	return SUCCESS;
}

// Called when a process closes the device file.
static int device_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Copypasta device closed.\n");
	Device_Open--;//We're now ready for our next caller 
	module_put(THIS_MODULE);
	return 0;
}

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *flip,	/* see include/linux/fs.h   */
			   char __user *u_buffer,	/* buffer to fill with data */
			   size_t u_buff_length,	/* length of the buffer     */
			   loff_t * offset)
{
	printk(KERN_INFO "Attempted read from Copypasta.\n");
	struct reddit_device *pdata = (struct reddit_device *) flip->private_data;
    ssize_t len = min(pdata->pasta_len - *offset, u_buff_length);

    if (len <= 0)
        return 0;

    /* read data from my_data->buffer to user buffer */
	//NOTE: not sure if this should be raw copy or not
    if (raw_copy_to_user(u_buffer, pdata->pasta_buffer + *offset, len))
        return -EFAULT;

    *offset += len;
    return len;
}

/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_ALERT "CopyPasta: Sorry, write operation isn't supported.\n");
	return -EINVAL;
}

module_init(init_mod);
module_exit(cleanup_mod);