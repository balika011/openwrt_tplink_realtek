/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

/*
 * Include Files
 */
#include "phy_rtl826xb_patch.h"
#include "construct/conf_rtl8264b.c"
#include "construct/conf_rtl8261n_c.c"
#include "rtk_phy.h"
/*
 * Symbol Definition
 */
#define PHY_PATCH_WAIT_TIMEOUT     10000000

#define PHY_PATCH_LOG    LOG_INFO

/*
 * Data Declaration
 */

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */
static u16 _phy_rtl826xb_mmd_convert(u16 page, u16 addr)
{
    u16 reg = 0;
    if (addr < 16)
    {
        reg = 0xA400 + (page * 2);
    }
    else if (addr < 24)
    {
        reg = (16*page) + ((addr - 16) * 2);
    }
    else
    {
        reg = 0xA430 + ((addr - 24) * 2);
    }
    return reg;
}

static int
_phy_rtl826xb_patch_wait(struct phy_device *phydev, u32 mmdAddr, u32 mmdReg, u32 data, u32 mask)
{
    u32 rData = 0;
    u32 cnt = 0;
    WAIT_COMPLETE_VAR()

    WAIT_COMPLETE(PHY_PATCH_WAIT_TIMEOUT)
    {
        if ((rData = phy_read_mmd(phydev, mmdAddr, mmdReg)) < 0)
            return rData;

        ++cnt;
        if ((rData & mask) == data)
            break;

        mdelay(1);
    }

    if (WAIT_COMPLETE_IS_TIMEOUT())
    {
        phydev_err(phydev, "826XB patch wait[%u,0x%X,0x%X,0x%X]:0x%X cnt:%u\n", mmdAddr, mmdReg, data, mask, rData, cnt);
        return -ETIME;
    }

    return 0;
}

static int
_phy_rtl826xb_patch_wait_not_equal(struct phy_device *phydev, u32 mmdAddr, u32 mmdReg, u32 data, u32 mask)
{
    u32 rData = 0;
    u32 cnt = 0;
    WAIT_COMPLETE_VAR()


    WAIT_COMPLETE(PHY_PATCH_WAIT_TIMEOUT)
    {
        if ((rData = phy_read_mmd(phydev, mmdAddr, mmdReg)) < 0)
            return rData;

        ++cnt;
        if ((rData & mask) != data)
            break;

        mdelay(1);
    }
    if (WAIT_COMPLETE_IS_TIMEOUT())
    {
        phydev_err(phydev, "826xb patch wait[%u,0x%X,0x%X,0x%X]:0x%X cnt:%u\n", mmdAddr, mmdReg, data, mask, rData, cnt);
        return -ETIME;
    }

    return 0;
}

static int
_phy_rtl826xb_patch_top_get(struct phy_device *phydev, u32 topPage, u32 topReg, u32 *pData)
{
    u32 rData = 0;
    u32 topAddr = (topPage * 8) + (topReg - 16);

    if ((rData = phy_read_mmd(phydev, MDIO_MMD_VEND1, topAddr)) < 0)
        return rData;

    *pData = rData;

    return 0;
}

static int
_phy_rtl826xb_patch_top_set(struct phy_device *phydev, u32 topPage, u32 topReg, u32 wData)
{
    u32 topAddr = (topPage * 8) + (topReg - 16);
    return phy_write_mmd(phydev, MDIO_MMD_VEND1, topAddr, wData);
}

static int
_phy_rtl826xb_patch_sds_get(struct phy_device *phydev, u32 sdsPage, u32 sdsReg, u32 *pData)
{
    int ret = 0;
    u32 rData = 0;
    u32 sdsAddr = 0x8000 + (sdsReg << 6) + sdsPage;

    if ((ret = _phy_rtl826xb_patch_top_set(phydev, 40, 19, sdsAddr)) < 0)
        return ret;
    if ((ret = _phy_rtl826xb_patch_top_get(phydev, 40, 18, &rData)) < 0)
        return ret;
    *pData = rData;
    return _phy_rtl826xb_patch_wait(phydev, MDIO_MMD_VEND1, 0x143, 0, BIT(15));
}

