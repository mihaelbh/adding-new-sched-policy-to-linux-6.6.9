#include <linux/printk.h>
#include "../../include/linux/sched.h"
#include <linux/sched.h>
#include "sched.h"
#include <linux/list.h>
#include "../../include/linux/list.h"
#include <linux/types.h>
#include <linux/sched/rt.h>


const struct sched_class new_sched_class;

u64 new_id = 1;

/*
function for initializing new_rq for every cpu
called in sched_init() in /kernel/sched/core.c
*/
void init_new_rq(struct new_rq* new_rq) {
    for(int i=0; i<10; i++) {
        new_rq->sched_queue[i].prev = &new_rq->sched_queue[i];
        new_rq->sched_queue[i].next = &new_rq->sched_queue[i];
    }
    new_rq->nr_running = 0;
    printk(KERN_INFO "init_new_rq\n");
}

/*
function for enqueueing tasks
*/
static void enqueue_task_new(struct rq *rq, struct task_struct *p, int flags) {
    
    printk(KERN_INFO "enter enqueue_task_new\n");

    if(! p) {
        printk(KERN_INFO "exit enqueue_task_new, p is NULL\n");
        return;
    }

    if(! rq) {
        printk(KERN_INFO "exit enqueue_task_new, rq is NULL\n");
        return;
    }

    //set id for new_task_struct
    if(p->nst.id == 0) {
        p->nst.id = new_id;
        new_id++;
    }

    //set time_slice (default RR time_slice is used)
    p->nst.time_slice = RR_TIMESLICE;

    //add task to the end of the list
    list_add_tail(&p->nst.node, &rq->new_rq.sched_queue[p->nst.priority]);

    //mark that the task is on runqueue
    p->nst.on_rq = 1;

    //increment the number of tasks on runqueue
    (rq->new_rq.nr_running)++;
    printk(KERN_INFO "exit enqueue_task_new, task enqueued\n");
}

/*
function for dequeueing tasks
*/
static void dequeue_task_new(struct rq *rq, struct task_struct *p, int flags) {
    
    printk(KERN_INFO "enter dequeue_task_new\n");

    if(! p) {
        printk(KERN_INFO "exit dequeue_task_new, p is NULL\n");
        return;
    }

    if(! rq) {
        printk(KERN_INFO "exit dequeue_task_new, rq is NULL\n");
        return;
    }

    //remove task from the list
    struct new_sched_task* found = NULL;
    list_for_each_entry(found, &rq->new_rq.sched_queue[p->nst.priority], node) {
        if(found->id == p->nst.id) {
            list_del(&found->node);
            break;
        }
    }

    //set that task isn't on runqueue
    p->nst.on_rq = 0;

    //decrement number of tasks on runqueue
    (rq->new_rq.nr_running)--;
    printk(KERN_INFO "exit dequeue_task_new, task dequeued\n");
}

/*
find list for the biggest priority that is not empty
*/
int find_not_empty(struct rq* rq) {
    for(int i=9; i>=0; i--) {
        if(rq->new_rq.sched_queue[i].next != rq->new_rq.sched_queue[i].prev) {
            return i;
        }
    }

    return -1;
}

/*
pick next task to run on a CPU
*/
static struct task_struct* pick_next_task_new(struct rq *rq) {

    printk(KERN_INFO "enter pick_next_task_new\n");

    if(! rq) {
        printk(KERN_INFO "exit pick_next_task_new, rq is NULL\n");
        return NULL;
    }

    //find the biggest priority list that isn't empty
    int pos = find_not_empty(rq);

    //if it returns -1 all lists are empty
    if(pos < 0) {
        return NULL;
    }

    //get new_sched_task from list_head
    struct new_sched_task* new_task = container_of(rq->new_rq.sched_queue[pos].next, struct new_sched_task, node);

    //get task_struct from new_sched_task
    struct task_struct* task = container_of(new_task, struct task_struct, nst);
    
    printk(KERN_INFO "exit pick_next_task_new\n");
    return task;
}

/*
doesn't do anything but is part of sched_class
*/
static void put_prev_task_new(struct rq *rq, struct task_struct *p) {
    printk(KERN_INFO "put_prev_task_new\n");
}

