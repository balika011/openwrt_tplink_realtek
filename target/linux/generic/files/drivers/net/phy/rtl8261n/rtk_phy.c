/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#include <linux/module.h>
#include <linux/phy.h>
#include <linux/property.h>
#include <linux/netdevice.h>
#include <linux/crc32.h>

#include "phy_patch.h"
#include "phy_patch_rtl826xb.h"
#include "rtk_phy.h"

#define REALTEK_SERDES_GLOBAL_CFG       0x1c
#define   REALTEK_HSO_INV               BIT(7)
#define   REALTEK_HSI_INV               BIT(6)

static int rtl826xb_get_features(struct phy_device *phydev)
{
    int ret;
    struct rtk_phy_priv *priv = phydev->priv;

    ret = genphy_c45_pma_read_abilities(phydev);
    if (ret)
        return ret;

    linkmode_or(phydev->supported, phydev->supported, PHY_BASIC_FEATURES);

    linkmode_set_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
                       phydev->supported);
    linkmode_set_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
                       phydev->supported);
    /* not support 10M modes */
    linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT,
                       phydev->supported);
    linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT,
                       phydev->supported);

    switch (priv->phytype)
    {
        case RTK_PHY_RTL8251L:
        case RTK_PHY_RTL8254B:
            linkmode_clear_bit(ETHTOOL_LINK_MODE_10000baseT_Full_BIT,
                       phydev->supported);
            break;

        default:
            break;
    }

    return 0;
}

static int rtl826xb_probe(struct phy_device *phydev)
{
    struct device *dev = &phydev->mdio.dev;
    struct rtk_phy_priv *priv = NULL;
    int data = 0;

    priv = devm_kzalloc(&phydev->mdio.dev, sizeof(struct rtk_phy_priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    memset(priv, 0, sizeof(struct rtk_phy_priv));

    if (phy_patch_rtl826xb_db_init(phydev, &(priv->patch)) < 0)
        return -ENOMEM;

    if (phydev->drv->phy_id == REALTEK_PHY_ID_RTL8261N)
    {
        data = phy_read_mmd(phydev, MDIO_MMD_VEND1, 0x103);
        if (data < 0)
            return data;

        if (data == 0x8251)
        {
            priv->phytype = RTK_PHY_RTL8251L;
        }
        else
        {
            data = phy_read_mmd(phydev, MDIO_MMD_VEND1, 0x104);
            if (data < 0)
                return data;

            if ((data & 0xFFC0) == 0x1140)
            {
                priv->phytype = RTK_PHY_RTL8261BE;
            }
            else
            {
                priv->phytype = RTK_PHY_RTL8261N;
            }
        }
    }
    else if (phydev->drv->phy_id == REALTEK_PHY_ID_RTL8264B)
    {
        data = phy_read_mmd(phydev, MDIO_MMD_VEND1, 0x103);
        if (data < 0)
            return data;

        if (data == 0x8254)
        {
            priv->phytype = RTK_PHY_RTL8254B;
        }
        else
        {
            priv->phytype = RTK_PHY_RTL8264B;
        }
    }
    else if (phydev->drv->phy_id == REALTEK_PHY_ID_RTL8264)
    {
         priv->phytype = RTK_PHY_RTL8264;
    }

    priv->pnswap_tx = device_property_read_bool(dev, "realtek,pnswap-tx");
    priv->pnswap_rx = device_property_read_bool(dev, "realtek,pnswap-rx");
    priv->rtk_serdes_patch = device_property_read_bool(dev, "realtek,rtk-serdes-patch");
    phydev->priv = priv;

    return 0;
}

static const char *rtkphy_get_phy_name(struct phy_device *phydev)
{
    struct rtk_phy_priv *priv = phydev->priv;
    switch (priv->phytype)
    {
        case RTK_PHY_RTL8251L:  return "RTL8251L";
        case RTK_PHY_RTL8254B:  return "RTL8254B";
        case RTK_PHY_RTL8261N:  return "RTL8261N";
        case RTK_PHY_RTL8261BE: return "RTL8261BE";
        case RTK_PHY_RTL8264:   return "RTL8264";
        case RTK_PHY_RTL8264B:  return "RTL8264B";
        default:                   return "RTL82????";
    }
}

static int rtk_phylib_826xb_intr_init(struct phy_device *phydev)
{
    int ret = 0;

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

    /* init common link change & WOL*/
    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xA424, UINT32_BITS_MASK(15, 0), BIT(4) | BIT(7))) < 0)
        return ret;

    /* clear status */
    phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0x2DC, UINT32_BITS_MASK(15, 0), 0xFF);

    return ret;
}

