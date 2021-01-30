#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kprobes.h>
#include <linux/init.h>
#include <linux/hashtable.h>
#include <linux/stacktrace.h>
#include <linux/jhash.h>
#include <linux/rbtree.h>
//static struct kprobe kp;
static char func_name[NAME_MAX] = "pick_next_task_fair";
//static unsigned int counter = 0;
//static unsigned int pid;
struct my_data {	
	u64 nextts;
};


extern unsigned int stack_trace_save_user(unsigned long *store, unsigned int size);
typedef typeof(&stack_trace_save_user) stack_trace_save_user_fn;
#define stack_trace_save_user (* (stack_trace_save_user_fn)kallsyms_stack_trace_save_user)
void *kallsyms_stack_trace_save_user = NULL;
//function init() %{ kallysms_stack_trace_save_user = (void*)kallsyms_lookup_name("stack_trace_save_user"); %}

//static DEFINE_HASHTABLE(hlist,10);
struct rb_root rbroot = RB_ROOT;

/*struct hlist_entry {
	unsigned int pid;
	unsigned int key;
	unsigned long arraykey[20];
	unsigned int size;
	struct hlist_node node;
	unsigned int schcount;
	unsigned int size2;
	u64 td;
	//unsigned long stackLog(MAX_TRACE_SIZE);
};
*/
struct rbtree_entry {
	unsigned int key;
	struct rb_node rbentry;
	unsigned long arraykey[40];
	unsigned int cputime;
	unsigned int jkey;
	unsigned int size;
	unsigned int size2;
	unsigned long array[20];
	u64 td;
};

/*static int store_hlist_value(int key, unsigned long *array, unsigned int size2, u64 td)
{
        struct hlist_entry* h;
	int i;
        //int store_pid;
	//store_pid = pid;
	hash_for_each_possible(hlist,h,node,key)
	{
		if(h != NULL){
			h->td = td;
			return 0;
		}
	}
	h = (struct hlist_entry*)kmalloc(sizeof(struct hlist_entry),GFP_ATOMIC);
        if(h != NULL)
                 {
                         h->td = td;
                         for(i = 1; i <size2; i++)
			 {
				 h->arraykey[i] = array[i];
			}
			 hash_add(hlist,&h->node,key);
                         return 0;
                 }
                 else
                 {
                         return -ENOMEM;
                 }

}
*/
static int store_rbtree_value(unsigned int jkey, unsigned long *array, unsigned int size2, u64 td)
{
	struct rbtree_entry *node3, *anode, *travnode;
	struct rb_node **cnode, *parent = NULL, *prevnode;
	unsigned int cputime,key;
	int i;
	prevnode = rb_last(&rbroot);
	while(prevnode)
	{
		travnode = rb_entry(prevnode,struct rbtree_entry,rbentry);
		prevnode = rb_prev(prevnode);
		if(travnode->jkey == key)
		{
			cputime = travnode->td; 
			rb_erase(&travnode->rbentry,&rbroot);
			kfree(travnode);
		}	
	}
	node3 = (struct rbtree_entry*)kmalloc(sizeof(struct rbtree_entry),GFP_ATOMIC);
	if(node3 != NULL)
	{
		node3->jkey = jkey;
		node3->td = td+cputime;
		
		for(i = 0; i < 20; i++)
		{
			node3->arraykey[i] = array[i];
		}
		cnode = &rbroot.rb_node;
		while(*cnode != NULL)
		{
			parent = *cnode;
			anode = rb_entry(parent, struct rbtree_entry, rbentry);
			if(anode->td < td+cputime)
			{
				cnode = &((*cnode)->rb_right);
			}
			else
			{
				cnode = &((*cnode)->rb_left);
			}
		}
		rb_link_node(&node3->rbentry, parent, cnode);
		rb_insert_color(&(node3->rbentry),&rbroot);
	}
	else
	{
		printk(KERN_INFO "No memory to add\n");
		return -ENOMEM;
	}
}


