#include <linux/module.h>               //allows to load the kernel module
#include <linux/init.h>                 //tells linux what to do when starting or stopping
#include <linux/gpio/consumer.h>        //help control gpio pins
#include <linux/ktime.h>                //for tracking time
#include <linux/interrupt.h>            //used to notice when button is pressed/released
#include <linux/input.h>                //makes the button act like a keyboard key
#include <linux/delay.h>                // to pause program for a short while (to time led)
#include <linux/string.h>               // need for strcmp
#include <linux/fs.h>                   // to create a device to read and write
#include <linux/proc_fs.h>              // to create a proc file
#include <linux/ioctl.h>                // to tweak ioctl settings
#include <linux/uaccess.h>              // to move data between user space to kernel

#include "ioctl.h"                      // custom file with custom command codes

#define LED_OFFSET 21               // led on GPIO PIN 21
#define BUTTON_OFFSET 20            // the button is connected to GPIO PIN 20
#define CHIP_BASE 571               // due to it being on gpio chip 4 this is its base
#define CUSTOM_BUTTON BTN_0         //new keyboard button
#define PROC_FILE_BUFFER_SIZE 256   //long enough for most information

static struct gpio_desc *led, *button;  // used for getting memory address of button and led for IO
static int button_irq;                  // tells us when the button interrupts
static struct input_dev *button_key;    // where button acts like a keyboard key
static ktime_t last_event_time;         // remembers when the last time something happened
static ktime_t press_time;              // time when button is pressed
static int button_state;                // 0 = released, 1 = pressed

static unsigned long button_press_count = 0;  //for proc file
static unsigned int last_press_duration = 0;  //for proc file
static unsigned int last_release_duration = 0;  //for proc file
static struct proc_dir_entry *proc_entry;  


static char morse_chars_for_output[256];            // stores all the morse code for the user 
static int morse_count_for_output;
static char morse_chars_for_input[256];            // stores all the morse code from the user
static int major;
static int32_t  time_for_space = 3000;
static int32_t time_for_char = 1000;
static int32_t time_for_long = 500;

//Checks the morse code array and lights up the LED accordingly
static void blink_morse(const char *morse_array, int length) {

    for (int i = 0; i < length; i++) {
        switch (morse_array[i]) {
            case '.':  // dot
                gpiod_set_value(led, 1);
                mdelay(150);  // short on
                gpiod_set_value(led, 0);
                mdelay(200);  // gap between elements
                break;
                
            case '-':  // dash
                gpiod_set_value(led, 1);
                mdelay(time_for_long);  // long on
                gpiod_set_value(led, 0);
                mdelay(200);  // gap between elements
                break;
                
            case '/':  // letter gap
                mdelay(time_for_char);
                break;
                
            case ' ':  // word gap
                mdelay(time_for_space);
                break;
                
            default:
                // Skip unknown characters
                break;
        }
    }
   
}

// function that runs when someone opens the file
//https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/399af73e1266b6ed82ea1d8753ff2a976951aa6e inspo for fileoperations
static int my_open  (struct inode *inode, struct file *filp){
    morse_count_for_output = 0;
    printk("file opened");
    
    return 0;
}

//used to select/change input speed
static long int my_ioctl(struct file *file, unsigned cmd, unsigned long arg){
    switch (cmd)
    {
    case SET_TIME_CHAR:
        if(copy_from_user(&time_for_char,(int32_t *)arg,sizeof(time_for_char))){        //gets new timeing 
            pr_warn("MorseDriver - Failed to copy data from user");
        }
        else{
            pr_info("MorseDriver - time for character set to %d",time_for_char);
        }
        break;
    case SET_TIME_SPACE:
        if(copy_from_user(&time_for_space,(int32_t *)arg,sizeof(time_for_space))){      //gets new timeing
            pr_warn("MorseDriver - Failed to copy data from user");
        }
        else{
            pr_info("MorseDriver - date copied successfully from user");
        }
        break;
    
    case SET_TIME_LONG:
         if(copy_from_user(&time_for_long, (int32_t *)arg, sizeof(time_for_long))){       // gets new time
            pr_warn("MorseDriver - Failed to copy data from user");
        }
        else{
            pr_info("MorseDriver - date copied successfully from user");
        }    

        break;
    default:
        return -EINVAL; // Invalid command
        break;

    }

    return 0;

}

//runs when someone reads from the file (file operations)
static ssize_t my_read(struct file *filp, char __user *user_buf, size_t len, loff_t *off)       //(file pointer (to character device file), user_array(from user space), length (how many bytes to read), offset (tail to keep track of what is read))
{

	int not_copied, copied_bytes, to_copy = (len + *off) < morse_count_for_output ? len : (morse_count_for_output - *off);

	pr_info("MorseDriver - Read is called, we want to read %ld bytes, but actually only copying %d bytes. The offset is %lld\n", len, to_copy, *off);

	if (*off >= morse_count_for_output)     //if read everything already
		return 0;

	not_copied = copy_to_user(user_buf, &morse_chars_for_output[*off], to_copy);
	copied_bytes = to_copy - not_copied;        //how much was actually sent (copied)
	if (not_copied) 
		pr_warn("MorseDriver - Could only copy %d bytes\n", copied_bytes);

	*off += copied_bytes;           //move reading position forward

	return copied_bytes;
	
}