static int rtkphy_config_init(struct phy_device *phydev)
{
    struct rtk_phy_priv *priv = phydev->priv;
    int ret = 0;
    switch (phydev->drv->phy_id)
    {
        case REALTEK_PHY_ID_RTL8261N:
        case REALTEK_PHY_ID_RTL8264:
        case REALTEK_PHY_ID_RTL8264B:
            phydev_info(phydev, "%s:%u [%s] phy_id: 0x%X PHYAD:%d swap_tx: %d swap_rx: %d\n", __FUNCTION__, __LINE__,
                        rtkphy_get_phy_name(phydev), phydev->drv->phy_id, phydev->mdio.addr, priv->pnswap_tx, priv->pnswap_rx);

            /* toggle reset */
            phy_modify_mmd_changed(phydev, MDIO_MMD_VEND1, 0x145, BIT(0), 1);
            phy_modify_mmd_changed(phydev, MDIO_MMD_VEND1, 0x145, BIT(0), 0);
            mdelay(30);

            ret = phy_patch(phydev);
            if (ret)
            {
                phydev_err(phydev, "%s:%u [RTL8261N/RTL826XB] patch failed!! 0x%X\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }

            if (priv->pnswap_tx)
                phy_set_bits_mmd(phydev, MDIO_MMD_VEND1, REALTEK_SERDES_GLOBAL_CFG, REALTEK_HSO_INV);
            if (priv->pnswap_rx)
                phy_set_bits_mmd(phydev, MDIO_MMD_VEND1, REALTEK_SERDES_GLOBAL_CFG, REALTEK_HSI_INV);

            ret = rtk_phylib_826xb_intr_init(phydev);
            if (ret)
            {
                phydev_err(phydev, "%s:%u [RTL8261N/RTL826XB] rtk_phylib_826xb_intr_init failed!! 0x%X\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }

            break;
        default:
            phydev_err(phydev, "%s:%u Unknow phy_id: 0x%X\n", __FUNCTION__, __LINE__, phydev->drv->phy_id);
            return -EPERM;
    }

    return ret;
}

static int rtkphy_c45_config_aneg(struct phy_device *phydev)
{
    bool changed = false;
    u16 reg = 0;
    int ret = 0;

    phydev->mdix_ctrl = ETH_TP_MDI_AUTO;
    if (phydev->autoneg == AUTONEG_DISABLE)
        return genphy_c45_pma_setup_forced(phydev);

    ret = genphy_c45_an_config_aneg(phydev);
    if (ret < 0)
        return ret;
    if (ret > 0)
        changed = true;

    reg = 0;
    if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
                  phydev->advertising))
        reg |= BIT(9);

    if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Half_BIT,
                  phydev->advertising))
        reg |= BIT(8);

    ret = phy_modify_mmd_changed(phydev, MDIO_MMD_VEND2, 0xA412, // MII_CTRL1000
                                 BIT(9) | BIT(8), reg);
    if (ret < 0)
        return ret;
    if (ret > 0)
        changed = true;

    return genphy_c45_check_and_restart_aneg(phydev, changed);
}

