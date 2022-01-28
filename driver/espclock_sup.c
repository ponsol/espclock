#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

#include <linux/gpio/consumer.h>
#include <linux/gpio.h>

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/uaccess.h>


MODULE_LICENSE("GPL");

#define MYDEV_CDEV

extern const struct of_device_id espclock_ids[] ;
extern void _mycdev_exit(void);
extern int _mycdev_init(void);


DECLARE_WAIT_QUEUE_HEAD (myqueue);
//extern struct wait_queue_head myqueue ;

int clock_data ;
int dataready ;

int espclock_en_gpio ;
int espclock_int_gpio ;



static struct gpio_desc *desc_espclock_en_gpio; 
static struct gpio_desc *desc_espclock_int_gpio ;

static unsigned int irqnum;
const struct of_device_id *of_id ;



static irq_handler_t espclock_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){

   //printk(KERN_INFO "ESPCLOCK: isr reached\n");
   //switch off the clock
   gpio_set_value( espclock_en_gpio, 0);  

   //read clock tics 
   clock_data++;

   dataready = 1; wake_up_interruptible(&myqueue);
   return (irq_handler_t) IRQ_HANDLED;  
}

int espclock_probe(struct platform_device *pdev) {

   int res = 0;
   struct device_node *np;


   printk(KERN_INFO "ESPCLOCK: Initializing.....\n");

   of_id = of_match_device( of_match_ptr(espclock_ids), &pdev->dev);

   if (! of_id )  {
      printk(KERN_INFO "ESPCLOCK: no matching device found\n");
      return -ENODEV;
   }

   if ( of_id ) {

      np = pdev->dev.of_node;
      if (!np) return -1;

      //platform_get_resource(pdev, IORESOURCE_MEM, 0);
      irqnum  = platform_get_irq(pdev, 0);
      printk(KERN_INFO "ESPCLOCK: gpio irq number: %d\n", irqnum);

   
      desc_espclock_int_gpio  = gpiod_get(&pdev->dev, "clock_int", GPIOD_OUT_LOW);
      espclock_int_gpio = desc_to_gpio( desc_espclock_int_gpio);
      if (!gpio_is_valid(espclock_int_gpio)){
         printk(KERN_INFO "ESPCLOCK: invalid gpio for espclock interrupt\n");
         return -ENODEV;
      }


      desc_espclock_en_gpio  = gpiod_get(&pdev->dev, "clock_en", GPIOD_OUT_LOW);
      espclock_en_gpio = desc_to_gpio( desc_espclock_en_gpio);
      printk(KERN_INFO "ESPCLOCK: espclock_en gpio: %d\n", espclock_en_gpio);
      if (!gpio_is_valid(espclock_en_gpio)){
         printk(KERN_INFO "ESPCLOCK: invalid gpio for espclock enable\n");
         return -ENODEV;
      }


      //if ( get_i2c_pins(pdev) ) {
      //   printk(KERN_INFO "ESPCLOCK: error in i2c pins");
      //   return -1;
      //}


      //enable pin
        gpio_request(espclock_en_gpio, "sysfs");         
      //default level 0 
       gpio_direction_output(espclock_en_gpio, 0);
       gpio_export(espclock_en_gpio, true);


      //interrupt pin
      gpio_request(espclock_int_gpio, "sysfs");  
      gpio_direction_input(espclock_int_gpio);   
      gpio_set_debounce(espclock_int_gpio, 200); 
      gpio_export(espclock_int_gpio, false);    


      //irqnum  = gpio_to_irq(espclock_int_gpio);
      printk(KERN_INFO "ESPCLOCK: espclock_int_gpio mapped to irq: %d\n", irqnum);
      res = request_irq(irqnum, (irq_handler_t) espclock_irq_handler, IRQF_TRIGGER_RISING,
                        "espclock_irq_handler", NULL);  
      printk(KERN_INFO "ESPCLOCK: Interrupt request status: %d\n", res);


#ifdef MYDEV_CDEV
      _mycdev_init();
#endif


   }

   return res;
}
 

int espclock_remove(struct platform_device *pdev) {


#ifdef MYDEV_CDEV
      _mycdev_exit();
#endif

   free_irq(irqnum, NULL); 
   gpio_unexport(espclock_int_gpio);   
   gpio_free(espclock_int_gpio);

   gpio_set_value(espclock_en_gpio, 0);  
   gpio_unexport(espclock_en_gpio);  
   gpio_free(espclock_en_gpio);

   printk(KERN_INFO "ESPCLOCK: espclock module removed\n");

 return 0;
}


