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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "base/xalloc.h"
#include "drivers/board/board.h"
#include "drivers/storage/storage.h"
#include "net/uip.h"
#include "netboot/params.h"

static NetbootParam netboot_params[NetbootParamIdMax];

const char netboot_sig[] = "netboot";

NetbootParam *netboot_params_val(NetbootParamId param)
{
	assert(param < NetbootParamIdMax);
	return &netboot_params[param];
}

static uintptr_t size32(uintptr_t size)
{
	return (size + sizeof(uint32_t) - 1) / sizeof(uint32_t);
}

int netboot_params_init(void *data, uintptr_t size)
{
	assert(data);

	memset(netboot_params, 0, sizeof(netboot_params));

	if (size < sizeof(netboot_sig))
		return 1;

	if (memcmp(data, netboot_sig, sizeof(netboot_sig)))
		return 1;

	uintptr_t max_pos = size / sizeof(uint32_t);
	uint32_t *data32 = (uint32_t *)data;
	uintptr_t pos = size32(sizeof(netboot_sig));

	if (pos >= max_pos)
		return 1;

	uint32_t count = data32[pos++];

	while (count--) {
		uint32_t val_type = data32[pos++];
		if (pos >= max_pos)
			return 1;
		uint32_t val_size = data32[pos++];
		if (pos >= max_pos)
			return 1;
		void *val_data = &data32[pos];
		pos += size32(val_size);
		if (pos >= max_pos)
			return 1;

		NetbootParam *param = &netboot_params[val_type];
		param->data = val_data;
		param->size = val_size;
	}
	return 0;
}

int netboot_params_read(uip_ipaddr_t **tftp_ip, char *cmd_line,
			size_t cmd_line_max, char **bootfile, char **argsfile)
{
	NetbootParam *param;

	*tftp_ip = NULL;
	*bootfile = NULL;
	*argsfile = NULL;

	static void *data;
	if (!data) {
		// Retrieve settings from the non volatile scratch space.
		StorageOps *scratch = board_storage_nv_scratch();
		int size = storage_size(scratch);
		if (size < 0)
			return 1;

		data = xmalloc(size);
		if (storage_read(scratch, data, 0, size) ||
		    netboot_params_init(data, size)) {
			free(data);
			data = NULL;
			return 1;
		}
	}

	// Get TFTP server IP and file names from params if specified
	param = netboot_params_val(NetbootParamIdTftpServerIp);
	if (param->data && param->size >= sizeof(uip_ipaddr_t))
		*tftp_ip = (uip_ipaddr_t *)param->data;

	param = netboot_params_val(NetbootParamIdBootfile);
	if (param->data && param->size > 0 && strnlen((char *)param->data,
			param->size) < param->size)
		*bootfile = (char *)param->data;

	param = netboot_params_val(NetbootParamIdArgsFile);
	if (param->data && param->size > 0 && strnlen((char *)param->data,
			param->size) < param->size)
		*argsfile = (char *)param->data;

	if (!cmd_line || cmd_line_max == 0)
		return 0;

	// Add extra arguments from params to factory default command line
	param = netboot_params_val(NetbootParamIdKernelArgs);
	if (param->data && param->size > 0 && strnlen((char *)param->data,
			param->size) < param->size) {
		int cmd_line_size = strnlen(cmd_line, cmd_line_max - 1) + 1;
		cmd_line[cmd_line_size - 1] = ' ';
		strncpy(&cmd_line[cmd_line_size], param->data,
			cmd_line_max - cmd_line_size);
	}
	cmd_line[cmd_line_max - 1] = '\0';

	return 0;
}
