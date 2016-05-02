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

#include "base/container_of.h"
#include "base/die.h"
#include "base/fwdb.h"
#include "drivers/gpio/fwdb.h"
#include "vboot/util/flag.h"

static int fwdb_gpio_get(GpioOps *me)
{
	FwdbGpio *gpio = container_of(me, FwdbGpio, ops);
	if (!gpio->value) {
		FwdbEntry entry;
		die_if(fwdb_access(gpio->name, &entry, NULL),
		       "No FWDB entry for GPIO %s.\n", gpio->name);

		die_if(entry.size != sizeof(uint8_t),
		       "GPIO entry is the wrong size.");
		gpio->value = *(uint8_t *)entry.ptr + 1;
	}
	return gpio->value - 1;
}

FwdbGpio fwdb_gpio_wpsw = {
	.ops = {
		.get = &fwdb_gpio_get,
	},
	.name = "gpio.write protect",
};

FwdbGpio fwdb_gpio_recsw = {
	.ops = {
		.get = &fwdb_gpio_get,
	},
	.name = "gpio.recovery",
};

FwdbGpio fwdb_gpio_devsw = {
	.ops = {
		.get = &fwdb_gpio_get,
	},
	.name = "gpio.developer",
};

FwdbGpio fwdb_gpio_oprom = {
	.ops = {
		.get = &fwdb_gpio_get,
	},
	.name = "gpio.oprom",
};

FwdbGpio fwdb_gpio_lidsw = {
	.ops = {
		.get = &fwdb_gpio_get,
	},
	.name = "gpio.lid",
};

FwdbGpio fwdb_gpio_pwrsw = {
	.ops = {
		.get = &fwdb_gpio_get,
	},
	.name = "gpio.power",
};

FwdbGpio fwdb_gpio_ecinrw = {
	.ops = {
		.get = &fwdb_gpio_get,
	},
	.name = "gpio.EC in RW",
};

void fwdb_install_flags(GpioOps *lid, GpioOps *power, GpioOps *ec_in_rw)
{
	// If a GPIO is not defined, we will just flag_install() a NULL, which
	// will only hit a die_if() if that flag is actually flag_fetch()ed.
	flag_install(FLAG_WPSW, &fwdb_gpio_wpsw.ops);
	flag_install(FLAG_RECSW, &fwdb_gpio_recsw.ops);
	flag_install(FLAG_DEVSW, &fwdb_gpio_devsw.ops);
	flag_install(FLAG_OPROM, &fwdb_gpio_oprom.ops);

	flag_install(FLAG_LIDSW, lid ? lid : &fwdb_gpio_lidsw.ops);
	flag_install(FLAG_PWRSW, power ? power : &fwdb_gpio_pwrsw.ops);
	flag_install(FLAG_ECINRW, ec_in_rw ? ec_in_rw : &fwdb_gpio_ecinrw.ops);
}