static int
_phy_rtl826xb_patch_sds_set(struct phy_device *phydev, u32 sdsPage, u32 sdsReg, u32 wData)
{
    int ret = 0;
    u32 sdsAddr = 0x8800 + (sdsReg << 6) + sdsPage;

    if ((ret = _phy_rtl826xb_patch_top_set(phydev, 40, 17, wData)) < 0)
        return ret;
    if ((ret = _phy_rtl826xb_patch_top_set(phydev, 40, 19, sdsAddr)) < 0)
        return ret;
    return _phy_rtl826xb_patch_wait(phydev, MDIO_MMD_VEND1, 0x143, 0, BIT(15));
}

static int _phy_rtl826xb_flow_r1(struct phy_device *phydev)
{
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);

    //PHYReg_bit w $PHYID 0xb82 16 4 4 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xb82, 16, 4, 4, 0x1), ret);
    //PHYReg_bit w $PHYID 0xb82 16 4 4 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xb82, 16, 4, 4, 0x1), ret);

    //set patch_rdy [PHYReg_bit r $PHYID 0xb80 16 6 6] ; Wait for patch ready = 1
    RT_ERR_CHK(_phy_rtl826xb_patch_wait(phydev, MDIO_MMD_VEND1, _phy_rtl826xb_mmd_convert(0xb80, 16), BIT(6), BIT(6)), ret);

    //PHYReg w $PHYID 0xa43 27 $0x8023
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 27, 15, 0, 0x8023), ret);
    //PHYReg w $PHYID 0xa43 28 $0x3802
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 28, 15, 0, 0x3802), ret);
    //PHYReg w $PHYID 0xa43 27 0xB82E
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 27, 15, 0, 0xB82E), ret);
    //PHYReg w $PHYID 0xa43 28 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 28, 15, 0, 0x1), ret);

     return 0;
}

static int _phy_rtl826xb_flow_r12(struct phy_device *phydev)
{
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);

    //PHYReg_bit w $PHYID 0xb82 16 4 4 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xb82, 16, 4, 4, 0x1), ret);
    //PHYReg_bit w $PHYID 0xb82 16 4 4 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xb82, 16, 4, 4, 0x1), ret);

    //set patch_rdy [PHYReg_bit r $PHYID 0xb80 16 6 6] ; Wait for patch ready = 1
    RT_ERR_CHK(_phy_rtl826xb_patch_wait(phydev, MDIO_MMD_VEND1, _phy_rtl826xb_mmd_convert(0xb80, 16), BIT(6), BIT(6)), ret);

    //PHYReg w $PHYID 0xa43 27 $0x8023
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 27, 15, 0, 0x8023), ret);
    //PHYReg w $PHYID 0xa43 28 $0x3800
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 28, 15, 0, 0x3800), ret);
    //PHYReg w $PHYID 0xa43 27 0xB82E
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 27, 15, 0, 0xB82E), ret);
    //PHYReg w $PHYID 0xa43 28 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 28, 15, 0, 0x1), ret);

    return 0;
}


static int _phy_rtl826xb_flow_r2(struct phy_device *phydev)
{
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);

    //PHYReg w $PHYID 0xa43 27 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 27, 15, 0, 0x0000), ret);
    //PHYReg w $PHYID 0xa43 28 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 28, 15, 0, 0x0000), ret);
    //PHYReg_bit w $PHYID 0xB82 23 0 0 0x0
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xB82, 23, 0, 0, 0x0), ret);
    //PHYReg w $PHYID 0xa43 27 $0x8023
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 27, 15, 0, 0x8023), ret);
    //PHYReg w $PHYID 0xa43 28 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 28, 15, 0, 0x0000), ret);

    //PHYReg_bit w $PHYID 0xb82 16 4 4 0x0
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xb82, 16, 4, 4, 0x0), ret);
    //set patch_rdy [PHYReg_bit r $PHYID 0xb80 16 6 6] ; Wait for patch ready != 1
    RT_ERR_CHK(_phy_rtl826xb_patch_wait_not_equal(phydev, MDIO_MMD_VEND1, _phy_rtl826xb_mmd_convert(0xb80, 16), BIT(6), BIT(6)), ret);

    return 0;
}

static int _phy_rtl826xb_flow_l1(struct phy_device *phydev)
{
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);

    //PHYReg_bit w $PHYID 0xa4a 16 10 10 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa4a, 16, 10, 10, 0x1), ret);
    //PHYReg_bit w $PHYID 0xa4a 16 10 10 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa4a, 16, 10, 10, 0x1), ret);

    //set pcs_state [PHYReg_bit r $PHYID 0xa60 16 7 0] ; Wait for pcs state = 1
    RT_ERR_CHK(_phy_rtl826xb_patch_wait(phydev, MDIO_MMD_VEND1, _phy_rtl826xb_mmd_convert(0xa60, 16), 0x1, 0xFF), ret);

    return 0;
}

