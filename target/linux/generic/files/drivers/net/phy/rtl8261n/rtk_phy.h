/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */
#ifndef __RTK_PHY_H
#define __RTK_PHY_H

#define REALTEK_PHY_ID_RTL8261N         0x001CCAF3
#define REALTEK_PHY_ID_RTL8264          0x001CCAF2
#define REALTEK_PHY_ID_RTL8264B         0x001CC813

typedef enum rtk_phy_e
{
    RTK_PHY_NONE,
    RTK_PHY_RTL8251L,
    RTK_PHY_RTL8254B,
    RTK_PHY_RTL8261N,
    RTK_PHY_RTL8261BE,
    RTK_PHY_RTL8264,
    RTK_PHY_RTL8264B,
} rtk_phy_t;

struct rtk_phy_priv {
    rtk_phy_t phytype;
    rt_phy_patch_db_t *patch;

    bool pnswap_tx:1;
    bool pnswap_rx:1;
    bool rtk_serdes_patch:1;
};

#define REG32_FIELD_SET(_data, _val, _fOffset, _fMask)      ((_data & ~(_fMask)) | ((_val << (_fOffset)) & (_fMask)))
#define REG32_FIELD_GET(_data, _fOffset, _fMask)            ((_data & (_fMask)) >> (_fOffset))
#define UINT32_BITS_MASK(_mBit, _lBit)                      ((0xFFFFFFFF >> (31 - _mBit)) ^ ((1 << _lBit) - 1))

#endif /* __RTK_PHY_H */