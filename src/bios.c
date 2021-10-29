#include <arch/irq.h>
#include <boot/serial.h>
#include <console.h>
#include <crc.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <uart.h>

#include <boot/common.h>

#if CONFIG_SOC_WITH_SDRAM
# include <litex/sdram.h>
#endif

#if CONFIG_SOC_WITH_ETHMAC
# include <litex/mdio.h>
# include <litex/udp.h>
# include <boot/net.h>

static void mdiow(char *c_phyadr, char *c_reg, char *c_val)
{
	char *c;
	unsigned int phyadr = strtoul(c_phyadr, &c, 0);
	if (*c != 0) {
		printf("incorrect address\n");
		return;
	}
	unsigned int reg = strtoul(c_reg, &c, 0);
	if (*c != 0) {
		printf("incorrect register\n");
		return;
	}
	unsigned int val = strtoul(c_val, &c, 0);
	if (*c != 0) {
		printf("incorrect value\n");
		return;
	}

	printf("MDIO write @ 0x%04x: 0x%02x 0x%04x\n", phyadr, reg, val);
	mdio_write(phyadr, reg, val);
}

static void mdior(char *c_phyadr, char *c_reg)
{
	char *c;
	unsigned int phyadr = strtoul(c_phyadr, &c, 0);
	if (*c != 0) {
		printf("incorrect address\n");
		return;
	}
	unsigned int reg = strtoul(c_reg, &c, 0);
	if (*c != 0) {
		printf("incorrect register\n");
		return;
	}

	printf("MDIO read @ 0x%04x:\n", phyadr);
	unsigned int val = mdio_read(phyadr, reg);
	printf("0x%02x 0x%04x\n", reg, val);
}

static void mdiod(char *c_phyadr, char *c_count)
{
	char *c;
	unsigned int phyadr = strtoul(c_phyadr, &c, 0);
	if (*c != 0) {
		printf("incorrect address\n");
		return;
	}
	unsigned int count = strtoul(c_count, &c, 0);
	if (*c != 0) {
		printf("incorrect count\n");
		return;
	}

	printf("MDIO dump @ 0x%04x:\n", phyadr);
	for (int i = 0; i < count; i++) {
		unsigned int val = mdio_read(phyadr, i);
		printf("0x%02x 0x%04x\n", i, val);
	}
}
#endif

/* General address space functions */

#define NUMBER_OF_BYTES_ON_A_LINE 16
static void dump_bytes(unsigned int *ptr, int count, unsigned addr)
{
	char *data = (char *)ptr;
	int line_bytes = 0, i = 0;

	putsnonl("Memory dump:");
	while(count > 0){
		line_bytes =
			(count > NUMBER_OF_BYTES_ON_A_LINE)?
				NUMBER_OF_BYTES_ON_A_LINE : count;

		printf("\n0x%08x  ", addr);
		for(i=0;i<line_bytes;i++)
			printf("%02x ", *(unsigned char *)(data+i));

		for(;i<NUMBER_OF_BYTES_ON_A_LINE;i++)
			printf("   ");

		printf(" ");

		for(i=0;i<line_bytes;i++) {
			if((*(data+i) < 0x20) || (*(data+i) > 0x7e))
				printf(".");
			else
				printf("%c", *(data+i));
		}

		for(;i<NUMBER_OF_BYTES_ON_A_LINE;i++)
			printf(" ");

		data += (char)line_bytes;
		count -= line_bytes;
		addr += line_bytes;
	}
	printf("\n");
}

static void mr(char *startaddr, char *len)
{
	char *c;
	unsigned int *addr;
	unsigned int length;

	if(*startaddr == 0) {
		printf("mr <address> [length]\n");
		return;
	}
	addr = (unsigned *)strtoul(startaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect address\n");
		return;
	}
	if(*len == 0) {
		length = 4;
	} else {
		length = strtoul(len, &c, 0);
		if(*c != 0) {
			printf("incorrect length\n");
			return;
		}
	}

	dump_bytes(addr, length, (unsigned)addr);
}

static void mw(char *addr, char *value, char *count)
{
	char *c;
	unsigned int *addr2;
	unsigned int value2;
	unsigned int count2;
	unsigned int i;

	if((*addr == 0) || (*value == 0)) {
		printf("mw <address> <value> [count]\n");
		return;
	}
	addr2 = (unsigned int *)strtoul(addr, &c, 0);
	if(*c != 0) {
		printf("incorrect address\n");
		return;
	}
	value2 = strtoul(value, &c, 0);
	if(*c != 0) {
		printf("incorrect value\n");
		return;
	}
	if(*count == 0) {
		count2 = 1;
	} else {
		count2 = strtoul(count, &c, 0);
		if(*c != 0) {
			printf("incorrect count\n");
			return;
		}
	}
	for (i=0;i<count2;i++) *addr2++ = value2;
}

static void mc(char *dstaddr, char *srcaddr, char *count)
{
	char *c;
	unsigned int *dstaddr2;
	unsigned int *srcaddr2;
	unsigned int count2;
	unsigned int i;

	if((*dstaddr == 0) || (*srcaddr == 0)) {
		printf("mc <dst> <src> [count]\n");
		return;
	}
	dstaddr2 = (unsigned int *)strtoul(dstaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect destination address\n");
		return;
	}
	srcaddr2 = (unsigned int *)strtoul(srcaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect source address\n");
		return;
	}
	if(*count == 0) {
		count2 = 1;
	} else {
		count2 = strtoul(count, &c, 0);
		if(*c != 0) {
			printf("incorrect count\n");
			return;
		}
	}
	for (i=0;i<count2;i++) *dstaddr2++ = *srcaddr2++;
}

