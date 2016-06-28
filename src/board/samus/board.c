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

#include <pci.h>
#include <stdint.h>
#include <sysinfo.h>

#include "base/init_funcs.h"
#include "base/io.h"
#include "base/list.h"
#include "board/board.h"
#include "board/board_helpers.h"
#include "board/samus/device_nvs.h"
#include "drivers/bus/i2c/designware.h"
#include "drivers/bus/i2c/i2c.h"
#include "drivers/bus/i2s/broadwell/alc5677.h"
#include "drivers/bus/i2s/broadwell/broadwell.h"
#include "drivers/bus/i2s/i2s.h"
#include "drivers/ec/cros/lpc.h"
#include "drivers/flash/flash.h"
#include "drivers/flash/memmapped.h"
#include "drivers/gpio/fwdb.h"
#include "drivers/gpio/gpio.h"
#include "drivers/gpio/lynxpoint_lp.h"
#include "drivers/keyboard/dynamic.h"
#include "drivers/keyboard/ps2.h"
#include "drivers/power/pch.h"
#include "drivers/sound/i2s.h"
#include "drivers/sound/route.h"
#include "drivers/sound/rt5677.h"
#include "drivers/storage/ahci.h"
#include "drivers/storage/blockdev.h"
#include "drivers/tpm/lpc.h"
#include "drivers/tpm/tpm.h"
#include "drivers/uart/8250.h"
#include "drivers/video/display.h"
#include "drivers/video/intel_i915.h"

PRIV_DYN(ec_in_rw_gpio, &new_lp_pch_gpio_input(25)->ops);

PUB_STAT(flag_write_protect, gpio_get(&fwdb_gpio_wpsw.ops))
PUB_STAT(flag_recovery, gpio_get(&fwdb_gpio_recsw.ops))
PUB_STAT(flag_developer_mode, gpio_get(&fwdb_gpio_devsw.ops))
PUB_STAT(flag_option_roms_loaded, gpio_get(&fwdb_gpio_oprom.ops))
PUB_STAT(flag_lid_open, gpio_get(&fwdb_gpio_lidsw.ops))
PUB_STAT(flag_power, gpio_get(&fwdb_gpio_pwrsw.ops))
PUB_STAT(flag_ec_in_rw, gpio_get(get_ec_in_rw_gpio()))

// Put device in D0 state.
static void device_enable(int sio_index)
{
	device_nvs_t *nvs = lib_sysinfo.acpi_gnvs + DEVICE_NVS_OFFSET;
	uint32_t *reg_pcs =
		(uint32_t *)(uintptr_t)(nvs->bar1[sio_index] + 0x84);

	// Put device in D0 state. Disable D3Hot.
	write32(reg_pcs, read32(reg_pcs) & ~3);
	(void)read32(reg_pcs);
}

static DesignwareI2c *i2c_enable(int sio_index)
{
	device_nvs_t *nvs = lib_sysinfo.acpi_gnvs + DEVICE_NVS_OFFSET;
	uint32_t *ppr_clock =
		(uint32_t *)(uintptr_t)(nvs->bar0[sio_index] + 0x800);

	device_enable(sio_index);

	// Enable clock to the device.
	write32(ppr_clock, read32(ppr_clock) | (1 << 0));
	(void)read32(ppr_clock);

	return new_designware_i2c(nvs->bar0[sio_index], 400000);
}

static BdwI2s *i2s_enable(int ssp)
{
	device_nvs_t *nvs = lib_sysinfo.acpi_gnvs + DEVICE_NVS_OFFSET;

	device_enable(SIO_NVS_ADSP);

	return new_bdw_i2s(nvs->bar0[SIO_NVS_ADSP], ssp, 16,
		&broadwell_alc5677_settings);
}

static int board_setup(void)
{
	CrosEcLpcBus *cros_ec_lpc_bus =
		new_cros_ec_lpc_bus(CROS_EC_LPC_BUS_GENERIC);
	cros_ec_set_bus(&cros_ec_lpc_bus->ops);

	flash_set_ops(&new_mem_mapped_flash(0xff800000, 0x800000)->ops);

	AhciCtrlr *ahci = new_ahci_ctrlr(PCI_DEV(0, 31, 2));
	list_insert_after(&ahci->ctrlr.list_node, &fixed_block_dev_controllers);

	tpm_set_ops(&new_lpc_tpm((void *)(uintptr_t)0xfed40000)->ops);

	// Setup sound route via I2S to RT5677 codec.
	BdwI2s *i2s = i2s_enable(0);
	I2sSource *i2s_source = new_i2s_source(&i2s->ops, 48000, 2, 16000);
	SoundRoute *sound_route = new_sound_route(&i2s_source->ops);

	DesignwareI2c *i2c0 = i2c_enable(SIO_NVS_I2C0);
	rt5677Codec *codec = new_rt5677_codec(&i2c0->ops, 0x2c, 16,
					      48000, 256, 0, 0);

	list_insert_after(&codec->component.list_node,
			  &sound_route->components);
	sound_set_ops(&sound_route->ops);

	if (lib_sysinfo.framebuffer != NULL) {
		uintptr_t i915_base = pci_read_config32(PCI_DEV(0, 2, 0),
							PciConfBar0) & ~0xf;
		display_set_ops(new_intel_i915_display(i915_base));
	}

	return 0;
}

PUB_STAT(power, &pch_power_ops)

PUB_DYN(debug_uart, &new_uart_8250_io(0x3f8)->uart.ops)

PUB_ARR(trusted_keyboards, &new_ps2_keyboard()->ops)
PUB_ARR(untrusted_keyboards, &dynamic_keyboards.ops)

INIT_FUNC(board_setup);
