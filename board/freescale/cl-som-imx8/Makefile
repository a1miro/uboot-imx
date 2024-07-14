#
# Copyright 2017 NXP
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += cl-som-imx8.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o

#ifdef CONFIG_IMX8MQ_4GB_DDR_TIMING
#obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing_4g.o
#else
#obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing_b0.o lpddr4_timing.o
#endif

#ifdef CONFIG_2OP_FULL_SUPPORT
#obj-y += ddr/lpddr4_timing_05_10_2g.o
#obj-y += ddr/lpddr4_timing_ff000110_4g_2op.o
#obj-y += ddr/lpddr4_timing_1g.o
#obj-y += ddr/lpddr4_timing_2g.o
#endif
obj-y += ddr/lpddr4_timing_05_10_2g_3op.o
#obj-n += ddr/lpddr4_timing_1g_3op.o
#obj-y += ddr/lpddr4_timing_1g_3op_low.o
#obj-y += ddr/lpddr4_timing_2g_3op.o
#obj-y += ddr/lpddr4_timing_ff000110_4g_3op.o
#obj-y += ddr/lpddr4_timing_ff000010_2g_3op.o
#obj-y += ddr/lpddr4_timing_ff000010_4g_3op.o
#obj-y += ddr/ddr.o

endif
