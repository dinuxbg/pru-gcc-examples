# Copyright (c) 2015, Dimitar Dimitrov
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#   * Neither the name of the copyright holders nor the names of
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.

# Very simple makefile to cross-compile for PRU


# Common flags
CROSS_COMPILE ?= pru-
CFLAGS += -g -Os
CFLAGS += -Wall -Wextra

# Headers needed by the TI rpmsg library.
CFLAGS += -I./include -I./include/am335x

# Define this to squeeze code size by removing atexit, exit, constructors
# and destructors from CRT.
CFLAGS += -minrt

# Per-PRU core flags. The -mmcu option will select the correct linker
# script and will predefine mcu-specific macros.
CFLAGS0 += -mmcu=am335x.pru0
CFLAGS1 += -mmcu=am335x.pru1

# List of source files to compile for each PRU core.
SRC0 := main0.c
SRC1 := main1.c hc-sr04.c pru_rpmsg.c pru_virtqueue.c

# GCC's -MMD does not yield the needed C dependencies when compiling all
# C source files at once. So manually list headers here.
HEADERS := $(wildcard *.h) $(wildcard include/*.h include/*/*.h)

# Where to output compiled objects
OUT := out

# Final ELF image file names
ELF0 := $(OUT)/pru-halt.elf
ELF1 := $(OUT)/hc-sr04-range-sensor.elf

# ============================ DO NOT TOUCH BELOW ============================
all: $(ELF0) $(ELF1)
	@echo Success: $^

%.s : %.elf
	$(CROSS_COMPILE)objdump -S -d $< > $@

$(OUT):
	mkdir $(OUT)

$(ELF0): $(SRC0) $(HEADERS) | $(OUT)
	$(CROSS_COMPILE)gcc $(CFLAGS) $(CFLAGS0) $(SRC0) $(LDFLAGS) -o $@

$(ELF1): $(SRC1) $(HEADERS) | $(OUT)
	$(CROSS_COMPILE)gcc $(CFLAGS) $(CFLAGS1) $(SRC1) $(LDFLAGS) -o $@

clean:
	$(RM) -fr $(ELF0) $(ELF1) $(OUT)

cscope:
	cscope -bRk

.PHONY: all clean cscope
