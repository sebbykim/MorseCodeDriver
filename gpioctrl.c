#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/ktime.h>


#define IO_LED 21
#define IO_BUTTON 20
#define IO_OFFSET 571
#define DEBOUNCE_MS 100 // Debounce time in milliseconds
#define KEY_CODE KEY_SPACE // Emulate spacebar

static struct timer_starter *thread;

static struct gpio_desc *led, *button;
static int button_irq;
static struct input_dev *input_dev;
static ktime_t last_event_time;
static int button_state = 0; // 0 = released, 1 = pressed

// Interrupt handler for button press and release
static irqreturn_t button_isr(int irq, void *data) {
    ktime_t now = ktime_get();
s64 delta_ms = ktime_to_ms(ktime_sub(now, last_event_time));

    last_event_time = now;


    // Read button state (1= pressed, 0 = released due to pull-up)
    int val = gpiod_get_value(button);
    if (val == 1 && button_state == 1) { // Button pressed
        gpiod_set_value(led, 1); // Turn LED on
        input_report_key(input_dev, KEY_CODE, 1); // Key down
        input_sync(input_dev);
        button_state = 0;
        printk("Time released: %dms\n" , delta_ms);
        printk("gpioctrl - Button pressed\n");
    } else if (val == 0 && button_state == 0) { // Button released
        gpiod_set_value(led, 0); // Turn LED off
        input_report_key(input_dev, KEY_CODE, 0); // Key up
        input_sync(input_dev);
	button_state = 1;
        printk("Time pressed: %dms\n" , delta_ms);
        if (delta_ms <= 200) {
                printk(".\n");
        }
        if (delta_ms > 200 && delta_ms < 1000) {
                printk("-\n");
        }
        printk("gpioctrl - Button released\n");
    }
    return IRQ_HANDLED;
}


static int __init my_module_init(void) {
    int status;

    // Get GPIO descriptors
    button = gpio_to_desc(IO_BUTTON + IO_OFFSET);
    if (!button){
        printk("gpioctrl - Error getting pin %d\n", IO_BUTTON + IO_OFFSET);
        return -ENODEV;
    }
    led = gpio_to_desc(IO_LED + IO_OFFSET);
    if (!led) {
        printk("gpioctrl - Error getting pin %d\n", IO_LED + IO_OFFSET);
        return -ENODEV;
    }

    // Configure LED as output, initially off
    status = gpiod_direction_output(led, 0);
    if (status) {
	printk("gpioctrl - Error setting pin %d to output\n", IO_LED + IO_OFFSET);
        return status;
    }

    // Configure button as input
    status = gpiod_direction_input(button);
    if (status) {
        printk("gpioctrl - Error setting pin %d to input\n", IO_BUTTON + IO_OFFSET);        return status;
    }

    // Set up input device for keyboard emulation
    input_dev = input_allocate_device();
    if (!input_dev) {
        printk("gpioctrl - Failed to allocate input device\n");
        return -ENOMEM;
    }
    input_dev->name = "gpio_button";
	set_bit(EV_KEY, input_dev->evbit);
    set_bit(KEY_CODE, input_dev->keybit);
    status = input_register_device(input_dev);
    if (status) {
        printk("gpioctrl - Failed to register input device\n");
        input_free_device(input_dev);
        return status;
    }

    // Get IRQ for button
    button_irq = gpiod_to_irq(button);
    if (button_irq < 0) {
        printk("gpioctrl - Failed to get IRQ for pin %d\n", IO_BUTTON + IO_OFFSET);
        input_unregister_device(input_dev);
        return button_irq;
    }

    // Request IRQ for both edges
    status = request_irq(button_irq, button_isr,
                         IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
                         "gpio_button", NULL);
    if (status) {
        printk("gpioctrl - Failed to request IRQ %d\n", button_irq);
        input_unregister_device(input_dev);
        return status;
    }

    // Initialize last event time
    last_event_time = ktime_get();
    printk("gpioctrl - Module initialized, button is %spressed\n",
           gpiod_get_value(button) ? "" : "not ");
    return 0; // Success
}

static void __exit my_module_exit(void)
    free_irq(button_irq, NULL);
    input_unregister_device(input_dev);
    gpiod_set_value(led, 0); // Turn LED off on exit
    printk("gpioctrl - Module unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Patrick");
MODULE_DESCRIPTION("GPIO button with LED control and keyboard emulation");
