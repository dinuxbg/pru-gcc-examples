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
CXXFLAGS += -g -Os
CXXFLAGS += -Wall -Wextra
CXXFLAGS += -mmcu=am335x.pru0

CXXFLAGS += -fno-rtti

# Define this to squeeze code size by removing atexit, exit, constructors
# and destructors from CRT.
CXXFLAGS += -minrt

# List of object files to compile
OBJS := main.o

# Where to output compiled objects
OUT := out

# ============================ DO NOT TOUCH BELOW ============================
CXXFLAGS += $(CFLAGS)

all: $(OUT)/pru-core.elf
	@echo Success: $^

OBJS := $(addprefix $(OUT)/, $(OBJS))

DEPS := $(OBJS:.o=.d)
-include $(DEPS)

$(OUT)/%.o : %.c
	$(CROSS_COMPILE)g++ $(CXXFLAGS) $< -c -o $@ -MMD

$(OUT)/%.o : %.cpp
	$(CROSS_COMPILE)g++ $(CXXFLAGS) $< -c -o $@ -MMD

$(OUT)/%.o : %.S
	$(CROSS_COMPILE)g++ $(CXXFLAGS) $< -c -o $@ -MMD

$(OUT)/%.s : %.c
	$(CROSS_COMPILE)g++ $(CXXFLAGS) $< -S -o $@

$(OUT)/%.s : %.cpp
	$(CROSS_COMPILE)g++ $(CXXFLAGS) $< -S -o $@

%.s : %.elf
	$(CROSS_COMPILE)objdump -S -d $< > $@

# Binary images for IMEM and DMEM memories. Might be useful
# for firmware loaders that cannot handle ELF.
%.imem.bin : %.elf
	$(CROSS_COMPILE)objcopy -O binary  --only-section=.text* $< $@

%.dmem.bin : %.elf
	$(CROSS_COMPILE)objcopy -O binary              \
	  --only-section=.data*                        \
	  --only-section=.bss*                         \
	  --set-section-flags .bss=alloc,load,contents \
	  $< $@

$(OBJS): | $(OUT)

$(OUT):
	mkdir $(OUT)

$(OUT)/pru-core.elf: $(OBJS) | $(OUT)
	$(CROSS_COMPILE)g++ $(CXXFLAGS) $^ $(LDFLAGS) -o $@

clean:
	$(RM) -fr pru-core.elf $(OBJS) $(OUT)

cscope:
	cscope -bRk

.PHONY: all clean cscope
