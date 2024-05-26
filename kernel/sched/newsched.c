#include <linux/printk.h>
#include "../../include/linux/sched.h"
#include <linux/sched.h>
#include "sched.h"
#include <linux/list.h>
#include "../../include/linux/list.h"
#include <linux/types.h>
#include <linux/sched/rt.h>
#include "./stats.h"

const struct sched_class new_sched_class;

/*
function for initializing new_rq for every cpu
called in sched_init() in /kernel/sched/core.c
*/
void init_new_rq(struct new_rq* new_rq) {
    for(int i=0; i<10; i++) {
        INIT_LIST_HEAD(&new_rq->sched_queue[i]);
    }
    new_rq->nr_running = 0;
    printk(KERN_INFO "init_new_rq\n");
}

void print_rq(struct list_head* head) {
    struct new_sched_task* new_task = NULL;
    int i=0;
    list_for_each_entry(new_task, head, node) {
        struct task_struct* p = container_of(new_task, struct task_struct, nst);
        printk(KERN_INFO "new_rq[%d]: %d\n", i, p->pid);
        i++;
    }
}

/*
updates current task's runtime statistics
*/
static void update_curr_new(struct rq *rq) {
    printk(KERN_INFO "enter update_curr_new\n");
    struct task_struct *curr = rq->curr;
	u64 now = rq_clock_task(rq);
    u64 delta_exec = now - curr->se.exec_start;

    if (curr->sched_class != &new_sched_class) {
		return;
    }

    if (unlikely((s64)delta_exec <= 0)) {
		return;
    }

    schedstat_set(curr->stats.exec_max, max(curr->stats.exec_max, delta_exec));

    trace_sched_stat_runtime(curr, delta_exec, 0);

	update_current_exec_runtime(curr, now, delta_exec);
    printk(KERN_INFO "exit update_curr_new\n");
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

    if(p->nst.on_rq) {
        printk(KERN_INFO "exit enqueue_task_new, task on rq\n");
        return;
    }

    //set time_slice (default RR time_slice is used)
    p->nst.time_slice = RR_TIMESLICE;

    //add task to the end of the list
    list_add_tail(&p->nst.node, &rq->new_rq.sched_queue[p->nst.priority]);

    //mark that the task is on runqueue
    p->nst.on_rq = 1;
    p->on_rq = 1;

    //increment the number of tasks on runqueue
    (rq->new_rq.nr_running)++;
    printk(KERN_INFO "exit enqueue_task_new, task %d enqueued\n", p->pid);
    print_rq(&rq->new_rq.sched_queue[p->nst.priority]);
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

    update_curr_new(rq);

    //remove task from the list
    list_del_init(&p->nst.node);

    //set that task isn't on runqueue
    p->nst.on_rq = 0;
    p->on_rq = 0;

    //decrement number of tasks on runqueue
    (rq->new_rq.nr_running)--;
    printk(KERN_INFO "exit dequeue_task_new, task %d dequeued\n", p->pid);
    print_rq(&rq->new_rq.sched_queue[p->nst.priority]);
}

/*
find list for the biggest priority that is not empty
*/
int find_not_empty(struct rq* rq) {
    for(int i=9; i > -1; i--) {
        if(!list_empty(&rq->new_rq.sched_queue[i])) {
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
        printk(KERN_INFO "exit pick_next_task_new, no task to run next\n");
        return NULL;
    }

    //get new_sched_task from list_head
    struct new_sched_task* new_task = list_entry(rq->new_rq.sched_queue[pos].next, struct new_sched_task, node);

    //get task_struct from new_sched_task
    struct task_struct* task = container_of(new_task, struct task_struct, nst);
    
    printk(KERN_INFO "exit pick_next_task_new\n");
    return task;
}

/*
puts a running task back into a runqueue
*/
static void put_prev_task_new(struct rq *rq, struct task_struct *p) {
    printk(KERN_INFO "enter put_prev_task_new\n");
    update_curr_new(rq);
    printk(KERN_INFO "exit put_prev_task_new\n");
}

/*
marks a task that needs to stop to give cpu to more important task
*/
static void check_preempt_curr_new(struct rq *rq, struct task_struct *p, int flags) {
    
    printk(KERN_INFO "enter check_preempt_curr_new\n");

    if ((p->prio < rq->curr->prio) || (rq->curr->prio == p->prio && p->prio == 98 && p->nst.priority > rq->curr->nst.priority)) {
        //if there is a task with smaller prio (dl_prio = -1, rt_prio = [0, 97], new_prio = 98, fair_prio > 98) or
        //if the tasks have same prio 98 (prio of new_sched_class) and current task has lower priority

        //remove task from the list
        list_del_init(&rq->curr->nst.node);

        //increment priority
        if(rq->curr->nst.priority < 9){
            rq->curr->nst.priority++;
        }

        //add task to the end of the list
        list_add_tail(&rq->curr->nst.node, &rq->new_rq.sched_queue[rq->curr->nst.priority]);

        printk(KERN_INFO "task %d preempted\n", rq->curr->pid);

        //reschedul current runqueue
		resched_curr(rq);
	}

    printk(KERN_INFO "exit check_preempt_curr_new\n");
}

/*
called when task changes policy or group
*/
static void set_next_task_new(struct rq *rq, struct task_struct *p, bool first) {
    printk(KERN_INFO "enter set_next_task_new\n");
    p->se.exec_start = rq_clock_task(rq);
    printk(KERN_INFO "exit set_next_task_new\n");
}

/*
called when timer interupt happends
*/
static void task_tick_new(struct rq *rq, struct task_struct *p, int queued) {
    printk(KERN_INFO "enter task_tick_new\n");

    update_curr_new(rq);

    //decrement time_slice and if it isn't 0 return
    if(--p->nst.time_slice) {
        printk(KERN_INFO "exit task_tick_new, time_slice decremented\n");
        return;
    }

    //if time_slice is 0 after decrement

    //reset time_slice (default RR time_slice is used)
    p->nst.time_slice = RR_TIMESLICE;

    //remove task from list with current priority
    list_del_init(&p->nst.node);

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

    //if we are running
    if(task_current(rq, p)) {
        printk(KERN_INFO "exit switched_to_new, we are running\n");
        return;
    }

    //if we are not running, we must check if current task needs to be preempted
    check_preempt_curr_new(rq, p, 0);
    printk(KERN_INFO "exit switched_to_new, we are not running\n");
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
called when process voluntarily gives up the CPU (for example when it goes to sleep)
*/
static void yield_task_new(struct rq *rq) {
    printk(KERN_INFO "enter yield_task_new\n");

    //remove task from list with current priority
    list_del_init(&rq->curr->nst.node);

    //lower priority
    if(rq->curr->nst.priority > 0) {
        rq->curr->nst.priority--;
    }

    //add task to list with new priority
    list_add_tail(&rq->curr->nst.node, &rq->new_rq.sched_queue[rq->curr->nst.priority]);

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

    .update_curr		= update_curr_new
};