
#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <sys/times.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
// #include <sys/syscall.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>

#include "catos_config.h"
#include "catos_types.h"
#include "port.h"
#include "cat_task.h"
#include "cat_error.h"

#define CAT_LINUX_MAX_TASK_THREAD 5

#define SIG_SUSPEND     SIGUSR1
#define SIG_RESUME      SIGUSR2
#define SIG_TICK        SIGALRM
#define TIMER_TYPE      ITIMER_REAL

/**< 任务-线程描述块 */
typedef struct _cat_linux_task_thread_t
{
    pthread_t        thread;
    int              is_valid;    /* Treated as a boolean */
    void            *task;
    unsigned int     disable_irq_nesting_cnt;
    void            *entry;
    void            *arg;
    int              index;
}_cltt_thread_t;

#if 0
/**< 线程集合 */
static _cltt_thread_t cat_linux_task_thread[CAT_LINUX_MAX_TASK_THREAD];
/**< 只执行一次的初始化函数 */
static pthread_once_t cat_linux_sig_setup_once = PTHREAD_ONCE_INIT;

/**< 阻塞和唤醒 */
static pthread_mutex_t cat_linux_suspend_resume_mutex = PTHREAD_MUTEX_INITIALIZER;
/**< 模拟单核，即只能有一个线程同时运行 */
static pthread_mutex_t cat_linux_single_thread_mutex = PTHREAD_MUTEX_INITIALIZER;

/**< 主线程 */
static pthread_t cat_linux_main_thread;
/**< 调用结束调度器的线程 */
static pthread_t cat_linux_end_scheduler_caller_thread;
/**< 调用结束调度器的线程位置 */
int cat_linux_end_scheduler_caller_thread_index;
/**< 要阻塞的线程(临时变量) */
static pthread_t cat_linux_thread_to_suspend;
/**< 要恢复的线程(临时变量) */
static pthread_t cat_linux_thread_to_resume;

/**< 开中断标志 */
static volatile unsigned int cat_linux_is_irq_enabled = 0;
/**< 停止调度器 */
static volatile unsigned int cat_linux_is_scheduler_end = 0;
/**< 正在执行时钟中断 */
static volatile unsigned int cat_linux_is_tick_servicing = 0;
/**< 新线程在数组中的位置 */
static volatile unsigned int cat_linux_new_thead_index = 0;
/**< 接下来的操作涉及切换任务，故该线程完成切换操作后忙等到切换回来 */
static volatile unsigned int cat_linux_wait_until_back = 0;
/**< 中断层数 */
static volatile unsigned int cat_linux_irq_nesting_cnt = 0;
/**< 因为关中断而错过了时钟中断 */
static volatile unsigned int cat_linux_tick_missed = 0;
#else
/**< 线程集合 */
_cltt_thread_t cat_linux_task_thread[CAT_LINUX_MAX_TASK_THREAD];
/**< 只执行一次的初始化函数 */
pthread_once_t cat_linux_sig_setup_once = PTHREAD_ONCE_INIT;

/**< 阻塞和唤醒 */
pthread_mutex_t cat_linux_suspend_resume_mutex = PTHREAD_MUTEX_INITIALIZER;
/**< 模拟单核，即只能有一个线程同时运行 */
pthread_mutex_t cat_linux_single_thread_mutex = PTHREAD_MUTEX_INITIALIZER;

/**< 主线程 */
pthread_t cat_linux_main_thread;
/**< 调用结束调度器的线程 */
pthread_t cat_linux_end_scheduler_caller_thread;
/**< 调用结束调度器的线程位置 */
int cat_linux_end_scheduler_caller_thread_index;
/**< 要阻塞的线程(临时变量) */
pthread_t cat_linux_thread_to_suspend;
/**< 要恢复的线程(临时变量) */
pthread_t cat_linux_thread_to_resume;

