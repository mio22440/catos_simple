/**
 * @file cat_config.h
 * @brief the config MACROs
 * @author amoigus (648137125@qq.com)
 * @version 0.1
 * @date 2021-03-19
 * 
 * @copyright Copyright (c) 2021
 * 
 * @par 修改日志：
 * Date              Version Author      Description
 * 2021-03-19 1.0    amoigus             内容
 */

#ifndef CATOS_CONFIG_H
#define CATOS_CONFIG_H



/** 系统相关 **/
#define CATOS_SYSTICK_FRQ           (1000)   /**< 系统时钟频率，每秒的tick数(周期的倒数)*/
#define CATOS_SYSTICK_MS            ((1000 * 1) / CATOS_SYSTICK_FRQ) /**< 每个tick经过的毫秒数*/

/** 任务相关 **/
#define CATOS_MAX_TASK_PRIO         (32)    /**< 任务最大优先级 */
#define CATOS_MAX_SLICE             (10)    /**< 最大时间片 */

#define CATOS_IDLE_STACK_SIZE       (1024)  /**< 空闲任务栈大小*/

/** component **/
/* sysprint */
#define CATOS_ENABLE_SYS_PRINTF          (1)     /**< 系统输出 */
#if (CATOS_ENABLE_SYS_PRINTF == 1)
    #define CATOS_ENABLE_DEBUG_PRINTF    (1)     /**< 调试打印功能 */
#endif

/* catshell */
#define CATOS_ENABLE_CAT_SHELL           (1)     /**< 启用shell */


#endif
