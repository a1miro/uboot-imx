#include "common.h"
#include <asm/arch/clock.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm-generic/gpio.h>
#include <asm/mach-imx/gpio.h>
#include <usb.h>
#include <dwc3-uboot.h>

void set_wdog_reset(struct wdog_regs *wdog);

#ifdef CONFIG_MXC_SPI

#define ECSPI_PAD_CTRL	(PAD_CTL_DSE2 | PAD_CTL_HYS)
#define CL_SOM_IMX8_ECSPI_BUS0_CS0	IMX_GPIO_NR(5, 9)

static iomux_v3_cfg_t const ecspi_pads[] = {
	IMX8MQ_PAD_ECSPI1_SCLK__ECSPI1_SCLK  | MUX_PAD_CTRL(ECSPI_PAD_CTRL),
	IMX8MQ_PAD_ECSPI1_MOSI__ECSPI1_MOSI  | MUX_PAD_CTRL(ECSPI_PAD_CTRL),
	IMX8MQ_PAD_ECSPI1_MISO__ECSPI1_MISO  | MUX_PAD_CTRL(ECSPI_PAD_CTRL),
	IMX8MQ_PAD_ECSPI1_SS0__GPIO5_IO9 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

int board_ecspi_init(void)
{
	imx_iomux_v3_setup_multiple_pads(ecspi_pads, ARRAY_SIZE(ecspi_pads));
	gpio_request(CL_SOM_IMX8_ECSPI_BUS0_CS0, "ECSPI_BUS0_CS0");
	/* CS must be up */
	/* mxc_spi driver drives it low each time it issues a transaction */
	gpio_direction_output(CL_SOM_IMX8_ECSPI_BUS0_CS0, 1);
	set_clk_ecspi();
	return 0;
}

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (CL_SOM_IMX8_ECSPI_BUS0_CS0);
}
#endif

#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE)
#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)

static iomux_v3_cfg_t const wdog_pads[] = {
	IMX8MQ_PAD_GPIO1_IO02__WDOG1_WDOG_B | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

static iomux_v3_cfg_t const uart3_pads[] = {
	IMX8MQ_PAD_UART3_RXD__UART3_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART3_TXD__UART3_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const uart1_pads[] = {
	IMX8MQ_PAD_UART1_RXD__UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART1_TXD__UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	imx_iomux_v3_setup_multiple_pads(uart3_pads, ARRAY_SIZE(uart3_pads));
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));

	return 0;
}

#ifdef CONFIG_USB_DWC3

int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	dwc3_uboot_handle_interrupt(dev);
	return 0;
}

void dwc3_nxp_usb_phy_init(struct dwc3_device *dwc3)
{
	u32 RegData;

	RegData = readl(dwc3->base + USB_PHY_CTRL1);
	RegData &= ~(USB_PHY_CTRL1_VDATSRCENB0 | USB_PHY_CTRL1_VDATDETENB0 |
			USB_PHY_CTRL1_COMMONONN);
	RegData |= USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET;
	writel(RegData, dwc3->base + USB_PHY_CTRL1);

	RegData = readl(dwc3->base + USB_PHY_CTRL0);
	RegData |= USB_PHY_CTRL0_REF_SSP_EN;
	writel(RegData, dwc3->base + USB_PHY_CTRL0);

	RegData = readl(dwc3->base + USB_PHY_CTRL2);
	RegData |= USB_PHY_CTRL2_TXENABLEN0;
	writel(RegData, dwc3->base + USB_PHY_CTRL2);

	RegData = readl(dwc3->base + USB_PHY_CTRL1);
	RegData &= ~(USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET);
	writel(RegData, dwc3->base + USB_PHY_CTRL1);
}
#endif