static int _phy_rtl826xb_flow_l2(struct phy_device *phydev)
{
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);

    //PHYReg_bit w $PHYID 0xa4a 16 10 10 0x0
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa4a, 16, 10, 10, 0x0), ret);

    //set pcs_state [PHYReg_bit r $PHYID 0xa60 16 7 0] ; Wait for pcs state != 1
    RT_ERR_CHK(_phy_rtl826xb_patch_wait_not_equal(phydev, MDIO_MMD_VEND1, _phy_rtl826xb_mmd_convert(0xa60, 16), 0x1, 0xFF), ret);

    return 0;
}

static int _phy_rtl826xb_flow_pi(struct phy_device *phydev)
{
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;
    u32 rData = 0, cnt = 0;

    PHYPATCH_DB_GET(phydev, pPatchDb);

    _phy_rtl826xb_flow_l1(phydev);

    //  PP_PHYReg_bit w $PHYID 0xbf86 9 9 0x1; #SS_EN_XG = 1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 9, 9, 0x1), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 8 8 0x0;
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 8, 8, 0x0), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 7 7 0x1;
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 7, 7, 0x1), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 6 6 0x1;
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 6, 6, 0x1), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 5 5 0x1;
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 5, 5, 0x1), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 4 4 0x1;
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 4, 4, 0x1), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 6 6 0x0;
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 6, 6, 0x0), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 9 9 0x0;
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 9, 9, 0x0), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 7 7 0x0;
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 7, 7, 0x0), ret);

    //PP_PHYReg_bit r $PHYID 0xbc62 12 8
    if ((rData = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xbc62)) < 0)
        return rData;

    rData = REG32_FIELD_GET(rData, 8, 0x1F00);
    for (cnt = 0; cnt <= rData; cnt++)
    {
        //PP_PHYReg_bit w $PHYID 0xbc62 12 8 $t
        RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbc62, 12, 8, cnt), ret);
    }

    //   PP_PHYReg_bit w $PHYID 0xbc02 2 2 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbc02, 2, 2, 0x1), ret);
    //   PP_PHYReg_bit w $PHYID 0xbc02 3 3 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbc02, 3, 3, 0x1), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 6 6 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 6, 6, 0x1), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 9 9 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 9, 9, 0x1), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 7 7 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 7, 7, 0x1), ret);
    //   PP_PHYReg_bit w $PHYID 0xbc04 9 2 0xff
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbc04, 9, 2, 0xff), ret);

    _phy_rtl826xb_flow_l2(phydev);
    return 0;
}

static int _phy_rtl826xb_flow_n01(struct phy_device *phydev)
{
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);

    //PHYReg_bit w $PHYID 0xa01 21 15 0 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 21, 15, 0, 0x1), ret);
    //PHYReg_bit w $PHYID 0xa01 19 15 0 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 19, 15, 0, 0x0000), ret);
    //# PHYReg_bit w $PHYID 0xa01 17 15 0 0x0000
    //RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 17, 15, 0, 0x0000), ret);

    return 0;
}

static int _phy_rtl826xb_flow_n02(struct phy_device *phydev)
{
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);

    //PHYReg_bit w $PHYID 0xa01 21 15 0 0x0
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 21, 15, 0, 0x0), ret);
    //PHYReg_bit w $PHYID 0xa01 19 15 0 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 19, 15, 0, 0x0000), ret);
    //PHYReg_bit w $PHYID 0xa01 17 15 0 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 17, 15, 0, 0x0000), ret);
    return 0;
}

static int _phy_rtl826xb_flow_n11(struct phy_device *phydev)
{
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);

    //PHYReg_bit w $PHYID 0xa01 21 15 0 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 21, 15, 0, 0x1), ret);
    //PHYReg_bit w $PHYID 0xa01 19 15 0 0x0010
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 19, 15, 0, 0x0010), ret);
    //# PHYReg_bit w $PHYID 0xa01 17 15 0 0x0000
    //RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 17, 15, 0, 0x0000), ret);

    return 0;
}

