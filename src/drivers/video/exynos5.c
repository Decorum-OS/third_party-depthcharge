/*
 * Copyright 2013 Google Inc.
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

#include <stdint.h>

#include "base/cleanup.h"
#include "base/init_funcs.h"
#include "base/io.h"

enum {
	FIMD_VIDCON0_ENVID = 2,
	FIMD_VIDCON0_ENVID_F = 1
};

enum {
	FIMD_WINCON_ENWIN_F = 1
};

static int disable_fimd_dma(DcEvent *event)
{
	uintptr_t base = CONFIG_DRIVER_VIDEO_EXYNOS5_BASE;

	uint32_t *fimd_vidcon0 = (uint32_t *)(base + 0);
	uint32_t *fimd_shadowcon = (uint32_t *)(base + 0x34);

	// Disable video output and control signals.
	uint32_t vidcon0 = read32(fimd_vidcon0);
	vidcon0 &= ~(FIMD_VIDCON0_ENVID | FIMD_VIDCON0_ENVID_F);
	write32(fimd_vidcon0, vidcon0);

	// Disable all channels.
	write32(fimd_shadowcon, 0);

	return 0;
}

static int install_dma_cleanup(void)
{
	static CleanupEvent dma_cleanup = {
		.event = { .trigger = &disable_fimd_dma, },
		.types = CleanupOnHandoff,
	};

	cleanup_add(&dma_cleanup);
	return 0;
}

INIT_FUNC(install_dma_cleanup);
