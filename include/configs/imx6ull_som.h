/*
 * Copyright (C) 2017-2020 Emcraft Systems
 *
 * Configuration settings for the Emcraft i.MX 6ULL System-On-Module.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef __IMX6ULL_SOM_CONFIG_H
#define __IMX6ULL_SOM_CONFIG_H

#include "mx6_common.h"

#ifdef CONFIG_IMX_HAB
#ifndef CONFIG_CSF_SIZE
#define CONFIG_CSF_SIZE			0x4000
#endif
#endif

#define CONFIG_SYS_MALLOC_LEN		(16 * SZ_1M)

#define CONFIG_MXC_UART_BASE		UART1_BASE

#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR
#define CONFIG_SYS_FSL_USDHC_NUM	1

#define CONFIG_FEC_ENET_DEV		0
#define CONFIG_FEC_XCV_TYPE             RMII
#define CONFIG_ETHPRIME                 "FEC"
#if (CONFIG_FEC_ENET_DEV == 0)
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR          5
#elif (CONFIG_FEC_ENET_DEV == 1)
#define IMX_FEC_BASE			ENET2_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR          1
#endif /* CONFIG_FEC_ENET_DEV */

#define CONFIG_BOOTCOMMAND \
    "nboot ${loadaddr} 0 ${kernel_offset}" \
    " && nand read ${dtb_addr} ${dtb_offset} ${dtb_size}" \
    " && acmeboot; reset"

#define SPLASH_FLASH_BASE		0x400000
#define SPLASH_PART_SIZE		0xa00000

#define DTB_PART_SIZE			0x20000
#define KERNEL_PART_SIZE		0xc00000
#define ROOTFS_PART_SIZE		0xa000000

# define DTB_FLASH_BASE			0xe00000	/* (SPLASH_FLASH_BASE + SPLASH_PART_SIZE) */
# define KERNEL_FLASH_BASE		0xe20000	/* (DTB_FLASH_BASE + DTB_PART_SIZE) */
# define ROOTFS_FLASH_BASE		0x1a20000	/* (KERNEL_FLASH_BASE + KERNEL_PART_SIZE) */

#define REV_EXTRA_ENV							\
	"nandboot=nboot ${loadaddr} 0 ${kernel_offset}" \
	" && nand read ${dtb_addr} ${dtb_offset} ${dtb_size}"		\
	" && run args && run boot_dtb\0"				\
									\
	"boot_dtb=bootm ${loadaddr} - ${dtb_addr}\0"			\
									\
	"verify=yes\0"							\
	"splash_offset="	__stringify(SPLASH_FLASH_BASE) "\0"	\
	"dtb_offset="		__stringify(DTB_FLASH_BASE) "\0"	\
	"kernel_offset="	__stringify(KERNEL_FLASH_BASE) "\0"	\
	"rootfs_offset="	__stringify(ROOTFS_FLASH_BASE) "\0"	\
	"splash_size="		__stringify(SPLASH_PART_SIZE) "\0" 	\
	"dtb_size="		__stringify(DTB_PART_SIZE) "\0"		\
	"kernel_size="		__stringify(KERNEL_PART_SIZE) "\0"	\
	"rootfs_size="		__stringify(ROOTFS_PART_SIZE) "\0"	\

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"autoload=yes\0"						\
	"image=imx/imx6ull-som/rootfs.uImage\0"				\
	"dtbimage=imx/imx6ull-som/rootfs.dtb\0"				\
	"rootfsimage=imx/imx6ull-som/rootfs.ubi\0"			\
	"splashimage=imx/imx6ull-som/splash.rgb\0"			\
	"args=setenv bootargs quiet${ubirfs} quiet\0"					\
									\
	"ubirfs=rootwait=1 rw ubi.mtd=4,2048 rootfstype=ubifs"		\
	" root=ubi0:rootfs ubi.fm_autoconvert=1\0"			\
									\
	"dtb_addr=0x83000000\0"						\
	REV_EXTRA_ENV

/* Miscellaneous configurable options */

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		MXS_GPMI_BASE

#endif /* __IMX6ULL_SOM_CONFIG_H */
