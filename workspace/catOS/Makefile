### BSP 相关 START
CINCLUDE_FILE += catOS/bsp/interface
ifeq ($(CATOS_BOARD_STM32F103VET6_FIRE), y)
CINCLUDE_FILE += catOS/bsp/stm32f103vet6_fire/drivers
endif
ifeq ($(CATOS_BOARD_LINUX), y)
CINCLUDE_FILE += catOS/bsp/linux/drivers
endif
### BSP 相关 END

CINCLUDE_FILE += catOS/include

CINCLUDE_FILE += catOS/src/component/cat_device
CINCLUDE_FILE += catOS/src/component/cat_shell
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
subdir-$(CATOS_ENABLE_TEST) += test

#VERIFY_MSG = catOS:$(RELATIVE_PATH)


include $(RULE_DIR)/rules.mk