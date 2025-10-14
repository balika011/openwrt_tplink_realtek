/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#ifndef __RTK_PHYLIB_H
#define __RTK_PHYLIB_H

#include <linux/types.h>
#include "phy_patch.h"

typedef enum rtk_phylib_phy_e
{
    RTK_PHYLIB_NONE,
    RTK_PHYLIB_RTL8251L,
    RTK_PHYLIB_RTL8254B,
    RTK_PHYLIB_RTL8261N,
    RTK_PHYLIB_RTL8261BE,
    RTK_PHYLIB_RTL8264,
    RTK_PHYLIB_RTL8264B,
} rtk_phylib_phy_t;

struct rtk_phy_priv {
    rtk_phylib_phy_t phytype;
    rt_phy_patch_db_t *patch;

    bool isBasePort:1;
    bool pnswap_tx:1;
    bool pnswap_rx:1;
    bool rtk_serdes_patch:1;
};

typedef enum rtk_phy_intr_e
{
    RTK_PHY_INTR_NEXT_PAGE_RECV      = BIT(0),
    RTK_PHY_INTR_AN_COMPLETE         = BIT(1),
    RTK_PHY_INTR_LINK_CHANGE         = BIT(2),
    RTK_PHY_INTR_ALDPS_STATE_CHANGE  = BIT(3),
    RTK_PHY_INTR_RLFD                = BIT(4),
    RTK_PHY_INTR_TM_LOW              = BIT(5),
    RTK_PHY_INTR_TM_HIGH             = BIT(6),
    RTK_PHY_INTR_FATAL_ERROR         = BIT(7),
    RTK_PHY_INTR_MACSEC              = BIT(8),
    RTK_PHY_INTR_PTP1588             = BIT(9),
    RTK_PHY_INTR_WOL                 = BIT(10),
} rtk_phy_intr_t;

typedef enum rtk_wol_opt_e
{
    RTK_WOL_OPT_LINK   = BIT(0),
    RTK_WOL_OPT_MAGIC  = BIT(1),
    RTK_WOL_OPT_UCAST  = BIT(2),
    RTK_WOL_OPT_MCAST  = BIT(3),
    RTK_WOL_OPT_BCAST  = BIT(4),
} rtk_wol_opt_t;

#define REG32_FIELD_SET(_data, _val, _fOffset, _fMask)      ((_data & ~(_fMask)) | ((_val << (_fOffset)) & (_fMask)))
#define REG32_FIELD_GET(_data, _fOffset, _fMask)            ((_data & (_fMask)) >> (_fOffset))
#define UINT32_BITS_MASK(_mBit, _lBit)                      ((0xFFFFFFFF >> (31 - _mBit)) ^ ((1 << _lBit) - 1))

/* Register Access*/
int rtk_phylib_826xb_sds_read(struct phy_device *phydev, u32 page, u32 reg, u8 msb, u8 lsb, u32 *pData);
int rtk_phylib_826xb_sds_write(struct phy_device *phydev, u32 page, u32 reg, u8 msb, u8 lsb, u32 data);

/* Interrupt */
int rtk_phylib_826xb_intr_enable(struct phy_device *phydev, u32 en);
int rtk_phylib_826xb_intr_read_clear(struct phy_device *phydev, u32 *status);
int rtk_phylib_826xb_intr_init(struct phy_device *phydev);

/* Link-down-power-saving/EDPD */
int rtk_phylib_826xb_link_down_power_saving_set(struct phy_device *phydev, u32 ena);
int rtk_phylib_826xb_link_down_power_saving_get(struct phy_device *phydev, u32 *pEna);

/* Wake on Lan */
int rtk_phylib_826xb_wol_reset(struct phy_device *phydev);
int rtk_phylib_826xb_wol_set(struct phy_device *phydev, u32 wol_opts);
int rtk_phylib_826xb_wol_get(struct phy_device *phydev, u32 *pWol_opts);
int rtk_phylib_826xb_wol_unicast_addr_set(struct phy_device *phydev, u8 *mac_addr);
int rtk_phylib_826xb_wol_multicast_mask_add(struct phy_device *phydev, u32 offset);
int rtk_phylib_826xb_wol_multicast_mask_reset(struct phy_device *phydev);
u32 rtk_phylib_826xb_wol_multicast_mac2offset(u8 *mac_addr);

#endif /* __RTK_PHYLIB_H */
