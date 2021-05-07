/*
 * @Description    : This file is for
 * @Date           : 2021-05-05 22:19:41
 * @Author         : gesefudiao
 * @LastEditors    : gesefudiao
 * @LastEditTime   : 2021-05-06 14:39:03
 * @FilePath       : \undefinedc:\Users\zuoze\Desktop\lab3\procfs_test.c
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/moduleparam.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL");


static int pid = 0;
module_param(pid, int, 0644);

static struct proc_dir_entry* proc_test;

static int test_show(struct seq_file* m, void* v)
{
    pid_t* pid_n = (pid_t*)m->private;
    if (pid_n != NULL)
    {
        seq_printf(m, "%d\n", *pid_n);
    }
    return 0;
}

static int test_open(struct inode* inode, struct file* file)
{
    return single_open(file, test_show, PDE_DATA(inode));
}

// linux5.9.0
// 低版本需要查看proc_fs.h文件定义的结构体
struct proc_ops test_ops = {
    .proc_open = test_open,
    .proc_read = seq_read,
    .proc_release = single_release,
};



static int test_init(void) {
    // 在 proc 根目录创建 test 文件夹
    proc_test = proc_mkdir("test", NULL);
    if (proc_test == NULL) {
        printk("%s proc create %s failed\n", __func__, "test");
        return -EINVAL;
    }
    proc_create_data("pid", 0664, proc_test, &test_ops, &pid);
    return 0;
}

static void test_exit(void) {
    proc_remove(proc_test);
    printk(KERN_ALERT "test exit!>>>>>>>>>>>>>>>>>>>>>\n");
}

module_init(test_init);
module_exit(test_exit);