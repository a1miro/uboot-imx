/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DDR_H__
#define __DDR_H__

extern struct dram_timing_info dram_timing_2g;
extern struct dram_timing_info dram_timing_2g_3op;
extern struct dram_timing_info dram_timing_1g;
extern struct dram_timing_info dram_timing_1g_3op;
extern struct dram_timing_info dram_timing_1g_3op_low;
extern struct dram_timing_info dram_timing_05_10_2g;
extern struct dram_timing_info dram_timing_05_10_2g_3op;
extern struct dram_timing_info dram_timing_ff000110_4g_2op;
extern struct dram_timing_info dram_timing_ff000110_4g_3op;
extern struct dram_timing_info dram_timing_ff000010_2g_3op;
extern struct dram_timing_info dram_timing_ff000010_4g_3op;
//void ddr_init(struct dram_timing_info *dram_timing);
//void spl_dram_init(void);

#endif