//runs when someone writes to the file
static ssize_t my_write(struct file *filp, const char __user *user_buf, size_t len, loff_t *off)
{

	int not_copied, copied_bytes, to_copy = (len + *off) < sizeof(morse_chars_for_input) ? len : (sizeof(morse_chars_for_input) - *off);

	pr_info("MorseDriver - Write  is called, we want to write %ld bytes, but actually only copying %d bytes. The offset is %lld\n", len, to_copy, *off);

	if (*off >= sizeof(morse_chars_for_input))  //if no space left
    {
		return 0;
    }

	not_copied = copy_from_user( &morse_chars_for_input[*off], user_buf, to_copy);      //takes morse code from user
	copied_bytes = to_copy - not_copied;            //how much we actually took (copied)
	if (not_copied) 
		pr_warn("MorseDriver - Could only copy %d bytes\n", copied_bytes);

	*off += copied_bytes;           //move writing position forward
    blink_morse(morse_chars_for_input, (int)*off); //once the data is copied from user it will pulse the led to the offset in the array

	return copied_bytes;
	
}

//runs when someone closes file
static int my_release(struct inode *inode, struct file *filp){
    pr_info("MorseDriver - file is closed\n");
    return 0;
}

//list of jobs that our file can do
static struct file_operations fops = {
    //https://elixir.bootlin.com/linux/v6.13.6/source/include/linux/fs.h#L2071
    .read = my_read,                //used for writing to user space
    .write = my_write,              //used for writing to the module
    .open = my_open,
    .release = my_release,          // when someone closes
    .unlocked_ioctl = my_ioctl,     // used for changing timeings
};

//shows stats for proc file in /proc/morse_stats
static ssize_t proc_morse_read(struct file *file, char __user *buf, size_t length, loff_t *off) {
    char proc_buffer[PROC_FILE_BUFFER_SIZE];        //space to build our stats message
    int written;                                    // Counts how many bytes we write into the buffer

    if (*off > 0) return 0;     //only show data once per read (already sent data)

    written = snprintf(proc_buffer, PROC_FILE_BUFFER_SIZE,
        "Morse Code Driver Log:\n"
        "Button presses: %lu\n"
        "Current Morse sequence: %s\n"
        "Last press duration: %u ms\n"
        "Last release duration: %u ms\n"
        "Buffer count: %d/256\n"  
        "Button state: %s\n"
        "LED state: %s\n",
        button_press_count,
        morse_count_for_output > 0 ? morse_chars_for_output : "None",
        last_press_duration,
        last_release_duration,
        morse_count_for_output,
        button_state ? "Released" : "Pressed",
        gpiod_get_value(led) ? "On" : "Off");

    if (written <= 0 || written >= PROC_FILE_BUFFER_SIZE) return -EFAULT;   //if nothing or too much is written
    if (length < written) return -EINVAL;       //makes sure user gives us enough space to send message (if buffer is too small)

    if (copy_to_user(buf, proc_buffer, written)) return -EFAULT;        //copies stats to buffer

    *off += written;        //move position forward
    return written;         //tells system how many bytes we gave them
}

//tells system how to handle /proc file (morse_stats)
static const struct proc_ops proc_fops = {
    .proc_read = proc_morse_read,
};


// Handles button IRQs (when button is pressed/released)
static irqreturn_t button_handler(int irq, void *data)
{
    ktime_t now = ktime_get(); // gets the current time
    s64 time_pressed = ktime_to_ms(ktime_sub(now, last_event_time));
    int button_value = gpiod_get_value(button); // checks if button is pressed(0)/released

    if (time_pressed < 50)
    { // if less than 50 ms then ignore, prob button bounce/noise
        return IRQ_HANDLED;
    }

    last_event_time = now;

    if (button_value == 0)
    { // Button pressed (0)
        if (button_state == 0)
        { // only do something if button wasn't pressed
            press_time = now;
            input_report_key(button_key, CUSTOM_BUTTON, 1); // tells button is pressed
            input_sync(button_key);                         // makes sure system knows button is pressed
            button_state = 1;                               // marks the button as pressed
            pr_info("MorseDriver - Button pressed for %lld ms\n", time_pressed);

            if(time_pressed <= time_for_long){
                morse_chars_for_output[morse_count_for_output] = '.';
            }
            else{
                morse_chars_for_output[morse_count_for_output] = '-';
            }

            last_press_duration = time_pressed;     //for proc file

            morse_count_for_output++;
            button_press_count++;                   //for proc file
        }
    }
    else
    { // Button released (1)
        if (button_state == 1)
        {   // Only act if state change is new
            s64 time_released = ktime_to_ms(ktime_sub(now, press_time)); // how long button is not pressed (time in between presses)
            input_report_key(button_key, CUSTOM_BUTTON, 0); // tells button is released
            input_sync(button_key);                         // makes sure linux knows is released
            button_state = 0;                              // says button is up
            pr_info("MorseDriver - Button released for %lld ms\n", time_released);
            if(time_released > time_for_space){ //if space, separate by space
                morse_chars_for_output[morse_count_for_output] = ' ';
                morse_count_for_output++;
            }
            else if(time_released > time_for_char){   //if letter, but same word, separate by /
                morse_chars_for_output[morse_count_for_output] = '/';
                morse_count_for_output++;
            }

            last_release_duration = time_released;      //for proc file
        }
    }

    return IRQ_HANDLED;
}


