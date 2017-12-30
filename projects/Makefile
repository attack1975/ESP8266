# Base directory for the compiler
ifndef TOOLCHAIN_CUSTOM
    TOOLCHAIN_BASE :=$(TOOLCHAIN_BASE)/bsp/xtensa-2/
endif

SDK_CLOSED :=$(SDK_CLOSED)
SDK_BASE :=$(SDK_BASE)
TOOLCHAIN_INCLUDE ?= $(TOOLCHAIN_BASE)/include 

SDK_EXTRA_INCLUDE := $(addprefix $(SDK_BASE)/../, $(LIBS))
SDK_EXTRA_INCLUDE := $(addsuffix /include, $(SDK_EXTRA_INCLUDE))

SDK_INCLUDE ?= $(SDK_BASE)/include include $(SDK_EXTRA_INCLUDE)
export PATH := $(TOOLCHAIN_BASE)/bin:$(PATH)

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

include $(ROOT_DIR)/settings.mk


ifndef TARGET
    TARGET := out
endif


export PATH := $(TOOLCHAIN_BASE)/bin:$(PATH)

# which modules (subdirectories) of the project to include in compiling
MODULES		= $(MODULES_USER) src

# all MODULES/include will be included
EXTRA_INCDIR	= \
		$(SDK_INCLUDE) \
		$(TOOLCHAIN_INCLUDE) \
		$(EXTRA_INCLUDES) 


# libraries used in this project, mainly provided by the SDK
LIBS		:= $(CUSTOM_LIBS)  $(LIBS) 



# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.ld

# various paths from the SDK used in this project
SDK_LIBDIR_ORI	= lib_build
TOOLCHAIN_LIBDIR= lib
SDK_LDDIR	= ld
# these are the names and options to generate them
FW_FILE_1	= 0x00000
FW_FILE_1_ARGS	= -bo $@ -bs .text -bs .data -bs .rodata -bc -ec
FW_FILE_2	= 0x40000
FW_FILE_2_ARGS	= -es .irom0.text $@ -ec



####
#### no user configurable options below here
####

SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR		:= $(addprefix $(SDK_BASE)/, $(SDK_LIBDIR_ORI))
SDK_LIBDIR_EXTRA	:= $(addprefix $(SDK_BASE)/../, $(CUSTOM_LIBS) )
SDK_LIBDIR_EXTRA_L	:=  $(addsuffix /$(SDK_LIBDIR_ORI), -L$(SDK_LIBDIR_EXTRA))


SDK_CLOSED_INCDIR := $(SDK_CLOSED)/include
SDK_CLOSED_LIBDIR := $(SDK_CLOSED)/lib
TOOLCHAIN_LIBDIR	:= $(addprefix $(TOOLCHAIN_BASE)/,$(TOOLCHAIN_LIBDIR)) -L$(TOOLCHAIN_BASE)/lib/gcc/


SDK_CLOSED_INCDIR	:= $(addprefix -I,$(SDK_CLOSED_INCDIR)) 



SDK_INCDIR	:= $(addprefix -I,$(SDK_INCLUDE))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
SRCPP   := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))

OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
OBJPP	:= $(patsubst %.cpp,$(BUILD_BASE)/%.o,$(SRCPP))

LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

FW_FILE_1	:= $(addprefix $(FW_BASE)/,$(FW_FILE_1).bin)
FW_FILE_2	:= $(addprefix $(FW_BASE)/,$(FW_FILE_2).bin)

Q :=
vecho := @echo
smalldefine=



vpath %.c $(SRC_DIR)
vpath %.cpp $(SRC_DIR)

define compile-objects
$1/%.o: %.cpp
	$(vecho) "CC $$<"
	$(Q) $(CPP) $(smalldefine) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(SDK_CLOSED_INCDIR) $(CPPFLAGS)  -c $$< -o $$@

$1/%.o: %.c
	$(vecho) "CC - $$< "
	$(Q) $(CC) $(smalldefine) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(SDK_CLOSED_INCDIR) $(CFLAGS)  -c $$< -o $$@
endef


.PHONY: all checkdirs clean
.DEFAULT: all

FILE=buildnr
BUILDNR=$(shell cat $(FILE))


white=0
ledfix=0
#version=0
ifeq ($(strip $(MAKECMDGOALS)),clean)
	white=0
	version=0
endif

ifeq ($(strip $(MAKECMDGOALS)),html)
	white=0
	version=0
endif


ifeq ($(strip $(MAKECMDGOALS)),cleanfirmware)
	white=0
	version=0
endif


version?=$(shell read -r -p "Which version?  a. Cap  b. Shirt  c. Pocket  d. USB  e. Dev: " CONTINU; \
	export ver="a"; \
	if [ "$$CONTINU" = a ]; then \
		export ver=CAP; \
	elif [ "$$CONTINU" = b ]; then \
		export ver=SHIRT; \
	elif [ "$$CONTINU" = c ]; then \
		export ver=POCKET; \
	elif [ "$$CONTINU" = d ]; then \
		export ver=USB; \
	elif [ "$$CONTINU" = e ]; then \
		export ver=DEV; \
	fi; \
	echo $$ver)
	
