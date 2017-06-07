#define current_thread (get_current_thread())

struct thread
{
	unsigned char *stack_pointer;
	void (*initial_function)(void *);
	void *initial_argument;
	enum 
	{
    		RUNNING, // The thread is currently running.
    		READY,   // The thread is not running, but is runnable.
    		BLOCKED, // The thread is not running, and not runnable.
    		DONE     // The thread has finished. 
  	}state_t;
	struct mutex *m;
	struct condition *c;
};
extern struct thread * current_thread;

extern struct thread * get_current_thread();
extern void set_current_thread(struct thread*);

struct mutex 
{
	int held;
     	struct queue *waiting_threads;
};

struct condition 
{
     	struct queue *waiting_threads;
};


void scheduler_begin();
struct thread* thread_fork(void (*target) (void *)  , void *arg);
void yield();
void scheduler_end();
void thread_exit();

/*MUTEX*/
void mutex_init(struct mutex *);
void mutex_lock(struct mutex *);
void mutex_unlock(struct mutex *);

/*CONDITION VARIABLES*/
void thread_join(struct thread *);
void condition_init(struct condition *);
void condition_wait(struct condition *, struct mutex *);
void condition_signal(struct condition *);
void condition_broadcast(struct condition *);


