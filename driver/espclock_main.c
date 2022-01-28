#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>

#include <linux/uaccess.h>


extern  int espclock_probe(struct platform_device *pdev);
extern  int espclock_remove(struct platform_device *pdev);

const struct of_device_id espclock_ids[] = {
    { .compatible = "espclock", },
    { /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, espclock_ids);


static struct platform_driver mypdrv = {
    .probe      = espclock_probe,
    .remove     = espclock_remove,
    .driver     = {
    .name     = "escplock",  //module name
    .of_match_table = of_match_ptr(espclock_ids),  
    .owner    = THIS_MODULE,
    },
};

module_platform_driver(mypdrv);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("sjk");
MODULE_DESCRIPTION("gpio espclock rpi");
MODULE_VERSION("0.1");