ver:=$(version)
ifeq ($(ver),SHIRT)
smalldefine+=-DCOLUMNS=16 -DLEDTYPE=\"SHIRT\"
else ifeq ($(ver),USB)
smalldefine+=-DCOLUMNS=16 -DLEDTYPE=\"USB\"
else ifeq ($(ver),POCKET)
smalldefine+=-DCOLUMNS=8 -DLEDTYPE=\"POCKET\"
small=1
else ifeq ($(ver),CAP)
smalldefine+=-DCOLUMNS=16 -DLEDTYPE=\"CAP\"
else ifeq ($(ver),DEV)
smalldefine+=-DCOLUMNS=8 -DLEDTYPE=\"DEV\"
small=1
endif

ifeq ($(ledfix),1)
smalldefine+=-DLEDFIX=1
endif

ifeq ($(white),1)
smalldefine+=-DWHITE=1
endif

smalldefine+=-DBUILDVERSION=$(BUILDNR)

NEWBUILDNR=$$(( $(BUILDNR) + 1 ))

$(shell echo $(NEWBUILDNR) > $(FILE))

all: checkdirs $(TARGET_OUT) $(FW_FILE_1) $(FW_FILE_2) 

$(FW_FILE_1): $(TARGET_OUT) firmware
	$(vecho) "FW $@"
	$(Q) $(FW_TOOL) -eo $(TARGET_OUT) $(FW_FILE_1_ARGS)

$(FW_FILE_2): $(TARGET_OUT) firmware
	$(vecho) "FW $@"
	$(Q) $(FW_TOOL) -eo $(TARGET_OUT) $(FW_FILE_2_ARGS)

$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SDK_LIBDIR) $(SDK_LIBDIR_EXTRA_L) -L$(TOOLCHAIN_LIBDIR) -L$(SDK_CLOSED_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@

$(APP_AR): $(OBJPP) $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^



checkdirs: $(BUILD_DIR) $(FW_BASE)
	@echo Compiling submodules
	cd $(MODULES_BASE)/modules && make
	
	
$(BUILD_DIR):
	$(Q) mkdir -p $@

firmware:
	$(Q) mkdir -p $@

flash: $(FW_FILE_1) $(FW_FILE_2)
	($(ESPTOOL) -b921600 --port $(ESPPORT) write_flash 0x00000 firmware/0x00000.bin 0x40000 firmware/0x40000.bin)||(true)

flashclean: $(FW_FILE_1) $(FW_FILE_2)
	($(ESPTOOL) -b921600 --port $(ESPPORT) write_flash 0x7C000 $(SDK_CLOSED)/bin/esp_init_data_default.bin  0x7E000 $(SDK_CLOSED)/bin/blank.bin  0x3D000 $(SDK_CLOSED)/bin/blank.bin   0x00000 firmware/0x00000.bin 0x40000 firmware/0x40000.bin)||(true)
	
	
	
burn : $(FW_FILE_1) $(FW_FILE_2)
	($(ESPTOOL) -b921600 --port $(ESPPORT) write_flash 0x00000 firmware/0x00000.bin 0x40000 firmware/0x40000.bin)||(true)

netflash: $(FW_FILE_1) $(FW_FILE_2)
	cd ./giftest/ && make && ./send ../firmware/0x00000.bin ../firmware/0x40000.bin && cd -

netgif:
	cd ./giftest && make  && ./out ./gif && ./send_gif 0x100000 ./test.out && cd -

nethtml:
	cd ./giftest && make  && ./gzip ./html && ./out ./html_compr && ./send_gif 0x150000 ./test.out && cd -

flashgif:
ifeq ($(strip $(small)),1)
	cd ./giftest && make  && ./out ./gif_8 && cd .. && ($(ESPTOOL) -b921600 --port $(ESPPORT) write_flash 0x100000 giftest/test.out)||(true)
else
	cd ./giftest && make  && ./out ./gif && cd .. && ($(ESPTOOL) -b921600 --port $(ESPPORT) write_flash 0x100000 giftest/test.out)||(true)
endif	


flashhtml:
	cd ./giftest && make  && ./gzip ./html && ./out ./html_compr && cd .. && ($(ESPTOOL) -b921600 --port $(ESPPORT) write_flash 0x150000 giftest/test.out)||(true)
	

./firmware/gif.out:
ifeq ($(strip $(small)),1)
	cd ./giftest && make  && ./out ./gif_8 && cp ./test.out ../firmware/gif8.out && cd .. ||(true)
else
	cd ./giftest && make  && ./out ./gif && cp ./test.out ../firmware/gif.out && cd .. ||(true)	
endif

./firmware/web.out:
	cd ./giftest && make  && ./gzip ./html && ./out ./html_compr &&  cp ./test.out ../firmware/web.out &&  cd .. ||(true)


html: ./firmware/web.out

gif: ./firmware/gif.out


	
cleanfirmware:
	rm -f ./firmware/web.out
	rm -f ./firmware/web8.out
	rm -f ./firmware/gif.out
	rm -f ./firmware/gif.out
		
		
package: clean cleanfirmware  all $(FW_FILE_1) $(FW_FILE_2) ./firmware/gif.out ./firmware/web.out

flashall:  $(FW_FILE_1) $(FW_FILE_2)
	($(ESPTOOL) -b921600 --port $(ESPPORT) write_flash 0x00000 firmware/0x00000.bin 0x40000 firmware/0x40000.bin 0x150000 firmware/web.out 0x100000 firmware/gif.out)||(true)
	

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -rf $(BUILD_BASE)	
	$(Q) rm -f $(TARGET_OUT)
	$(Q) rm -f $(FW_FILE_1)
	$(Q) rm -f $(FW_FILE_2)
	$(Q) rm -rf $(FW_BASE)
		 rm -rf src/html_data.c

clean_modules: clean
	cd ../../modules && make clean

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
