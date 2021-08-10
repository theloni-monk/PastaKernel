/* kernel headers */
#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/fs.h>
#include <linux/init.h>	 /* Needed for the macros */
#include <asm/uaccess.h> /* Write to Uspace */
#include <linux/cdev.h>

/* For socket etc */
#include <linux/socket.h>
#include <linux/net.h>
#include <net/sock.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <asm/uaccess.h>
#include <linux/file.h>
#include <linux/socket.h>
#include <linux/slab.h>

//info available using modinfo
MODULE_LICENSE("GPL"); // license type affects runtime behavior
MODULE_AUTHOR("Your mother\'s lover");
MODULE_DESCRIPTION("A simple Hello world LKM!");
MODULE_VERSION("0.1");

#define DEVICE_NAME "copypasta" /* Dev name as it appears in /proc/devices   */
//mknod copypasta c 42 2
#define RCP_MAJOR 42			// arbitrary in this case
#define RCP_MAX_MINORS 5

// core kernel memory/file structure of our device
struct reddit_device
{
	struct cdev cdev;
	char *pasta_buffer;
	uint pasta_len;
};

struct reddit_device devs[RCP_MAX_MINORS];

static int Device_Open = 0; // Used to prevent multiple access to device

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static struct file_operations fops = {
	owner : THIS_MODULE,
	open : device_open,
	release : device_release,
	read : device_read,
	write : device_write,
};

#define SUCCESS 0
/** 
 * Called when mod is loaded via insmod, registers bytecode to the device file
*/
static int __init init_mod(void){
	printk(KERN_INFO "Loading hello module...\n");

	int err = register_chrdev_region(MKDEV(RCP_MAJOR, 0), RCP_MAX_MINORS,
									 DEVICE_NAME);
	if (err != 0)
		return err;

	int i;
	for (i = 0; i < RCP_MAX_MINORS; i++)
	{
		/* initialize devs[i] fields */
		cdev_init(&devs[i].cdev, &fops);
		cdev_add(&devs[i].cdev, MKDEV(RCP_MAJOR, i), 1);
	}

	printk(KERN_INFO "Hello world. Thus I am born both of and into the eternal void. Beyond conciousness, I am not a being, I simply am.\n");
	return SUCCESS;
}

static void __exit cleanup_mod(void){
	int i;
	for (i = 0; i < RCP_MAX_MINORS; i++)
	{
		/* release devs[i] fields */
		cdev_del(&devs[i].cdev);
	}
	unregister_chrdev_region(MKDEV(RCP_MAJOR, 0), RCP_MAX_MINORS);
	printk(KERN_INFO "Goodbye cruel world.\n");
}
char __reddit_site_ip[4] = {151, 101, 65, 140};
#define REDDIT_ADDR __reddit_site_ip
char __reddit_api_ip[4] = {172, 67, 174, 211};
#define API_ADDR __reddit_api_ip
#define PORT 80
/**
 * Allocate and return string buffer recieved from an emulated http request
*/
#define REQ_LEN 2048
char * kHttpReq(char *addr, char *hostname){
	printk(KERN_INFO, "request called");
	char *request = kmalloc(REQ_LEN,0); 
	sprintf(request,
			"GET %s HTTP/1.1 \nConnection: close ",
			  hostname);
	printk(KERN_INFO,"allocated request");
	/* Build the socket. */
	struct socket *sock;
	int err;
	err = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
	if (err < 0){
		printk(KERN_INFO, "failed to create socket"); 
		return "err";
	}
	/* Build the address. */
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr =
		htonl(addr);
	printk(KERN_INFO, "constructed addr");
	/* Actually connect. */
	err = sock->ops->connect(sock,
								(struct sockaddr *)&servaddr,
								sizeof(servaddr), O_RDWR);
	if (err< 0){
		printk(KERN_INFO, "socket failed to connect");
		return "err";
	}
	printk(KERN_INFO, "socket connected");
	/* Send HTTP request. */
	struct msghdr * msg;
	struct iovec iov;
	msg->msg_name = 0;
	msg->msg_namelen = 0;
	iov.iov_base = request;
	iov.iov_len = REQ_LEN;
	iov_iter_init(&msg->msg_iter, READ, &iov, 1, REQ_LEN);
	msg->msg_control = NULL;
	msg->msg_controllen = 0;
	printk(KERN_INFO, "msg constructed");
	int len = sock_sendmsg(sock, msg);
	//TODO: handle err
	printk(KERN_INFO, "msg set");
	/* Read the response. */
	len = sock_recvmsg(sock, msg, 0);
	printk(KERN_INFO, "response recieved");
	//FIXME: might have to do this in multiple recvs
	char * ret = kmalloc(msg->msg_iter.iov->iov_len,0);
    memcpy(ret, msg->msg_iter.iov->iov_base, len);
	printk(KERN_INFO, "copied response to ret buffer");
	return ret;
	//FIXME: test this
}

/** WRITEME
 * use api.pushshift.io/reddit/search/submission endpoint to find latest top copypasta addr
 * then fetch the post itself 
*/
//char * getPost(void );

/**  WRITEME
 * Parse fetched copypasta post into body text and allocate it to the reddit_device field
 * Note: remember to deallocate shit
*/
//void fetchPasta(reddit_device * pdata);

/* 
 * Fetch and parse Copypasta when a process tries to open the device file
 * ex: "cat /dev/copypasta"
 */
static int device_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "Copypasta device opened.\n");

	if (Device_Open)
		return -EBUSY;

	Device_Open++;

	try_module_get(THIS_MODULE);

	struct reddit_device *pdata =
		container_of(inode->i_cdev, struct reddit_device, cdev);

	pdata->pasta_buffer = kHttpReq(REDDIT_ADDR, "reddit.com/r/copypasta");
	pdata->pasta_len = 2048;
	
	/* validate access to device */
	file->private_data = pdata;
	//WRITEME fetch top copypasta post and load it into device pasta buffer

	
	return SUCCESS;
}

// Called when a process closes the device file.
static int device_release(struct inode *inode, struct file *file){
	//TODO: deallocate everything
	printk(KERN_INFO "Copypasta device closed.\n");
	Device_Open--; //We're now ready for our next caller
	module_put(THIS_MODULE);
	return 0;
}

/* 
 * Send Copypasta string to userspace when a process attempts a device read
 */
static ssize_t device_read(struct file *flip,	  /* dev 'file' contains info of our struct, we retrive by casting it back to a reddit_device*/
						   char __user *u_buffer, /* buffer to fill with data */
						   size_t u_buff_length,  /* length of the buffer     */
						   loff_t *offset){
	printk(KERN_INFO "Attempted read from Copypasta.\n");
	struct reddit_device *pdata = (struct reddit_device *)flip->private_data;
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
 * Throws err when a process writes to dev file: echo "hi" > /dev/copypasta
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t *off){
	printk(KERN_ALERT "CopyPasta: Sorry, write operation isn't supported.\n");
	return -EINVAL;
}

//register device "constructor" to kernel
module_init(init_mod);
module_exit(cleanup_mod);