static int _phy_rtl826xb_flow_n12(struct phy_device *phydev)
{
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);

    //PHYReg_bit w $PHYID 0xa01 21 15 0 0x0
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 21, 15, 0, 0x0), ret);
    //PHYReg_bit w $PHYID 0xa01 19 15 0 0x0010
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 19, 15, 0, 0x0010), ret);
    //PHYReg_bit w $PHYID 0xa01 17 15 0 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 17, 15, 0, 0x0000), ret);

    return 0;
}

static int _phy_rtl826xb_flow_n21(struct phy_device *phydev)
{
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);

    //PHYReg_bit w $PHYID 0xa01 21 15 0 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 21, 15, 0, 0x1), ret);
    //PHYReg_bit w $PHYID 0xa01 19 15 0 0x0020
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 19, 15, 0, 0x0020), ret);
    //# PHYReg_bit w $PHYID 0xa01 17 15 0 0x0000
    //RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 17, 15, 0, 0x0000), ret);

    return 0;
}

static int _phy_rtl826xb_flow_n22(struct phy_device *phydev)
{
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);

    //PHYReg_bit w $PHYID 0xa01 21 15 0 0x0
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 21, 15, 0, 0x0), ret);
    //PHYReg_bit w $PHYID 0xa01 19 15 0 0x0020
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 19, 15, 0, 0x0020), ret);
    //PHYReg_bit w $PHYID 0xa01 17 15 0 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 17, 15, 0, 0x0000), ret);

    return 0;
}

static int _phy_rtl826xb_flow_s(struct phy_device *phydev)
{
    struct rtk_phy_priv *priv = phydev->priv;
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;

    if (priv->rtk_serdes_patch)
    {
        PHYPATCH_DB_GET(phydev, pPatchDb);

        RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PSDS0, 0xff, 0x07, 0x10, 15, 0, 0x80aa), ret);
        RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PSDS0, 0xff, 0x06, 0x12, 15, 0, 0x5078), ret);
    }

    return 0;
}

static int _phy_rtl826xb_flow_cmpstart(struct phy_device *phydev)
{
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 1, 0, 11, 11, 0x0), ret);
    return 0;
}

static int _phy_rtl826xb_flow_cmpend(struct phy_device *phydev)
{
    int ret = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);
    RT_ERR_CHK(phy_patch_op(pPatchDb, phydev, RTK_PATCH_OP_PHYOCP, 0xFF, 1, 0, 11, 11, 0x1), ret);
    return 0;
}

static int phy_rtl826xb_patch_op(struct phy_device *phydev, rtk_hwpatch_t *pPatch_data)
{
    int ret = 0;
    u32 rData = 0, wData = 0;
    u16 reg = 0;
    u32 mask = UINT32_BITS_MASK(pPatch_data->msb, pPatch_data->lsb);

    phydev_dbg(phydev, "[%s,%d] patch_op: %u\n", __FUNCTION__, __LINE__, pPatch_data->patch_op);

    switch (pPatch_data->patch_op)
    {
        case RTK_PATCH_OP_PHY:
            reg = _phy_rtl826xb_mmd_convert(pPatch_data->pagemmd, pPatch_data->addr);
            if ((pPatch_data->msb != 15) || (pPatch_data->lsb != 0))
            {
                if ((rData = phy_read_mmd(phydev, MDIO_MMD_VEND2, reg)) < 0)
                    return rData;
            }
            wData = REG32_FIELD_SET(rData, pPatch_data->data, pPatch_data->lsb, mask);
            RT_ERR_CHK(phy_write_mmd(phydev, MDIO_MMD_VEND2, reg, wData), ret);
            break;

        case RTK_PATCH_OP_PHYOCP:
            if ((pPatch_data->msb != 15) || (pPatch_data->lsb != 0))
            {
                if ((rData = phy_read_mmd(phydev, MDIO_MMD_VEND2, pPatch_data->addr)) < 0)
                    return rData;
            }
            wData = REG32_FIELD_SET(rData, pPatch_data->data, pPatch_data->lsb, mask);
            RT_ERR_CHK(phy_write_mmd(phydev, MDIO_MMD_VEND2, pPatch_data->addr, wData), ret);
            break;

        case RTK_PATCH_OP_TOP:
            if ((pPatch_data->msb != 15) || (pPatch_data->lsb != 0))
            {
                RT_ERR_CHK(_phy_rtl826xb_patch_top_get(phydev, pPatch_data->pagemmd, pPatch_data->addr, &rData), ret);
            }
            wData = REG32_FIELD_SET(rData, pPatch_data->data, pPatch_data->lsb, mask);
            RT_ERR_CHK(_phy_rtl826xb_patch_top_set(phydev, pPatch_data->pagemmd, pPatch_data->addr, wData), ret);
            break;

        case RTK_PATCH_OP_PSDS0:
            if ((pPatch_data->msb != 15) || (pPatch_data->lsb != 0))
            {
                RT_ERR_CHK(_phy_rtl826xb_patch_sds_get(phydev, pPatch_data->pagemmd, pPatch_data->addr, &rData), ret);
            }
            wData = REG32_FIELD_SET(rData, pPatch_data->data, pPatch_data->lsb, mask);
            RT_ERR_CHK(_phy_rtl826xb_patch_sds_set(phydev, pPatch_data->pagemmd, pPatch_data->addr, wData), ret);
            break;

        case RTK_PATCH_OP_DELAY_MS:
            mdelay(pPatch_data->data);
            break;

        case RTK_PATCH_OP_SKIP:
            return 0;

        default:
            phydev_err(phydev, "patch_op:%u not implemented yet!\n", pPatch_data->patch_op);
            return -EINVAL;
    }

    return ret;
}

