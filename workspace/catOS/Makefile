
ifeq ($(CATOS_BOARD_STM32F103VET6_FIRE), y)
# CINCLUDE_FILE += Libraries/CMSIS/Include
# CINCLUDE_FILE += Libraries/CMSIS/Device/ST/STM32F1xx/Include
# CINCLUDE_FILE += Libraries/STM32F1xx_HAL_Driver/Inc

CINCLUDE_FILE += catOS/bsp/interface
CINCLUDE_FILE += catOS/bsp/stm32f103vet6_fire/drivers
endif


CINCLUDE_FILE += catOS/include

CINCLUDE_FILE += catOS/src/component/cat_device
CINCLUDE_FILE += catOS/src/component/cat_stdio
CINCLUDE_FILE += catOS/src/component/cat_string

CINCLUDE_FILE += catOS/src/core/idle_task
CINCLUDE_FILE += catOS/src/core/init
CINCLUDE_FILE += catOS/src/core/interrupt
CINCLUDE_FILE += catOS/src/core/schedule
CINCLUDE_FILE += catOS/src/core/support

CINCLUDE_FILE += user

subdir-y += bsp
subdir-y += src

#VERIFY_MSG = catOS:$(RELATIVE_PATH)


include $(RULE_DIR)/rules.mk