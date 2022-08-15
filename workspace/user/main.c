
#include "catos.h"
#ifdef CATOS_BOARD_IS_STM32F103VET6_FIRE
    #include "bsp_board_led.h"
    #include "key.h"
#endif /* #ifdef CATOS_BOARD_IS_STM32F103VET6_FIRE */

#define TASK1_STACK_SIZE 512

struct _cat_task_t task1;
// struct _cat_task_t task2;


cat_task_stack_unit_t task1_env[TASK1_STACK_SIZE];
// uint32_t task2_env[1024];

uint32_t sched_task1_times = 0;
// uint32_t sched_task2_times = 0;

int do_test_device(void);

void task1_entry(void *arg)
{

    for(;;)
    {
        sched_task1_times++;
        //printf("task1\n");
#ifdef CATOS_BOARD_IS_STM32F103VET6_FIRE
	    board_led_on();
#endif /* #ifdef CATOS_BOARD_IS_STM32F103VET6_FIRE */
        cat_sp_task_delay(100);
#ifdef CATOS_BOARD_IS_STM32F103VET6_FIRE
		board_led_off();
#endif /* #ifdef CATOS_BOARD_IS_STM32F103VET6_FIRE */
        cat_sp_task_delay(100);
    }
}

void task2_entry(void *arg)
{
    for(;;)
    {
        cat_sp_task_delay(100);
        // CAT_DEBUG_PRINTF("[task2] %d\r\n", catos_systicks);
    }
}



int main(void)
{
#ifdef CATOS_BOARD_IS_STM32F103VET6_FIRE
	board_led_init();
    // EXTI_Key_Config();
#endif /* #ifdef CATOS_BOARD_IS_STM32F103VET6_FIRE */

#ifdef CATOS_BOARD_IS_LINUX
    cat_linux_pre_init();
#endif /* #ifdef CATOS_BOARD_IS_LINUX */
#if 1
    /* 测试创建任务运行 */
    cat_sp_task_create(
      (const uint8_t *)"task1_task",
      &task1,
      task1_entry,
      NULL,
      0,
      task1_env,
      TASK1_STACK_SIZE
    );

    // cat_sp_task_create(
    //   (const uint8_t *)"task2_task",
    //   &task2,
    //   task2_entry,
    //   NULL,
    //   0,
    //   task2_env,
    //   sizeof(task2_env)
    // );

    catos_start_sched();
#else
    /* 测试不创建任务下运行 */
    uint32_t i = 0;

    while(i++ < 0xffff);
    board_led_on();

    while(i-- > 0xd);
    board_led_off();
#endif

    return 0;
}
