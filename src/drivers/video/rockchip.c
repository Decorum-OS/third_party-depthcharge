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

#include "base/container_of.h"
#include "base/io.h"
#include "base/time.h"
#include "base/xalloc.h"
#include "drivers/video/rockchip.h"

typedef struct {
	DisplayOps ops;
	GpioOps *backlight_gpio;
} RkDisplay;

#define RK_CLRSETBITS(clr, set) ((((clr) | (set)) << 16) | set)
#define RK_SETBITS(set) RK_CLRSETBITS(0, set)
#define RK_CLRBITS(clr) RK_CLRSETBITS(clr, 0)

#define VOP_STANDBY_EN    22

static uint32_t *vop0_sys_ctrl = (uint32_t *)0xff930008;
static uint32_t *vop1_sys_ctrl = (uint32_t *)0xff940008;

static int rockchip_backlight_update(DisplayOps *me, uint8_t enable)
{
	RkDisplay *display = container_of(me, RkDisplay, ops);
	gpio_set(display->backlight_gpio, enable);
	return 0;
}

static int rockchip_display_stop(DisplayOps *me)
{
	/* set vop0 to standby */
	write32(vop0_sys_ctrl, RK_SETBITS(1 << VOP_STANDBY_EN));

	/* set vop1 to standby */
	write32(vop1_sys_ctrl, RK_SETBITS(1 << VOP_STANDBY_EN));

	/* wait frame complete (60Hz) to enter standby */
	mdelay(17);

	return 0;
}

DisplayOps *new_rockchip_display(GpioOps *backlight)
{
	RkDisplay *display = xzalloc(sizeof(*display));
	display->ops.stop = rockchip_display_stop;
	if (backlight) {
		display->backlight_gpio = backlight;
		display->ops.backlight_update = rockchip_backlight_update;
	}

	return &display->ops;
}
