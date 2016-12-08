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
// m: pointer to seq_file
// v: not used
static int show_process(struct seq_file *m, void *v)
{
    struct task_struct *p;
    // pointer to task_struct, use in iteration
    char name[TASK_COMM_LEN], pname[TASK_COMM_LEN];
    // buffer for process command name and parent process command name

    // below are counters for processes in different states
    long running = 0;
    long inter = 0;
    long uninter = 0;
    long zombie = 0;
    long stop = 0;
    long traced = 0;
    // process in other states
    long other = 0;
    // total process counter
    long total = 0;

    // iterate through all processes
    for_each_process(p)
    {
        total++;
        // copy the process' command name into name buffer
        get_task_comm(name, p);
        if (p->parent)
            // copy the process' parent's command name into name buffer if there is a parent
            get_task_comm(pname, p->parent);
        else
            strcpy(pname, "None");
        // print to seq_file m
        seq_printf(m, "name:%-20spid:%d state:%ld parent:%-20s\n", name, p->pid, p->state, pname);
        // increment counters according to process state
        switch(p->state)
        {
            case TASK_RUNNING:
                running++;
                break;
            case TASK_INTERRUPTIBLE:
                inter++;
                break;
            case TASK_UNINTERRUPTIBLE:
                uninter++;
                break;
            case TASK_WAKEKILL | __TASK_STOPPED:    // a stopped process has both flags set
                stop++;
                break;
            case TASK_WAKEKILL | __TASK_TRACED:
                traced++;
                break;
            default:
                other++;
        }
        // zombie state is stored in p->exit_state
        if (p->exit_state & EXIT_ZOMBIE)
            zombie++;
    }
    // print statistical information to seq_file
    seq_printf(m, "===================STAT==================\n");
    seq_printf(m, "running:%ld\ninterruptible: %ld\nuninterruptible: %ld\nstopped: %ld\ntraced: %ld\nother: %ld\nzombie: %ld\ntotal: %ld\n",
        running, inter, uninter, stop, traced, other, zombie, total);
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
    .open = ls_process_open,    // point to open routine
    .read = seq_read,           // seqfile read handler
    .llseek = seq_lseek,        // seqfile seek handler
    .release = seq_release,     // seqfile close handler
};

// module init handler
static int __init ls_process_init(void)
{
    // create psinfo file in /proc directory
    struct proc_dir_entry *entry = proc_create("psinfo", 0666, NULL, &fops);
    if (entry == NULL)
    {
        // error handling
        printk(KERN_ERR "Couldn't create proc entry\n");
        return -ENOMEM;
    }
    // load successfully
    printk(KERN_INFO "Module loaded successfully\n");
    return 0;
}

// module exit callback
static void __exit ls_process_exit(void)
{
    // remove psinfo in /proc
    remove_proc_entry("psinfo", NULL);
    // print message
    printk(KERN_INFO "Module unloaded successfully\n");
}

module_init(ls_process_init);   // register loading handler
module_exit(ls_process_exit);   // register unloading handler
