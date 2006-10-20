#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/addrspace.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sebastian Gottschall");

#define procfs_name "gpio"
#define PROCFS_MAX_SIZE         255
static unsigned long procfs_buffer_size = 0;
static char procfs_buffer[PROCFS_MAX_SIZE];
struct proc_dir_entry * Our_Proc_File;

extern void set434Reg(unsigned regOffs, unsigned bit, unsigned len, unsigned val);
extern void changeLatchU5(unsigned char orMask, unsigned char nandMask);
extern unsigned char getLatchU5(void);


int init_module(void);
void cleanup_module(void);
int procfile_read(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data);
int procfile_write(struct file *file, const char *buffer, unsigned long count, void *data);


int U5procfile_read(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data)
{
        int ret;
	ret = sprintf(buffer, "%u\n", getLatchU5());
        return ret;
}

int U5procfile_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned int bit_to_set;
	char *operand;

	/* get buffer size */
	procfs_buffer_size = count;
	if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}
	/* write data to the buffer */
	if ( copy_from_user(procfs_buffer, buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}

	bit_to_set = simple_strtol(buffer,&operand,10);
	
	// do the work
	if (strncmp(operand,"on",2) == 0 ){ 
	changeLatchU5(1<<bit_to_set,0); 
	}
	else if(strncmp(operand,"off",3) == 0){
	changeLatchU5(0,1<<bit_to_set); 
	}

	return procfs_buffer_size;
}

int init_module(void)
{
	
	printk(KERN_ALERT"Loading RB500 GPIO procfs Driver.\n");
	Our_Proc_File = create_proc_entry(procfs_name, 0644, NULL);
	if (Our_Proc_File == NULL) {
		remove_proc_entry(procfs_name, &proc_root);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", procfs_name);
		return -ENOMEM;
	}
	Our_Proc_File->read_proc = U5procfile_read;
	Our_Proc_File->write_proc = U5procfile_write;
	Our_Proc_File->owner 	 = THIS_MODULE;
	Our_Proc_File->uid 	 = 0;
	Our_Proc_File->gid 	 = 0;
	Our_Proc_File->size 	 = 37;

	return 0;
}

void cleanup_module(void)
{
	remove_proc_entry(procfs_name, &proc_root);
	printk(KERN_ALERT"unloading GPIO Driver\n");

}
