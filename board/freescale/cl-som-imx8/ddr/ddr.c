#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/gpio.h>
#include <asm-generic/gpio.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/gpio.h>
#include "ddr.h"

/* Forward declarations */
#if 0
u32 cl_eeprom_get_ddrinfo(void);
u32 cl_eeprom_set_ddrinfo(u32 ddrinfo);
u32 cl_eeprom_get_subind(void);
u32 cl_eeprom_set_subind(u32 subind);

u32 cl_eeprom_get_drate(unsigned int r, unsigned int c);
u32 cl_eeprom_set_drate(u32 drate, unsigned int r, unsigned int c);

u32 cl_eeprom_get_osize(void);
#endif

extern volatile bool ddrphy_train_pass;

unsigned int lpddr4_mr_read(unsigned int mr_rank, unsigned int mr_addr);

struct lpddr4_desc {
	char name[8];
	unsigned int id;
	unsigned int size;
	u8	subind;
	struct dram_timing_info *timing;
};

struct lpddr4_tcm_desc {
	unsigned int size;
	unsigned int sign;
	unsigned int index;
	unsigned int count;
};

#define DEFAULT (('D' << 24) + ('E' << 16) + ('F' << 8) + 'A')
static const struct lpddr4_desc lpddr4_array_2op[] = {
#if CONFIG_2OP_FULL_SUPPORT
	{ .name = "Samsung",.id = 0x01050008, .size = 1024, .timing = &dram_timing_1g },
	{ .name = "Nanya",  .id = 0x05000008, .size = 1024, .timing = &dram_timing_1g },
	{ .name = "Micron", .id = 0xFF020008, .size = 2048, .timing = &dram_timing_2g },
	{ .name = "Micron", .id = 0xFF000110, .size = 4096, .timing = &dram_timing_ff000110_4g_2op },
	{ .name = "Micron", .id = 0xFF020008, .size = 2048, .timing = &dram_timing_2g },
	{ .name = "Samsung",.id = 0x01050008, .size = 1024, .timing = &dram_timing_1g },
	{ .name = "Nanya",  .id = 0x05000008, .size = 1024, .timing = &dram_timing_1g },
	{ .name = "Nanya", . id = 0x05000010, .size = 2048, .timing = &dram_timing_05_10_2g },
#endif
};

static const struct lpddr4_desc lpddr4_array_3op[] = {
	{ .name = "Kingston", .id = 0xFF000010, .subind = 0x02, .size = 2048, .timing = &dram_timing_ff000010_2g_3op },
	{ .name = "Kingston", .id = 0xFF000010, .subind = 0x04, .size = 4096, .timing = &dram_timing_ff000010_4g_3op },
	{ .name = "Micron",   .id = 0xFF020008, .subind = 0xff, .size = 2048, .timing = &dram_timing_2g_3op },
	{ .name = "Micron",   .id = 0xFF000110, .subind = 0xff, .size = 4096, .timing = &dram_timing_ff000110_4g_3op },
	{ .name = "Samsung",  .id = 0x01050008, .subind = 0xff, .size = 1024, .timing = &dram_timing_1g_3op_low },
	{ .name = "Samsung",  .id = 0x01060008, .subind = 0xff, .size = 1024, .timing = &dram_timing_1g_3op_low },
	{ .name = "Samsung",  .id = 0x01061010, .subind = 0xff, .size = 2048, .timing = &dram_timing_05_10_2g_3op },
	{ .name = "Nanya",    .id = 0x05000008, .subind = 0xff, .size = 1024, .timing = &dram_timing_1g_3op_low },
	{ .name = "Nanya",    .id = 0x05000010, .subind = 0xff,.size = 2048, .timing = &dram_timing_05_10_2g_3op },
	{ .name = "Alien",    .id = 0x52000008, .subind = 0xff, .size = 1024, .timing = &dram_timing_1g_3op_low },
};

static unsigned int lpddr4_get_mr(void)
{
	int i = 0, attempts = 5;
	unsigned int ddr_info = 0;
	unsigned int regs[] = { 5, 6, 7, 8 };

	do {
		for ( i = 0 ; i < ARRAY_SIZE(regs) ; i++ ) {
			unsigned int data = 0;
			data = lpddr4_mr_read(0xF, regs[i]);
			ddr_info <<= 8;
			ddr_info += (data & 0xFF);
		}
		if ((ddr_info != 0xFFFFFFFF) && (ddr_info != 0))
			break; // The attempt was successfull
	} while ( --attempts );
	return	ddr_info;
}

static void spl_tcm_init(struct lpddr4_tcm_desc *lpddr4_tcm_desc) {
    if (lpddr4_tcm_desc->sign == DEFAULT)
	    return;

    lpddr4_tcm_desc->sign = DEFAULT;
    lpddr4_tcm_desc->index = 0;
}

static void spl_tcm_fini(struct lpddr4_tcm_desc *lpddr4_tcm_desc) {
    if (lpddr4_tcm_desc->sign != DEFAULT)
	    return;

    lpddr4_tcm_desc->sign = ~DEFAULT;
    lpddr4_tcm_desc->index = 0;
}

