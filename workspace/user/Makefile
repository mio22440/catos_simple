
CINCLUDE_FILE += catOS/bsp/stm32f103vet6_fire/Libraries/CMSIS/Include
CINCLUDE_FILE += catOS/bsp/stm32f103vet6_fire/Libraries/CMSIS/Device/ST/STM32F1xx/Include
CINCLUDE_FILE += catOS/bsp/stm32f103vet6_fire/Libraries/STM32F1xx_HAL_Driver/Inc

CINCLUDE_FILE += catOS/bsp/stm32f103vet6_fire/drivers/uart
CINCLUDE_FILE += catOS/include

CINCLUDE_FILE += user
CINCLUDE_FILE += user/bsp/board_led
CINCLUDE_FILE += user/bsp/key


subdir-y += bsp

obj-y += main.o
obj-y += stm32f1xx_it.o
obj-y += system_stm32f1xx.o

include $(RULE_DIR)/rules.mk