static int rtkphy_c45_read_status(struct phy_device *phydev)
{
    int ret = 0, status = 0;
    phydev->speed = SPEED_UNKNOWN;
    phydev->duplex = DUPLEX_UNKNOWN;
    phydev->pause = 0;
    phydev->asym_pause = 0;

    ret = genphy_c45_read_link(phydev);
    if (ret)
        return ret;

    if (phydev->autoneg == AUTONEG_ENABLE)
    {
        linkmode_clear_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
           phydev->lp_advertising);

        ret = genphy_c45_read_lpa(phydev);
        if (ret)
            return ret;

        status = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA414); // MII_CTRL1000
        if (status < 0)
            return status;
        linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
            phydev->lp_advertising, status & BIT(11));

        phy_resolve_aneg_linkmode(phydev);
    }
    else
    {
        ret = genphy_c45_read_pma(phydev);
    }

    /* mdix*/
    status = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_10GBT_SWAPPOL);
    if (status < 0)
        return status;

    switch (status & 0x3)
    {
        case MDIO_PMA_10GBT_SWAPPOL_ABNX | MDIO_PMA_10GBT_SWAPPOL_CDNX:
            phydev->mdix = ETH_TP_MDI;
            break;

        case 0:
            phydev->mdix = ETH_TP_MDI_X;
            break;

        default:
            phydev->mdix = ETH_TP_MDI_INVALID;
            break;
    }

    return ret;
}

static int rtl826xb_config_intr(struct phy_device *phydev)
{
    return phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0xE1, UINT32_BITS_MASK(0, 0), phydev->interrupts == PHY_INTERRUPT_ENABLED);
}

static irqreturn_t rtl826xb_handle_intr(struct phy_device *phydev)
{
    u32 rData = REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA43A), 0, UINT32_BITS_MASK(15, 0));
    if (rData < 0)
        return IRQ_NONE;

    if (phy_modify_mmd(phydev, MDIO_MMD_VEND1, 0x2DC, UINT32_BITS_MASK(15, 0), 0xFF) < 0)
        return IRQ_NONE;

    if (rData & BIT(4))
    {
        pr_debug("[%s,%d] RTK_PHY_INTR_LINK_CHANGE\n", __FUNCTION__, __LINE__);
        phy_mac_interrupt(phydev);
    }

    if (rData & BIT(7))
    {
        pr_debug("[%s,%d] RTK_PHY_INTR_WOL\n", __FUNCTION__, __LINE__);

        if (phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A2, UINT32_BITS_MASK(15, 15), 0) < 0)
            return IRQ_NONE;

        if (phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A2, UINT32_BITS_MASK(15, 15), 1 << 15) < 0)
            return IRQ_NONE;
    }

    return IRQ_HANDLED;
}

static int rtl826xb_get_tunable(struct phy_device *phydev, struct ethtool_tunable *tuna, void *data)
{
    int ret = 0;
    u32 val = 0;

    switch (tuna->id)
    {
        case ETHTOOL_PHY_EDPD:
            val = REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA430), 2, UINT32_BITS_MASK(2, 2));
            if (val < 0)
                return ret;

            *(u16 *)data = (val == 0) ? ETHTOOL_PHY_EDPD_DISABLE : ETHTOOL_PHY_EDPD_DFLT_TX_MSECS;
            break;

        default:
            return -EOPNOTSUPP;
    }

    return 0;
}

static int rtl826xb_set_tunable(struct phy_device *phydev, struct ethtool_tunable *tuna, const void *data)
{
    int ret = 0;
    u32 val = 0;

    switch (tuna->id)
    {
        case ETHTOOL_PHY_EDPD:
            switch (*(const u16 *) data)
            {
                case ETHTOOL_PHY_EDPD_DFLT_TX_MSECS:
                    val = BIT(2);
                    break;
                case ETHTOOL_PHY_EDPD_DISABLE:
                    val = 0;
                    break;
                default:
                    return -EINVAL;
            }

            if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xA430, UINT32_BITS_MASK(2, 2), val)) < 0)
                return ret;
            break;

        default:
            return -EOPNOTSUPP;
    }

    return 0;
}

