/* Copyright (c) 2010 Axel Wachtler
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the authors nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE. */

/* $Id$ */
/**
 * @file
 * @brief Command interface for wireless bootloader application
 * Prepared for interfacing with a PC Host + Python
 *
 * @author Daniel Thiele
 *         Dietzsch und Thiele PartG
 *         Bautzner Str. 67 01099 Dresden
 *         www.ib-dt.de daniel.thiele@ib-dt.de
 *
 * @ingroup grpAppWiBo
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include <board.h>
#include <hif.h>
#include <transceiver.h>
#include <radio.h>
#include <p2p_protocol.h>

#include "cmdif.h"
#include "wibohost.h"
#include "hexparse.h"

#define EOL "\n"
#define MAXLINELEN (80)

/* variable for function cmd_feedhex()
 * placed here (global) to store in SRAM at compile time
 */
static hexrec_t hexrec;
static uint8_t lnbuf[MAXLINELEN + 1];

/* following flags must be not zero to allow first run of "wait_previous_command" */
static volatile uint8_t tx_done = 1;
static volatile uint8_t flashcycle_done = 1;

static volatile radio_tx_done_t last_tx_status;

/*
 * \brief Wait for complete line, no character echoing
 *
 * @return 1 for line completed, 0 else
 */
static inline uint8_t getline()
{
	int16_t inchar;
	static uint8_t idx = 0;

	inchar = hif_getc();
	if (inchar != EOF)
	{
		lnbuf[idx] = 0x00; /* NULL terminated string */
		if ((inchar == '\n') || (inchar == '\r'))
		{
			idx = 0;
			return 1;
		}
		else if (idx < MAXLINELEN)
		{
			lnbuf[idx++] = inchar;
		}
		else
		{
			/* TODO: catch line length overflow */
		}
	}

	return 0;
}

/*
 * \brief Print "OK" to stdout, used to save code space
 */
static inline void printok()
{
	PRINT("OK"EOL);
}

/*
 * \brief Wait for the previous command to be finished
 *
 * This enables pipelining of commands
 */
static void wait_previous_command(void)
{
	/* Pipelining:
	 * wait for the previous cycle to be finished
	 *
	 * both flags have to be set to continue
	 */
	while ((0 == tx_done) || (0 == flashcycle_done))
		;

	tx_done = 0; /* for each command */
}

/*
 * \brief Called asynchronous when ping reply frame is received
 */
void cb_wibohost_pingreply(p2p_ping_cnf_t *pr)
{
	PRINTF(
			"OK {'short_addr':0x%04X, 'appname': '%s'," " 'boardname':'%s', 'version':0x%02X, " "'crc':0x%04X}"EOL,
			pr->hdr.src, pr->appname, pr->boardname, pr->version, pr->crc);
}

/*
 * \brief Timeout for ping request
 */
void cb_wibohost_pingtimeout(void)
{
	PRINT("ERR ping timeout"EOL);
}

/*
 *\brief Timeout for flash write cycle
 */
void cb_wibohost_flashcycletimeout(void)
{
	flashcycle_done = 1;
}

/*
 * \brief Called async from wibohost module
 */
void cb_wibohost_radio_error(radio_error_t err)
{
	PRINTF("ERR Radio: 0x%02X"EOL, err);
}

/*
 * \brief Called async from wibohost module
 */
void cb_wibohost_tx_done(radio_tx_done_t status)
{
	tx_done = 1;
	last_tx_status = status;
}

/*
 * \brief Command to execute wibohost_ping() functions
 *
 * @param params Pointer to list of strings containing the arguments
 *               checking for valid number of arguments is done outside this function
 *
 */
static inline void cmd_ping(char **params)
{
	uint16_t short_addr;

	short_addr = strtol(params[0], NULL, 16);
	wait_previous_command();
	wibohost_ping(short_addr);
}

/*
 * \brief Command to execute wibohost_deaf() function
 *
 * @param params Pointer to list of strings containing the arguments
 *               checking for valid number of arguments is done outside this function
 *
 */
static inline void cmd_deaf(char **params)
{
	uint16_t short_addr;

	short_addr = strtol(params[0], NULL, 16);
	wait_previous_command();
	wibohost_deaf(short_addr);
	printok();
}

/*
 * \brief Command to execute wibohost_finish() functions
 *
 * @param params Pointer to list of strings containing the arguments
 *               checking for valid number of arguments is done outside this function
 *
 */
static inline void cmd_finish(char **params)
{
	uint16_t short_addr;

	short_addr = strtol(params[0], NULL, 16);
	wait_previous_command();
	wibohost_finish(short_addr);
	printok();
}

/*
 * \brief Command to execute wibohost_feed() functions
 *
 * @param params Pointer to list of strings containing the arguments
 *               checking for valid number of arguments is done outside this function
 *
 * Expected parameters
 *  (1) short_addr
 *  (2) line from intel hex-file
 *
 */
