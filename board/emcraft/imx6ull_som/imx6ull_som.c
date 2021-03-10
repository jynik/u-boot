/*
 * Copyright (C) 2017 Emcraft Systems
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <common.h>
#include <fsl_esdhc.h>
#include <linux/sizes.h>
#include <mmc.h>
#include <phy.h>

// TODO: Evaluate whether mdelay()'s can be removed/replaced
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

/* MMDC P0/P1 Registers */
struct mmdc_p_regs {
	u32 mdctl;
	u32 mdpdc;
	u32 mdotc;
	u32 mdcfg0;
	u32 mdcfg1;
	u32 mdcfg2;
	u32 mdmisc;
	u32 mdscr;
	u32 mdref;
	u32 res1[2];
	u32 mdrwd;
	u32 mdor;
	u32 mdmrr;
	u32 mdcfg3lp;
	u32 mdmr4;
	u32 mdasp;
	u32 res2[239];
	u32 maarcr;
	u32 mapsr;
	u32 maexidr0;
	u32 maexidr1;
	u32 madpcr0;
	u32 madpcr1;
	u32 madpsr0;
	u32 madpsr1;
	u32 madpsr2;
	u32 madpsr3;
	u32 madpsr4;
	u32 madpsr5;
	u32 masbs0;
	u32 masbs1;
	u32 res3[2];
	u32 magenp;
	u32 res4[239];
	u32 mpzqhwctrl;
	u32 mpzqswctrl;
	u32 mpwlgcr;
	u32 mpwldectrl0;
	u32 mpwldectrl1;
	u32 mpwldlst;
	u32 mpodtctrl;
	u32 mprddqby0dl;
	u32 mprddqby1dl;
	u32 mprddqby2dl;
	u32 mprddqby3dl;
	u32 mpwrdqby0dl;
	u32 mpwrdqby1dl;
	u32 mpwrdqby2dl;
	u32 mpwrdqby3dl;
	u32 mpdgctrl0;
	u32 mpdgctrl1;
	u32 mpdgdlst0;
	u32 mprddlctl;
	u32 mprddlst;
	u32 mpwrdlctl;
	u32 mpwrdlst;
	u32 mpsdctrl;
	u32 mpzqlp2ctl;
	u32 mprddlhwctl;
	u32 mpwrdlhwctl;
	u32 mprddlhwst0;
	u32 mprddlhwst1;
	u32 mpwrdlhwst0;
	u32 mpwrdlhwst1;
	u32 mpwlhwerr;
	u32 mpdghwst0;
	u32 mpdghwst1;
	u32 mpdghwst2;
	u32 mpdghwst3;
	u32 mppdcmpr1;
	u32 mppdcmpr2;
	u32 mpswdar0;
	u32 mpswdrdr0;
	u32 mpswdrdr1;
	u32 mpswdrdr2;
	u32 mpswdrdr3;
	u32 mpswdrdr4;
	u32 mpswdrdr5;
	u32 mpswdrdr6;
	u32 mpswdrdr7;
	u32 mpmur0;
	u32 mpwrcadl;
	u32 mpdccr;
};

int dram_init(void)
{
	struct mmdc_p_regs *mmdc0 =
		(struct mmdc_p_regs *)MMDC_P0_BASE_ADDR;
	long mdctl;

	/*
	 * One of the following DDR devices can be installed:
	 *   MT41K128M16JT-107:K (256 MB)
	 *   MT41K256M16TW-107 IT:P (512 MB)
	 */
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM, 512 * 1024 * 1024);

	if (gd->ram_size == (256 * 1024 * 1024)) {
		/*
		 * Decrease Row to 14 bits.
		 * We cannot place MMDC into configuration mode
		 * since we are running from RAM.
		 */
		mdctl = readl(&mmdc0->mdctl);
		mdctl &= 0xf8ffffff;
		mdctl |= (14 - 11) << 24;
		writel(mdctl, &mmdc0->mdctl);

	} else if (gd->ram_size == (512 * 1024 * 1024)) {
		/* Do nothing: already configured in imximage.cfg */

	} else {
		printf("Auto DDR size detection failed! Detected %li MiB\n",
			gd->ram_size / 1024 / 1024);
	}

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int mmc_map_to_kernel_blk(int devno)
{
	return devno;
}

