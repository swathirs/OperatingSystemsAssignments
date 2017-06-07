#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include "scheduler.h"
#include "queue.h"

#define _GNU_SOURCE
#include <sched.h>


#define STACK_SIZE 1024 * 1024
//typedef unsigned char byte;

struct thread *current_thread;
struct queue *ready_list;
//struct thread *new_thread;

struct thread *next_thread;
struct queue *done_list;

void thread_start(struct thread * old, struct thread * new);
void thread_switch(struct thread * old, struct thread * new);

void scheduler_begin()
{
	set_current_thread(malloc(sizeof(struct thread)));
	current_thread -> state_t = RUNNING;
	
	ready_list = malloc(sizeof(struct queue));
	ready_list -> head = NULL;
	ready_list -> tail = NULL;

	done_list = malloc(sizeof(struct queue));
	done_list -> head = NULL;
	done_list -> tail = NULL;

}

struct thread* thread_fork(void (*target) (void *), void *arg)
{
	struct thread *new_thread;
	new_thread = malloc(sizeof(struct thread));
	new_thread -> stack_pointer = malloc(STACK_SIZE) + STACK_SIZE;
	new_thread -> initial_function = target;
	new_thread -> initial_argument = arg;
	
	current_thread -> state_t = READY;
	thread_enqueue(ready_list, current_thread);
	
	new_thread -> state_t = RUNNING;
	
	struct thread *temp;
	temp = current_thread;
	set_current_thread(new_thread);
	thread_start(temp, current_thread);
	
	//printf("\n DEBUG:Back in thread_fork() after thread_start \n");
	return new_thread; //What does this do?
		
}

void yield() 
{
	if((current_thread -> state_t) != DONE && (current_thread -> state_t) != BLOCKED) 
	{
		current_thread -> state_t = READY;
		thread_enqueue(ready_list, current_thread);	
		
	} 
	if(!is_empty(ready_list))
	{
		next_thread = thread_dequeue(ready_list);
		next_thread -> state_t = RUNNING;
	}
	else
	{
		printf("****FATAL ERROR!!! ReadyList is empty!****");	
		exit(1);
	}
	struct thread *temp;
	temp = current_thread;
	set_current_thread(next_thread);
  	thread_switch(temp, current_thread);

}

void thread_exit()
{
	//condition_signal(current_thread->c);
	current_thread->state_t = DONE;
	thread_enqueue(done_list, current_thread);//put threads in done_list
	yield();
}

void thread_wrap() 
{
    	current_thread->initial_function(current_thread->initial_argument);
	
	//printf("\nI'm done executing.. \n");
	
	thread_exit(); //don't have it here
	
}

void scheduler_end()
{
	while(!is_empty(ready_list))
	{
		yield();
	}
	
	while(!is_empty(done_list)) // deallocate the memory for the completed threads
	{
		next_thread =thread_dequeue(done_list);
		free(next_thread);
		
	}
	free(ready_list);
	free(done_list);
	free(current_thread);
		
	printf("\nDeallocating memory for queue and threads\n");
	
}

/*Initializing mutex, implementing lock and unlock functions*/
/*mutex_init should initialize all fields of struct mutex*/
void mutex_init(struct mutex * mutex_lock)
{
	mutex_lock->held = 0;
	mutex_lock->waiting_threads = malloc(sizeof(struct queue));
	mutex_lock->waiting_threads->head = NULL;
	mutex_lock->waiting_threads->tail = NULL;
}


void mutex_lock(struct mutex *mtx_lock)
{
	if(mtx_lock->held == 0)
	{
		mtx_lock->held = 1;
	}
	else
	{
		current_thread->state_t = BLOCKED;
		thread_enqueue(mtx_lock->waiting_threads, current_thread);
		yield(); 
	}
}

void mutex_unlock(struct mutex * mutex_lock)
{
	if(!is_empty(mutex_lock->waiting_threads))
	{
		struct thread * t = thread_dequeue(mutex_lock->waiting_threads);
		t->state_t = READY;
		thread_enqueue(ready_list, t);
	}
	else
	{
		mutex_lock->held = 0;
	} //When will the lock become free next?  How will Thread-2 get it when it's already held by Thread-1
} 


void condition_init(struct condition *cv) //condition_init should initialize all fields of struct condition
{
   
   cv->waiting_threads  = malloc(sizeof(struct queue));
   cv->waiting_threads->head = NULL;
   cv->waiting_threads->tail = NULL;

}

/*condition_wait should unlock the supplied mutex and cause the thread to block. The mutex should be re-locked after the thread wakes up.*/
void condition_wait(struct condition *cv, struct mutex *mtx_lock)
{
	if(mtx_lock->held == 1)
	{
  		//release  mutex
  		mutex_unlock(mtx_lock);
	}
  	current_thread->state_t = BLOCKED;
  	// adds current thread to the condition variable waiting list
  	thread_enqueue(cv->waiting_threads,current_thread);
  	yield();
  	mutex_lock(mtx_lock); // acquire lock as soon as signaled - control comes here after signaling
}

/*condition_signal should wake up a waiting thread by adding it back on to the ready list.*/
void condition_signal(struct condition *cv)
{
  	struct thread  *wokenUp_thread;
  	if(!is_empty(cv->waiting_threads))
  	{
   		wokenUp_thread = thread_dequeue(cv->waiting_threads);
   		wokenUp_thread->state_t=READY;
   		thread_enqueue(ready_list,wokenUp_thread);
   }
}

/*condition_broadcast should signal all waiting threads*/
void condition_broadcast(struct condition *cv)
{
    	struct thread *wokenUp_thread;
    
    	while(!is_empty(cv->waiting_threads))
    	{
         	wokenUp_thread = thread_dequeue(cv->waiting_threads);
         	wokenUp_thread->state_t=READY;
		thread_enqueue(ready_list, wokenUp_thread);
    	}
}

/*Test your condition variables by implementing thread_join using condition variables. This will require you to add a mutex and condition variable to the thread control block.*/
void thread_join(struct thread * t) // t is the forked thread
{
	t->c = malloc(sizeof(struct condition));
	condition_init(t->c);

	t->m = malloc(sizeof(struct mutex));
	mutex_init(t->m);
	if(t->state_t != DONE)
		condition_wait(t->c, t->m);
}



