#    thumb
#	preserve8
#	area    |.text|, code, readonly
    .cpu cortex-m3
    .syntax unified
    .thumb
    .text


    .global  PendSV_Handler

    .extern  cat_sp_cur_task
    .extern  cat_sp_next_task

    .type PendSV_Handler, %function
PendSV_Handler:
    mrs r0, psp                     @保存当前任务堆栈到r0
                                    @用户级堆栈psp， 特权级堆栈msp

    cbz r0, PendSV_Handler_nosave   @若 psp==0 ,跳转到第一个任务

    stmfd r0!, {r4-r11}             @保存 r4-r11（减前/递减满堆栈）
    
    ldr r1, =cat_sp_cur_task       @记录最后的指针到任务栈curstk->stack
    ldr r1, [r1]
    str r0, [r1]
		@nop
		
PendSV_Handler_nosave:               
    ldr r0, =cat_sp_cur_task       @curtask = rdytask
    ldr r1, =cat_sp_next_task         
    ldr r2, [r1]
    str r2, [r0]

    ldr r0, [r2]                    @恢复新任务的寄存器
    ldmfd r0!, {r4-r11}

    msr psp, r0                     @从r0恢复下一任务的堆栈指针
    orr lr, lr, #0x04               @设置退出异常后使用psp堆栈
    bx lr                           @退出异常，自动从堆栈恢复部分寄存器
		@nop
		
#		end
    .end