static inline void cmd_feedhexline(char **params)
{
	uint16_t short_addr;
	short_addr = strtol(params[0], NULL, 16);

	if (!parsehexline((uint8_t*) params[1], &hexrec))
	{
		PRINT("ERR parsing hexline"EOL);
	}
	else
	{
		if (HEX_RECTYPE_DATA == hexrec.type)
		{ /* only feed data type lines */

			wait_previous_command();

			flashcycle_done = 0; /* set explicitely */
			wibohost_feed(short_addr, hexrec.data, hexrec.len);
			if (TX_OK == last_tx_status)
			{
				printok();
			}
			else
			{
				PRINT("ERR Tx fail"EOL);
			}
		}
		else
		{
			PRINTF("WARN ignoring rec type 0x%02X"EOL, hexrec.type);
		}
	}
}

/*
 * \brief Command to receive a complete hex-file
 *
 * This is intended to use from a serial terminal instead of a python command interface
 *
 * Expected parameters
 *  (1) short_addr
 *
 */
static inline void cmd_feedhexfile(char **params)
{
	uint8_t error = 0;
	uint16_t short_addr;

	short_addr = strtol(params[0], NULL, 16);
	PRINT("OK upload hex-file now..."EOL);

	do
	{
		/* fetch line */
		do
		{
		} while (0 == getline());

		if (!parsehexline((uint8_t*) lnbuf, &hexrec))
		{
			PRINT("ERR parsing hexline");
			error = 1;
		}

		wait_previous_command();

		/* TODO: handling address jump records */

		if (HEX_RECTYPE_DATA == hexrec.type)
		{ /* only feed data type lines */
			flashcycle_done = 0; /* set explicitely */
			wibohost_feed(short_addr, hexrec.data, hexrec.len);
		}
		else if (HEX_RECTYPE_EOF == hexrec.type)
		{
			PRINT("OK end of file"EOL);
			error = 1;
		}
		else
		{
			PRINTF("WARN ignoring rec type %d"EOL, hexrec.type);
		}
	} while (0 == error);
}

static inline void cmd_setchannel(char **params)
{
	uint8_t channel;
	channel = strtol(params[0], NULL, 16);
	if (wibohost_setchannel(channel))
	{
		printok();
	}
	else
	{
		PRINT("ERR channel out of range"EOL);
	}
}

static inline void cmd_setpanid(char **params)
{
	uint16_t pan_id;
	pan_id = strtol(params[0], NULL, 16);

	if (wibohost_setpanid(pan_id))
	{
		printok();
	}
	else
	{
		PRINT("ERR setting PAN_ID"EOL);
	}
}

/*
 * \brief Command to execute wibohost_reset() function
 *
 * @param params Pointer to list of strings containing the arguments
 *               checking for valid number of arguments is done outside this function
 *
 */
static inline void cmd_reset(char **params)
{
	wait_previous_command();
	wibohost_reset(); /* always reset all nodes */
	printok();
}

/*
 * \brief Command to execute wibohost_addr() function
 *
 * @param params Pointer to list of strings containing the arguments
 *               checking for valid number of arguments is done outside this function
 *
 */
static inline void cmd_addr(char **params)
{
	uint16_t short_addr;
	uint32_t flash_addr;

	short_addr = strtol(params[0], NULL, 16);
	flash_addr = strtol(params[1], NULL, 16); /* TODO: possibly 16-bit only ?? */

	wait_previous_command();
	wibohost_addr(short_addr, flash_addr);
	printok();
}

/*
 * \brief Command to execute wibohost_exit() functions
 *-       PRINTF("OK panid=0x%04X shortaddr=0x%04X"EOL, cfg->pan_id,

 * @param params Pointer to list of strings containing the arguments
 *               checking for valid number of arguments is done outside this function
 *
 *
 * Expected parameters
 *  (1) short_addr
 *
 */
static inline void cmd_exit(char **params)
{
	uint16_t short_addr;

	short_addr = strtol(params[0], NULL, 16);

	wait_previous_command();

	wibohost_exit(short_addr);
	printok();
}

/*
 * \brief Command to execute wibohost_getcrc() functions
 *
 * @param params Pointer to list of strings containing the arguments
 *               checking for valid number of arguments is done outside this function
 *
 */
static inline void cmd_checkcrc(char **params)
{
	PRINTF("OK 0x%04X"EOL, wibohost_getcrc());
}

/*
 * \brief Command to echo a string
 * This is intended to check the UART connection
 *
 * @param params Pointer to list of strings containing the arguments
 *               checking for valid number of arguments is done outside this function
 *
 */
static inline void cmd_echo(char **params)
{
	PRINTF("OK %s"EOL, params[0]);
}

/*
 * \brief Command to echo a string
 * This is intended to check the UART connection
 *
 * @param params Pointer to list of strings containing the arguments
 *               checking for valid number of arguments is done outside this function
 *
 */
static inline void cmd_info(char **params)
{
	node_config_t *cfg = wibohost_getnodecfg();
	PRINTF("OK CHANNEL=0x%02X, PAN_ID=0x%04X, SHORT_ADDR=0x%04X"EOL,
			cfg->channel, cfg->pan_id, cfg->short_addr);
}