/*
marks a task that needs to stop to give cpu to more important task
*/
static void check_preempt_curr_new(struct rq *rq, struct task_struct *p, int flags) {
    
    printk(KERN_INFO "enter check_preempt_curr_new\n");

    if (p->prio < rq->curr->prio) {
        //if there is a task with bigger prio

        //increment priority
        if(rq->curr->nst.priority < 9){
            rq->curr->nst.priority++;
        }

        //reschedul current runqueue
		resched_curr(rq);
	} else if(rq->curr->prio == p->prio && p->prio == 98 && p->nst.priority > rq->curr->nst.priority) {
        //if the tasks have same prio 98 (prio of new_sched_class) and current task has lower priority

        //increment priority
        if(rq->curr->nst.priority < 9){
            rq->curr->nst.priority++;
        }

        //reschedul current runqueue
        resched_curr(rq);
    }

    printk(KERN_INFO "exit check_preempt_curr_new\n");
}

/*
called when task changes policy or group
changes when it started to current time
*/
static void set_next_task_new(struct rq *rq, struct task_struct *p, bool first) {
    printk(KERN_INFO "set_next_task_new\n");

    p->se.exec_start = rq_clock_task(rq);
}

/*
doesn't do anything but is part of sched_class
*/
static void update_curr_new(struct rq *rq) {
    printk(KERN_INFO "update_curr_new\n");
}

/*
called when timer interupt happends
*/
static void task_tick_new(struct rq *rq, struct task_struct *p, int queued) {
    printk(KERN_INFO "enter task_tick_new\n");

    //decrement time_slice and if it isn't 0 return
    if(--p->nst.time_slice) {
        printk(KERN_INFO "exit task_tick_new, time_slice decremented\n");
        return;
    }

    //if time_slice is 0 after decrement

    //reset time_slice (default RR time_slice is used)
    p->nst.time_slice = RR_TIMESLICE;

    //remove task from list with current priority
    struct new_sched_task* found = NULL;
    list_for_each_entry(found, &rq->new_rq.sched_queue[p->nst.priority], node) {
        if(found->id == p->nst.id) {
            list_del(&found->node);
            break;
        }
    }

    //lower priority
    if(p->nst.priority > 0) {
        p->nst.priority--;
    }

    //add task to list with new priority
    list_add_tail(&p->nst.node, &rq->new_rq.sched_queue[p->nst.priority]);

    //reschedul runqueue
    resched_curr(rq);

    printk(KERN_INFO "exit task_tick_new, task rescheduled\n");
}

/*
called when task switches to SCHED_NEW
*/
static void switched_to_new(struct rq *rq, struct task_struct *p) {
    printk(KERN_INFO "enter switched_to_new\n");
    //we must check if it needs to be preempted
    check_preempt_curr_new(rq, p, 0);
    printk(KERN_INFO "exit switched_to_new\n");
}

/*
called if prio is changed
*/
static void prio_changed_new(struct rq *rq, struct task_struct *p, int oldprio) {
    printk(KERN_INFO "enter prio_changed_new\n");
    //check if the task needs to be preempted
    check_preempt_curr_new(rq, p, 0);
    printk(KERN_INFO "exit prio_changed_new\n");
}

/*
moves task to the end of the list
*/
static void yield_task_new(struct rq *rq) {
    printk(KERN_INFO "enter yield_task_new\n");

    //move task to the end of the list
    list_move_tail(&rq->curr->nst.node, &rq->new_rq.sched_queue[rq->curr->nst.priority]);

    printk(KERN_INFO "exit yield_task_new\n");
}

/*
returns time_slice
*/
static unsigned int get_rr_interval_new(struct rq *rq, struct task_struct *task) {
    printk(KERN_INFO "get_rr_interval_new\n");
    return RR_TIMESLICE;
}

DEFINE_SCHED_CLASS(new) = {

	.enqueue_task		= enqueue_task_new,
	.dequeue_task		= dequeue_task_new,
	.yield_task		    = yield_task_new,

	.check_preempt_curr = check_preempt_curr_new,

	.pick_next_task		= pick_next_task_new,
	.put_prev_task		= put_prev_task_new,
	.set_next_task      = set_next_task_new,

	.task_tick		    = task_tick_new,

	.get_rr_interval	= get_rr_interval_new,

	.prio_changed		= prio_changed_new,
	.switched_to		= switched_to_new,

	.update_curr		= update_curr_new,
};