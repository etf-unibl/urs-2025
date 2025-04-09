/**
*******************************************************************************
Name 		: board_init.c
Author 		: Mladen Knezic
Version 	: 0.1
Copyright 	: BSD License
Description	: Implementation of board init functions
*******************************************************************************
*/

/* ========================================================================== */
/*                                   Includes                                 */
/* ========================================================================== */
#include "board_init.h"

/* ========================================================================== */
/*                               Macros & Typedefs                            */
/* ========================================================================== */
#define L4WD0				(6U)		// Bit position of WD0 in reset register
#define GPIO1				(26U)		// Bit position of GPIO1 in reset register
#define IOSCANCHAIN2		(2U)		// Bit position of scan chain 2 in scan manager

#define JTAG_BP_INSN		(1 << 0U)	// JTAG instruction bit position
#define JTAG_BP_TMS			(1 << 1U)	// JTAG TMS bit position
#define JTAG_BP_PAYLOAD		(1 << 2U)	// JTAG payload bit position
#define JTAG_BP_2BYTE		(1 << 3U)	// JTAG 2-bytes bit position
#define JTAG_BP_4BYTE		(1 << 4U)	// JTAG 4-bytes bit position

// The size of the scan chain
#define CFG_HPS_IOCSR_SCANCHAIN_LENGTH	955

/* ========================================================================== */
/*                               Global Variables                             */
/* ========================================================================== */
// Scan chain values to be set in IOCSRs for bank 7A
const unsigned long iocsr_scan_chain[] = {
	0x300C0300,
	0x00000000,
	0x0FF00000,
	0x00000000,
	0x000300C0,
	0x00008000,
	0x00080000,
	0x18060000,
	0x18000000,
	0x00018060,
	0x00020000,
	0x00004000,
	0x200300C0,
	0x10000000,
	0x00000000,
	0x00000040,
	0x00010000,
	0x00002000,
	0x10018060,
	0x06018000,
	0x06000000,
	0x00010018,
	0x00006018,
	0x00001000,
	0x0000C030,
	0x00000000,
	0x03000000,
	0x0000800C,
	0x00C0300C,
	0x00000800,
};

/* ========================================================================== */
/*                                 Declarations                               */
/* ========================================================================== */
static void scan_mgr_jtag_io(const unsigned int flags,
		const unsigned char iarg,
		const unsigned int parg);
static void wait_until_scan_chain_engine_is_idle(void);
static void scan_mgr_jtag_insn_data(const unsigned char iarg,
		const unsigned long *data,
		const unsigned int dlen);
static void board_iocsr_config();
static void sysmgr_vioctrl_freeze_req();
static void sysmgr_vioctrl_thaw_req();
static void board_pinmux_config();
static void board_reset_config();

/* ========================================================================== */
/*                                  Functions                                 */
/* ========================================================================== */
static void scan_mgr_jtag_io(const unsigned int flags,
		const unsigned char iarg,
		const unsigned int parg)
{
	unsigned int data = parg;
	volatile int* HPS_SCANMNGR_fifosinglebyte_ptr = (int*) 0xFFF02010;
	volatile int* HPS_SCANMNGR_fifodoublebyte_ptr = (int*) 0xFFF02014;
	volatile int* HPS_SCANMNGR_fifoquadbyte_ptr = (int*) 0xFFF0201C;

	if (flags & JTAG_BP_INSN)	// JTAG instruction
	{
		// The SCC JTAG register is LSB first
		data <<= 8;
		if (flags & JTAG_BP_TMS)
		{
			data |= (0 << 7);		// TMS instruction
			data |= iarg & 0x3f;	// TMS arg is 6 bits

			if (flags & JTAG_BP_PAYLOAD)
			{
				data |= (1 << 6);
			}
		}
		else
		{
			data |= (1 << 7);	// TDI/TDO instruction
			data |= iarg & 0xf;	// TDI/TDO arg is 4 bits

			if (flags & JTAG_BP_PAYLOAD)
			{
				data |= (1 << 4);
			}
		}
	}

	if (flags & JTAG_BP_4BYTE)
	{
		*HPS_SCANMNGR_fifoquadbyte_ptr = data;
	}
	else if (flags & JTAG_BP_2BYTE)
	{
		*HPS_SCANMNGR_fifodoublebyte_ptr = data & 0xffff;
	}
	else
	{
		*HPS_SCANMNGR_fifosinglebyte_ptr = data & 0xff;
	}
}

static void wait_until_scan_chain_engine_is_idle(void)
{
	// Base address of the scan manager
	volatile int* HPS_SCANMNGR_base_ptr = (int*) 0xFFF02000;
	int status;

	// Loop until scan manager is idle and WFIFO count is 0
	do
	{
		// Read the current value of stat register in scan manager
		status = *HPS_SCANMNGR_base_ptr;
	} while ((status & 0xF0000000) != 0);
}

