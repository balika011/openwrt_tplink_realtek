/* SPDX-License-Identifier: GPL-2.0 */

#ifndef REALTEK_H
#define REALTEK_H

#include <linux/phy.h>

#include "phy_patch.h"
#include "phy_patch_rtl826x.h"

struct rtl826x_priv {
	rt_phy_patch_db_t *patch;
	bool pnswap_tx : 1;
	bool pnswap_rx : 1;
	bool rtk_serdes_patch : 1;
};

int rtl822x_hwmon_init(struct phy_device *phydev);

#endif /* REALTEK_H */
