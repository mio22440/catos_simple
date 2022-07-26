/**
 * @file cat_init.c
 * @author mio (648137125@qq.com)
 * @brief 初始化
 * @version 0.1
 * @date 2022-07-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "port.h"
#include "cat_stdio.h"
#include "cat_task.h"
#include "cat_intr.h"
#include "cat_idle.h"

#include "cat_init.h"

extern void main(void);

/**
 * @brief 系统初始化
 */
void catos_init(void)
{
    /* 板级硬件初始化 */
    cat_port_hardware_init();
    
    /* 固定优先级调度初始化 */
    cat_sp_task_scheduler_init();
    
    /* 初始化系统计数器 */
    cat_systick_init();

    /* 创建空闲任务 */
    cat_idle_task_create();

    /* 禁止调度，若用户调用catos_start_sched()，则在其中打开调度锁 */
    cat_sp_task_sched_disable();

    /* 跳转到用户主函数 */
    main();

    /* 若用户主函数未启动调度器则可能会返回，正常项目不应该运行到该循环中 */
    CAT_KPRINTF("user do not start the scheduler, main() returned\r\n");
    while(1)
    {

    }
    
}