int board_early_init_f(void)
{
	struct mxc_ccm_reg *imx_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	u32 cacrr = __raw_readl(&imx_ccm->cacrr);
	u32 pll1 = __raw_readl(&imx_ccm->analog_pll_sys);
	/* PLL1 (ARM PLL) Fout = Fin * DIV_SELECT / 2, Fin = 24MHz */
	u32 div = get_cpu_speed_grade_hz() * 2 / MXC_HCLK;

	/* Set CACRR[ARM_PODF] divider to value 0: "divide by 1",
	 * making ARM_CLK_ROOT equal to PLL1.
	 */
	cacrr &= ~(MXC_CCM_CACRR_ARM_PODF_MASK << MXC_CCM_CACRR_ARM_PODF_OFFSET);
	__raw_writel(cacrr, &imx_ccm->cacrr);

	pll1 &= ~BM_ANADIG_PLL_SYS_DIV_SELECT;
	pll1 |= (div & BM_ANADIG_PLL_SYS_DIV_SELECT);
	__raw_writel(pll1, &imx_ccm->analog_pll_sys);

	timer_init();

	// ENET1_RX1, aka GPIO2_IO01, aka PHYAD2
	// Pull-down, open drain
	writel(5, 0x20e00c8);
	writel(0x18a0, 0x20e0354);
	writel(0x2, 0x20a0004);
	writel(0, 0x20a0000);

	// GPIO5_IO04, aka 3V3_P+5V_P
	writel(5, 0x2290018);
	writel(0x10, 0x20AC004);
	writel(0xb0a0, 0x229005C);

	// GPIO3_IO04, aka nWDOG_OUT
	writel(5, 0x20e0114);
	writel(0x10, 0x20A4004);
	writel(0xf8a0, 0x20E03A0);

	// GPIO5_IO04, aka 3V3_P+5V_P -> off
	writel(0, 0x20AC000);
	// GPIO3_IO04, aka nWDOG_OUT -> ASSRT
	writel(0, 0x20A4000);

	mdelay(100);

	// ENET1_RX1, aka GPIO2_IO01, aka PHYAD2
	// Configure as Pull-up
	writel(0x80a0, 0x20e0354);
#if (CONFIG_FEC_ENET_DEV == 0)
	writel((CONFIG_FEC_MXC_PHYADDR == 5) ? 0x2 : 0, 0x20a0000);
#endif

	// GPIO5_IO04, aka 3V3_P+5V_P -> on
	writel(0x10, 0x20AC000);

	// GPIO3_IO04, aka nWDOG_OUT -> OK
	writel(0x10, 0x20A4000);

	mdelay(50);

	// ENET1_RX1, aka GPIO2_IO01, aka PHYAD2
	writel(0, 0x20a0004);
	writel(0, 0x20a0000);
	writel(0, 0x20e00c8);
	writel((3 << 6) | (1 << 13) | (1 << 12) | (3 << 14), 0x20e0354);

	mdelay(50);

	return 0;
}

static int board_init_fec(void)
{
#if defined(CONFIG_FEC_ENET_DEV)
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int ret = 0;

	#if (CONFIG_FEC_ENET_DEV == 0)
	clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC1_MASK, IOMUX_GPR1_FEC1_CLOCK_MUX1_SEL_MASK);
	ret = enable_fec_anatop_clock(CONFIG_FEC_ENET_DEV, ENET_50MHZ);
	#else
	clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC2_MASK, 0);
	#endif

	if (ret)
		return ret;

	enable_enet_clk(true);
#endif /* CONFIG_FEC_ENET_DEV */

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	#if (CONFIG_FEC_ENET_DEV == 0)
	phy_write(phydev, CONFIG_FEC_MXC_PHYADDR, 0x1f, 0x8180);
	#endif

	if (phydev->drv->config)
		return phydev->drv->config(phydev);

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	board_init_fec();

	return 0;
}

u32 get_board_rev(void)
{
	return get_cpu_rev();
}