static int phy_rtl826xb_patch_flow(struct phy_device *phydev, u8 patch_flow)
{
    int ret = 0;

    phydev_info(phydev, "[%s] flow%u\n", __FUNCTION__, patch_flow - PHY_PATCH_TYPE_END);
    switch (patch_flow)
    {
        case RTK_PATCH_TYPE_FLOW(0):
            RT_ERR_CHK(_phy_rtl826xb_flow_r1(phydev), ret);
            break;
        case RTK_PATCH_TYPE_FLOW(1):
            RT_ERR_CHK(_phy_rtl826xb_flow_r2(phydev), ret);
            break;

        case RTK_PATCH_TYPE_FLOW(2):
            RT_ERR_CHK(_phy_rtl826xb_flow_l1(phydev), ret);
            break;
        case RTK_PATCH_TYPE_FLOW(3):
            RT_ERR_CHK(_phy_rtl826xb_flow_l2(phydev), ret);
            break;

        case RTK_PATCH_TYPE_FLOW(4):
            RT_ERR_CHK(_phy_rtl826xb_flow_n01(phydev), ret);
            break;
        case RTK_PATCH_TYPE_FLOW(5):
            RT_ERR_CHK(_phy_rtl826xb_flow_n02(phydev), ret);
            break;

        case RTK_PATCH_TYPE_FLOW(6):
            RT_ERR_CHK(_phy_rtl826xb_flow_n11(phydev), ret);
            break;
        case RTK_PATCH_TYPE_FLOW(7):
            RT_ERR_CHK(_phy_rtl826xb_flow_n12(phydev), ret);
            break;

        case RTK_PATCH_TYPE_FLOW(8):
            RT_ERR_CHK(_phy_rtl826xb_flow_n21(phydev), ret);
            break;
        case RTK_PATCH_TYPE_FLOW(9):
            RT_ERR_CHK(_phy_rtl826xb_flow_n22(phydev), ret);
            break;

        case RTK_PATCH_TYPE_FLOW(10):
            RT_ERR_CHK(_phy_rtl826xb_flow_s(phydev), ret);
            break;

        case RTK_PATCH_TYPE_FLOW(11):
            RT_ERR_CHK(_phy_rtl826xb_flow_pi(phydev), ret);
            break;
        case RTK_PATCH_TYPE_FLOW(12):
            RT_ERR_CHK(_phy_rtl826xb_flow_r12(phydev), ret);
            break;

        case RTK_PATCH_TYPE_FLOW(13):
            RT_ERR_CHK(_phy_rtl826xb_flow_cmpstart(phydev), ret);
            break;
        case RTK_PATCH_TYPE_FLOW(14):
            RT_ERR_CHK(_phy_rtl826xb_flow_cmpend(phydev), ret);
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

int phy_rtl826xb_patch_db_init(struct phy_device *phydev, rt_phy_patch_db_t **pPhy_patchDb)
{
    int ret = 0;
    rt_phy_patch_db_t *patch_db = NULL;

    patch_db = kmalloc(sizeof(rt_phy_patch_db_t), GFP_ATOMIC);
    RT_PARAM_CHK(NULL == patch_db, -ENOMEM);
    memset(patch_db, 0x0, sizeof(rt_phy_patch_db_t));

    /* patch callback */
    patch_db->fPatch_op = phy_rtl826xb_patch_op;
    patch_db->fPatch_flow = phy_rtl826xb_patch_flow;

    /* patch table */
    if (phydev->drv->phy_id != REALTEK_PHY_ID_RTL8261N)
    {
        PHYPATCH_TABLE_ASSIGN(patch_db,  0, RTK_PATCH_TYPE_FLOW(12), NULL);
        PHYPATCH_TABLE_ASSIGN(patch_db,  1, PHY_PATCH_TYPE_NCTL0, rtl8264b_nctl0_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db,  2, PHY_PATCH_TYPE_NCTL1, rtl8264b_nctl1_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db,  3, PHY_PATCH_TYPE_NCTL2, rtl8264b_nctl2_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db,  4, PHY_PATCH_TYPE_UC2, rtl8264b_uc2_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db,  5, PHY_PATCH_TYPE_UC, rtl8264b_uc_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db,  6, PHY_PATCH_TYPE_DATARAM, rtl8264b_dataram_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db,  7, RTK_PATCH_TYPE_FLOW(1), NULL);
        PHYPATCH_TABLE_ASSIGN(patch_db,  8, RTK_PATCH_TYPE_FLOW(2), NULL);
        PHYPATCH_TABLE_ASSIGN(patch_db,  9, PHY_PATCH_TYPE_ALGXG, rtl8264b_algxg_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db, 10, PHY_PATCH_TYPE_ALG1G, rtl8264b_alg_giga_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db, 11, PHY_PATCH_TYPE_NORMAL, rtl8264b_normal_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db, 12, PHY_PATCH_TYPE_TOP, rtl8264b_top_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db, 13, PHY_PATCH_TYPE_SDS, rtl8264b_sds_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db, 14, PHY_PATCH_TYPE_AFE, rtl8264b_afe_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db, 15, PHY_PATCH_TYPE_RTCT, rtl8264b_rtct_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db, 16, RTK_PATCH_TYPE_FLOW(3), NULL);
        PHYPATCH_TABLE_ASSIGN(patch_db, 17, RTK_PATCH_TYPE_FLOW(11), NULL);
        PHYPATCH_TABLE_ASSIGN(patch_db, 18, RTK_PATCH_TYPE_FLOW(10), NULL);
    }
    else
    {
        PHYPATCH_TABLE_ASSIGN(patch_db,  0, RTK_PATCH_TYPE_FLOW(0), NULL);
        PHYPATCH_TABLE_ASSIGN(patch_db,  1, PHY_PATCH_TYPE_NCTL0, rtl8261n_c_nctl0_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db,  2, PHY_PATCH_TYPE_NCTL1, rtl8261n_c_nctl1_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db,  3, PHY_PATCH_TYPE_NCTL2, rtl8261n_c_nctl2_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db,  4, PHY_PATCH_TYPE_UC2, rtl8261n_c_uc2_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db,  5, PHY_PATCH_TYPE_UC, rtl8261n_c_uc_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db,  6, PHY_PATCH_TYPE_DATARAM, rtl8261n_c_dataram_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db,  7, RTK_PATCH_TYPE_FLOW(1), NULL);
        PHYPATCH_TABLE_ASSIGN(patch_db,  8, RTK_PATCH_TYPE_FLOW(2), NULL);
        PHYPATCH_TABLE_ASSIGN(patch_db,  9, PHY_PATCH_TYPE_ALGXG, rtl8261n_c_algxg_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db, 10, PHY_PATCH_TYPE_ALG1G, rtl8261n_c_alg_giga_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db, 11, PHY_PATCH_TYPE_NORMAL, rtl8261n_c_normal_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db, 12, PHY_PATCH_TYPE_TOP, rtl8261n_c_top_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db, 13, PHY_PATCH_TYPE_SDS, rtl8261n_c_sds_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db, 14, PHY_PATCH_TYPE_AFE, rtl8261n_c_afe_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db, 15, PHY_PATCH_TYPE_RTCT, rtl8261n_c_rtct_conf);
        PHYPATCH_TABLE_ASSIGN(patch_db, 16, RTK_PATCH_TYPE_FLOW(3), NULL);
        PHYPATCH_TABLE_ASSIGN(patch_db, 17, RTK_PATCH_TYPE_FLOW(10), NULL);
    }
    *pPhy_patchDb = patch_db;
    return ret;
}
