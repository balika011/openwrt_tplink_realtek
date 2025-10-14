/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#ifndef __RTK_PHYLIB_RTL826XB_H
#define __RTK_PHYLIB_RTL826XB_H

#include "rtk_phylib.h"

/* Register Access*/
int rtk_phylib_826xb_sds_read(struct phy_device *phydev, uint32 page, uint32 reg, uint8 msb, uint8 lsb, uint32 *pData);
int rtk_phylib_826xb_sds_write(struct phy_device *phydev, uint32 page, uint32 reg, uint8 msb, uint8 lsb, uint32 data);

/* Interrupt */
int rtk_phylib_826xb_intr_enable(struct phy_device *phydev, uint32 en);
int rtk_phylib_826xb_intr_read_clear(struct phy_device *phydev, uint32 *status);
int rtk_phylib_826xb_intr_init(struct phy_device *phydev);

/* Link-down-power-saving/EDPD */
int rtk_phylib_826xb_link_down_power_saving_set(struct phy_device *phydev, uint32 ena);
int rtk_phylib_826xb_link_down_power_saving_get(struct phy_device *phydev, uint32 *pEna);

#endif /* __RTK_PHYLIB_RTL826XB_H */
