/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#include "rtk_phylib_rtl826xb.h"

/* Indirect Register Access APIs */
int rtk_phylib_826xb_sds_read(struct phy_device *phydev, uint32 page, uint32 reg, uint8 msb, uint8 lsb, uint32 *pData)
{
    int32  ret = 0;
    uint32 rData = 0;
    uint32 op = (page & 0x3f) | ((reg & 0x1f) << 6) | (0x8000);
    uint32 i = 0;
    uint32 mask = 0;
    mask = UINT32_BITS_MASK(msb,lsb);

    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 323, 15, 0, op));

    for (i = 0; i < 10; i++)
    {
        RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_read(phydev, 30, 323, 15, 15, &rData));
        if (rData == 0)
        {
            break;
        }
        rtk_phylib_udelay(10);
    }
    if (i == 10)
    {
        return -1;
    }

    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_read(phydev, 30, 322, 15, 0, &rData));
    *pData = REG32_FIELD_GET(rData, lsb, mask);

    return ret;
}

int rtk_phylib_826xb_sds_write(struct phy_device *phydev, uint32 page, uint32 reg, uint8 msb, uint8 lsb, uint32 data)
{
    int32  ret = 0;
    uint32 wData = 0, rData = 0;
    uint32 op = (page & 0x3f) | ((reg & 0x1f) << 6) | (0x8800);
    uint32 mask = 0;
    mask = UINT32_BITS_MASK(msb,lsb);

    RTK_PHYLIB_ERR_CHK(rtk_phylib_826xb_sds_read(phydev, page, reg, 15, 0, &rData));

    wData = REG32_FIELD_SET(rData, data, lsb, mask);

    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 321, 15, 0, wData));
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 323, 15, 0, op));

    return ret;
}

/* Interrupt */
int rtk_phylib_826xb_intr_enable(struct phy_device *phydev, uint32 en)
{
    int32  ret = 0;
    /* enable normal interrupt IMR_INT_PHY0 */
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 0xE1, 0, 0, en));

    return ret;
}

int rtk_phylib_826xb_intr_read_clear(struct phy_device *phydev, uint32 *status)
{
    int32  ret = 0;
    uint32 rData = 0;
    uint32 rStatus = 0;

    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_read(phydev, 31, 0xA43A, 15, 0, &rData));
    if(rData & BIT_1)
        rStatus |= RTK_PHY_INTR_RLFD;
    if(rData & BIT_2)
        rStatus |= RTK_PHY_INTR_NEXT_PAGE_RECV;
    if(rData & BIT_3)
        rStatus |= RTK_PHY_INTR_AN_COMPLETE;
    if(rData & BIT_4)
        rStatus |= RTK_PHY_INTR_LINK_CHANGE;
    if(rData & BIT_9)
        rStatus |= RTK_PHY_INTR_ALDPS_STATE_CHANGE;
    if(rData & BIT_11)
        rStatus |= RTK_PHY_INTR_FATAL_ERROR;

    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_read(phydev, 30, 0xE2, 15, 0, &rData));
    if(rData & BIT_3)
        rStatus |= RTK_PHY_INTR_TM_LOW;
    if(rData & BIT_4)
        rStatus |= RTK_PHY_INTR_TM_HIGH;
    if(rData & BIT_6)
        rStatus |= RTK_PHY_INTR_MACSEC;

    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 0xE2, 15, 0, 0xFF));
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 0x2DC, 15, 0, 0xFF));

    *status = rStatus;
    return ret;
}

int rtk_phylib_826xb_intr_init(struct phy_device *phydev)
{
    int32  ret = 0;
    uint32 status = 0;

    /* Disable all IMR*/
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 0xE1, 15, 0, 0));
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 0xE3, 15, 0, 0));

    /* source */
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 0xE4, 15, 0, 0x1));
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 0xE0, 15, 0, 0x2F));

    /* init common link change */
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 31, 0xA424,  15,  0, 0x10));

    /* init rlfd */
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 31, 0xA442, 15, 15, 0x1));
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 31, 0xA448, 7, 7, 0x1));

    /* init tm */
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 0x1A0, 11, 11, 0x1));
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 0x19D, 11, 11, 0x1));
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 0x1A1, 11, 11, 0x1));
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 0x19F, 11, 11, 0x1));

    /* clear status */
    RTK_PHYLIB_ERR_CHK(rtk_phylib_826xb_intr_read_clear(phydev, &status));

    return ret;
}
