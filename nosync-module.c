#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>  // for threads

#define MAXTHREADS  2
#define MAXITERS    50000
#define ARRAYSIZE   MAXITERS*MAXTHREADS

/*
 * This is a structure modelling a buffer.
 *
 * - The @elements[] array is where threads write to the buffer.
 *
 * - @idx points to the current index of the array into which a new
 *   value should be written.
 */
struct buffer {
  int elements[ARRAYSIZE];
  int idx;
};

/*
 * This is a shared buffer into which threads write their contents.
 */
static volatile struct buffer store_buffer;

/* The task structure for Thread1 */
static struct task_struct *threads[MAXTHREADS + 1];

/* Thread1 writes arg1 to the buffer */
static int thread_args[MAXTHREADS + 1];

/* Boolean array to singal if a particular thread is done writing */
static volatile bool threads_done[MAXTHREADS + 1];

/*
 * This function writes the value @val into the buffer @buf
 * at its current index value.
 *
 * This returns the index of the buffer where the value was written.
 */
int write_buffer(volatile struct buffer *buf, int val)
{
	int cur_idx;
	int new_idx;
	int ret;

	do {
		cur_idx = buf->idx;
		new_idx = cur_idx + 1;

		/*
		 * cmpxchg(ptr, oldval, newval) does the following:
		 *
		 * It atomically checks if the value of the memory
		 * pointed to by @ptr is the same as @oldval, and if
		 * so, it sets the value of the memory pointed to by
		 * @ptr to @newval.
		 *
		 * It returns the original value of the memory pointed
		 * to by @ptr.
		 *
		 * atomic {
		 *      ptrval = *ptr;
		 *
		 *      if (ptrval == oldval)
		 *          *ptr = newval
		 *
		 *      return ptrval;
		 * }
		 */
		ret = cmpxchg(&buf->idx, cur_idx, new_idx);
	} while (ret != cur_idx);

	buf->elements[cur_idx] = val;

	return cur_idx;
}

/*
 * This is the function called by each thread.
 * @arg == &thread_args[i] for Threadi.
 *
 * Note: Threads are Thread1, Thread2, ...
 */
int thread_fn(void *arg)
{
	int iter = 0;
	int myarg = *((int *)arg);

	printk(KERN_EMERG "Thread%02d: entering the loop\n", myarg);
	for (iter = 0; iter < MAXITERS; iter++) {
		write_buffer(&store_buffer, myarg);
		schedule();
	}

	threads_done[myarg] = true;

	printk(KERN_EMERG "Thread%02d: Done\n", myarg);
	return 0;
}

void print_observed_thread_buffer(void)
{
	int count[MAXTHREADS + 1] = {0};
	int extras = 0;
	int i, tid;

	for (i = 0; i < store_buffer.idx; i++) {
		for (tid = 1; tid < MAXTHREADS + 1; tid++) {
			if (store_buffer.elements[i]  == thread_args[tid]) {
				count[tid]++;
				break;
			}
		}

		if (tid == MAXTHREADS + 1)
			extras++;
	}

	printk(KERN_EMERG "Observed:");
	printk(KERN_EMERG "\t\t idx = %d,", store_buffer.idx);
	for (tid = 1; tid < MAXTHREADS + 1; tid++)
		printk(KERN_EMERG "\t\t count%02d = %d", tid, count[tid]);
	printk(KERN_EMERG "\t\t extras = %d\n", extras);
}

void print_expected_thread_buffer(void)
{
	int tid;

	printk(KERN_EMERG "Expected:");
	printk(KERN_EMERG "\t\t idx = %d,", MAXITERS*MAXTHREADS);
	for (tid = 1; tid < MAXTHREADS + 1; tid++)
		printk(KERN_EMERG "\t\t count%02d = %d", tid, MAXITERS);
	printk(KERN_EMERG "\t\t extras = %d\n", 0);
}

/*
 * This is the function for the master thread.
 *
 * The master thread creates MAXTHREAD number of threads where each
 * one of them calls the thread_fn function.
 *
 * It then waits for all the threads to finish writing to the array.
 *
 * It then prints the observed values of the array.
 */
int master_fn(void *arg)
{
	int tid;

	for (tid = 1; tid < MAXTHREADS + 1; tid++) {

		thread_args[tid] = tid;
		threads_done[tid] = false;
		threads[tid] = kthread_run(thread_fn, &thread_args[tid],
					   "Thread%02d", tid);
		if (IS_ERR(threads[tid])) {
			printk(KERN_EMERG "Master Thread: Error creating Threads%02d\n",
			       tid);
			return 0;
		}

		printk(KERN_EMERG "Master Thread: Created Thread%02d\n",
		       tid);
	}

	printk(KERN_EMERG "Master Thread: Waiting till the threads are done writing\n");

	while (1) {
		bool done = true;

		for (tid = 1; tid < MAXTHREADS + 1; tid++)
			done = done && threads_done[tid];
		if (done)
			break;

		schedule();
	}

	printk(KERN_EMERG "Master Thread: Tallying the data\n");
	print_observed_thread_buffer();
	print_expected_thread_buffer();
	return 0;
}

int thread_init(void)
{
	struct task_struct *master;

	master = kthread_run(master_fn, NULL, "master_thread");

	if (IS_ERR(master))
		printk(KERN_EMERG "Error creating the master thread\n");

	return 0;
}

static int __init two_threads_init(void)
{
	printk(KERN_EMERG "====== Hello World!! ======\n");
	thread_init();
	return 0;
}

static void __exit two_threads_cleanup(void)
{
	printk(KERN_ERR "======    Goodbye!!    ======\n");
	return;
}

module_init(two_threads_init);
module_exit(two_threads_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gautham R. Shenoy <ego@linux.vnet.ibm.com>");
MODULE_DESCRIPTION("A set of threads hoping to write their destinies without loss");