//unsigned int arraylength = length(array);
/*static int test_hlist_value(struct seq_file* proc)
{
        int bkt;
	int i;
	unsigned int size = 20;
        struct hlist_entry *h1;
        seq_printf(proc,"Hash table: \n");
        hash_for_each(hlist,bkt,h1,node)
        {
                if(h1 != NULL)
		{
                        if(h1->size == 1)
			{
				seq_printf(proc,"Stack trace absent");
			}
			else
			{
				//seq_printf(proc,"Stack trace: \n");
				//seq_printf(proc,"Schedule count = %d\n",h1->schcount);
				seq_printf(proc,"\nTime spent = %lld\n",h1->td);
				seq_printf(proc,"Stack trace: \n");
				for(i = 1; i < 20; i++)
				{
					//printk(KERN_INFO "Stack trace = %d Schedule count = %d\n",h1->jkey,h1->schcount);	
					//seq_printf(proc,"Stack trace: \n");
					seq_printf(proc,"%p",(void*) h1->arraykey[i]);
				}
			}
                }
        }
        //seq_printf(proc,"\n");
}
*/
static void test_rbtree(struct seq_file* proc)
{
	struct rb_node *cur;
	struct rbtree_entry* rdnode;
	int i,j,spaces = 2;
	cur = rb_last(&rbroot);
	//seq_printf(proc,"Stack trace:");
	for(i = 0; i < 20; i++)
	{
		if(cur != NULL)
		{
			rdnode = rb_entry(cur, struct rbtree_entry, rbentry);
			//printk(KERN_INFO "Red-black tree: %d\n",rdnode->val);
			//if(cur->size2 == 1)
			//{
			//	seq_printf(proc,"Stack trace absent");
			//}
			//else
			//{
				seq_printf(proc,"\nTime spent on CPU = %lld dtsc ticks\n",rdnode->td);
				seq_printf(proc,"Stack trace:\n");
				for (j = 0; j < 20 ; j++)
				{
					//seq_printf(proc,"\nTime spent on CPU = %lld\n",rdnode->td);
					//seq_printf(proc,"Stack trace:\n");
					seq_printf(proc,"%*c%pS\n", 1 + spaces, ' ', (void*)rdnode->arraykey[j]);
					//"%*c%pS\n", 1 + spaces, ' ', (void *)entries[i]
				}
			//}			
		}	//seq_printf(proc,"Time spent on CPU = %lld dtsc ticks\n",rdnode->td);
		cur = rb_prev(cur);
	}
}
static void run_tests(struct seq_file* proc)
{
	//test_hlist_value(proc);
	test_rbtree(proc);
}	


static int proc_show(struct seq_file *proc, void *v) {
	  //seq_printf(proc, "PID = %d\n", pid);
	  run_tests(proc);
	  return 0;
}

static int proc_open(struct inode *inode, struct  file *file) {
	  return single_open(file, proc_show, NULL);
}


static const struct file_operations proc_fops = {
	.owner = THIS_MODULE,
	.open = proc_open,
	.read = seq_read,
};

static int entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs){
	struct my_data *data;
	unsigned int pid;
	unsigned int jkey;
	unsigned int size2;
	unsigned long array[20];
	//unsigned long array2
	unsigned int size = 20;
	unsigned int skipnr = 0;
	u64 prevts;
	u64 td;
	data = (struct my_data *)ri->data;
	unsigned long task_struct_pointer = regs->si;
	struct task_struct* task = (struct task_struct*)task_struct_pointer;
	pid = (unsigned int)task->pid;
	//counter++;
	//printk("Pre-handler counter=%d\n",pid);
	//store_hlist_value(pid);
	if ((task->mm) == NULL)
	{
		size2 = stack_trace_save(array,size,skipnr);
	}
	else
	{
		size2 = stack_trace_save_user(array, size);
	}
	jkey = jhash(array, size2, skipnr);
	prevts = rdtsc(); 
	td = prevts - data->nextts;
	//store_hlist_value(jkey,array,size2,td);
	store_rbtree_value(jkey,array,size2,td);
	return 0;
}

static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs){
	//counter++;
	struct my_data *data = (struct my_data *)ri->data;
	data->nextts = rdtsc();
	return 0;
}

static struct kretprobe my_kretprobe = {
	.handler = ret_handler,
	.entry_handler = entry_handler,
	.data_size = sizeof(struct my_data),
	/* Probe up to 20 instances concurrently. */
	.maxactive = 20,
};
static int open_module(void)
{
	
	//printk("Module in\n ");
	kallsyms_stack_trace_save_user = (void*)kallsyms_lookup_name("stack_trace_save_user");
	int ret;
	printk("Module in\n ");
	my_kretprobe.kp.symbol_name = func_name;
        ret = register_kretprobe(&my_kretprobe);
	if (ret < 0) {
		pr_err("register_kretprobe failed, returned %d\n", ret);
		return -1;
	}
	proc_create("perftop", 0, NULL, &proc_fops);
	return 0;
}

static void close_module(void)
{
	remove_proc_entry("perftop", NULL);
	unregister_kretprobe(&my_kretprobe);
	printk("Module out\n ");
}


module_init(open_module);


module_exit(close_module);
MODULE_LICENSE("GPL");
