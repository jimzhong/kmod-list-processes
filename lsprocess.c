/*
This source code is originally written by Jim Zhong and posted on GitHub
*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>

/*
每个进程的名字、进程pid、进程的状态、父进程的名字
以及统计系统中进程个数，包括统计系统中TASK_RUNNING、TASK_INTERRUPTIBLE、TASK_UNINTERRUPTIBLE、TASK_ZOMBIE、TASK_STOPPED等（还有其他状态）状态进程的个数
*/

MODULE_LICENSE("GPL");

// print all processes
static int show_process(struct seq_file *m, void *v)
{
    struct task_struct *p;
    char name[TASK_COMM_LEN], pname[TASK_COMM_LEN];
    long running = 0;
    long inter = 0;
    long uninter = 0;
    long zombie = 0;
    long stop = 0;
    long other = 0;
    long total = 0;
    
    for_each_process(p)
    {
        total++;
        get_task_comm(name, p);
        if (p->parent)
            get_task_comm(pname, p->parent);
        else
            strcpy(pname, "None");
        seq_printf(m, "name:%-20spid:%d state:%ld parent:%-20s\n", name, p->pid, p->state, pname);
        switch(p->state)
        {
            case TASK_RUNNING:
                running++;break;
            case TASK_INTERRUPTIBLE:
                inter++;break;
            case TASK_UNINTERRUPTIBLE:
                uninter++;break;
            case __TASK_STOPPED:
                stop++;break;
            default:
                other++;
        }
        if (p->exit_state & EXIT_ZOMBIE)
            zombie++;
    }
    seq_printf(m, "===================STAT==================\n");
    seq_printf(m, "running:%ld\ninterruptible: %ld\nuninterruptible: %ld\nstopped: %ld\nother: %ld\nzombie: %ld\ntotal: %ld\n",
        running, inter, uninter, stop, other, zombie, total);
    return 0;
}

// callback for open
static int ls_process_open(struct inode *inode, struct file *file)
{
  return single_open(file, show_process, NULL);
}

// file operations
static struct file_operations fops = {
    .owner    = THIS_MODULE,
    .open = ls_process_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release,
};

// module init callback
static int __init ls_process_init(void)
{

    struct proc_dir_entry *entry = proc_create("psinfo", 0666, NULL, &fops);
    if (entry == NULL) {
      printk(KERN_ERR "Couldn't create proc entry\n");
      return -ENOMEM;
    }
                                                    
    printk(KERN_INFO "Module loaded successfully\n");
    return 0;
}

// module exit callback
static void __exit ls_process_exit(void)
{
    remove_proc_entry("psinfo", NULL);

    printk(KERN_INFO "Module unloaded successfully\n");
}

module_init(ls_process_init);
module_exit(ls_process_exit);


