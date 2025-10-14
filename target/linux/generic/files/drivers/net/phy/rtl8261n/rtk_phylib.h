/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#ifndef __RTK_PHYLIB_H
#define __RTK_PHYLIB_H

#include <linux/types.h>

#include "type.h"
#include "rtk_phylib_def.h"

typedef enum rtk_phylib_phy_e
{
    RTK_PHYLIB_NONE,
    RTK_PHYLIB_RTL8251L,
    RTK_PHYLIB_RTL8254B,
    RTK_PHYLIB_RTL8261N,
    RTK_PHYLIB_RTL8261BE,
    RTK_PHYLIB_RTL8264,
    RTK_PHYLIB_RTL8264B,
    RTK_PHYLIB_END
} rtk_phylib_phy_t;

struct rtk_phy_priv {
    rtk_phylib_phy_t phytype;
    uint8 isBasePort;
    rt_phy_patch_db_t *patch;

    bool pnswap_tx:1;
    bool pnswap_rx:1;
    bool rtk_serdes_patch:1;
};

#define RTK_PHYLIB_ERR_CHK(op)\
do {\
    if ((ret = (op)) != 0)\
        return ret;\
} while(0)

/* OSAL */
void rtk_phylib_udelay(uint32 usec);

/* Register Access APIs */
int32 rtk_phylib_mmd_write(struct phy_device *phydev, uint32 mmd, uint32 reg, uint8 msb, uint8 lsb, uint32 data);
int32 rtk_phylib_mmd_read(struct phy_device *phydev, uint32 mmd, uint32 reg, uint8 msb, uint8 lsb, uint32 *pData);

#endif /* __RTK_PHYLIB_H */
