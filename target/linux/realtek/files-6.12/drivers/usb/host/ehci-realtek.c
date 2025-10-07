#include <linux/err.h>
#include <linux/signal.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>

#ifndef REG32
#define REG32(reg)		(*(volatile unsigned int   *)(reg))
#endif

#define USB_PHY_UPCR						(0xa0000000 | 0x18000500)
#define USB_PHY_UPCR_VSTATUS_IN_MASK		0x000000ff

#define USB_PHY_EHCI_INSNREG05_UTMI			(0xa0000000 | 0x180210a4)					/* This is Little Endian format */
#define EHCI_INSNREG05_UTMI_VCTL_SHIFT		24		 									/* This is Little Endian format */
#define EHCI_INSNREG05_UTMI_VCTL_MASK		(0xf << EHCI_INSNREG05_UTMI_VCTL_SHIFT)		/* This is Little Endian format */
// 0F000000 -> 0000000F 0
#define EHCI_INSNREG05_UTMI_VCLM_SHIFT		16											/* This is Little Endian format */
#define EHCI_INSNREG05_UTMI_VCLM_MASK		(0x1 << EHCI_INSNREG05_UTMI_VCLM_SHIFT)		/* This is Little Endian format */
// 00010000 -> 00000100 8

#define USB_PHY_FM							(0xa0000000 | 0x18140210)
#define USB_PHY_FM_R_FDISCON_SHIFT						5								/* This is Big	Endian format */
#define USB_PHY_FM_R_FDISCON_MASK					(0x1 << USB_PHY_FM_R_FDISCON_SHIFT)		/* This is Big	Endian format */

#if 1
static int ehci_realtek_ehci_setup(struct usb_hcd *hcd);
#endif

static const struct hc_driver ehci_realtek_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "Realtek On-Chip EHCI Host Controller",
	.hcd_priv_size		= sizeof(struct ehci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq			= ehci_irq,
	.flags			= HCD_MEMORY | HCD_USB2 | HCD_BH,

	/*
	 * basic lifecycle operations
	 */
#if 1
	.reset			= ehci_realtek_ehci_setup,
#else
	.reset			= ehci_setup,
#endif
	.start			= ehci_run,
	.stop			= ehci_stop,
	.shutdown		= ehci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue		= ehci_urb_enqueue,
	.urb_dequeue		= ehci_urb_dequeue,
	.endpoint_disable	= ehci_endpoint_disable,
	.endpoint_reset		= ehci_endpoint_reset,

	/*
	 * scheduling support
	 */
	.get_frame_number	= ehci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
#ifdef	CONFIG_PM
	.bus_suspend		= ehci_bus_suspend,
	.bus_resume		= ehci_bus_resume,
#endif
	.relinquish_port	= ehci_relinquish_port,
	.port_handed_over	= ehci_port_handed_over,

	.clear_tt_buffer_complete	= ehci_clear_tt_buffer_complete,
};

#if 1
static int ehci_realtek_ehci_setup(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int retval;

	ehci->regs = (void __iomem *)ehci->caps +
	    HC_LENGTH(ehci, ehci_readl(ehci, &ehci->caps->hc_capbase));
	dbg_hcs_params(ehci, "reset");
	dbg_hcc_params(ehci, "reset");

	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);

	ehci->sbrn = HCD_USB2;

	ehci_reset(ehci);

	/* data structure init */
	retval = ehci_init(hcd);
	if (retval)
		return retval;

	return 0;
}
#endif

static void usb_host_discon_config(void)
{
	uint reg_val = 0;

	/*Do not By pass phy detect.*/
	reg_val = REG32(USB_PHY_FM);
	reg_val &= ~(USB_PHY_FM_R_FDISCON_MASK);
	REG32(USB_PHY_FM) = reg_val;
}

typedef struct usb_phy_conf_s
{
	unsigned int				reg;
	unsigned int				value;
} usb_phy_conf_t;

#if defined(CONFIG_RTL930X)
usb_phy_conf_t rtl9300_usbPhy_config_parameter[] = {
	{ 0xf4, 0x9b },
	{ 0xe5, 0xf2 }
};
#elif defined(CONFIG_RTL931X)
usb_phy_conf_t usbPhy_cali_flow[] = {
	{ 0xf4, 0x9b },
	{ 0xe4, 0x6c },
	{ 0xe7, 0x81 },
	{ 0xf4, 0xbb },
	{ 0xe0, 0x21 },
	{ 0xe0, 0x25 }
};