static int rtl826xb_set_wol(struct phy_device *phydev,
              struct ethtool_wolinfo *wol)
{
    const u32 cfg_reg[4] = {0xD8C6, 0xD8C8, 0xD8CA, 0xD8CC};
    int ret = 0;
    u8 *mac_addr = NULL;
    u32 idx, offset, multicast_cfg;

    struct net_device *ndev = phydev->attached_dev;
    if (!ndev)
        return -EINVAL;

    if (wol->wolopts & ~(WAKE_PHY | WAKE_MAGIC | WAKE_UCAST | WAKE_BCAST | WAKE_MCAST))
        return -EOPNOTSUPP;

    if (wol->wolopts & (WAKE_MAGIC | WAKE_UCAST))
    {
        mac_addr = (u8 *) ndev->dev_addr;

        if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8C0, UINT32_BITS_MASK(15, 0), (mac_addr[1] << 8 | mac_addr[0]))) < 0)
            return ret;

        if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8C2, UINT32_BITS_MASK(15, 0), (mac_addr[3] << 8 | mac_addr[2]))) < 0)
            return ret;

        if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8C4, UINT32_BITS_MASK(15, 0), (mac_addr[5] << 8 | mac_addr[4]))) < 0)
            return ret;
    }

    if (wol->wolopts & WAKE_MCAST)
    {
        for (idx = 0; idx < sizeof(cfg_reg) / sizeof(cfg_reg[0]); idx++)
            if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, cfg_reg[idx], UINT32_BITS_MASK(15, 0), 0)) < 0)
                return ret;

        if (!netdev_mc_empty(ndev))
        {
            struct netdev_hw_addr *ha;
            netdev_for_each_mc_addr(ha, ndev)
            {
                pr_info("[%s,%d] mac: %pM \n", __FUNCTION__, __LINE__, ha->addr);

                offset = crc32_be(~0, ha->addr, 6) >> 26;
                idx = offset >> 4;

                multicast_cfg = REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, cfg_reg[idx]), 0, UINT32_BITS_MASK(15, 0));

                multicast_cfg = (multicast_cfg | (1 << (offset & 0xF)));

                if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, cfg_reg[idx], UINT32_BITS_MASK(15, 0), multicast_cfg)) < 0)
                    return ret;
            }
        }
    }

    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0, UINT32_BITS_MASK(13, 13), ((wol->wolopts & WAKE_PHY) ? 1 : 0) << 13)) < 0)
        return ret;

    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0, UINT32_BITS_MASK(12, 12), ((wol->wolopts & WAKE_MAGIC) ? 1 : 0) << 12)) < 0)
        return ret;

    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0, UINT32_BITS_MASK(10, 10), ((wol->wolopts & WAKE_UCAST) ? 1 : 0) << 10)) < 0)
        return ret;

    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0, UINT32_BITS_MASK(9, 9), ((wol->wolopts & WAKE_MCAST) ? 1 : 0) << 9)) < 0)
        return ret;

    if ((ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0, UINT32_BITS_MASK(8, 8), ((wol->wolopts & WAKE_BCAST) ? 1 : 0) << 8)) < 0)
        return ret;

    return 0;
}

static void rtl826xb_get_wol(struct phy_device *phydev,
               struct ethtool_wolinfo *wol)
{
    wol->supported = WAKE_PHY | WAKE_MAGIC | WAKE_UCAST | WAKE_BCAST | WAKE_MCAST;
    wol->wolopts = 0;

