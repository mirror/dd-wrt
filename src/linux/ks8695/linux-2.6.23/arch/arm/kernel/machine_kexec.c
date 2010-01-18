/*
 * machine_kexec.c - handle transition of Linux booting another kernel
 */

#include <linux/mm.h>
#include <linux/kexec.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/mmu_context.h>
#include <asm/io.h>
//#include <asm/cacheflush.h>
#include <asm/mach-types.h>
#include <asm/setup.h>

const extern unsigned char relocate_new_kernel[];
const extern unsigned int relocate_new_kernel_size;

extern void setup_mm_for_reboot(char mode);

extern unsigned long kexec_start_address;
extern unsigned long kexec_indirection_page;
extern unsigned long kexec_mach_type;
extern unsigned long kexec_initrd_dest;
 
extern void setup_mm_for_reboot(char mode);

/*
 * Provide a dummy crash_notes definition while crash dump arrives to arm.
 * This prevents breakage of crash_notes attribute in kernel/ksysfs.c.
 */

int machine_kexec_prepare(struct kimage *image)
{
	return 0;
}

void machine_kexec_cleanup(struct kimage *image)
{
}

void machine_shutdown(void)
{
}

void machine_crash_shutdown(struct pt_regs *regs)
{
}

void machine_kexec(struct kimage *image)
{
	unsigned long page_list;
	unsigned long reboot_code_buffer_phys;
	void *reboot_code_buffer;
 	struct tag *params;
 	
 	page_list = image->head & PAGE_MASK;
 
 	/* we need both effective and real address here */
 	//reboot_code_buffer_phys =
 	//    page_to_pfn(image->control_code_page) << PAGE_SHIFT;
 	reboot_code_buffer = page_address(image->control_code_page);
 	reboot_code_buffer_phys = virt_to_phys((void*)reboot_code_buffer);
 
 	/* Prepare parameters for reboot_code_buffer*/
 	kexec_start_address = image->start;
 	kexec_indirection_page = page_list;
 	kexec_mach_type = machine_arch_type;
 	kexec_initrd_dest = 0xb00000;	// @11MB
 
 	/* copy our kernel relocation code to the control code page */
 	memcpy((void*)reboot_code_buffer,
 	       relocate_new_kernel, relocate_new_kernel_size);
 
 	flush_icache_range(reboot_code_buffer,
 			   reboot_code_buffer + KEXEC_CONTROL_CODE_SIZE);
 	printk(KERN_INFO "Bye!\n");
 	printk(KERN_INFO "reboot_code_buffer_phys: %08X\n", reboot_code_buffer_phys);
 	
 	cpu_proc_fin();
 	setup_mm_for_reboot(0); /* mode is not used, so just pass 0*/
 	/* now call reboot code  */
 
 	/* setup the ATAG structure at 0x100 (kernel command line)
 	 * we can directly write to the physical address - 1:1 mapping through setup_mm_for_reboot() */
 
 	params = (struct tag*)0x100;	// FIXME: no macro for future use in the kernel?
 	// CORE tag must be present & first
     params->hdr.size = (sizeof(struct tag_core) + sizeof(struct tag_header)) / sizeof(long);
 	params->hdr.tag = ATAG_CORE;
     params->u.core.flags = 0;
     params->u.core.pagesize = 0;
     params->u.core.rootdev = 0;
     params = (struct tag*)((long *)params + params->hdr.size);
 
     // memory size
 	params->hdr.size = (sizeof(struct tag_mem32) + sizeof(struct tag_header)) / sizeof(long);
     params->hdr.tag = ATAG_MEM;
     params->u.mem.size = 0x2000000;	// 32MB
     params->u.mem.start = 0x0;
     params = (struct tag*)((long *)params + params->hdr.size);
 
 	if (!strncmp(image->cmdline, "useinitrd", 9))
 	{
 		char temp[1024];
 		strcpy(temp, image->cmdline);
 		strcpy(image->cmdline, temp + 10);
 		params->hdr.size = (sizeof(struct tag_initrd) + sizeof(struct tag_header)) / sizeof(long);
    		params->hdr.tag = ATAG_INITRD2;
    		params->u.initrd.start = kexec_initrd_dest;
    		params->u.initrd.size = 0x500000;	// the size is also fixed to 5MB in relocate_kernel.S 
    		params = (struct tag *)((long *)params + params->hdr.size);
 	}
 	else
 		kexec_initrd_dest = 0x0;
 		
 	#define ROUNDUP(n) (((n)+3)&~3)
     // Kernel command line
 	params->hdr.size = (ROUNDUP(strlen(image->cmdline)) + sizeof(struct tag_header)) / sizeof(long);
     params->hdr.tag = ATAG_CMDLINE;
     strcpy(params->u.cmdline.cmdline, image->cmdline);
     params = (struct tag*)((long*)params + params->hdr.size);
     
 	// Mark end of parameter list
     params->hdr.size = 0;
     params->hdr.tag = ATAG_NONE;
 	
 
 	/* FIXME: cpu_arm922_reset function did not work for us, so:
 	 * calculate the current physical address for PC
	 * and jump to the code which switches of the MMU
 	 * and call the new kernel */
 	asm("	mov r0, pc \n\
 		    bic r0, r0, #0xc0000000 \n\
 		    add pc, r0, #4 \n\
 		    mrc p15, 0, r0, c1, c0 \n\
 			bic r0, r0, #1 \n\
 			mcr p15, 0, r0, c1, c0\n");
     
 	((void (*)(void))reboot_code_buffer_phys)();
 	//cpu_reset(reboot_code_buffer_phys);
 }


