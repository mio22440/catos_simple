
#include "catos.h"

//#include "app.h"

#include "cat_bsp_uart.h"
#include "bsp_board_led.h"

#include "key.h"


struct _cat_task_t task1;
struct _cat_task_t task2;


cat_stack_type_t task1_env[1024];
cat_stack_type_t task2_env[1024];

uint32_t sched_task1_times = 0;
uint32_t sched_task2_times = 0;

int do_test_device(void);

void task1_entry(void *arg)
{
#if (CATOS_ENABLE_DEVICE_MODEL == 1)
    do_test_device();
#endif
    for(;;)
    {
      sched_task1_times++;
		  //board_led_on();
      cat_sp_task_delay(100);
		  //board_led_off();
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
	  board_led_init();
    EXTI_Key_Config();

#if 1
    /* 测试创建任务运行 */
    cat_sp_task_create(
      (const uint8_t *)"task1_task",
      &task1,
      task1_entry,
      NULL,
      0,
      task1_env,
      sizeof(task1_env)
    );

    cat_sp_task_create(
      (const uint8_t *)"task2_task",
      &task2,
      task2_entry,
      NULL,
      0,
      task2_env,
      sizeof(task2_env)
    );

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
