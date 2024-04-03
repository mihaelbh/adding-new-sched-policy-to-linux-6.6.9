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


void init_new_rq(struct new_rq* new_rq) {
    for(int i=0; i<10; i++) {
        new_rq->sched_queue[i].prev = &new_rq->sched_queue[i];
        new_rq->sched_queue[i].next = &new_rq->sched_queue[i];
    }
    new_rq->nr_running = 0;
    printk(KERN_INFO "init_new_rq\n");
}




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

    list_add_tail(&p->nst.node, &rq->new_rq.sched_queue[p->nst.priority]);

    p->nst.on_rq = 1;
    (rq->new_rq.nr_running)++;
    printk(KERN_INFO "exit enqueue_task_new, task enqueued\n");
}

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

    struct new_sched_task* found = NULL;
    list_for_each_entry(found, &rq->new_rq.sched_queue[p->nst.priority], node) {
        if(found->id == p->nst.id) {
            list_del(&found->node);
            break;
        }
    }

    p->nst.on_rq = 0;
    (rq->new_rq.nr_running)--;
    printk(KERN_INFO "exit dequeue_task_new, task dequeued\n");
}

/*
find list for priority that is not empty
*/
int find_not_empty(struct rq* rq) {
    for(int i=0; i<10; i++) {
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

    int pos = find_not_empty(rq);

    if(pos < 0) {
        return NULL;
    }

    struct new_sched_task* new_task = container_of(rq->new_rq.sched_queue[pos].next, struct new_sched_task, node);

    struct task_struct* task = container_of(new_task, struct task_struct, nst);
    
    printk(KERN_INFO "exit pick_next_task_new\n");
    return task;
}

/*
is called before a task is removed from CPU
*/
static void put_prev_task_new(struct rq *rq, struct task_struct *p) {
    printk(KERN_INFO "put_prev_task_new\n");
}

/*
mark a task that has to stop executing
*/
static void check_preempt_curr_new(struct rq *rq, struct task_struct *p, int flags) {
    
    printk(KERN_INFO "enter check_preempt_curr_new\n");

    if (p->prio < rq->curr->prio) {
        if(rq->curr->nst.priority < 9){
            rq->curr->nst.priority++;
        }
		resched_curr(rq);
	} else if(rq->curr->prio == p->prio && p->prio == 98 && p->nst.priority > rq->curr->nst.priority) {
        if(rq->curr->nst.priority < 9){
            rq->curr->nst.priority++;
        }
        resched_curr(rq);
    }

    printk(KERN_INFO "exit check_preempt_curr_new\n");
}

/*
called when task changes policy or group
*/
static void set_next_task_new(struct rq *rq, struct task_struct *p, bool first) {
    printk(KERN_INFO "set_next_task_new\n");

    p->se.exec_start = rq_clock_task(rq);
}

/*
update runtime statistics
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

    //if time_slice is 0

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

    resched_curr(rq);

    printk(KERN_INFO "exit task_tick_new, task rescheduled\n");
}

/*
if task switches to SCHED_NEW we must check if it needs to be preempted
*/
static void switched_to_new(struct rq *rq, struct task_struct *p) {
    printk(KERN_INFO "enter switched_to_new\n");
    check_preempt_curr_new(rq, p, 0);
    printk(KERN_INFO "exit switched_to_new\n");
}

/*
if prio is changed check if the task needs to be preempted
*/
static void prio_changed_new(struct rq *rq, struct task_struct *p, int oldprio) {
    printk(KERN_INFO "enter prio_changed_new\n");
    check_preempt_curr_new(rq, p, 0);
    printk(KERN_INFO "exit prio_changed_new\n");
}

/*

*/
static void yield_task_new(struct rq *rq) {
    printk(KERN_INFO "enter yield_task_new\n");

    list_move_tail(&rq->curr->nst.node, &rq->new_rq.sched_queue[rq->curr->nst.priority]);

    printk(KERN_INFO "exit yield_task_new\n");
}

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