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

int rb_search(struct rb_root* root, struct new_sched_task* task) {
    struct rb_node* iterator = rb_first(root);
    int rez;

    while(iterator) {
        struct new_sched_task* this = container_of(&iterator, struct new_sched_task, node);
        rez = task->enqueued_at - this->enqueued_at;

        if(!rez) {
            return 1;
        }

        iterator = rb_next(iterator);
    }

    return 0;
}




static void enqueue_task_new(struct rq *rq, struct task_struct *p, int flags) {
    
    printk(KERN_INFO "enter enqueue_task_new\n");

    if(! p) {
        printk(KERN_INFO "exit enqueue_task_new, p is NULL\n");
        return;
    }

    p->nst.enqueued_at = sched_clock();

    if(! rq) {
        printk(KERN_INFO "exit enqueue_task_new, rq is NULL\n");
        return;
    }

    if(rb_search(&rq->new_rq.new_root, &p->nst)) {
        printk(KERN_INFO "exit enqueue_task_new, task is already in the rbtree\n");
        return;
    }

    rb_insert(&rq->new_rq.new_root, &p->nst);
    (rq->new_rq.nr_running)++;
    printk(KERN_INFO "exit enqueue_task_new\n");
}

static void dequeue_task_new(struct rq *rq, struct task_struct *p, int flags) {
    
    printk(KERN_INFO "enter dequeue_task_new\n");

    if(! p) {
        printk(KERN_INFO "exit dequeue_task_new, p is NULL\n");
        return;
    }

    p->nst.enqueued_at = 0;

    if(! rq) {
        printk(KERN_INFO "exit dequeue_task_new, rq is NULL\n");
        return;
    }

    if(!rb_search(&rq->new_rq.new_root, &p->nst)) {
        printk(KERN_INFO "exit enqueue_task_new, task is not in rbtree\n");
        return;
    }

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
        printk(KERN_INFO "exit pick_next_task_new, rq is NULL\n");
        return NULL;
    }

    struct rb_node** first = kzalloc(sizeof(struct rb_node*), GFP_KERNEL);
    *first = rb_first(&rq->new_rq.new_root);

    if(! (*first)) {
        printk(KERN_INFO "exit pick_next_task_new, rb_tree is empty\n");
        return NULL;
    }

    struct new_sched_task* new_task = container_of(first, struct new_sched_task, node);


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

    struct rb_node* first = rb_first(&rq->new_rq.new_root);

    if(! first) {
        printk(KERN_INFO "exit check_preempt_curr_new, rb_tree is empty\n");
        return;
    }

    struct new_sched_task* new_task = container_of(&first, struct new_sched_task, node);
    
    if((p->nst.enqueued_at < new_task->enqueued_at && rq->curr->policy == SCHED_NEW) || 
        rq->curr->policy == SCHED_FIFO || rq->curr->policy == SCHED_RR || rq->curr->policy == SCHED_DEADLINE) {
        resched_curr(rq);
    }

    printk(KERN_INFO "exit check_preempt_curr_new\n");
}

/*
called when task changes policy or group
*/
static void set_next_task_new(struct rq *rq, struct task_struct *p, bool first) {
    printk(KERN_INFO "set_next_task_new\n");
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
    printk(KERN_INFO "task_tick_new\n");
}

/*
if task switches from SCHED_NEW it must be dequeued
*/
static void switched_from_new(struct rq *rq, struct task_struct *p) {
    printk(KERN_INFO "enter switched_from_new\n");
    dequeue_task_new(rq, p, 0);
    printk(KERN_INFO "exit switched_from_new\n");
}

/*
if task switches to SCHED_NEW it must be enqueued
*/
static void switched_to_new(struct rq *rq, struct task_struct *p) {
    printk(KERN_INFO "enter switched_to_new\n");
    enqueue_task_new(rq, p, 0);
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
dequeue and enqueue again and check if it needs to be preempted
*/
static void yield_task_new(struct rq *rq) {
    printk(KERN_INFO "enter yield_task_new\n");

    struct task_struct* p = rq->curr;

    dequeue_task_new(rq, p, 0);
    enqueue_task_new(rq, p, 0);
    check_preempt_curr_new(rq, p, 0);

    printk(KERN_INFO "exit yield_task_new\n");
}

static unsigned int get_rr_interval_new(struct rq *rq, struct task_struct *task) {
    printk(KERN_INFO "get_rr_interval_new\n");
    return 0;
}

DEFINE_SCHED_CLASS(new) = {
    .enqueue_task           = enqueue_task_new,
    .dequeue_task           = dequeue_task_new,
    .pick_next_task         = pick_next_task_new,
    .check_preempt_curr     = check_preempt_curr_new,
    .put_prev_task          = put_prev_task_new,
    .task_tick              = task_tick_new,
    .set_next_task          = set_next_task_new,
    .update_curr            = update_curr_new,
    .switched_from          = switched_from_new,
    .switched_to            = switched_to_new,
    .prio_changed           = prio_changed_new,
    .yield_task		        = yield_task_new,
    .get_rr_interval	    = get_rr_interval_new
};