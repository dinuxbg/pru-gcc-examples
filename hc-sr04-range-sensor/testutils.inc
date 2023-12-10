# Copyright (C) 2016-2023 Free Software Foundation, Inc.
# Contributed by Dimitar Dimitrov <dimitar@dinux.eu>
#
# This file is part of the GNU simulators.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# MACRO: start
# All assembler tests should start with a call to "main_test"
	.macro start
	.text
	.global test_carry
	.type test_carry, function
test_carry:
	.endm

# MACRO: pass
	.macro pass
	ldi r14, 101
	ret
	.endm

# MACRO: fail
	.macro fail
	ldi32 r14, 606
	ret
	.endm

# MACRO: qbne32
# Like qbne instruction, but check a 32-bit constant value.
	.macro qbne32 label:req, op0:req, C0:req
	qbne \label, \op0\().b0, ((\C0) >> 0) & 0xff
	qbne \label, \op0\().b1, ((\C0) >> 8) & 0xff
	qbne \label, \op0\().b2, ((\C0) >> 16) & 0xff
	qbne \label, \op0\().b3, ((\C0) >> 24) & 0xff
	.endm
