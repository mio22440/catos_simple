/**
 * @file cat_drv_uart.c
 * @brief uart串口驱动程序
 * @author mio (648137125@qq.com)
 * @version 1.0
 * @date 2022-07-31
 * Change Logs:
 * Date           Author        Notes
 * 2022-07-31     mio     first verion
 * 
 */

#include "cat_drv_uart.h"
#include "cat_device.h"
#include "port.h"

#include <stdio.h>

struct _cat_linux_uart_config_t
{
    const uint8_t  *name;
    uint32_t        aval_mode;

    uint32_t        baudrate;
};

//串口1波特率
#define USART_1_BAUDRATE 115200

//名称
#define USART_1_NAME                    USART1


/* 公共函数 */
uint8_t uart_init(cat_device_t*dev)
{
    uint8_t ret = CAT_EOK;

    return ret;
}

uint32_t uart_read(cat_device_t*dev, int32_t pos, void *buffer, uint32_t size)
{
    (void)pos;
    
    uint32_t i = 0;
    for(i=0; i<size; i++)
    {
        ((uint8_t *)buffer)[i] = getchar();
    }
    return i;
}
uint32_t uart_write(cat_device_t*dev, int32_t pos, const void *buffer, uint32_t size)
{
    (void)pos;
    uint32_t i = 0;
    for(i=0; i<size; i++)
    {
        putchar(((uint8_t *)buffer)[i]);
    }
    return i;
}
uint8_t uart_ctrl(cat_device_t*dev, int cmd, void *args)
{
    return CAT_EOK;
}

/* uart1 */
#define UART1_CONFIG \
{ \
    .name               = (const uint8_t *)"linux_uart1", \
    .aval_mode          = CAT_DEVICE_MODE_RDWR, \
    \
    .baudrate           = 115200, \
}

struct _cat_linux_uart_config_t uart1_cfg_data = UART1_CONFIG;

cat_device_t uart1_dev = {
    .type = CAT_DEVICE_TYPE_CHAR,
    .init = uart_init,
    .open = NULL,
    .close = NULL,
    .read = uart_read,
    .write = uart_write,
    .ctrl = uart_ctrl,

    .pri_data = NULL
};

/* 挂载所有uart设备 */
uint8_t cat_drv_uart_register(void)
{
    uint8_t err = CAT_EOK;

    err = cat_device_register(
            &uart1_dev,
            uart1_cfg_data.name,
            uart1_cfg_data.aval_mode
            );

    if(CAT_EOK != err)
    {
        while(1);
    }

    return err;
}
