/**
 * @file cat_basic_task.c
 * @author mio (648137125@qq.com)
 * @brief 基础任务定义，仅作为其他任务调度器的基础被使用，用户不使用
 * @version 0.1
 * @date 2022-07-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <string.h>
#include "cat_basic_task.h"

void cat_task_init(
    const uint8_t *task_name,
    struct _cat_task_t *task, 
    void (*entry)(void *), 
    void *arg, 
    uint8_t prio, 
    cat_stack_type_t *stack_start_addr,
    uint32_t stack_size,
    uint32_t sched_strategy
)
{
    uint32_t *stack_top;

    task->stack_start_addr = stack_start_addr;
    task->stack_size = stack_size;
    
    memset(stack_start_addr, 0, stack_size);

    stack_top = stack_start_addr + (stack_size / sizeof(cat_stack_type_t));

    //pensv自动保存的部分
    *(--stack_top) = (uint32_t)(1 << 24);//spsr
    *(--stack_top) = (uint32_t)entry;//pc
    *(--stack_top) = (uint32_t)0x14;//lr(r14)
    *(--stack_top) = (uint32_t)0x12;//r12
    *(--stack_top) = (uint32_t)0x3;//r3
    *(--stack_top) = (uint32_t)0x2;//r2
    *(--stack_top) = (uint32_t)0x1;//r1
    *(--stack_top) = (uint32_t)arg;

    *(--stack_top) = (uint32_t)0x11;
    *(--stack_top) = (uint32_t)0x10;
    *(--stack_top) = (uint32_t)0x9;
    *(--stack_top) = (uint32_t)0x8;
    *(--stack_top) = (uint32_t)0x7;
    *(--stack_top) = (uint32_t)0x6;
    *(--stack_top) = (uint32_t)0x5;
    *(--stack_top) = (uint32_t)0x4;

    task->stack_top = stack_top;
    task->delay = 0;
    task->prio = prio;
    task->state = CATOS_TASK_STATE_RDY;
    task->slice = CATOS_MAX_SLICE;
    task->suspend_cnt = 0;

    task->task_name = (uint8_t *)task_name;
    task->sched_times = 0;

    task->sched_strategy = sched_strategy;

    cat_list_node_init(&(task->link_node));
    
}

