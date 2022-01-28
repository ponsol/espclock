#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>


#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>


MODULE_LICENSE("GPL");


extern struct wait_queue_head myqueue ;
extern int clock_data ;
extern int dataready ;

extern int espclock_int_gpio ;
extern int espclock_en_gpio ;



#define DEVICE_NAME "espclock"
#define CLASS_NAME  "clock"

static int    majornum=-1;
static struct class*  devclass  = NULL; 
static struct device* devdevice = NULL; 

static unsigned int *rec_data = NULL ;
static unsigned int *send_data = NULL ;
static int opencount=0;


static int     _mycdev_open( struct inode *, struct file * );
static ssize_t _mycdev_read( struct file * ,        char *  , size_t, loff_t *);
static ssize_t _mycdev_write(struct file * , const  char *  , size_t, loff_t *);
static int     _mycdev_release(struct inode *, struct file * );


DECLARE_WAIT_QUEUE_HEAD (my_queue);


struct file_operations mycdev_fops = {
        .read    =       _mycdev_read,
        .write   =       _mycdev_write,
        .open    =       _mycdev_open,
        .release =       _mycdev_release,
        .owner   =       THIS_MODULE
};


static int _mycdev_open(struct inode *inodep, struct file *filep){
   opencount++;
   pr_info("MYCDEV: dev open count: %d\n", opencount);
   return 0;
}


static ssize_t _mycdev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){

   int ct = 0;
   int err ;
   int curtime ;

   err =  !access_ok((void *)buffer, sizeof(int) );
   if (err) return -EFAULT;



   //save the current time
   curtime = 0;

   //fire the interrupt
   gpio_set_value(espclock_en_gpio,1);
   wait_event_interruptible(myqueue, dataready);

   //calculate the  time difference
   *send_data = clock_data - curtime;  

   ct = copy_to_user(buffer, send_data, sizeof(int) );

   dataready=0; 

return  ct ;
}


static ssize_t _mycdev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){

  int err;
  int t = sizeof(int);
  int ct;

  if ( len < t ) t = len;

  err =  !access_ok((void *)buffer, sizeof(int) );
  if (err) return -EFAULT;
  ct= copy_from_user (rec_data, buffer, t );
  if ( *rec_data == 1 ) {
   gpio_set_value(espclock_en_gpio,1);
  }

  return ct;
}

 
static int _mycdev_release(struct inode *inodep, struct file *filep){
   opencount--;
   pr_info("MYCDEV: dev open count: %d\n", opencount);
   return 0;
}


int _mycdev_init(void){

   pr_info("MYCDEV: Initializing mycdev\n");
   majornum = register_chrdev(0, DEVICE_NAME, &mycdev_fops);
   devclass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(devclass)){ return -1; }
 
   devdevice = device_create(devclass, NULL, MKDEV(majornum, 0), NULL, DEVICE_NAME);
   rec_data   = (unsigned int *) kmalloc( sizeof(int), GFP_KERNEL);
   send_data  = (unsigned int *) kmalloc( sizeof(int), GFP_KERNEL);

   pr_info("MYCDEV: device created\n");
   return 0;

}



void _mycdev_exit(void){

     device_destroy(devclass, MKDEV(majornum, 0));
     class_destroy(devclass);
     unregister_chrdev(majornum, DEVICE_NAME);
     kfree(rec_data);
     kfree(send_data);

     pr_info("MYCDEV: char device unloaded!\n");
}