static void crc(char *startaddr, char *len)
{
	char *c;
	char *addr;
	unsigned int length;

	if((*startaddr == 0)||(*len == 0)) {
		printf("crc <address> <length>\n");
		return;
	}
	addr = (char *)strtoul(startaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect address\n");
		return;
	}
	length = strtoul(len, &c, 0);
	if(*c != 0) {
		printf("incorrect length\n");
		return;
	}

	printf("CRC32: %08x\n", crc32((unsigned char *)addr, length));
}

/* Init + command line */

static void help(void)
{
	puts("LambdaSoC BIOS");
	puts("Available commands:");
	puts("help       - show this message");
	puts("mr         - read address space");
	puts("mw         - write address space");
	puts("mc         - copy address space");
#if CONFIG_SOC_WITH_ETHMAC
	puts("mdiow      - write MDIO register");
	puts("mdior      - read MDIO register");
	puts("mdiod      - dump MDIO registers");
#endif
	puts("crc        - compute CRC32 of a part of the address space");
	puts("serialboot - boot via SFL");
#if CONFIG_SOC_WITH_ETHMAC
	puts("tftpboot    - boot via TFTP");
#endif
}

static char *get_token(char **str)
{
	char *c, *d;

	c = (char *)strchr(*str, ' ');
	if (c == NULL) {
		d = *str;
		*str = *str+strlen(*str);
		return d;
	}
	*c = 0;
	d = *str;
	*str = c+1;
	return d;
}

static void do_command(char *c)
{
	char *token = get_token(&c);

	if (strcmp(token, "help") == 0)
		help();
        else if (strcmp(token, "mr") == 0)
            mr(get_token(&c), get_token(&c));
        else if (strcmp(token, "mw") == 0)
            mw(get_token(&c), get_token(&c), get_token(&c));
        else if (strcmp(token, "mc") == 0)
            mc(get_token(&c), get_token(&c), get_token(&c));
        else if (strcmp(token, "crc") == 0)
            crc(get_token(&c), get_token(&c));
#if CONFIG_SOC_WITH_ETHMAC
        else if (strcmp(token, "mdiow") == 0)
            mdiow(get_token(&c), get_token(&c), get_token(&c));
        else if (strcmp(token, "mdior") == 0)
            mdior(get_token(&c), get_token(&c));
        else if (strcmp(token, "mdiod") == 0)
            mdiod(get_token(&c), get_token(&c));
#endif
	else if (strcmp(token, "serialboot") == 0)
		serialboot();
#if CONFIG_SOC_WITH_ETHMAC
	else if (strcmp(token, "tftpboot") == 0)
		tftpboot("boot.bin"); // FIXME filename
#endif
	else if (strcmp(token, "") != 0)
		puts("Command not found");
}

extern unsigned int _ftext, _edata_bootrom;

static void crcbios(void)
{
	unsigned int offset_bios;
	unsigned int length;
	unsigned int expected_crc;
	unsigned int actual_crc;

	printf("BIOS CRC check... ");

	/*
	 * _edata_bootrom is located right after the end of the flat
	 * binary image. The CRC tool writes the 32-bit CRC here.
	 * We also use the address of _edata_bootrom to know the length
	 * of our code.
	 */
	offset_bios = (unsigned int)&_ftext;
	expected_crc = _edata_bootrom;
	length = (unsigned int)&_edata_bootrom - offset_bios;
	actual_crc = crc32((unsigned char *)offset_bios, length);
	if(expected_crc == actual_crc)
		printf("passed (%08x)\n", actual_crc);
	else {
		printf("failed (expected %08x, got %08x)\n", expected_crc, actual_crc);
		printf("The system will continue, but expect problems.\n");
	}
}

static void readstr(char *s, size_t size)
{
	char c[2];
	size_t ptr;

	c[1] = 0;
	ptr = 0;
	while(1) {
		c[0] = readchar();
		switch(c[0]) {
		case 0x7f:
		case 0x08:
			if(ptr > 0) {
				ptr--;
				putsnonl("\x08 \x08");
			}
			break;
		case 0x07:
			break;
		case '\r':
		case '\n':
			s[ptr] = 0x00;
			putsnonl("\n");
			return;
		default:
			putsnonl(c);
			s[ptr] = c[0];
			if(ptr + 1 < size)
				ptr++;
			break;
		}
	}
}

int main(void)
{
	char buffer[64];

	irq_setmask(0);
	irq_setie(1);
	uart_init();

	puts("\nLambdaSoC BIOS\n"
	     "(c) Copyright 2012-2021 Enjoy-Digital\n"
	     "(c) Copyright 2007-2020 M-Labs Limited\n"
	     "(c) Copyright 2021 LambdaConcept\n"
	     "Built "__DATE__" "__TIME__"\n");
	crcbios();

#if CONFIG_SOC_WITH_ETHMAC
	eth_init();
#endif
#if CONFIG_SOC_WITH_SDRAM
	if (!sdram_init()) {
		printf("Memory initialization failed\n");
	}
#endif

	while (1) {
		putsnonl("\e[1mBIOS>\e[0m ");
		readstr(buffer, 64);
		do_command(buffer);
	}

	return 0;
}
