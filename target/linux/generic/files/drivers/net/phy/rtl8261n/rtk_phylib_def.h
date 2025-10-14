/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#ifndef __RTK_PHYLIB_DEF_H
#define __RTK_PHYLIB_DEF_H

#include <linux/phy.h>
#include "type.h"

#define PHY_C22_MMD_PAGE            0x0A41
#define PHY_C22_MMD_DEV_REG         13
#define PHY_C22_MMD_ADD_REG         14

#define REG32_FIELD_SET(_data, _val, _fOffset, _fMask)      ((_data & ~(_fMask)) | ((_val << (_fOffset)) & (_fMask)))
#define REG32_FIELD_GET(_data, _fOffset, _fMask)            ((_data & (_fMask)) >> (_fOffset))
#define UINT32_BITS_MASK(_mBit, _lBit)                      ((0xFFFFFFFF >> (31 - _mBit)) ^ ((1 << _lBit) - 1))

/* ss\sdk\include\hal\phy\phydef.h */

/* unified patch format */
typedef enum rtk_phypatch_type_e
{
    PHY_PATCH_TYPE_NONE = 0,
    PHY_PATCH_TYPE_TOP = 1,
    PHY_PATCH_TYPE_SDS,
    PHY_PATCH_TYPE_AFE,
    PHY_PATCH_TYPE_UC,
    PHY_PATCH_TYPE_UC2,
    PHY_PATCH_TYPE_NCTL0,
    PHY_PATCH_TYPE_NCTL1,
    PHY_PATCH_TYPE_NCTL2,
    PHY_PATCH_TYPE_ALGXG,
    PHY_PATCH_TYPE_ALG1G,
    PHY_PATCH_TYPE_NORMAL,
    PHY_PATCH_TYPE_DATARAM,
    PHY_PATCH_TYPE_RTCT,
    PHY_PATCH_TYPE_END
} rtk_phypatch_type_t;

#define RTK_PATCH_TYPE_FLOW(_id)    (PHY_PATCH_TYPE_END + _id)
#define RTK_PATCH_TYPE_FLOWID_MAX   PHY_PATCH_TYPE_END
#define RTK_PATCH_SEQ_MAX     ( PHY_PATCH_TYPE_END + RTK_PATCH_TYPE_FLOWID_MAX -1)

/* Interrupt */
#define RTK_PHY_INTR_NEXT_PAGE_RECV       BIT(0)
#define RTK_PHY_INTR_AN_COMPLETE          BIT(1)
#define RTK_PHY_INTR_LINK_CHANGE          BIT(2)
#define RTK_PHY_INTR_ALDPS_STATE_CHANGE   BIT(3)
#define RTK_PHY_INTR_RLFD                 BIT(4)
#define RTK_PHY_INTR_TM_LOW               BIT(5)
#define RTK_PHY_INTR_TM_HIGH              BIT(6)
#define RTK_PHY_INTR_FATAL_ERROR          BIT(7)
#define RTK_PHY_INTR_MACSEC               BIT(8)
#define RTK_PHY_INTR_PTP1588              BIT(9)
#define RTK_PHY_INTR_WOL                  BIT(10)

typedef struct rtk_hwpatch_s
{
    uint8    patch_op;
    uint8    portmask;
    uint16   pagemmd;
    uint16   addr;
    uint8    msb;
    uint8    lsb;
    uint16   data;
} rtk_hwpatch_t;

typedef struct rtk_hwpatch_data_s
{
    rtk_hwpatch_t *conf;
    uint32        size;
} rtk_hwpatch_data_t;

typedef struct rtk_hwpatch_seq_s
{
    uint8 patch_type;
    union
    {
        rtk_hwpatch_data_t data;
        uint8 flow_id;
    } patch;
} rtk_hwpatch_seq_t;

typedef struct rt_phy_patch_db_s
{
    /* patch operation */
    int32   (*fPatch_op)(struct phy_device *phydev, rtk_hwpatch_t *pPatch_data);
    int32   (*fPatch_flow)(struct phy_device *phydev, uint8 patch_flow);

    /* patch data */
    rtk_hwpatch_seq_t table[RTK_PATCH_SEQ_MAX];

} rt_phy_patch_db_t;

typedef enum rtk_wol_opt_e
{
    RTK_WOL_OPT_LINK   = (0x1U << 0),
    RTK_WOL_OPT_MAGIC  = (0x1U << 1),
    RTK_WOL_OPT_UCAST  = (0x1U << 2),
    RTK_WOL_OPT_MCAST  = (0x1U << 3),
    RTK_WOL_OPT_BCAST  = (0x1U << 4),
} rtk_wol_opt_t;

#endif /* __RTK_PHYLIB_DEF_H */