/**< 开中断标志 */
volatile unsigned int cat_linux_is_irq_enabled = 0;
/**< 停止调度器 */
volatile unsigned int cat_linux_is_scheduler_end = 0;
/**< 正在执行时钟中断 */
volatile unsigned int cat_linux_is_tick_servicing = 0;
/**< 新线程在数组中的位置 */
volatile unsigned int cat_linux_new_thead_index = 0;
/**< 接下来的操作涉及切换任务，故该线程完成切换操作后忙等到切换回来 */
volatile unsigned int cat_linux_wait_until_back = 0;
/**< 中断层数 */
volatile unsigned int cat_linux_irq_nesting_cnt = 0;
/**< 因为关中断而错过了时钟中断 */
volatile unsigned int cat_linux_tick_missed = 0;
#endif

static void _cat_linux_suspend_thread(pthread_t thread);

static void _cat_linux_resume_signal_handler(int sig);
static void _cat_linux_resume_thread(pthread_t thread);

static void _cat_linux_thread_exit_cleanup(void *t);
static void *_cat_linux_task_entry_wrapper(void *t);
static void _cat_linux_sigaction_setup(void);
static void _cat_linux_setup_timer_interrupt(void);
static void _cat_linux_recored_task_into_cltt(struct _cat_task_t *task);
static  int _cat_linux_lookup_thread(struct _cat_task_t *task, pthread_t *thread);
static void _cat_linux_end_scheduler(void);
static void _cat_linux_suspend_signal_handler(int sig);
static void _cat_linux_tick_signal_handler(int sig);
static void _cat_linux_redeal_missed_tick(void);

static void _cat_linux_catos_init(void);


static void _cat_linux_suspend_thread(pthread_t thread)
{
    pthread_mutex_lock(&cat_linux_suspend_resume_mutex);
    cat_linux_wait_until_back = 0;
    pthread_mutex_unlock(&cat_linux_suspend_resume_mutex);

    pthread_kill(thread, SIG_SUSPEND);
    
    while(
        (0 == cat_linux_wait_until_back) &&
        (1 != cat_linux_is_tick_servicing)
    )
    {
        sched_yield();
    }
}

static void _cat_linux_resume_signal_handler(int sig)
{
    (void)sig;

    if(0 == pthread_mutex_lock(&cat_linux_single_thread_mutex))
    {
        pthread_mutex_unlock(&cat_linux_single_thread_mutex);
    }
}

static void _cat_linux_resume_thread(pthread_t thread)
{
    int result = 0;

    result = pthread_mutex_lock(&cat_linux_suspend_resume_mutex);
    CAT_ASSERT(0 == result);

    /* 如果当前线程是目标线程则恢复线程 */
    if(0 == pthread_equal(pthread_self(), thread))
    {
        pthread_kill(thread, SIG_RESUME);
    }

    pthread_mutex_unlock(&cat_linux_suspend_resume_mutex);
}

static void _cat_linux_thread_exit_cleanup(void *t)
{
    _cltt_thread_t *cltt_thread = (_cltt_thread_t *)t;

    cltt_thread->is_valid = 0;
    cltt_thread->task     = NULL;

    if(cltt_thread->disable_irq_nesting_cnt > 0)
    {
        cltt_thread->disable_irq_nesting_cnt = 0;
        cat_linux_irq_nesting_cnt = 0;
        cat_hw_irq_enable(0);
    }
}

/**
 * @brief 
 * 
 * @param t (_cltt_thread_t *)任务线程的描述块指针
 * @return void* 
 */
static void *_cat_linux_task_entry_wrapper(void *t)
{
    _cltt_thread_t *cltt_thread = (_cltt_thread_t *)t;

    /* 允许线程取消 */
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* 设置线程退出清理函数 */
    pthread_cleanup_push(_cat_linux_thread_exit_cleanup, cltt_thread);

    pthread_mutex_lock(&cat_linux_single_thread_mutex);

    /* 阻塞等待开始 */
    _cat_linux_suspend_thread(pthread_self());

    /* 调用入口函数 */
    void (*f)(void *) = cltt_thread->entry;
    f(cltt_thread->arg);

    pthread_cleanup_pop( 1 );

    return NULL;
}

