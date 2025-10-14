/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#ifndef __RTK_PHYLIB_RTL826XB_H
#define __RTK_PHYLIB_RTL826XB_H

#include "rtk_phylib.h"

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

#endif /* __RTK_PHYLIB_RTL826XB_H */
