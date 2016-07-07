/*
 * Copyright 2014 Rockchip Electronics Co., Ltd.
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

#include "base/io.h"
#include "drivers/blockdev/dw_mmc.h"
#include "drivers/gpio/rockchip.h"

struct rk3288_cru_reg {
	uint32_t cru_apll_con[4];
	uint32_t cru_dpll_con[4];
	uint32_t cru_cpll_con[4];
	uint32_t cru_gpll_con[4];
	uint32_t cru_npll_con[4];
	uint32_t cru_mode_con;
	uint32_t reserved0[3];
	uint32_t cru_clksel_con[43];
	uint32_t reserved1[21];
	uint32_t cru_clkgate_con[19];
	uint32_t reserved2;
	uint32_t cru_glb_srst_fst_value;
	uint32_t cru_glb_srst_snd_value;
	uint32_t cru_softrst_con[12];
	uint32_t cru_misc_con;
	uint32_t cru_glb_cnt_th;
	uint32_t cru_glb_rst_con;
	uint32_t reserved3;
	uint32_t cru_glb_rst_st;
	uint32_t reserved4;
	uint32_t cru_sdmmc_con[2];
	uint32_t cru_sdio0_con[2];
	uint32_t cru_sdio1_con[2];
	uint32_t cru_emmc_con[2];
};

static struct rk3288_cru_reg *cru_ptr = (void *)0xff760000;

#define RK_CLRSETBITS(clr, set) ((((clr) | (set)) << 16) | set)

void rkclk_configure_emmc(DwmciHost *host, unsigned int freq)
{
	int src_clk_div;

	dwmci_write32(host, DWMCI_CLKDIV, 0);
	src_clk_div = ALIGN_UP(host->src_hz / 2, freq) / freq;

	if (src_clk_div > 0x3f) {
		src_clk_div = (24000000 / 2 + freq - 1) / freq;
		write32(&cru_ptr->cru_clksel_con[12],
			RK_CLRSETBITS(0xff << 8 , 2 << 14 |
			((src_clk_div - 1) << 8)));
	} else
		write32(&cru_ptr->cru_clksel_con[12],
			RK_CLRSETBITS(0xff << 8 , 1 << 14 |
			((src_clk_div - 1) << 8)));
}

void rkclk_configure_sdmmc(DwmciHost *host, unsigned int freq)
{
	int src_clk_div;

	dwmci_write32(host, DWMCI_CLKDIV, 0);
	src_clk_div = ALIGN_UP(host->src_hz / 2, freq) / freq;

	if (src_clk_div > 0x3f) {
		src_clk_div = (24000000 / 2 + freq - 1) / freq;
		write32(&cru_ptr->cru_clksel_con[11],
			RK_CLRSETBITS(0xff, 2 << 6 | (src_clk_div - 1)));
	} else
		write32(&cru_ptr->cru_clksel_con[11],
			RK_CLRSETBITS(0xff, 1 << 6 | (src_clk_div - 1)));
}

DwmciHost *new_rkdwmci_host(uintptr_t ioaddr, uint32_t src_hz,
				int bus_width, int removable,
				GpioOps *card_detect)
{
	DwmciHost *mmc;

	mmc = new_dwmci_host(ioaddr, src_hz, bus_width, removable,
			     card_detect, 0);
	if(removable)
		mmc->set_clk = &rkclk_configure_sdmmc;
	else
		mmc->set_clk = &rkclk_configure_emmc;
	return mmc;
}
