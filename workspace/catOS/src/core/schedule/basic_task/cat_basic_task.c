/**
 * @file cat_basic_task.c
 * @brief 
 * @author mio (44330346+mio22440@users.noreply.github.com)
 * @version 1.0
 * @date 2022-07-14
 * Change Logs:
 * Date           Author        Notes
 * 2022-07-14    mio     first verion
 * 2022-08-09    mio     增加task_stat
 * 
 */
/* INC FILE START */
#include "cat_string.h"
#include "cat_basic_task.h"
#include "port.h"
/* INC FILE END */
/* MAC DEFS START */
/* MAC DEFS END */
/* PRI TYPE DEF START */
/**< 任务信息结构体 */
struct _cat_task_info
{
    void               *sp;                             /**< 栈顶(堆栈指针)*/
    uint8_t            *task_name;                      /**< 任务名称*/
    uint8_t             sched_strategy;                 /**< 调度策略 */
    void               *stack_start_addr;               /**< 堆栈起始地址*/
    uint32_t            stack_size;                     /**< 堆栈大小*/
    uint32_t            delay;                          /**< 延时剩余tick数*/
    uint32_t            state;                          /**< 当前状态*/
    uint8_t             prio;                           /**< priority of task*/
    uint32_t            slice;                          /**< 时间片(剩余时间)*/
    uint32_t            suspend_cnt;                    /**< 被挂起的次数*/
    uint32_t            sched_times;                    /**< 调度次数*/
    uint32_t            free_stack;                     /**< 栈空闲空间大小 */
};
/* PRI TYPE DEF END */
/* PUB VAR DEFS START */
/* PUB VAR DEFS END */
/* PRI VAR DEFS START */
#if (CATOS_TASK_ENABLE_STAT == 1)
static struct _cat_task_t *cat_task_stat_list[CATOS_TASK_STAT_MAX_TASK];
static uint8_t cat_task_stat_cnt = 0;
#endif /* #if (CATOS_TASK_ENABLE_STAT == 1) */
/* PRI VAR DEFS END */
/* PRI FUNC DECL START */
/**
 * @brief 默认任务退出函数
 * 
 */
static void _default_task_exit(void);
/**
 * @brief 获取任务信息存入结构体
 * 
 * @param task 
 * @param info 
 */
#if (CATOS_TASK_ENABLE_STAT == 1)
static void cat_task_get_info(struct _cat_task_t *task, struct _cat_task_info *info);
#endif /* #if (CATOS_TASK_ENABLE_STAT == 1) */
/* PRI FUNC DECL END */
/* PUB FUNC DEFS START */
void cat_task_init(
    const uint8_t *task_name,
    struct _cat_task_t *task, 
    void (*entry)(void *), 
    void *arg, 
    uint8_t prio, 
    void *stack_start_addr,
    uint32_t stack_size,
    uint32_t sched_strategy
)
{
    task->task_name = (uint8_t *)task_name;
    task->sched_strategy = sched_strategy;

    cat_memset(stack_start_addr, 0, stack_size);

#ifndef CATOS_BOARD_IS_LINUX
    task->sp = (void *)cat_hw_stack_init(
        (void*)entry,
        (void*)arg,
        (uint8_t *)(stack_start_addr + stack_size - sizeof(uint32_t)),
        (void *)_default_task_exit
    );
#else    
    task->sp = (void *)cat_hw_stack_init(
        (void*)entry,
        (void*)arg,
        (uint8_t *)(stack_start_addr + stack_size - sizeof(uint32_t)),
        (void *)task
    );
#endif

    

    task->entry = (void *)entry;
    task->arg   = arg;

    /* 初始化栈 */
    task->stack_start_addr = stack_start_addr;
    task->stack_size = stack_size;    

#if 0
    sp = stack_start_addr + (stack_size / sizeof(uint32_t));

    //pensv自动保存的部分
    *(--sp) = (uint32_t)(1 << 24);//spsr
    *(--sp) = (uint32_t)entry;//pc
    *(--sp) = (uint32_t)0x14;//lr(r14)
    *(--sp) = (uint32_t)0x12;//r12
    *(--sp) = (uint32_t)0x3;//r3
    *(--sp) = (uint32_t)0x2;//r2
    *(--sp) = (uint32_t)0x1;//r1
    *(--sp) = (uint32_t)arg;//r0

    *(--sp) = (uint32_t)0x11;
    *(--sp) = (uint32_t)0x10;
    *(--sp) = (uint32_t)0x9;
    *(--sp) = (uint32_t)0x8;
    *(--sp) = (uint32_t)0x7;
    *(--sp) = (uint32_t)0x6;
    *(--sp) = (uint32_t)0x5;
    *(--sp) = (uint32_t)0x4;

    task->sp = sp;
#endif

    cat_list_node_init(&(task->link_node));

    task->delay = 0;

    task->state = CATOS_TASK_STATE_RDY;

    task->prio = prio;
    task->slice = CATOS_MAX_SLICE;
    task->suspend_cnt = 0;

    task->sched_times = 0;

#if (CATOS_TASK_ENABLE_STAT == 1)
    cat_task_stat_list[cat_task_stat_cnt++] = task;
#endif /* #if (CATOS_TASK_ENABLE_STAT == 1) */

}
/* PUB FUNC DEFS END */
/* PRI FUNC DEFS START */
/**
 * @brief 默认任务退出函数
 * 
 */
