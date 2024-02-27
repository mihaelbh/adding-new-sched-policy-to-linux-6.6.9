#include<linux/rbtree.h>
#include <linux/printk.h>
#include "../../include/linux/container_of.h"
#include "../../include/linux/sched.h"
#include "../../include/linux/sched/clock.h"
#include <linux/sched.h>
#include "sched.h"



const struct sched_class new_sched_class;



void init_new_rq(struct new_rq* new_rq) {
    new_rq->new_root = RB_ROOT;
    new_rq->nr_running = 0;
    printk(KERN_INFO "init_new_rq\n");
}


int rb_insert(struct rb_root* root, struct new_sched_task* task) {
    struct rb_node *new = root->rb_node;
    struct rb_node *parent = NULL;

    while(new) {
        parent = new;
        struct new_sched_task* this = container_of(&new, struct new_sched_task, node);
        int rez = task->enqueued_at - this->enqueued_at;

        if(rez < 0) {
            new = new->rb_left;
        } else if(rez > 0) {
            new = new->rb_right;
        } else {
            return 0;
        }
    }

    rb_link_node(task->node, parent, &new);
    rb_insert_color(task->node, root);

    return 1;
}





static void enqueue_task_new(struct rq *rq, struct task_struct *p, int flags) {
    
    printk(KERN_INFO "enter enqueue_task_new\n");

    if(! p) {
        return;
    }

    p->nst.enqueued_at = sched_clock();

    if(! rq) {
        return;
    }

    /*
    spremi task u rb stablo
    */
    rb_insert(&rq->new_rq.new_root, &p->nst);
    (rq->new_rq.nr_running)++;
    printk(KERN_INFO "exit enqueue_task_new\n");
}

static void dequeue_task_new(struct rq *rq, struct task_struct *p, int flags) {
    
    printk(KERN_INFO "enter dequeue_task_new\n");

    if(! p) {
        return;
    }

    p->nst.enqueued_at = 0;

    /*
    makni task iz rb stabla
    */
    rb_erase(p->nst.node, &rq->new_rq.new_root);
    (rq->new_rq.nr_running)--;
    printk(KERN_INFO "exit dequeue_task_new\n");
}

/*
pick next task to run on a CPU
*/
static struct task_struct* pick_next_task_new(struct rq *rq) {

    printk(KERN_INFO "enter pick_next_task_new\n");

    if(! rq) {
        return NULL;
    }

    struct rb_node* first = rb_first(&rq->new_rq.new_root);

    if(!first) {
        return NULL;
    }

    struct new_sched_task* new_task = container_of(&first, struct new_sched_task, node);
    
    struct task_struct* task = container_of(new_task, struct task_struct, nst);
    
    printk(KERN_INFO "exit pick_next_task_new\n");
    return task;
}

/*
is called before a task is removed from CPU
*/
void put_prev_task_new(struct rq *rq, struct task_struct *p) {
    printk(KERN_INFO "put_prev_task_new\n");
}

/*
mark a task that has to stop executing
*/
static void check_preempt_curr_new(struct rq *rq, struct task_struct *p, int flags) {
    
    printk(KERN_INFO "enter check_preempt_curr_new\n");

    struct rb_node* first = rb_first(&rq->new_rq.new_root);

    if(! first) {
        return;
    }

    struct new_sched_task* new_task = container_of(&first, struct new_sched_task, node);
    
    if(p->nst.enqueued_at < new_task->enqueued_at || rq->curr->rt_priority < 1) {
        resched_curr(rq);
    }

    printk(KERN_INFO "exit check_preempt_curr_new\n");
}

/*
called when timer interupt happends
*/
void task_tick_new(struct rq *rq, struct task_struct *p, int queued) {
    printk(KERN_INFO "task_tick_new\n");
}

/*
called when task changes policy or group
*/
static void set_next_task_new(struct rq *rq, struct task_struct *p, bool first) {
    printk(KERN_INFO "enter set_next_task_new\n");
    rq->curr = p;
    printk(KERN_INFO "exit set_next_task_new\n");
}

/*
update runtime statistics
*/
static void update_curr_new(struct rq *rq) {

    printk(KERN_INFO "enter update_curr_new\n");

    struct task_struct* curr = rq->curr;
    u64 now = rq_clock_task(rq);
	u64 delta_exec =  now - curr->se.exec_start;

    if (curr->sched_class != &new_sched_class || (s64) delta_exec <= 0) {
		return;
    }

    curr->se.exec_start = now;
    curr->se.sum_exec_runtime += delta_exec;

    printk(KERN_INFO "exit update_curr_new\n");
}

DEFINE_SCHED_CLASS(new) = {
    .enqueue_task   = enqueue_task_new,
    .dequeue_task   = dequeue_task_new,
    .pick_next_task = pick_next_task_new,
    .check_preempt_curr = check_preempt_curr_new,
    .put_prev_task = put_prev_task_new,
    .task_tick = task_tick_new,
    .set_next_task = set_next_task_new,
    .update_curr = update_curr_new
};