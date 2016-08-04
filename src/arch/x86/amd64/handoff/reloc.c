/* reloc_x86_64.c - position independent x86_64 ELF shared object relocator
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

#include "base/die.h"
#include "base/init_funcs.h"

#include <elf.h>
#include <stdint.h>
#include <stddef.h>

static uint32_t _uefi_handoff_unhandled_relocation_type;

static int _uefi_report_unhandled_relocation(void)
{
	die_if(_uefi_handoff_unhandled_relocation_type != R_X86_64_NUM,
	       "Unhandled relocation type %d!\n",
	       _uefi_handoff_unhandled_relocation_type);
	return 0;
}

INIT_FUNC(_uefi_report_unhandled_relocation);

void _uefi_handoff_relocate(uintptr_t ldbase, Elf64_Sym symtab[],
			    Elf64_Rela rel[], size_t relsz)
{
	uint32_t unhandled_reloc_type = R_X86_64_NUM;
	int count = relsz / sizeof(rel[0]);
	for (int i = 0; i < count; i++) {
		// Apply the relocs.
		switch (Elf64_R_Type(rel[i].r_info)) {
		case R_X86_64_NONE:
			break;

		case R_X86_64_RELATIVE:
			*(uintptr_t *)(ldbase + rel[i].r_offset) += ldbase;
			break;

		case R_X86_64_64:
			*(uintptr_t *)(ldbase + rel[i].r_offset) =
				symtab[Elf64_R_Sym(rel[i].r_info)].st_value +
				ldbase;
			break;

		default:
			unhandled_reloc_type = Elf64_R_Type(rel[i].r_info);
			break;
		}
	}

	_uefi_handoff_unhandled_relocation_type = unhandled_reloc_type;
}