static void _default_task_exit(void)
{
    while(1);
}
/**
 * @brief 获取任务信息存入结构体
 * 
 * @param task 
 * @param info 
 */
#if (CATOS_TASK_ENABLE_STAT == 1)
static void cat_task_get_info(struct _cat_task_t *task, struct _cat_task_info *info)
{
    uint32_t status = cat_hw_irq_disable();

    info->sp               = task-> sp;         
    info->task_name        = task-> task_name;  
    info->sched_strategy   = task-> sched_strategy;
    info->stack_start_addr = task-> stack_start_addr;
    info->stack_size       = task-> stack_size; 
    info->delay            = task-> delay;      
    info->state            = task-> state;      
    info->prio             = task-> prio;       
    info->slice            = task-> slice;      
    info->suspend_cnt      = task-> suspend_cnt;
    info->sched_times      = task-> sched_times;

    cat_hw_irq_enable(status);

    info->free_stack = 0;//初始化空闲堆栈长度
#if 0
    uint32_t *stack_end = task->stack_start_addr;//开始地址是低地址

    while((*stack_end++ == 0) && (stack_end <= task->stack_start_addr + (task->stack_size / sizeof(cat_stack_type_t))))
    {
        info->free_stack_size++;
    }
    info->free_stack_size *= sizeof(cat_stack_type_t);
#else //#if 0
    info->free_stack = info->sp - info->stack_start_addr;
#endif //#if 0
#endif /* #if (CATOS_TASK_ENABLE_STAT == 1) */
}
/* PRI FUNC DEFS END */
/* MAC UNDEF START */
/* MAC UNDEF END */
/* OTHER START */
#if (CATOS_ENABLE_CAT_SHELL == 1)
#if (CATOS_TASK_ENABLE_STAT == 1)
#include "cat_shell.h"
#include "cat_stdio.h"

void *do_task_stat(void *arg)
{
    (void)arg;
    struct _cat_task_info tmp_info = {0};
    uint32_t i = 0;

    //CAT_SYS_PRINTF("no\tname\tstrategy\tdelay\tprio\tstate\tstack\r\n");

    for(i=0; i<cat_task_stat_cnt; i++)
    {
        cat_task_get_info(cat_task_stat_list[i], &tmp_info);
        CAT_SYS_PRINTF("%d: name=%s, strategy=%d, delay=%d, state=%d, prio=%d, schedtimes=%d, stack(free/total):%d/%d\r\n", 
            i,
            // tmp_info.task_name,
            // tmp_info.sched_times,
            // tmp_info.task_delay,
            // tmp_info.prio,
            // tmp_info.state,
            // tmp_info.free_stack_size,
            // tmp_info.stack_size

            // tmp_info.sp;         
            tmp_info.task_name,  
            tmp_info.sched_strategy,
            // tmp_info.stack_start_addr;
            tmp_info.delay,      
            tmp_info.state,      
            tmp_info.prio,  
            // tmp_info.slice;      
            // tmp_info.suspend_cnt;
            tmp_info.sched_times,
            tmp_info.free_stack,
            tmp_info.stack_size
        );
    }

    return NULL;
}
CAT_DECLARE_CMD(task_stat, print all task info, do_task_stat);
#endif //#if (CATOS_ENABLE_TASK_STAT == 1)
#endif //#if (CATOS_ENABLE_SHELL == 1)

/* OTHER END */
