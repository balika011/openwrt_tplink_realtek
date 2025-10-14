/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#include "rtk_phylib.h"

/* Interrupt */
int rtk_phylib_826xb_intr_enable(struct phy_device *phydev, u32 en)
{
	return phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0xE1, UINT32_BITS_MASK(0, 0), en);
}

int rtk_phylib_826xb_intr_read_clear(struct phy_device *phydev, u32 *status)
{
    int ret = 0;
    u32 rData = 0;
    u32 rStatus = 0;

    rData = REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA43A), 0, UINT32_BITS_MASK(15, 0));
    if(rData & BIT(1))
        rStatus |= RTK_PHY_INTR_RLFD;
    if(rData & BIT(2))
        rStatus |= RTK_PHY_INTR_NEXT_PAGE_RECV;
    if(rData & BIT(3))
        rStatus |= RTK_PHY_INTR_AN_COMPLETE;
    if(rData & BIT(4))
        rStatus |= RTK_PHY_INTR_LINK_CHANGE;
    if(rData & BIT(7))
        rStatus |= RTK_PHY_INTR_WOL;
    if(rData & BIT(9))
        rStatus |= RTK_PHY_INTR_ALDPS_STATE_CHANGE;
    if(rData & BIT(11))
        rStatus |= RTK_PHY_INTR_FATAL_ERROR;

    rData = REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND1, 0xE2), 0, UINT32_BITS_MASK(15, 0));
    if(rData & BIT(3))
        rStatus |= RTK_PHY_INTR_TM_LOW;
    if(rData & BIT(4))
        rStatus |= RTK_PHY_INTR_TM_HIGH;
    if(rData & BIT(6))
        rStatus |= RTK_PHY_INTR_MACSEC;

    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0xE2, UINT32_BITS_MASK(15, 0), 0xFF)) < 0)
        return ret;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0x2DC, UINT32_BITS_MASK(15, 0), 0xFF)) < 0)
        return ret;

    *status = rStatus;
    return ret;
}

int rtk_phylib_826xb_intr_init(struct phy_device *phydev)
{
    int ret = 0;
    u32 status = 0;

    /* Disable all IMR*/
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0xE1, UINT32_BITS_MASK(15, 0), 0)) < 0)
        return ret;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0xE3, UINT32_BITS_MASK(15, 0), 0)) < 0)
        return ret;

    /* source */
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0xE4, UINT32_BITS_MASK(15, 0), 0x1)) < 0)
        return ret;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0xE0, UINT32_BITS_MASK(15, 0), 0x2F)) < 0)
        return ret;

    /* init common link change */
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xA424, UINT32_BITS_MASK(15, 0), 0x10)) < 0)
        return ret;

    /* init rlfd */
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xA442, UINT32_BITS_MASK(15, 15), 0x1 << 15)) < 0)
        return ret;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xA448, UINT32_BITS_MASK(7, 7), 0x1 << 7)) < 0)
        return ret;

    /* init tm */
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0x1A0, UINT32_BITS_MASK(11, 11), 0x1 << 11)) < 0)
        return ret;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0x19D, UINT32_BITS_MASK(11, 11), 0x1 << 11)) < 0)
        return ret;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0x1A1, UINT32_BITS_MASK(11, 11), 0x1 << 11)) < 0)
        return ret;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0x19F, UINT32_BITS_MASK(11, 11), 0x1 << 11)) < 0)
        return ret;

    /* init WOL */
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xA424, UINT32_BITS_MASK(7, 7), 0x1 << 7)) < 0)
        return ret;

    /* clear status */
    if ((ret = rtk_phylib_826xb_intr_read_clear(phydev, &status)) < 0)
        return ret;

    return ret;
}

/* Link-down-power-saving/EDPD */
int rtk_phylib_826xb_link_down_power_saving_set(struct phy_device *phydev, u32 ena)
{
    u32 data =  (ena > 0) ? 0x1 : 0x0;

	return phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xA430, UINT32_BITS_MASK(2, 2), data << 2);
}

int rtk_phylib_826xb_link_down_power_saving_get(struct phy_device *phydev, u32 *pEna)
{
	*pEna = REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA430), 2, UINT32_BITS_MASK(2, 2));
	return 0;
}

/* Wake on Lan */
int rtk_phylib_826xb_wol_reset(struct phy_device *phydev)
{
    int ret = 0;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A2, UINT32_BITS_MASK(15, 15), 0)) < 0)
        return ret;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A2, UINT32_BITS_MASK(15, 15), 1 << 15)) < 0)
        return ret;
    return ret;
}