    if (REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0), 13, UINT32_BITS_MASK(13, 13)))
        wol->wolopts |= WAKE_PHY;
    if (REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0), 12, UINT32_BITS_MASK(12, 12)))
        wol->wolopts |= WAKE_MAGIC;
    if (REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0), 10, UINT32_BITS_MASK(10, 10)))
        wol->wolopts |= WAKE_UCAST;
    if (REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0), 9, UINT32_BITS_MASK(9, 9)))
        wol->wolopts |= WAKE_MCAST;
    if (REG32_FIELD_GET(phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xD8A0), 8, UINT32_BITS_MASK(8, 8)))
        wol->wolopts |= WAKE_BCAST;
}

static struct phy_driver rtk_phy_drivers[] = {
    {
        PHY_ID_MATCH_EXACT(REALTEK_PHY_ID_RTL8261N),
        .name               = "Realtek RTL8261N/8261BE/8251L",
        .get_features       = rtl826xb_get_features,
        .config_init        = rtkphy_config_init,
        .probe              = rtl826xb_probe,
        .suspend            = genphy_c45_pma_suspend,
        .resume             = genphy_c45_pma_resume,
        .config_aneg        = rtkphy_c45_config_aneg,
        .aneg_done          = genphy_c45_aneg_done,
        .read_status        = rtkphy_c45_read_status,
        .config_intr        = rtl826xb_config_intr,
        .handle_interrupt   = rtl826xb_handle_intr,
        .get_tunable        = rtl826xb_get_tunable,
        .set_tunable        = rtl826xb_set_tunable,
        .set_wol            = rtl826xb_set_wol,
        .get_wol            = rtl826xb_get_wol,
    },
    {
        PHY_ID_MATCH_EXACT(REALTEK_PHY_ID_RTL8264),
        .name               = "Realtek RTL8264",
        .get_features       = rtl826xb_get_features,
        .config_init        = rtkphy_config_init,
        .probe              = rtl826xb_probe,
        .suspend            = genphy_c45_pma_suspend,
        .resume             = genphy_c45_pma_resume,
        .config_aneg        = rtkphy_c45_config_aneg,
        .aneg_done          = genphy_c45_aneg_done,
        .read_status        = rtkphy_c45_read_status,
        .config_intr        = rtl826xb_config_intr,
        .handle_interrupt   = rtl826xb_handle_intr,
        .get_tunable        = rtl826xb_get_tunable,
        .set_tunable        = rtl826xb_set_tunable,
        .set_wol            = rtl826xb_set_wol,
        .get_wol            = rtl826xb_get_wol,
    },
    {
        PHY_ID_MATCH_EXACT(REALTEK_PHY_ID_RTL8264B),
        .name               = "Realtek RTL8264B/8254B",
        .get_features       = rtl826xb_get_features,
        .config_init        = rtkphy_config_init,
        .probe              = rtl826xb_probe,
        .suspend            = genphy_c45_pma_suspend,
        .resume             = genphy_c45_pma_resume,
        .config_aneg        = rtkphy_c45_config_aneg,
        .aneg_done          = genphy_c45_aneg_done,
        .read_status        = rtkphy_c45_read_status,
        .config_intr        = rtl826xb_config_intr,
        .handle_interrupt   = rtl826xb_handle_intr,
        .get_tunable        = rtl826xb_get_tunable,
        .set_tunable        = rtl826xb_set_tunable,
        .set_wol            = rtl826xb_set_wol,
        .get_wol            = rtl826xb_get_wol,
    },
};

module_phy_driver(rtk_phy_drivers);


static struct mdio_device_id __maybe_unused rtk_phy_tbl[] = {
    { PHY_ID_MATCH_EXACT(REALTEK_PHY_ID_RTL8261N) },
    { PHY_ID_MATCH_EXACT(REALTEK_PHY_ID_RTL8264) },
    { PHY_ID_MATCH_EXACT(REALTEK_PHY_ID_RTL8264B) },
    { },
};

MODULE_DEVICE_TABLE(mdio, rtk_phy_tbl);

MODULE_AUTHOR("Realtek");
MODULE_DESCRIPTION("Realtek PHY drivers");
MODULE_LICENSE("GPL");
