ifeq ($(CATOS_BOARD_STM32F103VET6_FIRE), y)
CINCLUDE_FILE += catOS/bsp/stm32f103vet6_fire/Libraries/CMSIS/Include
CINCLUDE_FILE += catOS/bsp/stm32f103vet6_fire/Libraries/CMSIS/Device/ST/STM32F1xx/Include
CINCLUDE_FILE += catOS/bsp/stm32f103vet6_fire/Libraries/STM32F1xx_HAL_Driver/Inc
CINCLUDE_FILE += catOS/bsp/stm32f103vet6_fire/drivers/uart

CINCLUDE_FILE += user
CINCLUDE_FILE += user/bsp/board_led
CINCLUDE_FILE += user/bsp/key
endif #ifeq ($(CATOS_BOARD_STM32F103VET6_FIRE), y)
ifeq ($(CATOS_BOARD_LINUX), y)
CINCLUDE_FILE += catOS/bsp/linux/drivers/uart
endif #ifeq ($(CATOS_BOARD_LINUX), y)

CINCLUDE_FILE += catOS/include

subdir-y += bsp

obj-y += main.o

ifeq ($(CATOS_BOARD_STM32F103VET6_FIRE), y)
obj-y += stm32f1xx_it.o
obj-y += system_stm32f1xx.o
endif

include $(RULE_DIR)/rules.mk