usb_phy_conf_t rtl9310_usbPhy_config_parameter[] = {
	{ 0xf4, 0x9b },
	{ 0xe0, 0x92 },
	{ 0xe1, 0x30 },
	{ 0xe2, 0xda },
	{ 0xe3, 0x4d },
	{ 0xe4, 0x84 },
	{ 0xe5, 0x65 },
	{ 0xe6, 0x01 },
	{ 0xe7, 0xa1 },
	{ 0xf0, 0xfc },
	{ 0xf1, 0x8c },
	{ 0xf2, 0x00 },
	{ 0xf3, 0x11 },
	{ 0xf4, 0x9b },
	{ 0xf5, 0x95 },
	{ 0xf6, 0x00 },
	{ 0xf7, 0x0A },
	{ 0xf4, 0xbb },
	{ 0xe0, 0x25 },
	{ 0xe1, 0xef },
	{ 0xe2, 0x60 },
	{ 0xe3, 0x00 },
	{ 0xe4, 0x00 },
	{ 0xe5, 0x11 },
	{ 0xe6, 0x06 },
	{ 0xe7, 0x66 }
};
#endif

static void usb_phy_reg_write(unsigned int reg, unsigned int value)
{
	unsigned int	ctrl_reg_value, data_reg_value;

	data_reg_value = REG32(USB_PHY_UPCR);
	printk("\n[0]data_reg_value = 0x%08x 0x%08x\n", data_reg_value, ehci_readl(ehci, USB_PHY_UPCR));
	data_reg_value &= ~(USB_PHY_UPCR_VSTATUS_IN_MASK);
	data_reg_value |= (value & USB_PHY_UPCR_VSTATUS_IN_MASK);						/* Need to reserve [31:8]*/
	REG32(USB_PHY_UPCR) = data_reg_value;								/*Setup Write Data*/
	printk("\n[1]data_reg_value = 0x%08x 0x%08x\n", data_reg_value, ehci_readl(ehci, USB_PHY_UPCR));

	ctrl_reg_value = 0x00200000;										/*USB Port number : 1; (We used PHY Port 0 only)*/

	// 0x00002000 bit 21/13

	/* Set low nibble of Reg addres */
	/* VCTL 1->0->1, Send address Nibble to USB PHY*/
	ctrl_reg_value |= ((reg << EHCI_INSNREG05_UTMI_VCTL_SHIFT) & EHCI_INSNREG05_UTMI_VCTL_MASK);
	ctrl_reg_value |= EHCI_INSNREG05_UTMI_VCLM_MASK;							  /*Set VCTL to HIGH*/
	printk("[1]ctrl_reg_value = 0x%08x\n",ctrl_reg_value);
	REG32(USB_PHY_EHCI_INSNREG05_UTMI) = ctrl_reg_value;

	udelay(1);

	ctrl_reg_value &= ~(EHCI_INSNREG05_UTMI_VCLM_MASK);						   /*Set VCTL to LOW*/
	printk("[2]ctrl_reg_value = 0x%08x\n",ctrl_reg_value);
	REG32(USB_PHY_EHCI_INSNREG05_UTMI) = ctrl_reg_value;

	udelay(1);

	ctrl_reg_value |= EHCI_INSNREG05_UTMI_VCLM_MASK;							  /*Set VCTL to HIGH*/
	printk("[3]ctrl_reg_value = 0x%08x\n",ctrl_reg_value);
	REG32(USB_PHY_EHCI_INSNREG05_UTMI) = ctrl_reg_value;

	udelay(1);

	/* Set high nibble of Reg addres */
	/* VCTL 1->0->1, Send address Nibble to USB PHY*/
	ctrl_reg_value = 0x00200000;
	ctrl_reg_value |= ((reg << (EHCI_INSNREG05_UTMI_VCTL_SHIFT - 4)) & EHCI_INSNREG05_UTMI_VCTL_MASK);
	ctrl_reg_value |= EHCI_INSNREG05_UTMI_VCLM_MASK;							  /*Set VCTL to HIGH*/
	printk("[4]ctrl_reg_value = 0x%08x\n",ctrl_reg_value);
	REG32(USB_PHY_EHCI_INSNREG05_UTMI) = ctrl_reg_value;

	udelay(1);

	ctrl_reg_value &= ~(EHCI_INSNREG05_UTMI_VCLM_MASK);						   /*Set VCTL to LOW*/
	printk("[5]ctrl_reg_value = 0x%08x\n",ctrl_reg_value);
	REG32(USB_PHY_EHCI_INSNREG05_UTMI) = ctrl_reg_value;

	udelay(1);

	ctrl_reg_value |= EHCI_INSNREG05_UTMI_VCLM_MASK;							  /*Set VCTL to HIGH*/
	printk("[6]ctrl_reg_value = 0x%08x\n",ctrl_reg_value);
	REG32(USB_PHY_EHCI_INSNREG05_UTMI) = ctrl_reg_value;

	udelay(1);
}

