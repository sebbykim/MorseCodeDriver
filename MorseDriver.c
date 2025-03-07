#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio/consumer.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>

#define LED_OFFSET 21// led on GPIO PIN 21

#define BUTTON_OFFSET 20 // the button is connected to GPI PIN 20
			
#define CHIP_BASE 571 //due to it being on gpio chip 4 this is its base

static struct gpio_desc *led *button;//used for getting memory address of button
				     //and led for IO

static int button_state = 0;


//used to load the module
static int __init my_module_init(void){
	printk("Module loded");
	return 0;

}
//used for unloading the module
static void __exit my_module_exit(void){
	printk("Module unloded");
	return 0;
}

module_init(my_module_init);
module_exit(my_module_exit);

