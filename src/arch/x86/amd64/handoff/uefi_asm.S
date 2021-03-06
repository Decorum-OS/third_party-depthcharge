/* crt0-efi-x86_64.S - x86_64 EFI startup code.
   Copyright (C) 1999 Hewlett-Packard Co.
	Contributed by David Mosberger <davidm@hpl.hp.com>.
   Copyright (C) 2005 Intel Co.
	Contributed by Fenghua Yu <fenghua.yu@intel.com>.

    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials
      provided with the distribution.
    * Neither the name of Hewlett-Packard Co. nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
    BE LIABLE FOR ANYDIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

	.code64
	.global _entry
	.section .text._entry
	.align 4

_entry:
	sub $8, %rsp
	push %rcx
	push %rdx

	lea ImageBase(%rip), %rdi
	lea _dynsym_start(%rip), %rsi
	lea _reloc_start(%rip), %rdx
	lea _reloc_end(%rip), %rcx
	sub %rdx, %rcx

	call _uefi_handoff_relocate

	pop %rdx
	mov %rdx, _uefi_handoff_system_table(%rip)
	pop %rcx
	mov %rcx, _uefi_handoff_image_handle(%rip)

	jmp handoff_common_asm

	.data
	.align 4

	.global _uefi_handoff_image_handle
_uefi_handoff_image_handle:
	.quad 0

	.global _uefi_handoff_system_table
_uefi_handoff_system_table:
	.quad 0

 	// hand-craft a dummy .reloc section so EFI knows it's a relocatable executable:

 	.data
dummy:	.long	0

#define IMAGE_REL_ABSOLUTE	0
 	.section .reloc, "a"
label1:
	.long	dummy-label1				// Page RVA
 	.long	10					// Block Size (2*4+2)
	.word	(IMAGE_REL_ABSOLUTE<<12) +  0		// reloc for dummy