// used to load the module, sets up everything when module is loaded
static int __init my_module_init(void)
{
    int status;     //holds error codes if something fails

    button = gpio_to_desc(BUTTON_OFFSET + CHIP_BASE);       //finds button
    if (!button)
    {
        pr_err("MorseDriver - Error getting button on pin %d \n", BUTTON_OFFSET);
        return -ENODEV;
    }

    led = gpio_to_desc(LED_OFFSET + CHIP_BASE);     //finds LED
    if (!led)
    {
        pr_err("MorseDriver - Error getting led on pin %d\n", LED_OFFSET);
        return -ENODEV;
    }
    status = gpiod_direction_output(led, 0);        //set LED pin to set signals (starts off)
    if (status)
    {
        pr_err("MorseDriver- Error setting Button on pin %d to output\n", BUTTON_OFFSET);
        return status;
    }

    status = gpiod_direction_input(button);     //set button to recieve signals
    if (status)
    {
        pr_err("MorseDriver - Error setting pin %d to input\n", LED_OFFSET);
        return status;
    }


    // Sync initial button state
    button_state = gpiod_get_value(button) ? 0 : 1; // checks button state - 0 = released, 1 = pressed
    if (button_state)
    {
        //gpiod_set_value(led, 1);  // LED on if pressed at start
        press_time = ktime_get(); // Set initial press time
    }

    button_key = input_allocate_device();       //makes fake keyboard device
    if (!button_key)
    {
        pr_err("MorseDriver - Failed to allocate input device\n");
        return -ENOMEM;
    }

    button_key->name = "gpio_button";               // name it (for debugging)
    set_bit(EV_KEY, button_key->evbit);             // say it can send key events
    set_bit(CUSTOM_BUTTON, button_key->keybit);     // say it sends our custom button key (BTN_0)
    status = input_register_device(button_key);     // tell our system about the new button
    if (status)
    {
        pr_err("MorseDriver - Failed to register input device\n");
        input_free_device(button_key);      // free memory we made
        return status;
    }

    button_irq = gpiod_to_irq(button);      // get a signal number for button interrupts
    if (button_irq < 0)
    {
        pr_err("MorseDriver - Failed to get IRQ for button on pin %d\n", BUTTON_OFFSET);
        return button_irq;
    }

    /*An ISR (interrupt service routine) is just a function that executes when an interrupt is triggered*/
    // tells linux to run button_handler(ISR) when button interrupts
    status = request_irq(button_irq, button_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, // interrupt on press and release
                         "gpio_button", NULL);
    if (status)
    {
        printk("MorseDriver - Failed to request IRQ %d\n", button_irq);
        input_unregister_device(button_key); // if fails, removes key
        return status;
    }

    major = register_chrdev(0,"Morse_Driver",&fops); //registers character device (a new file with our functions)

    //error handling for if the character device isnt registered
    if(major < 0){
        pr_err("error registering character device\n");
        free_irq(button_irq, NULL);
        input_unregister_device(button_key);
        return major;
    }
    printk("MorseDriver - Major device number = %d \n", major);//used for creating the character device file

    last_event_time = ktime_get(); // saves starting time


    // proc file stuff
    proc_entry = proc_create("morse_stats", 0444, NULL, &proc_fops);        //makes a proc file called /proc/morse_stats
    if (!proc_entry) {//error handling for if the procfile isnt registered
        pr_err("MorseDriver - Failed to create /proc/morse_stats\n");
        unregister_chrdev(major, "Morse_Driver");
        free_irq(button_irq, NULL);
        input_unregister_device(button_key);
        return -ENOMEM;
    }

    printk("Module Loaded in\n");
    return 0;
}
// used for unloading the module
static void __exit my_module_exit(void)
{
    if (proc_entry)remove_proc_entry("morse_stats", NULL);
    free_irq(button_irq, NULL);
    input_unregister_device(button_key);
    gpiod_set_value(led, 0);
    unregister_chrdev(major, "Morse_Driver");
    printk("Module Unloaded\n");

}


module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aaron, Patrick, Sebastian");
MODULE_DESCRIPTION("Uses GPIO pins on a pi 5 to connect to a button and an LED on a breadboard");