static void scan_mgr_jtag_insn_data(const unsigned char iarg,
		const unsigned long *data,
		const unsigned int dlen)
{
	int i, j;

	// Send JTAG instruction to initiate process
	scan_mgr_jtag_io(JTAG_BP_INSN | JTAG_BP_2BYTE, iarg, dlen - 1);

	// 32 bits or more remain
	for (i = 0; i < dlen / 32; i++)
	{
		scan_mgr_jtag_io(JTAG_BP_4BYTE, 0x0, data[i]);
	}

	if ((dlen % 32) > 24)	// 31...24 bits remain
	{
		scan_mgr_jtag_io(JTAG_BP_4BYTE, 0x0, data[i]);
	}
	else if (dlen % 32)		// 24...1 bits remain
	{
		for (j = 0; j < dlen % 32; j += 8)
		{
			scan_mgr_jtag_io(0, 0x0, data[i] >> j);
		}
	}

	wait_until_scan_chain_engine_is_idle();
}

static void board_iocsr_config()
{
	// Base address of the scan manager
	volatile int* HPS_SCANMNGR_base_ptr = (int*) 0xFFF02000;
	unsigned int io_scan_chain_len_in_bits;
	unsigned int rem, idx = 0;

	io_scan_chain_len_in_bits = CFG_HPS_IOCSR_SCANCHAIN_LENGTH;

	// Ensure that scan chain engine is inactive before enabling the IO scan chain
	wait_until_scan_chain_engine_is_idle();

	// Enable IO Scan chain 2 (by accessing the en register in scan manager)
	*(HPS_SCANMNGR_base_ptr + 1) |= (1 << IOSCANCHAIN2);

	// Program IO scan chain
	while (io_scan_chain_len_in_bits)
	{
		// We send up to 128 bits at time
		if (io_scan_chain_len_in_bits > 128)
		{
			rem = 128;
		}
		else
		{
			rem = io_scan_chain_len_in_bits;
		}

		scan_mgr_jtag_insn_data(0x0, &iocsr_scan_chain[idx], rem);
		io_scan_chain_len_in_bits -= rem;
		idx += 4;
	}

	// Disable IO Scan chain when configuration done (by accessing the en register in scan manager)
	*(HPS_SCANMNGR_base_ptr + 1) &= ~(1 << IOSCANCHAIN2);
}

static void sysmgr_vioctrl_freeze_req()
{
	// The address of vioctrl register (VIO bank 4) in system manager
	volatile int* HPS_SYSMNGR_vioctrl_ptr = (int*) 0xFFD08040;

	// Freeze request for channel 2 (VIO bank 4)
	*(HPS_SYSMNGR_vioctrl_ptr + 2) &= 0xFFFFFFE0;
}

static void sysmgr_vioctrl_thaw_req()
{
	// The address of vioctrl register (VIO bank 4) in system manager
	volatile int* HPS_SYSMNGR_vioctrl_ptr = (int*) 0xFFD08040;

	// Thaw request for channel 2 (VIO bank 4)
	*(HPS_SYSMNGR_vioctrl_ptr + 2) |= 0x1F;
}

static void board_pinmux_config()
{
	// The address of GPLMUX53 register in system manager
	volatile int* HPS_SYSMNGR_GPLMUX53_ptr = (int*) 0xFFD086A8;
	// The address of GPLMUX54 register in system manager
	volatile int* HPS_SYSMNGR_GPLMUX54_ptr = (int*) 0xFFD086AC;

	// Select GPIO53 to control the pin
	*HPS_SYSMNGR_GPLMUX53_ptr |= 0x01;
	// Select GPIO54 to control the pin
	*HPS_SYSMNGR_GPLMUX54_ptr |= 0x01;
}

static void board_reset_config()
{
	// The address of persmodrst register in reset manager
	volatile int* HPS_RSTMNGR_permodrst_ptr = (int*) 0xFFD05014;

	// Reset watchdog 0 (to avoid persistent board reset)
	*HPS_RSTMNGR_permodrst_ptr |= 1 << L4WD0;
	*HPS_RSTMNGR_permodrst_ptr &= ~(1 << L4WD0);

	// Take the GPIO1 module out of reset state
	*HPS_RSTMNGR_permodrst_ptr &= ~(1 << GPIO1);
}

void board_init()
{
	// Freeze the IO banks (force safe values during configuration)
	sysmgr_vioctrl_freeze_req();
	// Configure IOCSR for bank 7A
	board_iocsr_config();
	// Configure pinmux for GPIO1
	board_pinmux_config();
	// Unfreeze the IO banks so configured IOCSR values take place
	sysmgr_vioctrl_thaw_req();
	// Configure reset manager (reset WD0 and release GPIO1 reset)
	board_reset_config();
}
