#ifndef __CL_SOM_IMX8_COMMON_H__
#define __CL_SOM_IMX8_COMMON_H__

#include <asm/mach-imx/iomux-v3.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <asm/io.h>


int board_early_init_f(void);

#ifdef CONFIG_MXC_SPI
int board_ecspi_init(void);
int board_spi_cs_gpio(unsigned bus, unsigned cs);
#else
static inline int board_ecspi_init(void) { return 0; }
#endif

#define I2C_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE)
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)

#ifdef CONFIG_USB_DWC3

#define USB_PHY_CTRL0			0xF0040
#define USB_PHY_CTRL0_REF_SSP_EN	BIT(2)

#define USB_PHY_CTRL1			0xF0044
#define USB_PHY_CTRL1_RESET		BIT(0)
#define USB_PHY_CTRL1_COMMONONN		BIT(1)
#define USB_PHY_CTRL1_ATERESET		BIT(3)
#define USB_PHY_CTRL1_VDATSRCENB0	BIT(19)
#define USB_PHY_CTRL1_VDATDETENB0	BIT(20)

#define USB_PHY_CTRL2			0xF0048
#define USB_PHY_CTRL2_TXENABLEN0	BIT(8)



int dm_usb_gadget_handle_interrupts(struct udevice *dev);
void dwc3_nxp_usb_phy_init(struct dwc3_device *dwc3);
#endif


#endif /* __CL_SOM_IMX8_COMMON_H__ */