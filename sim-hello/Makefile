# Copyright (c) 2014, Dimitar Dimitrov
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


# Use the same build options for each core.
CROSS_COMPILE ?= pru-
CFLAGS += -g -Os
CFLAGS += -Wall -Wextra

# TODO - until "-mmcu=" option is added to the toolchain, we must use
# a special linker script.
CFLAGS += -Xlinker -Tprusim.x

# We cannot activate minrt when using stdio 
#CFLAGS += -minrt

# List of object files to compile
OBJS := main.o

# Where to output compiled objects
OUT := out

# ============================ DO NOT TOUCH BELOW ============================
all: $(OUT)/pru-core.elf
	$(CROSS_COMPILE)run $<

trace: $(OUT)/pru-core.elf
	$(CROSS_COMPILE)run -v -t $<

OBJS := $(addprefix $(OUT)/, $(OBJS))

DEPS := $(OBJS:.o=.d)
-include $(DEPS)

$(OUT)/%.o : %.c
	$(CROSS_COMPILE)gcc $(CFLAGS) $< -c -o $@ -MMD

$(OUT)/%.o : %.S
	$(CROSS_COMPILE)gcc $(CFLAGS) $< -c -o $@ -MMD

$(OUT)/%.s : %.c
	$(CROSS_COMPILE)gcc $(CFLAGS) $< -S -o $@

%.s : %.elf
	$(CROSS_COMPILE)objdump -S -d $< > $@

$(OBJS): | $(OUT)

$(OUT):
	mkdir $(OUT)

$(OUT)/pru-core.elf: $(OBJS) | $(OUT)
	$(CROSS_COMPILE)gcc $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	$(RM) -fr pru-core.elf $(OBJS) $(OUT)

cscope:
	cscope -bRk

.PHONY: all clean cscope
