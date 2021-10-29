#include <boot/common.h>
#include <boot/net.h>
#include <config.h>
#include <litex/tftp.h>
#include <litex/udp.h>
#include <stdio.h>

#ifndef TFTP_SERVER_PORT
#define TFTP_SERVER_PORT 69
#endif

static unsigned char macadr[6] = {0x10, 0xe2, 0xd5, 0x00, 0x00, 0x00};

#ifdef CONFIG_SOC_WITH_SDRAM
#define MAIN_RAM_BASE CONFIG_SDRAM_BASE
#else
#define MAIN_RAM_BASE CONFIG_SRAM_BASE
#endif

union ipv4_addr {
	struct ipv4_addr_s {
		uint32_t d : 8;
		uint32_t c : 8;
		uint32_t b : 8;
		uint32_t a : 8;
	} f;
	uint32_t n;
};

static int copy_file_from_tftp_to_ram(unsigned int ip, unsigned short server_port,
		const char *filename, char *buffer)
{
	printf("Copying %s to %p...", filename, buffer);
	int size = tftp_get(ip, server_port, filename, buffer);
	if (size > 0) {
		printf(" (%d bytes)", size);
	}
	printf("\n");
	return size;
}

void tftpboot(const char *filename) {
	union ipv4_addr local_ip  = { .n = CONFIG_ETHMAC_LOCAL_IP  };
	union ipv4_addr remote_ip = { .n = CONFIG_ETHMAC_REMOTE_IP };

	printf("Booting from network...\n");
	printf("Local IP: %d.%d.%d.%d\n", local_ip.f.a, local_ip.f.b, local_ip.f.c, local_ip.f.d);
	printf("Remote IP: %d.%d.%d.%d\n", remote_ip.f.a, remote_ip.f.b, remote_ip.f.c, remote_ip.f.d);

	udp_start(macadr, local_ip.n);

	int size = copy_file_from_tftp_to_ram(remote_ip.n, TFTP_SERVER_PORT, filename, (void *)MAIN_RAM_BASE);
	if (size > 0) {
		boot(0, 0, 0, MAIN_RAM_BASE);
	}

	/* Boot failed if reached */
	printf("Network boot failed.\n");
}
