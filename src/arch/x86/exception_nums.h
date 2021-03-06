/*
 * Copyright 2013 Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __ARCH_X86_EXCEPTION_NUMS_H__
#define __ARCH_X86_EXCEPTION_NUMS_H__

enum {
	EXC_DE = 0, // Divide by zero
	EXC_DB = 1, // Debug 
	EXC_NMI = 2, // Non maskable interrupt 
	EXC_BP = 3, // Breakpoint 
	EXC_OF = 4, // Overflow 
	EXC_BR = 5, // Bound range 
	EXC_UD = 6, // Invalid opcode 
	EXC_NM = 7, // Device not available 
	EXC_DF = 8, // Double fault 
	EXC_TS = 10, // Invalid TSS 
	EXC_NP = 11, // Segment not present 
	EXC_SS = 12, // Stack 
	EXC_GP = 13, // General protection 
	EXC_PF = 14, // Page fault 
	EXC_MF = 16, // x87 floating point 
	EXC_AC = 17, // Alignment check 
	EXC_MC = 18, // Machine check 
	EXC_XF = 19, // SIMD floating point 
	EXC_SX = 30, // Security 
	EXC_COUNT
};

#endif /* __ARCH_X86_EXCEPTION_NUMS_H__ */
