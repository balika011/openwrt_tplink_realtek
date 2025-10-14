/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#ifndef __HAL_PHY_PATCH_H__
#define __HAL_PHY_PATCH_H__

/*
 * Include Files
 */
#include "rtk_phylib_def.h"

/*
 * Symbol Definition
 */
#define RTK_PATCH_OP_SECTION_SIZE           50
#define RTK_PATCH_OP_TO_CMP(_op, _cmp)       (_op + (RTK_PATCH_OP_SECTION_SIZE * _cmp))
/* 0~49 normal op */
#define RTK_PATCH_OP_PHY                     0
#define RTK_PATCH_OP_PHYOCP                  1
#define RTK_PATCH_OP_TOP                     2
#define RTK_PATCH_OP_TOPOCP                  3
#define RTK_PATCH_OP_PSDS0                   4
#define RTK_PATCH_OP_PSDS1                   5
#define RTK_PATCH_OP_MSDS                    6
#define RTK_PATCH_OP_MAC                     7

/* 200~255 control op */
#define RTK_PATCH_OP_DELAY_MS                200
#define RTK_PATCH_OP_SKIP                    255


/*
   patch type  PHY_PATCH_TYPE_NONE => empty
   patch type: PHY_PATCH_TYPE_TOP ~ (PHY_PATCH_TYPE_END-1)  => data array
   patch type: PHY_PATCH_TYPE_END ~ (PHY_PATCH_TYPE_END + RTK_PATCH_TYPE_FLOW_MAX)  => flow
*/
#define RTK_PATCH_TYPE_IS_DATA(_patch_type)    (_patch_type > PHY_PATCH_TYPE_NONE && _patch_type < PHY_PATCH_TYPE_END)
#define RTK_PATCH_TYPE_IS_FLOW(_patch_type)    (_patch_type >= PHY_PATCH_TYPE_END && _patch_type <= (PHY_PATCH_TYPE_END + RTK_PATCH_TYPE_FLOWID_MAX))


/*
 * Macro Definition
 */
#define PHYPATCH_DB_GET(_pPhy_device, _pPatchDb) \
    do { \
        struct rtk_phy_priv *_pPriv = (_pPhy_device)->priv; \
        rt_phy_patch_db_t *_pDb = _pPriv->patch; _pPatchDb = _pDb; \
        /*printk("[PHYPATCH_DB_GET] ? [%s]\n", (_pDb != NULL) ? "E":"N");*/ \
    } while(0)

#define PHYPATCH_TABLE_ASSIGN(_pPatchDb, _idx, _patch_type, _para) \
    do {\
        if (RTK_PATCH_TYPE_IS_DATA(_patch_type)) {\
            _pPatchDb->table[_idx].patch_type = _patch_type;\
            _pPatchDb->table[_idx].patch.data.conf = _para;\
            _pPatchDb->table[_idx].patch.data.size = sizeof(_para);\
        }\
        else if (RTK_PATCH_TYPE_IS_FLOW(_patch_type)) {\
            _pPatchDb->table[_idx].patch_type = _patch_type;\
            _pPatchDb->table[_idx].patch.flow_id = _patch_type;\
        }\
        else {\
            _pPatchDb->table[_idx].patch_type = PHY_PATCH_TYPE_NONE;\
        }\
    } while(0)

#define PHYPATCH_COMPARE(_mmdpage, _reg, _msb, _lsb, _exp, _real, _mask) \
    do {\
        uint32 _rData = REG32_FIELD_GET(_real, _lsb, _mask);\
        if (_exp != _rData) {\
            printk("PATCH CHECK: %u(0x%X).%u(0x%X)[%u:%u] = 0x%X (!= 0x%X)\n", _mmdpage, _mmdpage, _reg, _reg, _msb, _lsb, _rData, _exp);\
            return RT_ERR_CHECK_FAILED;\
        }\
    } while (0)

/*
 * Function Declaration
 */

extern int32 phy_patch_op(rt_phy_patch_db_t *pPhy_patchDb, struct phy_device *phydev,
                            uint8 patch_op, uint16 portmask, uint16 pagemmd, uint16 addr, uint8 msb, uint8 lsb, uint16 data);


/* Function Name:
 *      phy_patch
 * Description:
 *      apply initial patch data to PHY
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      portOffset - the index offset of port based the base port in the PHY chip
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHECK_FAILED
 *      RT_ERR_NOT_SUPPORTED
 * Note:
 *      None
 */
extern int32 phy_patch(struct phy_device *phydev);

#endif /* __HAL_PHY_PATCH_H__ */
