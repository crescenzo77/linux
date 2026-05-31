// SPDX-License-Identifier: GPL-2.0
/*
 * Minimal Allwinner A733 CCU support.
 *
 * This intentionally exposes only the clocks and resets needed by the first
 * Cubie A7S UART0/SDMMC0 DTS skeleton. The full parent tree still needs to be
 * implemented from a reviewed A733 clock/reset map.
 */

#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include "../clk.h"

#include "ccu_common.h"
#include "ccu_gate.h"
#include "ccu_reset.h"

#include "ccu-sun60i-a733.h"

static CLK_FIXED_FACTOR_FW_NAME(apb1_clk, "apb1", "hosc", 1, 1, 0);
static CLK_FIXED_FACTOR_HW(apb_uart_clk, "apb-uart", &apb1_clk.hw, 1, 1, 0);

static const struct clk_hw *apb_uart_hws[] = {
	&apb_uart_clk.hw
};

static const struct clk_parent_data hosc[] = {
	{ .fw_name = "hosc" }
};

static SUNXI_CCU_GATE_DATA(mmc0_clk, "mmc0", hosc, 0xd00, BIT(31), 0);
static SUNXI_CCU_GATE_DATA(bus_mmc0_clk, "bus-mmc0", hosc, 0xd0c, BIT(0), 0);
static SUNXI_CCU_GATE_HWS(bus_uart0_clk, "bus-uart0", apb_uart_hws,
			  0xe00, BIT(0), 0);

static struct ccu_common *sun60i_a733_ccu_clks[] = {
	&mmc0_clk.common,
	&bus_mmc0_clk.common,
	&bus_uart0_clk.common,
};

static struct clk_hw_onecell_data sun60i_a733_hw_clks = {
	.hws	= {
		[CLK_APB1]	= &apb1_clk.hw,
		[CLK_APB_UART]	= &apb_uart_clk.hw,
		[CLK_MMC0]	= &mmc0_clk.common.hw,
		[CLK_BUS_MMC0]	= &bus_mmc0_clk.common.hw,
		[CLK_BUS_UART0]	= &bus_uart0_clk.common.hw,
	},
	.num	= CLK_NUMBER,
};

static struct ccu_reset_map sun60i_a733_ccu_resets[] = {
	[RST_BUS_MMC0]	= { 0xd0c, BIT(16) },
	[RST_BUS_UART0]	= { 0xe00, BIT(16) },
};

static const struct sunxi_ccu_desc sun60i_a733_ccu_desc = {
	.ccu_clks	= sun60i_a733_ccu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sun60i_a733_ccu_clks),

	.hw_clks	= &sun60i_a733_hw_clks,

	.resets		= sun60i_a733_ccu_resets,
	.num_resets	= ARRAY_SIZE(sun60i_a733_ccu_resets),
};

static int sun60i_a733_ccu_probe(struct platform_device *pdev)
{
	void __iomem *reg;

	reg = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(reg))
		return PTR_ERR(reg);

	return devm_sunxi_ccu_probe(&pdev->dev, reg, &sun60i_a733_ccu_desc);
}

static const struct of_device_id sun60i_a733_ccu_ids[] = {
	{ .compatible = "allwinner,sun60i-a733-ccu" },
	{ }
};
MODULE_DEVICE_TABLE(of, sun60i_a733_ccu_ids);

static struct platform_driver sun60i_a733_ccu_driver = {
	.probe	= sun60i_a733_ccu_probe,
	.driver	= {
		.name			= "sun60i-a733-ccu",
		.suppress_bind_attrs	= true,
		.of_match_table		= sun60i_a733_ccu_ids,
	},
};
module_platform_driver(sun60i_a733_ccu_driver);

MODULE_IMPORT_NS("SUNXI_CCU");
MODULE_DESCRIPTION("Minimal support for the Allwinner A733 CCU");
MODULE_LICENSE("GPL");