#define SPL_TCM_DATA 0x7e0000
#define SPL_TCM_INIT spl_tcm_init(lpddr4_tcm_desc)
#define SPL_TCM_FINI spl_tcm_fini(lpddr4_tcm_desc)

void spl_dram_init(void)
{
	unsigned int ddr_info = 0xdeadbeef;
	unsigned int ddr_info_mrr = 0xdeadbeef;
	unsigned int ddr_asize = 0;
	unsigned int ddr_found = 0;
	int i = 0;

	const struct lpddr4_desc *lpddr4_array = NULL;
	struct lpddr4_tcm_desc *lpddr4_tcm_desc = (struct lpddr4_tcm_desc *) SPL_TCM_DATA;

	if ((get_cpu_rev() & 0xfff) == CHIP_REV_2_1) {
		lpddr4_array = lpddr4_array_3op;
		ddr_asize = ARRAY_SIZE(lpddr4_array_3op);
	} else {
		lpddr4_array = lpddr4_array_2op;
		ddr_asize = ARRAY_SIZE(lpddr4_array_2op);
	}
#if 0
	if (lpddr4_tcm_desc->sign != DEFAULT) {
		/* get ddr type from the eeprom if not in tcm scan mode */
		ddr_info = cl_eeprom_get_ddrinfo();
		for ( i = 0; i < ddr_asize ; i++ ) {
			if (lpddr4_array[i].id == ddr_info &&
			lpddr4_array[i].subind == cl_eeprom_get_subind()) {
				ddr_found = 1;
				break;
			}
		}
	}
#endif

	/* Walk trought all available ddr ids and apply
	 * one by one. Save the index at the tcm memory that
	 * persists after the reset.
	 */
	if (ddr_found == 0) {

		SPL_TCM_INIT;

		if (lpddr4_tcm_desc->index < ddr_asize) {
			printf("DDRINFO: Cfg attempt: [ %d/%d ]\n", lpddr4_tcm_desc->index+1, ddr_asize);
			i = lpddr4_tcm_desc->index;
			lpddr4_tcm_desc->index += 1;
		} else {
			/* Ran out all available ddr setings */
			printf("DDRINFO: Run out all [ %d ] cfg attempts. A non supported configuration.\n", ddr_asize);
			while ( 1 ) {};
		}
		ddr_info = lpddr4_array[i].id;
	}

	printf("DDRINFO(%s): %s %d GiB @ %d MHz\n", (ddr_found ? "D" : "?" ), lpddr4_array[i].name,
			(lpddr4_array[i].size >> 10), lpddr4_array[i].timing->fsp_table[0]);

	ddr_init(lpddr4_array[i].timing);

	if(ddrphy_train_pass) {
	ddr_info_mrr = lpddr4_get_mr();
		if (ddr_info_mrr == 0xFFFFFFFF || ddr_info_mrr == 0) {
			printf("DDRINFO(M): mr5-8 [ 0x%x ] is invalid; reset\n", ddr_info_mrr);
			ddrphy_train_pass = false;
		}
	}
	else
		printf("DDR PHY training failed\n");

	if (!ddrphy_train_pass) {
		SPL_TCM_INIT;
		#if 0
		do_reset(NULL,0,0,NULL);
		#endif
	}

	printf("DDRINFO(mr5-8 ): mr5-8 [ 0x%x ]\n", ddr_info_mrr);
	printf("DDRINFO(%s): mr5-8 [ 0x%x ]\n", (ddr_found ? "EEPROM" : "TCM   " ), ddr_info);

	if (ddr_info_mrr != ddr_info) {
		SPL_TCM_INIT;
		#if 0
		do_reset(NULL,0,0,NULL);
		#endif
	}

	SPL_TCM_FINI;

#if 0
	if (ddr_found == 0) {
		/* Update eeprom */
		cl_eeprom_set_ddrinfo(ddr_info_mrr);
		mdelay(10);
		ddr_info = cl_eeprom_get_ddrinfo();
		mdelay(10);
		cl_eeprom_set_subind(lpddr4_array[i].subind);
		/* make sure that the ddr_info has reached the eeprom */
		printf("DDRINFO(EEPROM): mr5-8 [ 0x%.8x ], read back\n", ddr_info);
		if (ddr_info_mrr != ddr_info || cl_eeprom_get_subind() != lpddr4_array[i].subind) {
			printf("DDRINFO(EEPROM): make sure that the eeprom is accessible\n");
			printf("DDRINFO(EEPROM): i2c dev 1; i2c md 0x51 0x40 0x50\n");
		}
	}

	/* Pass the dram size to th U-Boot through the tcm memory */
	{ /* To figure out what to store into the TCM buffer */
	  /* For debug purpouse only. To override the real memsize */
		unsigned int ddr_tcm_size = cl_eeprom_get_osize();
		if ((ddr_tcm_size == 0) || (ddr_tcm_size == -1))
			ddr_tcm_size = lpddr4_array[i].size;

		lpddr4_tcm_desc->size = ddr_tcm_size;
	}
#endif
}
