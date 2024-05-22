# SPDX-License-Identifier: BSD-2-Clause
# 
# Copyright (c) 2022 Jonathan Armstrong. All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions 
# are met:
# 
# 1. Redistributions of source code must retain the above copyright 
# notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright 
# notice, this list of conditions and the following disclaimer in the 
# documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
# POSSIBILITY OF SUCH DAMAGE.

# Prerequisites --------------------------------------------------------
#
# Besides make, his project requires: 
#
# - sdcc


# Usage ----------------------------------------------------------------
#
# Build executables:
#   make
#
# Clean project (remove all build files):
#   make clean

# Target MCU settings --------------------------------------------------
# sonoff black box
TARGET_BOARD = EFM8BB1
#TARGET_BOARD = EFM8BB1LCB

# sonoff white box
TARGET_BOARD = OB38S003


ifndef TARGET_BOARD
$(error Please define TARGET_BOARD in makefile)
endif

ifeq ($(TARGET_BOARD), EFM8BB1)
# for EFM8BB1 in Sonoff v2.0 receivers (black color box)
MCU_FREQ_KHZ = 24500
else ifeq ($(TARGET_BOARD), EFM8BB1LCB)
MCU_FREQ_KHZ = 24500
else ifeq ($(TARGET_BOARD), OB38S003)
# for OB38S003 used in Sonoff v2.2 receivers (white color box)
MCU_FREQ_KHZ = 16000
#else
#	$(error Please define TARGET_BOARD as EFM8BB1, EFM8BB1LCB, or OB38S003 in makefile)
endif

#
MEMORY_SIZES  = --iram-size 256 --xram-size 256 --code-size 8192
MEMORY_MODEL  = --model-small
HAS_DUAL_DPTR = n

# 
SOURCE_DIR = src

ifeq ($(TARGET_BOARD), EFM8BB1)
DRIVER_SRC_DIR = drivers/efm8bb1/src
else ifeq ($(TARGET_BOARD), OB38S003)
DRIVER_SRC_DIR = drivers/ob38s003/src
endif


INCLUDE_DIR = inc/

ifeq ($(TARGET_BOARD), EFM8BB1)
DRIVER_DIR = drivers/efm8bb1/inc
else ifeq ($(TARGET_BOARD), OB38S003)
DRIVER_DIR = drivers/ob38s003/inc
endif


OBJECT_DIR = object
BUILD_DIR  = build

SOURCES = $(SOURCE_DIR)/main_passthrough.c \
		  $(SOURCE_DIR)/main_rcswitch.c \
		  $(SOURCE_DIR)/rcswitch.c \
		  $(SOURCE_DIR)/state_machine.c \
		  $(SOURCE_DIR)/uart.c \
		  $(SOURCE_DIR)/ticks.c \
          $(DRIVER_SRC_DIR)/delay.c \
		  $(DRIVER_SRC_DIR)/hal.c	\
		  $(DRIVER_SRC_DIR)/timer_interrupts.c
OBJECT_NAMES = $(notdir $(SOURCES:.c=.rel))
OBJECTS = $(patsubst %,$(OBJECT_DIR)/%,$(OBJECT_NAMES))

OBJECTS_RCSWITCH = 		$(OBJECT_DIR)/main_rcswitch.rel \
						$(OBJECT_DIR)/rcswitch.rel \
						$(OBJECT_DIR)/state_machine.rel \
						$(OBJECT_DIR)/uart.rel \
						$(OBJECT_DIR)/delay.rel \
						$(OBJECT_DIR)/hal.rel	\
						$(OBJECT_DIR)/ticks.rel \
						$(OBJECT_DIR)/timer_interrupts.rel
OBJECTS_PASSTHROUGH = 	$(OBJECT_DIR)/main_passthrough.rel \
						$(OBJECT_DIR)/delay.rel \
						$(OBJECT_DIR)/hal.rel
						
TARGET_RCSWITCH     = $(BUILD_DIR)/main_rcswitch_$(TARGET_BOARD).ihx
TARGET_PASSTHROUGH  = $(BUILD_DIR)/main_passthrough_$(TARGET_BOARD).ihx

###########################################################
# Toolchain settings
###########################################################

TARGET_ARCH = -mmcs51

AS       = sdas8051
CC       = sdcc
ASFLAGS  = -plosgffw
CPPFLAGS = $(PROJECT_FLAGS) -DTARGET_BOARD_$(TARGET_BOARD) -DMCU_FREQ=$(MCU_FREQ_KHZ)000UL -I$(INCLUDE_DIR) -I$(DRIVER_DIR)
CFLAGS   = $(TARGET_ARCH) $(MEMORY_MODEL) $(CPPFLAGS)
LDFLAGS  = $(TARGET_ARCH) $(MEMORY_MODEL) $(MEMORY_SIZES)

###########################################################
# Phony targets
###########################################################

.PHONY: all clean

all: $(TARGET_RCSWITCH) $(TARGET_PASSTHROUGH)

clean:
	# it is safer to remove wildcard with file extension instead of the entire directory
	# but can revisit this issue in the future (I think other projects just remove directory)
	rm -f $(BUILD_DIR)/*.hex
	rm -f $(BUILD_DIR)/*.ihx
	rm -f $(BUILD_DIR)/*.lk
	rm -f $(BUILD_DIR)/*.map
	rm -f $(BUILD_DIR)/*.mem
	rm -f $(OBJECT_DIR)/*.asm
	rm -f $(OBJECT_DIR)/*.lst
	rm -f $(OBJECT_DIR)/*.rel
	rm -f $(OBJECT_DIR)/*.rst
	rm -f $(OBJECT_DIR)/*.sym
	
###########################################################
# Build
# $@ is equal to the target (in this case %.rel)
# $^ is equal to the input (in this case %.c)
###########################################################

# basically the linking step
$(TARGET_RCSWITCH): $(OBJECTS_RCSWITCH)
	@echo "Linking $^"
	mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $^
	
	# hex lines are a short, fixed length (compared with ihx) and therefore works with upload tools
	# unix style line endings (LF instead of LFCR) work with upload tools
	packihx $@ > $(basename $@).hex
	dos2unix $(basename $@).hex
	
$(TARGET_PASSTHROUGH): $(OBJECTS_PASSTHROUGH)
	@echo "Linking $^"
	mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $^
	
	# hex lines are a short, fixed length (compared with ihx) and therefore works with upload tools
	# unix style line endings (LF instead of LFCR) work with upload tools
	packihx $@ > $(basename $@).hex
	dos2unix $(basename $@).hex

# basically the compilation step
$(OBJECT_DIR)/%.rel: $(SOURCE_DIR)/%.c
	@echo "Compiling $^"
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^
	
$(OBJECT_DIR)/%.rel: $(DRIVER_SRC_DIR)/%.c
	@echo "Compiling $^"
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^