int rtk_phylib_826xb_wol_set(struct phy_device *phydev, u32 wol_opts)
{
    int ret = 0;

    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0, UINT32_BITS_MASK(13, 13), ((wol_opts & RTK_WOL_OPT_LINK) ? 1 : 0) << 13)) < 0)
        return ret;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0, UINT32_BITS_MASK(12, 12), ((wol_opts & RTK_WOL_OPT_MAGIC) ? 1 : 0) << 12)) < 0)
        return ret;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0, UINT32_BITS_MASK(10, 10), ((wol_opts & RTK_WOL_OPT_UCAST) ? 1 : 0) << 10)) < 0)
        return ret;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0, UINT32_BITS_MASK(9, 9), ((wol_opts & RTK_WOL_OPT_MCAST) ? 1 : 0) << 9)) < 0)
        return ret;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0, UINT32_BITS_MASK(8, 8), ((wol_opts & RTK_WOL_OPT_BCAST) ? 1 : 0) << 8)) < 0)
        return ret;

    return ret;
}

int rtk_phylib_826xb_wol_get(struct phy_device *phydev, u32 *pWol_opts)
{
    int ret = 0;
    u32 data = 0;
    u32 wol_opts = 0;

    data = REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0), 13, UINT32_BITS_MASK(13, 13));
    wol_opts |= ((data) ? RTK_WOL_OPT_LINK : 0);
    data = REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0), 12, UINT32_BITS_MASK(12, 12));
    wol_opts |= ((data) ? RTK_WOL_OPT_MAGIC : 0);
    data = REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0), 10, UINT32_BITS_MASK(10, 10));
    wol_opts |= ((data) ? RTK_WOL_OPT_UCAST : 0);
    data = REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0), 9, UINT32_BITS_MASK(9, 9));
    wol_opts |= ((data) ? RTK_WOL_OPT_MCAST : 0);
    data = REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0), 8, UINT32_BITS_MASK(8, 8));
    wol_opts |= ((data) ? RTK_WOL_OPT_BCAST : 0);

    *pWol_opts = wol_opts;
    return ret;
}

int rtk_phylib_826xb_wol_unicast_addr_set(struct phy_device *phydev, u8 *mac_addr)
{
    int ret = 0;

    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8C0, UINT32_BITS_MASK(15, 0), (mac_addr[1] << 8 | mac_addr[0]))) < 0)
        return ret;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8C2, UINT32_BITS_MASK(15, 0), (mac_addr[3] << 8 | mac_addr[2]))) < 0)
        return ret;
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8C4, UINT32_BITS_MASK(15, 0), (mac_addr[5] << 8 | mac_addr[4]))) < 0)
        return ret;
    return ret;
}

u32 rtk_phylib_826xb_wol_multicast_mac2offset(u8 *mac_addr)
{
    u32 crc = 0xFFFFFFFF;
    u32 i = 0, j = 0;
    u32 b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0;

    for (i = 0; i < 6; i++) {
        crc ^= mac_addr[i];
        for (j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    crc = ~crc;

    b5 = ((crc & 0b000001) << 5 );
    b4 = ((crc & 0b000010) << 3 );
    b3 = ((crc & 0b000100) << 1 );
    b2 = (((crc & 0b001000) ? 0 : 1) << 2 );
    b1 = (((crc & 0b010000) ? 0 : 1) << 1 );
    b0 = (((crc & 0b100000) ? 0 : 1) << 0 );

    return (b5 | b4 | b3 | b2 | b1 | b0);
}

int rtk_phylib_826xb_wol_multicast_mask_add(struct phy_device *phydev, u32 offset)
{
    const u32 cfg_reg[4] = {0xD8C6, 0xD8C8, 0xD8CA, 0xD8CC};
    u32 idx = offset/16;
    u32 multicast_cfg = 0;

    multicast_cfg = REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, cfg_reg[idx]), 0, UINT32_BITS_MASK(15, 0));

    multicast_cfg = (multicast_cfg | (0b1 << (offset % 16)));

	return phy_modify_mmd(phydev, MDIO_MMD_VEND2, cfg_reg[idx], UINT32_BITS_MASK(15, 0), multicast_cfg);
}

int rtk_phylib_826xb_wol_multicast_mask_reset(struct phy_device *phydev)
{
    const u32 cfg_reg[4] = {0xD8C6, 0xD8C8, 0xD8CA, 0xD8CC};
    int ret = 0;
    u32 idx = 0;

    for (idx = 0; idx < 4; idx++)
    {
        if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, cfg_reg[idx], UINT32_BITS_MASK(15, 0), 0)) < 0)
        	return ret;
    }

    return ret;
}