/* 初始化信号设置 */
static void _cat_linux_sigaction_setup(void)
{
    int result = 0;
    int i;
    struct sigaction sigact_suspend_self;
    struct sigaction sigact_resume;
    struct sigaction sigact_tick;

    memset(cat_linux_task_thread, 0, sizeof(cat_linux_task_thread));

    for(i = 0; i < CAT_LINUX_MAX_TASK_THREAD; i++)
    {
        cat_linux_task_thread[i].index = i;
    }

    /* 初始化信号 */
    sigact_suspend_self.sa_flags = 0;
    sigact_suspend_self.sa_handler = _cat_linux_suspend_signal_handler;
    sigfillset(&(sigact_suspend_self.sa_mask));

    sigact_resume.sa_flags = 0;
    sigact_resume.sa_handler = _cat_linux_resume_signal_handler;
    sigfillset(&(sigact_resume.sa_mask));

    sigact_tick.sa_flags = 0;
    sigact_tick.sa_handler = _cat_linux_tick_signal_handler;
    sigfillset(&(sigact_tick.sa_mask));

    /* 安装信号处理函数 */
    result = sigaction( SIG_SUSPEND, &sigact_suspend_self, NULL );
    CAT_ASSERT(0 == result);
    result = sigaction( SIG_RESUME, &sigact_resume, NULL );
    CAT_ASSERT(0 == result);
    result = sigaction( SIG_TICK, &sigact_tick, NULL );
    CAT_ASSERT(0 == result);

    _cat_linux_catos_init();

    /* 此线程作为主线程 */
    cat_linux_main_thread = pthread_self();
}

static void _cat_linux_setup_timer_interrupt(void)
{
    int result = 0;
    struct itimerval itimer, oitimer;
    suseconds_t micro_seconds = (suseconds_t)((CATOS_SYSTICK_MS * 1000) % 1000000);
    time_t      seconds       = CATOS_SYSTICK_MS / 1000;

    result = getitimer(TIMER_TYPE, &itimer);
    CAT_ASSERT(0 == result);

    /* 设置时钟事件间隔(tick中断间隔) */
    itimer.it_interval.tv_sec  = seconds;
    itimer.it_interval.tv_usec = micro_seconds;

    /* 设置当前计数值 */
    itimer.it_value.tv_sec     = seconds;
    itimer.it_value.tv_usec    = micro_seconds;

    result = setitimer(TIMER_TYPE, &itimer, &oitimer);
    CAT_ASSERT(0 == result);
}

static void _cat_linux_recored_task_into_cltt(struct _cat_task_t *task)
{
    int i;

    cat_linux_task_thread[cat_linux_new_thead_index].task = task;

    for (i = 0; i < CAT_LINUX_MAX_TASK_THREAD; i++)
    {
        if ( pthread_equal(cat_linux_task_thread[i].thread, cat_linux_task_thread[cat_linux_new_thead_index].thread))
        {
            if ( cat_linux_task_thread[i].task != cat_linux_task_thread[cat_linux_new_thead_index].task )
            {
                cat_linux_task_thread[i].is_valid = 0;
                cat_linux_task_thread[i].task = NULL;
                cat_linux_task_thread[i].disable_irq_nesting_cnt = 0;
            }
        }
    }
}

static  int _cat_linux_lookup_thread(struct _cat_task_t *task, pthread_t *thread)
{
    int ret = CAT_ERROR;
    int i;

    for(i = 0; i < CAT_LINUX_MAX_TASK_THREAD; i++)
    {
        if(cat_linux_task_thread[i].task == task)
        {
            *thread = cat_linux_task_thread[i].thread;
            ret = CAT_EOK;
        }
    }

    return ret;
}

