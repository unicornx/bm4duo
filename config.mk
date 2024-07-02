OPTFLAGS := \
	-O0 \
	-Wall \
	-Werror

COMMONFLAGS := \
	-march=rv64imafdc -mcmodel=medany -mabi=lp64d

ASFLAGS := \
	$(OPTFLAGS) \
	$(COMMONFLAGS) \
	-std=gnu99

CFLAGS := \
	$(COMMONFLAGS) \
	$(OPTFLAGS) \
	-ffunction-sections \
	-fdata-sections \
	-nostdlib \
	-std=gnu99

CXXFLAGS := \
	$(COMMONFLAGS) \
	$(OPTFLAGS) \
	-ffunction-sections \
	-fdata-sections \
	-nostdlib \
	-fno-threadsafe-statics \
	-fno-rtti \
	-fno-exceptions \
	-std=c++14

LDFLAGS := \
	$(COMMONFLAGS) \
	-Wl,--build-id=none \
	-Wl,--cref \
	-Wl,--check-sections \
	-Wl,--gc-sections \
	-Wl,--unresolved-symbols=report-all \
	-nostartfiles

INC += \
	-I$(ROOT)/include \
	-I$(ROOT)/drivers/include \

C_SRC += \
	$(ROOT)/source/main.c			\
	$(ROOT)/source/system.c			\
	$(ROOT)/source/pinmux.c			\
	$(ROOT)/source/trap.c			\
	$(ROOT)/source/syscalls.c		\
	$(ROOT)/source/clint_timer.c		\
	$(ROOT)/source/uart.c			\
	$(ROOT)/source/irq.c			\

A_SRC +=

ifeq ($(DEBUG), 1)
ASFLAGS += -g
CFLAGS += -g
CXXFLAGS += -g
DEFS += -DCONFIG_ENABLE_DEBUG
endif

DEFS += -DCONFIG_C906B

ifeq (${BOARD}, duo)
  DEFS += -DCONFIG_BOARD_DUO
else ifeq (${BOARD}, duo256)
  DEFS += -DCONFIG_BOARD_DUO256
else
  $(error BOARD "${BOARD}" is unkonwn!)
endif

ifneq ($(TEST), )
DEFS += -DCONFIG_TEST_${TEST}
endif

include $(ROOT)/boot/config.mk

