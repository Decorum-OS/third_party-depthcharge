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

#include <stdio.h>

#include "base/algorithm.h"
#include "drivers/net/net.h"
#include "net/uip.h"
#include "net/uip_arp.h"
#include "netboot/dhcp.h"
#include "netboot/netboot.h"
#include "netboot/params.h"
#include "netboot/tftp.h"
#include "vboot/boot.h"

static void print_ip_addr(const uip_ipaddr_t *ip)
{
	printf("%d.%d.%d.%d", uip_ipaddr1(ip), uip_ipaddr2(ip),
		uip_ipaddr3(ip), uip_ipaddr4(ip));
}

static void print_mac_addr(const uip_eth_addr *mac)
{
	for (int i = 0; i < ARRAY_SIZE(mac->addr); i++)
		printf("%s%02x", i ? ":" : "", mac->addr[i]);
}

static void * const payload = (void *)(uintptr_t)CONFIG_KERNEL_START;
static const uint32_t MaxPayloadSize = CONFIG_KERNEL_SIZE;

char cmd_line[4096];

void netboot(uip_ipaddr_t *tftp_ip, char *bootfile, char *argsfile, char *args)
{
	net_wait_for_link();

	// Start up the network stack.
	uip_init();

	// Plug in the MAC address.
	const uip_eth_addr *mac_addr = net_get_mac();
	if (!mac_addr)
		halt();
	printf("MAC: ");
	print_mac_addr(mac_addr);
	printf("\n");
	uip_setethaddr(*mac_addr);

	// Find out who we are.
	uip_ipaddr_t my_ip, next_ip, server_ip;
	const char *dhcp_bootfile;
	while (dhcp_request(&next_ip, &server_ip, &dhcp_bootfile))
		printf("Dhcp failed, retrying.\n");

	printf("My ip is ");
	uip_gethostaddr(&my_ip);
	print_ip_addr(&my_ip);
	printf("\nThe DHCP server ip is ");
	print_ip_addr(&server_ip);
	printf("\n");

	if (!tftp_ip) {
		tftp_ip = &next_ip;
		printf("TFTP server IP supplied by DHCP server: ");
	} else {
		printf("TFTP server IP predefined by user: ");
	}
	print_ip_addr(tftp_ip);
	printf("\n");

	// Download the bootfile.
	uint32_t size;
	if (!bootfile) {
		bootfile = (char *)dhcp_bootfile;
		printf("Bootfile supplied by DHCP server: %s\n", bootfile);
	} else {
		printf("Bootfile predefined by user: %s\n", bootfile);
	}

	if (tftp_read(payload, tftp_ip, bootfile, &size, MaxPayloadSize)) {
		printf("Tftp failed.\n");
		if (dhcp_release(server_ip))
			printf("Dhcp release failed.\n");
		halt();
	}
	printf("The bootfile was %d bytes long.\n", size);

	// Try to download command line file via TFTP if argsfile is specified
	if (argsfile && !(tftp_read(cmd_line, tftp_ip, argsfile, &size,
			sizeof(cmd_line) - 1))) {
		while (cmd_line[size - 1] <= ' ')  // strip trailing whitespace
			if (!--size) break;	   // and control chars (\n, \r)
		cmd_line[size] = '\0';
		while (size--)			   // replace inline control
			if (cmd_line[size] < ' ')  // chars with spaces
				cmd_line[size] = ' ';
		printf("Command line loaded dynamically from TFTP file: %s\n",
				argsfile);
	// If that fails or file wasn't specified fall back to args parameter
	} else if (args) {
		strncpy(cmd_line, args, sizeof(cmd_line) - 1);
		printf("Command line predefined by user.\n");
	}

	// We're done on the network, so release our IP.
	if (dhcp_release(server_ip)) {
		printf("Dhcp release failed.\n");
		halt();
	}

	// Add tftp server IP into command line.
	static const char def_tftp_cmdline[] = " tftpserverip=xxx.xxx.xxx.xxx";
	int cmd_line_size = strlen(cmd_line);
	if (cmd_line_size + sizeof(def_tftp_cmdline) - 1 >= sizeof(cmd_line)) {
		printf("Out of space adding TFTP server IP " \
		       "to the command line.\n");
		return;
	}
	sprintf(&cmd_line[cmd_line_size], " tftpserverip=%d.%d.%d.%d",
		uip_ipaddr1(tftp_ip), uip_ipaddr2(tftp_ip),
		uip_ipaddr3(tftp_ip), uip_ipaddr4(tftp_ip));
	printf("The command line is: %s\n", cmd_line);

	// Boot.
	boot(payload, cmd_line, NULL, NULL);
}