static void _cat_linux_end_scheduler(void)
{
    int i;
    int result;
    struct sigaction sigtickdeinit;

    cat_linux_is_irq_enabled = 0;

    /* Signal the scheduler to exit its loop. */
    cat_linux_is_scheduler_end = 0;

    /* Ignore next or pending SIG_TICK, it mustn't execute anymore. */
    sigtickdeinit.sa_flags = 0;
    sigtickdeinit.sa_handler = SIG_IGN;
    sigfillset(&sigtickdeinit.sa_mask);

    result = sigaction(SIG_TICK, &sigtickdeinit, NULL);
    CAT_ASSERT(result == 0);

    result = sigaction(SIG_RESUME, &sigtickdeinit, NULL);
    CAT_ASSERT(result == 0);

    result = sigaction(SIG_SUSPEND, &sigtickdeinit, NULL);
    CAT_ASSERT(result == 0);


    for (i = 0; i < CAT_LINUX_MAX_TASK_THREAD; i++)
    {
        if ( cat_linux_task_thread[i].is_valid )
        {
            //pxThreads[i].Valid = 0;

            /* Don't kill yourself */
            if (0 != pthread_equal(cat_linux_task_thread[i].thread, pthread_self()))
            {
                cat_linux_end_scheduler_caller_thread = cat_linux_task_thread[i].thread;
                cat_linux_end_scheduler_caller_thread_index = cat_linux_task_thread[i].index;
                continue;
            }
            else
            {
                /* Kill all of the threads, they are in the detached state. */
                pthread_cancel(cat_linux_end_scheduler_caller_thread);
                sleep(1);
            }
        }
    }

    pthread_kill(cat_linux_main_thread, SIG_RESUME);
}

static void _cat_linux_suspend_signal_handler(int sig)
{
    sigset_t signals;
    int result;

    sigemptyset(&signals);
    sigaddset(&signals, SIG_RESUME);
    cat_linux_wait_until_back = 1;

    result = pthread_mutex_unlock(&cat_linux_single_thread_mutex);
    CAT_ASSERT(0 == result);

    result = sigwait(&signals, &sig);
    CAT_ASSERT(0 == result);

    if(0 == cat_linux_irq_nesting_cnt)
    {
        cat_hw_irq_enable(0);
    }
    else
    {
        (void)cat_hw_irq_disable();
    }

}

static void _cat_linux_tick_signal_handler(int sig)
{
    int result;

    (void)sig;

    if (
        (1 == cat_linux_is_irq_enabled) && 
        (1 != cat_linux_is_tick_servicing)
    )
    {
        if (0 == pthread_mutex_trylock(&cat_linux_single_thread_mutex))
        {
            cat_linux_is_tick_servicing = 1;

            //xTaskIncrementTick();

            /* Select Next Task. */
#if ( configUSE_PREEMPTION == 1 )
            vTaskSwitchContext();
#endif
            cat_intr_systemtick_handler();


            /* The only thread that can process this tick is the running thread. */
            // if (!pthread_equal(thread_to_suspend, thread_to_resume))
            // {
            //     /* Remember and switch the critical nesting. */
            //     //prvSetTaskCriticalNesting( ThreadToSuspend, uxCriticalNesting );
            //     //uxCriticalNesting = prvGetTaskCriticalNesting( ThreadToResume );
            //     /* Resume next task. */
            //     _cat_linux_resume_thread(thread_to_resume);
            //     /* Suspend the current task. */
            //     _cat_linux_suspend_thread(thread_to_suspend);
            // }
            // else
            if (0 != pthread_equal(cat_linux_thread_to_suspend, cat_linux_thread_to_resume))
            {
                /* Release the lock as we are Resuming. */
                pthread_mutex_unlock(&cat_linux_single_thread_mutex);
            }
            cat_linux_is_tick_servicing = 0;
        }
        else
        {
            cat_linux_tick_missed = 1;
        }
    }
    else
    {
        cat_linux_tick_missed = 1;
    }
}

static void _cat_linux_redeal_missed_tick(void)
{
    int result;
    // pthread_t thread_to_suspend;
    // pthread_t thread_to_resume;

    result = pthread_mutex_lock(&cat_linux_single_thread_mutex);
    CAT_ASSERT(0 == result);

    result = _cat_linux_lookup_thread(cat_sp_cur_task, &cat_linux_thread_to_suspend);
    CAT_ASSERT(0 == result);

    cat_intr_systemtick_handler();

    result = _cat_linux_lookup_thread(cat_sp_cur_task, &cat_linux_thread_to_resume);
    CAT_ASSERT(0 == result);

    if (!pthread_equal(cat_linux_thread_to_suspend, cat_linux_thread_to_resume))
    {
        /* Remember and switch the critical nesting. */
        //prvSetTaskCriticalNesting( thread_to_suspend, cat_linux_irq_nesting_cnt );
        //uxCriticalNesting = prvGetTaskCriticalNesting( thread_to_resume );
        /* Switch tasks. */
        _cat_linux_resume_thread(cat_linux_thread_to_resume);
        _cat_linux_suspend_thread(cat_linux_thread_to_suspend);
    }
    else
    {
        /* Yielding to self */
        pthread_mutex_unlock(&cat_linux_single_thread_mutex);
    }
}

