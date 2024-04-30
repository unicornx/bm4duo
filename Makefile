ROOT := $(shell pwd)
TOOL_PATH :=  /home/u/ws/duo/sophpi/host-tools/gcc/riscv64-elf-x86_64/bin

DEBUG := 0

# duo/duo256/duoS/...
# default is duo256
BOARD     ?= duo256

CROSS_COMPILE := $(TOOL_PATH)/riscv64-unknown-elf-

# bm is the abbreviation of Bare Mental
OUTPUT_NAME := bm
OUTPUT_PATH := out
RELEASE_PATH := ./release_out

include config.mk

OUTPUT_NAME := $(OUTPUT_NAME)_$(BOARD)
C_SRC += $(wildcard $(ROOT)/source/test/*.c)

AS := $(CROSS_COMPILE)as
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
MKDIR := mkdir -p
RM := rm -rf
CP := cp -r
SSEC := *.slow*

EXT_C_OBJ := $(patsubst %,   %.o, $(EXT_C_SRC))
A_OBJ   := $(addprefix $(OUTPUT_PATH)/, $(patsubst $(ROOT)/%,   %.o, $(A_SRC)))
C_OBJ   := $(addprefix $(OUTPUT_PATH)/, $(patsubst $(ROOT)/%,   %.o, $(C_SRC)))
CXX_OBJ := $(addprefix $(OUTPUT_PATH)/, $(patsubst $(ROOT)/%,   %.o, $(CXX_SRC)))
BIN_OBJ := $(addprefix $(OUTPUT_PATH)/, $(patsubst $(ROOT)/%,   %.o, $(BIN_SRC)))

MAP	:= $(OUTPUT_PATH)/$(OUTPUT_NAME).map
ELF	:= $(OUTPUT_PATH)/$(OUTPUT_NAME).elf
BIN	:= $(OUTPUT_PATH)/$(OUTPUT_NAME).bin
ASM	:= $(OUTPUT_PATH)/$(OUTPUT_NAME).asm

CFLAGS += -g

# default definition is for duo
MONITOR_RUNADDR := 0x0000000080000000
BLCP_2ND_RUNADDR := 0x0000000083f40000
ifeq (${BOARD}, duo256)
	BLCP_2ND_RUNADDR := 0x000000008fe00000
endif

all: $(OUTPUT_PATH) $(BIN) $(ASM)

.PHONY: $(USER_PHONY) $(OUTPUT_PATH) clean release all_src all_def all_inc all_flag

$(OUTPUT_PATH):
	$(MKDIR) $(OUTPUT_PATH) $(dir $(A_OBJ)) $(dir $(C_OBJ)) $(dir $(CXX_OBJ)) $(dir $(MAP)) $(dir $(ELF)) $(dir $(BIN)) $(dir $(ASM)) $(dir $(BIN_OBJ))

clean: $(USER_CLEAN)
	$(RM) $(OUTPUT_PATH)
	$(RM) $(RELEASE_PATH)

release: all gen_fip
	$(MKDIR) $(RELEASE_PATH)
	$(CP) $(MAP) $(RELEASE_PATH)/$(OUTPUT_NAME)_$(shell date "+%F-%H-%M-%S").map
	$(CP) $(ELF) $(RELEASE_PATH)/$(OUTPUT_NAME)_$(shell date "+%F-%H-%M-%S").elf
	$(CP) $(BIN) $(RELEASE_PATH)/$(OUTPUT_NAME)_$(shell date "+%F-%H-%M-%S").bin
	$(CP) $(ASM) $(RELEASE_PATH)/$(OUTPUT_NAME)_$(shell date "+%F-%H-%M-%S").asm
	$(CP) ${OUTPUT_PATH}/fip.bin ${RELEASE_PATH}/

$(EXT_C_OBJ): %.o: %
	"$(CC)" -c $(DEFS) $(CFLAGS) $< -o $@ $(INC)

$(A_OBJ): $(OUTPUT_PATH)/%.o: $(ROOT)/%
	"$(CC)" -c $(DEFS) $(ASFLAGS) $< -o $@ $(INC)

$(C_OBJ): $(OUTPUT_PATH)/%.o: $(ROOT)/%
	"$(CC)" -c $(DEFS) $(CFLAGS) $< -o $@ $(INC)

$(CXX_OBJ): $(OUTPUT_PATH)/%.o: $(ROOT)/%
	"$(CXX)" -c $(DEFS) $(CXXFLAGS) $< -o $@ $(INC)

$(BIN_OBJ): $(OUTPUT_PATH)/%.o: $(ROOT)/%
	"$(OBJCOPY)" -I binary -O elf64-littleaarch64 --binary-architecture aarch64 $< $@
	"$(OBJCOPY)" --redefine-sym _binary_`echo $< | sed -z 's/[/.]/_/g'`_start=_binary_`echo $* | sed -z 's/[/.]/_/g'`_start $@

$(ELF): $(A_OBJ) $(EXT_C_OBJ) $(C_OBJ) $(CXX_OBJ) $(LIB_OBJ) $(BIN_OBJ)
	$(CC) -T$(LINK) -o $@ -Wl,-Map,$(MAP) $(LDFLAGS) \
	-Wl,--start-group \
	$^ \
	-lm ${_LDFLAGS} \
	-Wl,--end-group

$(BIN): $(ELF)
	$(OBJCOPY) -O binary -R $(SSEC) $^ $@


$(ASM): $(ELF)
	$(OBJDUMP) -DS $^ > $@

all_obj:
	@echo "ALL OBJECT FILES: =============================================="
	@echo "ASM OBJ:<"$(A_OBJ)">"
	@echo "EXT C OBJ:<"$(EXT_C_OBJ)">"
	@echo "C OBJ:<"$(C_OBJ)">"
	@echo "CXX OBJ:<"$(CXX_OBJ)">"

all_src:
	@echo "ALL SOURCE FILES: =============================================="
	@echo "ASM SRC:<"$(A_SRC)">"
	@echo "C SRC:<"$(C_SRC)">"
	@echo "CXX SRC:<"$(CXX_SRC)">"

all_def:
	@echo "DEFS: ======================================================="
	@echo $(DEFS)

all_inc:
	@echo "INC: ========================================================"
	@echo $(INC)

all_flag:
	@echo "ALL FLAGS: ====================================================="
	@echo "LDFLAGS:<"$(LDFLAGS)">"
	@echo "ASFLAGS:<"$(ASFLAGS)">"
	@echo "CFLAGS:<"$(CFLAGS)">"

all_config:
	@echo "ALL CONFIGS: ==================================================="
	@$(CC) -E -dM $(DEFS) $(CFLAGS) ./include/config.h | sort | grep "^#define CONFIG_"

chip_conf:
	python3 ${ROOT}/scripts/chip_conf.py ${OUTPUT_PATH}/chip_conf.bin

gen_fip: $(BIN) chip_conf
	python3 ${ROOT}/scripts/fiptool.py -v genfip \
		'${OUTPUT_PATH}/fip.bin' \
		--MONITOR_RUNADDR="${MONITOR_RUNADDR}" \
		--BLCP_2ND_RUNADDR="${BLCP_2ND_RUNADDR}" \
		--CHIP_CONF='${OUTPUT_PATH}/chip_conf.bin' \
		--NOR_INFO='FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF' \
		--NAND_INFO='00000000'\
		--BL2='${ROOT}/prebuild/${BOARD}/bl2.bin' \
		--BLCP_IMG_RUNADDR=0x05200200 \
		--BLCP_PARAM_LOADADDR=0 \
		--BLCP='${ROOT}/prebuild/empty.bin' \
		--DDR_PARAM='${ROOT}/prebuild/ddr_param.bin' \
		--BLCP_2ND='${ROOT}/prebuild/${BOARD}/rtos.bin' \
		--MONITOR='$(BIN)' \
		--LOADER_2ND='${ROOT}/prebuild/${BOARD}/u-boot-raw.bin' \
		--compress='lzma'