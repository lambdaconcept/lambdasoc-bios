#include <arch/irq.h>
#include <boot/serial.h>
#include <console.h>
#include <crc.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <uart.h>

static void help(void)
{
	puts("LambdaSoC BIOS");
	puts("Available commands:");
	puts("help       - show this message");
	puts("serialboot - boot via SFL");
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
	else if (strcmp(token, "serialboot") == 0)
		serialboot();
	else if (strcmp(token, "") != 0)
		puts("Command not found");
}

extern unsigned int _ftext, _erodata;

static void crcbios(void)
{
	unsigned int offset_bios;
	unsigned int length;
	unsigned int expected_crc;
	unsigned int actual_crc;

	/*
	 * _erodata is located right after the end of the flat
	 * binary image. The CRC tool writes the 32-bit CRC here.
	 * We also use the address of _erodata to know the length
	 * of our code.
	 */
	offset_bios = (unsigned int)&_ftext;
	expected_crc = _erodata;
	length = (unsigned int)&_erodata - offset_bios;
	actual_crc = crc32((unsigned char *)offset_bios, length);
	if(expected_crc == actual_crc)
		printf("BIOS CRC passed (%08x)\n", actual_crc);
	else {
		printf("BIOS CRC failed (expected %08x, got %08x)\n", expected_crc, actual_crc);
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
	     "(c) Copyright 2007-2020 M-Labs Limited\n"
	     "(c) Copyright 2020 LambdaConcept\n"
	     "Built "__DATE__" "__TIME__"\n");
	crcbios();

	while (1) {
		putsnonl("\e[1mBIOS>\e[0m ");
		readstr(buffer, 64);
		do_command(buffer);
	}

	return 0;
}