#include "cat_device.h"
#include "cat_shell.h"
#include "cat_stdio.h"

#include "cat_task.h"
#include "cat_idle.h"


#include "uart/cat_drv_uart.h"
static void _cat_linux_catos_init(void)
{
    /* 板级硬件初始化 */
    cat_hw_init();

    /* 初始化设备框架 */
    cat_device_module_init();

    /* 注册串口 */
    cat_drv_uart_register();

    /* 设置标准输入输出使用的串口 */
    cat_stdio_set_device((uint8_t *)"linux_uart1");
    
    /* 固定优先级调度初始化 */
    cat_sp_task_scheduler_init();
    
    /* 初始化系统计数器 */
    cat_systick_init();

    /* 创建空闲任务 */
    cat_idle_task_create();

    /* 创建shell任务 */
    cat_shell_task_create();

    /* 禁止调度，若用户调用catos_hw_start_sched()，则在其中打开调度锁 */
    cat_sp_task_sched_disable();
}

/**
 * @brief 硬件初始化
 */
void cat_hw_init(void)
{

}

/**
 * @brief 开始调度
 * 
 */
void catos_start_sched(void)
{
    int      isignal;
    sigset_t signals;
    sigset_t signal_to_block;
    sigset_t signals_blocked;
    pthread_t first_thread;
    int result;

    sigfillset(&signal_to_block);
    pthread_sigmask(SIG_SETMASK, &signal_to_block, &signals_blocked);

    /* 初始化时钟事件 */
    _cat_linux_setup_timer_interrupt();

    /* 初始化中断层数 */
    cat_linux_irq_nesting_cnt = 0;
    cat_hw_irq_enable(0);

    cat_sp_task_before_start_first();
    result = _cat_linux_lookup_thread(cat_sp_next_task, &first_thread);
    CAT_ASSERT(0 == result);

    cat_sp_cur_task = cat_sp_next_task;

    /* 恢复第一个线程的运行 */
    _cat_linux_resume_thread(first_thread);

    /* 等待终止信号到来，以便退出线程 */
    sigemptyset(&signals);
    sigaddset(&signals, SIG_RESUME);

    while(1 != cat_linux_is_scheduler_end)
    {
        if(0 != sigwait(&signals, &isignal))
        {
            //CAT_SYS_PRINTF("main thread recieved signal: %d\n", isignal);
        }
    }

    /* 结束调用关闭调度器的线程 */
    pthread_cancel(cat_linux_end_scheduler_caller_thread);
    sleep(1);

    pthread_mutex_destroy(&cat_linux_suspend_resume_mutex);
    pthread_mutex_destroy(&cat_linux_single_thread_mutex);
    sleep(1);

    /* 不应该到达这里 */
    while(1);
}

/**
 * @brief 上下文切换
 * 
 */
void cat_hw_context_switch(void)
{
    int result;

    result = _cat_linux_lookup_thread(cat_sp_cur_task, &cat_linux_thread_to_suspend);
    CAT_ASSERT(0 == result);

    cat_sp_cur_task = cat_sp_next_task;

    result = _cat_linux_lookup_thread(cat_sp_cur_task, &cat_linux_thread_to_resume);
    CAT_ASSERT(0 == result);

    if (!pthread_equal(cat_linux_thread_to_suspend, cat_linux_thread_to_resume))
    {
        /* Remember and switch the critical nesting. */
        //prvSetTaskCriticalNesting( ThreadToSuspend, uxCriticalNesting );
        //uxCriticalNesting = prvGetTaskCriticalNesting( ThreadToResume );
        /* Resume next task. */
        _cat_linux_resume_thread(cat_linux_thread_to_resume);
        /* Suspend the current task. */
        _cat_linux_suspend_thread(cat_linux_thread_to_suspend);
    }

}

