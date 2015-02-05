/* Copyright (c) 2014, Dimitar Dimitrov
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

#ifndef __CXX_IO__
#define __CXX_IO__

#include <stdint.h>

class __R30_type {
public:
	void operator= (uint32_t rhs) const {
		register uint32_t r30 asm ("r30");
		r30 = rhs;
		asm volatile (";":"+r"(r30) :);
	}
	operator uint32_t() const {
		register uint32_t r30 asm ("r30");
		asm volatile (";":"+r"(r30));
		return r30;
	}
};

class __R31_type {
public:
	void operator= (uint32_t rhs) const {
		register uint32_t r31 asm ("r31");
		r31 = rhs;
		asm volatile (";":"+r"(r31) :);
	}
	operator uint32_t() const {
		register uint32_t r31 asm ("r31");
		asm volatile (";":"+r"(r31));
		return r31;
	}
};

__R30_type __R30 __attribute__((weak));
__R31_type __R31 __attribute__((weak));

#endif	// __CXX_IO__
