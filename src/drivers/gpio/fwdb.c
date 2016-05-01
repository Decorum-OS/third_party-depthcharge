/*
 * Copyright 2012 Google Inc.
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

#include <assert.h>
#include <libpayload.h>
#include <sysinfo.h>

#include "base/die.h"
#include "base/fwdb.h"
#include "drivers/gpio/fwdb.h"
#include "vboot/util/flag.h"

GpioOps *fwdb_lookup_gpio(const char *name)
{
	FwdbEntry gpio_entry;
	if (fwdb_access(name, &gpio_entry, NULL)) {
		printf("coreboot did not sample '%s' GPIO!\n", name);
		return NULL;
	}

	uint8_t value;
	die_if(gpio_entry.size != sizeof(value),
		"GPIO entry is the wrong size.");
	value = *(uint8_t *)gpio_entry.ptr;

	return value ? new_gpio_high() : new_gpio_low();
}

void fwdb_install_flags(GpioOps *lid, GpioOps *power, GpioOps *ec_in_rw)
{
	// If a GPIO is not defined, we will just flag_install() a NULL, which
	// will only hit a die_if() if that flag is actually flag_fetch()ed.
	flag_install(FLAG_WPSW, fwdb_lookup_gpio("gpio.write protect"));
	flag_install(FLAG_RECSW, fwdb_lookup_gpio("gpio.recovery"));
	flag_install(FLAG_DEVSW, fwdb_lookup_gpio("gpio.developer"));
	flag_install(FLAG_OPROM, fwdb_lookup_gpio("gpio.oprom"));

	if (lid)
		flag_install(FLAG_LIDSW, lid);
	else
		flag_install(FLAG_LIDSW, fwdb_lookup_gpio("gpio.lid"));
	if (power)
		flag_install(FLAG_PWRSW, power);
	else
		flag_install(FLAG_PWRSW, fwdb_lookup_gpio("gpio.power"));
	if (ec_in_rw)
		flag_install(FLAG_ECINRW, ec_in_rw);
	else
		flag_install(FLAG_ECINRW, fwdb_lookup_gpio("gpio.EC in RW"));
}