/**
 * @brief 关中断进临界区
 * 
 * @return uint32_t 
 */
uint32_t cat_hw_irq_disable(void)
{
    uint32_t status = cat_linux_is_irq_enabled & 0xffffffff;

    cat_linux_irq_nesting_cnt++;
    cat_linux_is_irq_enabled = 0;
    

    return status;
}

/**
 * @brief 开中断出临界区
 * 
 * @param status 
 */
void cat_hw_irq_enable(uint32_t status)
{
    (void)status;
    if(cat_linux_irq_nesting_cnt > 0)
    {
        cat_linux_irq_nesting_cnt--;
    }

    if(0 == cat_linux_irq_nesting_cnt)
    {
        if(1 == cat_linux_tick_missed)
        {
            cat_linux_tick_missed = 0;
            _cat_linux_redeal_missed_tick();
        }
        cat_linux_is_irq_enabled = 1;
    }
    
}

/**
 * @brief 栈初始化
 * 
 * @param task_entry    任务入口函数地址
 * @param parameter     参数
 * @param stack_addr    栈起始地址
 * @param exit          任务退出函数地址
 * @return uint8_t*     初始化后的栈顶地址
 */
uint8_t *cat_hw_stack_init(void *task_entry, void *parameter, uint8_t *stack_addr, void *exit)
{
    // (void)exit;
    struct _cat_task_t *task = (struct _cat_task_t *)exit;

    int result = 0;
    int i = 0;
    pthread_attr_t thread_attributes;
    cpu_set_t cpuset;
    
    /* 保证该初始化函数只执行一次 */
    // pthread_once(&cat_linux_sig_setup_once, _cat_linux_sigaction_setup);

    pthread_attr_init(&thread_attributes);
    pthread_attr_setdetachstate(&thread_attributes, PTHREAD_CREATE_DETACHED);

    (void)cat_hw_irq_disable();

    /* 寻找合适的位置放新线程 */
    for(i = 0; i < CAT_LINUX_MAX_TASK_THREAD; i++)
    {
        if(cat_linux_task_thread[i].is_valid == 0)
        {
            cat_linux_new_thead_index = i;
            break;
        }
    }

    CAT_ASSERT(i < CAT_LINUX_MAX_TASK_THREAD);

    cat_linux_task_thread[cat_linux_new_thead_index].entry = task_entry;
    cat_linux_task_thread[cat_linux_new_thead_index].arg   = parameter;

    _cat_linux_recored_task_into_cltt(task);

    /* 模拟单核 */
    result = pthread_mutex_lock(&cat_linux_single_thread_mutex);
    CAT_ASSERT(0 == result);

    /* 创建完线程后等待任务切换回来 */
    cat_linux_wait_until_back = 0;

    /* 创建线程 */
    result = pthread_create(
        &(cat_linux_task_thread[cat_linux_new_thead_index].thread),
        &thread_attributes,
        _cat_linux_task_entry_wrapper,
        (void *)&(cat_linux_task_thread[cat_linux_new_thead_index])
    );
    CAT_ASSERT(0 == result);

    /* 保证线程都在同一个核上运行(防止并行?) */
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    result = pthread_setaffinity_np(
        cat_linux_task_thread[cat_linux_new_thead_index].thread,
        sizeof(cpu_set_t),
        &cpuset
    );
    CAT_ASSERT(0 == result);

    cat_linux_task_thread[cat_linux_new_thead_index].is_valid = 1;

    /* 让其他线程运行 */
    pthread_mutex_unlock(&cat_linux_single_thread_mutex);

    while(0 == cat_linux_wait_until_back);
    cat_hw_irq_enable(0);

    return stack_addr;
}

void cat_linux_pre_init(void)
{
    pthread_once(&cat_linux_sig_setup_once, _cat_linux_sigaction_setup);
}

