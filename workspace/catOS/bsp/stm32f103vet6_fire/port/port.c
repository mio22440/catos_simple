/**
 * @file port.c
 * @brief 
 * @author amoigus (648137125@qq.com)
 * @version 0.1
 * @date 2021-03-22
 * 
 * @copyright Copyright (c) 2021
 * 
 * @par 修改日志：
 * Date              Version Author      Description
 * 2021-03-22 1.0    amoigus             内容
 */

#include "cat_task.h"
#include "cat_bsp_uart.h"
#include "cat_intr.h"

#include "../../interface/port.h"


//#define STM32F103xE
#include "../Libraries/CMSIS/Device/ST/STM32F1xx/Include/stm32f1xx.h"

#define NVIC_INT_CTRL   0xE000ED04
#define NVIC_PENDSVSET  0x10000000
#define NVIC_SYSPRI2    0xE000ED22
#define NVIC_PENDSV_PRI 0x000000FF


#if defined(__GNUC__)
int __wrap_atexit(void __attribute__((unused)) (*function)(void)) {
    return -1;
}
#endif /* #if defined(__GNUC__) */

/* PRIVATE FUNCS DECL START */
static void SystemClock_Config(void);
static void cat_set_systick_period(uint32_t ms);
/* PRIVATE FUNCS DECL END */

/**
 * @brief 硬件初始化
 */
void cat_port_hardware_init(void)
{
    /* 系统时钟初始化成72 MHz */
    SystemClock_Config();

    /* 设置系统时钟中断频率为100Hz(每秒100次) */
    cat_set_systick_period(CATOS_SYSTICK_MS);

    /* 初始化串口 */
    cat_bsp_uart_init();

}

/* stm32的时钟中断处理函数 */
void SysTick_Handler(void)
{
    /* 调用自己的处理函数 */
    cat_intr_systemtick_handler();
}

/* 初始化系统时钟中断(分频器) */
void cat_set_systick_period(uint32_t ms)
{
    SysTick->LOAD = ms * SystemCoreClock / 1000;                    /* 重载计数器值 */
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
    SysTick->VAL = 0;

    /* 设定systick控制寄存器 */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |/* 设定为内核时钟FCLK */
                    SysTick_CTRL_TICKINT_Msk |/* 设定为systick计数器倒数到0时触发中断 */
                    ~SysTick_CTRL_ENABLE_Msk;/* 关闭定时器中断，若创建任务则在catos_start_sched()中开启该中断 */
}

/* 开始调度 */
void catos_start_sched(void)
{

    cat_sp_task_before_start_first();

    /* 开启定时器中断 */
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    __set_PSP(0);

    MEM8(NVIC_SYSPRI2)      = NVIC_PENDSV_PRI;
    MEM32(NVIC_INT_CTRL)    = NVIC_PENDSVSET;
}

/* 触发pendsv中断进行任务切换，pendsv中断处理函数定义在汇编port.s中 */
void cat_context_switch(void)
{
    static uint32_t cat_context_switch_times = 0;
    cat_context_switch_times++;
    MEM32(NVIC_INT_CTRL)    = NVIC_PENDSVSET;/* pendsv的优先级在开始第一个任务时已经设置 */
}

/* 关中断方式进入临界区 */
uint32_t cat_enter_critical(void)
{
    uint32_t primask = __get_PRIMASK();/* 读取中断配置 */
    __disable_irq();
    return primask;
}

/* 开中断方式出临界区 */
void cat_exit_critical(uint32_t status)
{
    __set_PRIMASK(status);
}

/* PRIVATE FUNCS DEF START */
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 72000000
  *            HCLK(Hz)                       = 72000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            HSE PREDIV1                    = 1
  *            PLLMUL                         = 9
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef clkinitstruct = {0};
  RCC_OscInitTypeDef oscinitstruct = {0};
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
  oscinitstruct.HSEState        = RCC_HSE_ON;
  oscinitstruct.HSEPredivValue  = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState    = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
  oscinitstruct.PLL.PLLMUL      = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&oscinitstruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
}
/* PRIVATE FUNCS DEF END */

