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

#include <libpayload.h>
#include <vboot_nvstorage.h>
#include <vboot_api.h>

#include "base/init_funcs.h"
#include "base/power.h"
#include "base/timestamp.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/net/net.h"
#include "drivers/timer/timer.h"
#include "drivers/video/display.h"
#include "net/uip.h"
#include "netboot/netboot.h"
#include "netboot/params.h"
#include "vboot/util/flag.h"
#include "vboot/vbnv.h"

static void enable_graphics(void)
{
	display_init();
	backlight_update(1);

	if (!CONFIG_OPROM_MATTERS)
		return;

	int oprom_loaded = flag_fetch(FLAG_OPROM);

	// Manipulating vboot's internal data and calling its internal
	// functions is NOT NICE and will give you athlete's foot and make
	// you unpopular at parties. Right now it's the only way to ensure
	// graphics are enabled, though, so it's a necessary evil.
	if (!oprom_loaded) {
		printf("Enabling graphics.\n");

		vbnv_write(VBNV_OPROM_NEEDED, 1);

		printf("Rebooting.\n");
		if (cold_reboot())
			halt();
	}
}

static char cmd_line[4096] = "lsm.module_locking=0 cros_netboot_ramfs "
			     "cros_factory_install cros_secure cros_netboot";

int main(void)
{
	// Initialize some consoles.
	serial_console_init();
	cbmem_console_init();
	video_console_init();

	printf("\n\nStarting netboot on " CONFIG_BOARD "...\n");

	timestamp_init();

	if (run_init_funcs())
		halt();

	// Make sure graphics are available if they aren't already.
	enable_graphics();

	srand(timer_raw_value());

	uip_ipaddr_t *tftp_ip;
	char *bootfile, *argsfile;
	if (netboot_params_read(&tftp_ip, cmd_line, sizeof(cmd_line),
				&bootfile, &argsfile))
		printf("ERROR: Failed to read netboot parameters from flash\n");

	netboot(tftp_ip, bootfile, argsfile, cmd_line);

	// We should never get here.
	printf("Got to the end!\n");
	halt();
	return 0;
}
