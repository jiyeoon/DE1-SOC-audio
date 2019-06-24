#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <asm/io.h>

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/ioport.h>

MODULE_LICENSE("GPL");

#define base_lwFPGA 0xFF200000
#define len_lwFPGA 0x00200000

#define addr_HEX3HEX0   0x20
#define addr_HEX5HEX4   0x30

#define HEX_DEVMAJOR    240
#define HEX_DEVNAME     "hex"

static void *mem_base;
static void *hex3hex0_addr;
static void *hex5hex4_addr;

int hex_conversions[8] = {
    0x39, 0x5E, 0x79, 0x71, 0x7D, 0x77, 0x7C, 0x00  //C D E F G A B dummy
};

//open operation
static int hex_open(struct inode *minode, struct file *mfile){
    //do nothing
    return 0;
}

//release operation
static int hex_release(struct inode *minode, struct file *mfile){
    //do nothing
    return 0;
}

//write operation
static int hex_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos){
    unsigned int hex_data = 0;  //32bit data
    int ret;
    unsigned int hex3hex0_data;

    //get data from buffer
    ret = copy_from_user(&hex_data, buf, count);

    hex3hex0_data = hex_conversions[hex_data];

    //읽어온 값을 hex0에 입력
    iowrite32(hex3hex0_data, hex3hex0_addr);

    return count;
}

//read opeartion
static int hex_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos){
    //do nothing
    return 4;
}

//file operation initialize
static struct file_operations hex_fops = {
    .read = hex_read,
    .write = hex_write,
    .open = hex_open,
    .release = hex_release
};

//driver init
static int __init hex_init(void){
    int res;

    res = register_chrdev(HEX_DEVMAJOR, HEX_DEVNAME, &hex_fops);
    if(res<0){
        printk(KERN_ERR " hex : failed to register device. \n");
        return res;
    }

    //mapping address
    mem_base = ioremap_nocache(base_lwFPGA, len_lwFPGA);

    if(!mem_base){
        printk("Error mapping memory!\n");
        release_mem_region(base_lwFPGA, len_lwFPGA);
        return -EBUSY;
    }

    hex3hex0_addr = mem_base + addr_HEX3HEX0;
    hex5hex4_addr = mem_base + addr_HEX5HEX4;

    printk(" Device : %s MAJOR : %d \n", HEX_DEVNAME, HEX_DEVMAJOR);
    return 0;
}

static void __exit hex_exit(void){
    unregister_chrdev(HEX_DEVMAJOR, HEX_DEVNAME);
    printk(" %s unreginsered. \n", HEX_DEVNAME);
    iounmap(mem_base);
}

module_init(hex_init);
module_exit(hex_exit);
