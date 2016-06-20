/*
 * Copyright 2016 Google Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


/*
 * ELF header structure offsets.
 */
#define E_IDENT     0
#define E_TYPE      16
#define E_MACHINE   18
#define E_VERSION   20
#define E_ENTRY     24
#define E_PHOFF     28
#define E_SHOFF     32
#define E_FLAGS     36
#define E_EHSIZE    40
#define E_PHENTSIZE 42
#define E_PHNUM     44
#define E_SHENTSIZE 46
#define E_SHNUM     48
#define E_SHSTRNDX  50

/* e_ident array indexes. */
#define EI_MAG0    0
#define EI_MAG1    1
#define EI_MAG2    2
#define EI_MAG3    3
#define EI_CLASS   4
#define EI_DATA    5
#define EI_VERSION 6
#define EI_PAD     7
#define EI_NIDENT  16

/* Magic values which should be in e_ident. */
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

/* ELF classes which could be in e_ident[EI_CLASS]. */
#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

/*
 * Program header structure offsets.
 */
#define P_TYPE   0
#define P_OFFSET 4
#define P_VADDR  8
#define P_PADDR  12
#define P_FILESZ 16
#define P_MEMSZ  20
#define P_FLAGS  24
#define P_ALIGN  28

/* Segment header types. */
#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7fffffff