/*
 * \brief Set target memory
 * All calls of feedhex.. work on this target.
 * Valid values are:
 *  F : Flash memory
 *  E : EEPROM
 *  L : Lock bits
 *  X : No memory, dry run
 *
 */
static inline void cmd_target(char **params)
{
	uint8_t targ = toupper(params[0][0]);
	if ((targ == 'F') || (targ == 'E') || (targ == 'L') || (targ == 'X'))
	{
		wibohost_target(0xFFFF, targ);
		PRINTF("OK memory target set to %c"EOL, targ);
	}
	else
	{
		PRINTF("ERR memory target unknown: %c"EOL, targ);
	}
}

/*
 * \brief Command to jump into bootloader
 * This is used to support an example application where the host does handle the bootloader itself and
 * an example application residing in the application section of the remote node
 *
 * @param params Pointer to list of strings containing the arguments
 *               checking for valid number of arguments is done outside this function
 *
 *
 * Expected parameters
 *  (1) short_addr
 *
 */
static inline void cmd_jbootl(char **params)
{
	uint16_t short_addr;

	short_addr = strtol(params[0], NULL, 16);

	wait_previous_command();
	wibohost_jbootl(short_addr);
	printok();
}

/*
 * \brief Update bootloader from start of application section
 *
 * @param params Pointer to list of strings containing the arguments
 *               checking for valid number of arguments is done outside this function
 *
 *
 * Expected parameters
 *  (1) short_addr
 *
 */
static inline void cmd_bootlup(char **params)
{
	uint16_t short_addr;

	short_addr = strtol(params[0], NULL, 16);

	wait_previous_command();
	wibohost_bootlup(short_addr);
	printok();
}

static inline void cmd_xmplled(char **params)
{
	uint16_t dst_addr = strtol(params[0], NULL, 16);
	p2p_xmpl_led_t frm;

	/* frm.hdr is filled by wibohost_sendcommand() */
	frm.led = strtol(params[1], NULL, 16);
	frm.state = strtol(params[2], NULL, 16);

	wibohost_sendcommand(dst_addr, P2P_XMPL_LED, (uint8_t*) &frm,
			sizeof(p2p_xmpl_led_t));
	printok();
}

/* forward declaration */
static inline void cmd_help(char **params);

/* List of commands that are available at the command interface */
const struct
{
	const char *name; /**< Name of the command */
	void (*execfunc)(char**); /**< Function to be called */
	uint8_t nbparams; /**< Expected number of parameters */
	const char *description; /**< Textual description of the command to print help screen */
} commands[] =
{
{ "addr", cmd_addr, 2, "Set flash target address of node" },
{ "ping", cmd_ping, 1, "Ping a node" },
{ "deaf", cmd_deaf, 1, "Set a node to deaf" },
{ "finish", cmd_finish, 1, "Finish a node (force write)" },
{ "feedhex", cmd_feedhexline, 2, "Feed a line of hex file to a node" },
{ "feedhexfile", cmd_feedhexfile, 1, "Feed hex file to a node" },
{ "reset", cmd_reset, 0, "Reset a node" },
{ "exit", cmd_exit, 1, "Exit node bootloader" },
{ "crc", cmd_checkcrc, 0, "Get data CRC" },
{ "echo", cmd_echo, 1, "Echo a string" },
{ "info", cmd_info, 0, "Information about myself" },
{ "target", cmd_target, 1, "Set target memory" },
{ "jbootl", cmd_jbootl, 1, "Jump into bootloader" },
{ "bootlup", cmd_bootlup, 1, "Update Bootloader" },
{ "channel", cmd_setchannel, 1, "Set radio channel" },
{ "panid", cmd_setpanid, 1, "Set radio PAN_ID" },
{ "xmplled", cmd_xmplled, 3, "Example of application: Set LED" },
{ "?", cmd_help, 0, "Help on commands" }, };

static inline void cmd_help(char **params)
{
	uint8_t i;
	PRINT("Available commands:"EOL);
	for (i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
	{
		PRINTF("%02d %12s %s\n", i, commands[i].name, commands[i].description);
	}
}

/*
 * \brief Parsing a shell input line
 *
 * @param *ln String containing the command line to parse
 */
static inline void process_cmdline(char *ln)
{
	char *params[10];
	uint8_t nbparams;
	uint8_t i;

	nbparams = hif_split_args(ln, 10, params);
	for (i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
	{
		if (!strcasecmp(params[0], commands[i].name))
		{
			if (commands[i].nbparams < nbparams)
			{ /* comparison including the command itself */
				commands[i].execfunc(&params[1]);
			}
			else
			{
				PRINT("ERR Parameter"EOL);
			}
			break;
		}
	}

	/* i > than size of list: did not find anything */
	if ((sizeof(commands) / sizeof(commands[0])) == i)
	{
		PRINT("ERR Unknown command"EOL);
	}
}

void cmdif_task(void)
{
	if (getline())
	{
		process_cmdline((char*) lnbuf);
	}
}

/* EOF */
