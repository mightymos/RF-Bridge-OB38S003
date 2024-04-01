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
SONOFF_BLACK = OB38S003
SONOFF_WHITE = EFM8BB1

# FIXME: for EFM8BB1 in Sonoff v2.0 receivers (black color box)
#MCU_FREQ_KHZ := 24500
# for OB38S003 used in Sonoff v2.2 receivers (white color box)
MCU_FREQ_KHZ = 16000

#
MEMORY_SIZES  = --iram-size 256 --xram-size 256 --code-size 8192
MEMORY_MODEL  = --model-small
HAS_DUAL_DPTR = n

# 
SOURCE_DIR     = src
DRIVER_SRC_DIR = drivers/ob38s003/src

INCLUDE_DIR    = inc
DRIVER_DIR     = drivers/ob38s003/inc

INCLUDE_DIRS = $(addprefix -I, $(INCLUDE_DIR) $(DRIVER_DIR))

BUILD_DIR      = build

# FIXME: add pass through mode
# FIXME: support multiple microcontrollers
SOURCES = $(SOURCE_DIR)/main_passthrough.c \
          $(DRIVER_SRC_DIR)/delay.c \
		  $(DRIVER_SRC_DIR)/hal.c
OBJECTS = $(BUILD_DIR)/main_passthrough.rel \
		  $(BUILD_DIR)/delay.rel \
		  $(BUILD_DIR)/hal.rel
TARGET  = $(BUILD_DIR)/main_passthrough.ihx

###########################################################
# Toolchain settings
###########################################################

TARGET_ARCH = -mmcs51

AS       = sdas8051
CC       = sdcc
ASFLAGS  = -plosgffw
CPPFLAGS = $(PROJECT_FLAGS) -DMCU_FREQ=$(MCU_FREQ_KHZ)000UL -I$(INCLUDE_DIRS)
CFLAGS   = $(TARGET_ARCH) $(MEMORY_MODEL) $(CPPFLAGS)
LDFLAGS  = $(TARGET_ARCH) $(MEMORY_MODEL) $(MEMORY_SIZES)

###########################################################
# Phony targets
###########################################################

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	
###########################################################
# Build
# $@ is equal to the target (in this case %.rel)
# $^ is equal to the input (in this case %.c)
###########################################################

# basically the linking step
$(TARGET): $(OBJECTS)
	@echo "Linking $^"
	$(CC) $(LDFLAGS) -o $@ $^
	packihx $@ > $(basename $@).hex
	dos2unix $(basename $@).hex

# basically the compilation step
$(BUILD_DIR)/%.rel: $(SOURCE_DIR)/%.c
	mkdir -p $(dir $@)
	@echo "Compiling $^"
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c -o $@ $^
	
$(BUILD_DIR)/%.rel: $(DRIVER_SRC_DIR)/%.c
	mkdir -p $(dir $@)
	@echo "Compiling $^"
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c -o $@ $^