static void usb_phy_configure_process(void)
{
	unsigned int loop_idx;

#if defined(CONFIG_RTL930X)
	/* USB 9300 PHY parameter setup*/
	for (loop_idx = 0; loop_idx < (sizeof(rtl9300_usbPhy_config_parameter)/sizeof(usb_phy_conf_t)); loop_idx++)
		usb_phy_reg_write(rtl9300_usbPhy_config_parameter[loop_idx].reg, rtl9300_usbPhy_config_parameter[loop_idx].value);
#elif defined(CONFIG_RTL931X)
	/* USB Calibariton*/
	for (loop_idx = 0; loop_idx < (sizeof(usbPhy_cali_flow)/sizeof(usb_phy_conf_t)); loop_idx++)
		usb_phy_reg_write(usbPhy_cali_flow[loop_idx].reg, usbPhy_cali_flow[loop_idx].value);

	/* USB 9310 PHY parameter setup*/
	for (loop_idx = 0; loop_idx < (sizeof(rtl9310_usbPhy_config_parameter)/sizeof(usb_phy_conf_t)); loop_idx++)
		usb_phy_reg_write(rtl9310_usbPhy_config_parameter[loop_idx].reg, rtl9310_usbPhy_config_parameter[loop_idx].value);
#endif
}

static int ehci_realtek_probe(struct platform_device *pdev)
{
	struct device_node *dn = pdev->dev.of_node;
	struct usb_hcd *hcd;
	struct ehci_hcd	*ehci = NULL;
	struct resource res;
	int irq;
	int ret;

	if (usb_disabled())
		return -ENODEV;

	dev_dbg(&pdev->dev, "initializing RTL-SOC USB Controller\n");

	usb_host_discon_config();
	usb_phy_configure_process();

	ret = of_address_to_resource(dn, 0, &res);
	if (ret)
		return ret;

	hcd = usb_create_hcd(&ehci_realtek_hc_driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd)
		return -ENOMEM;

	hcd->rsrc_start = res.start;
	hcd->rsrc_len = resource_size(&res);

	irq = irq_of_parse_and_map(dn, 0);
	if (!irq) {
		dev_err(&pdev->dev, "%s: irq_of_parse_and_map failed\n",
			__FILE__);
		ret = -EBUSY;
		goto err_irq;
	}

	hcd->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(hcd->regs)) {
		dev_err(&pdev->dev, "%s: ioremap failed\n",
			__FILE__);
		ret = PTR_ERR(hcd->regs);
		goto err_ioremap;
	}

	ehci = hcd_to_ehci(hcd);

	ehci->big_endian_desc = 1;

	ehci->caps = hcd->regs;

	ret = usb_add_hcd(hcd, irq, 0);
	if (ret)
		goto err_ioremap;

	// device_wakeup_enable(hcd->self.controller);

	platform_set_drvdata(pdev, hcd);

	return 0;

err_ioremap:
	irq_dispose_mapping(irq);
err_irq:
	usb_put_hcd(hcd);
	return ret;
}

static void ehci_realtek_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	dev_dbg(&pdev->dev, "stopping RTL-SOC USB Controller\n");

	usb_remove_hcd(hcd);

	irq_dispose_mapping(hcd->irq);

	usb_put_hcd(hcd);
}

static const struct of_device_id ehci_realtek_of_match[] = {
	{
		.compatible = "realtek,otto-ehci",
	},
	{
		.compatible = "realtek,rtl9300-ehci",
	},
	{
		.compatible = "realtek,rtl9310-ehci",
	},
	{}
};
MODULE_DEVICE_TABLE(of, ehci_realtek_of_match);

static struct platform_driver ehci_realtek_driver = {
	.probe		= ehci_realtek_probe,
	.remove_new	= ehci_realtek_remove,
	.shutdown	= usb_hcd_platform_shutdown,
	.driver = {
		.name = "realtek-otto-ehci",
		.of_match_table	= ehci_realtek_of_match,
	},
};


