/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

/*
 * Include Files
 */
#include "rtk_osal.h"

/*
 * Function Declaration
 */
int32 phy_patch_op(rt_phy_patch_db_t *pPhy_patchDb, struct phy_device *phydev, uint8 patch_op, uint16 portmask, uint16 pagemmd, uint16 addr, uint8 msb, uint8 lsb, uint16 data)
{
    rtk_hwpatch_t op;

    op.patch_op = patch_op;
    op.portmask = portmask;
    op.pagemmd  = pagemmd;
    op.addr     = addr;
    op.msb      = msb;
    op.lsb      = lsb;
    op.data     = data;

    return pPhy_patchDb->fPatch_op(phydev, &op);
}

static int32 _phy_patch_process(struct phy_device *phydev, rtk_hwpatch_t *pPatch, int32 size)
{
    int32 i = 0;
    int32 ret = 0;
    int32 chk_ret = RT_ERR_OK;
    int32 n;
    rtk_hwpatch_t *patch = pPatch;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);

    if (size <= 0)
    {
        return RT_ERR_OK;
    }
    n = size / sizeof(rtk_hwpatch_t);

    for (i = 0; i < n; i++)
    {
        ret = pPatchDb->fPatch_op(phydev, &patch[i]);
        if ((ret != RT_ERR_ABORT) && (ret != RT_ERR_OK))
        {
            phydev_err(phydev, "%s failed! %u[%u][0x%X][0x%X][0x%X] ret=0x%X\n", __FUNCTION__,
                        i + 1, patch[i].patch_op, patch[i].pagemmd, patch[i].addr, patch[i].data, ret);
            return ret;
        }

    }
    return (chk_ret == RT_ERR_CHECK_FAILED) ? chk_ret : RT_ERR_OK;
}

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
int32 phy_patch(struct phy_device *phydev)
{
    int32 ret = RT_ERR_OK;
    int32 chk_ret = RT_ERR_OK;
    uint32 i = 0;
    uint8 patch_type = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;
    rtk_hwpatch_seq_t *table = NULL;

    PHYPATCH_DB_GET(phydev, pPatchDb);

    if ((pPatchDb == NULL) || (pPatchDb->fPatch_op == NULL) || (pPatchDb->fPatch_flow == NULL))
    {
        phydev_err(phydev, "phy_patch, db is NULL\n");
        return RT_ERR_DRIVER_NOT_SUPPORTED;
    }

    table = pPatchDb->table;

    for (i = 0; i < RTK_PATCH_SEQ_MAX; i++)
    {
        patch_type = table[i].patch_type;
        phydev_info(phydev, "phy_patch: table[%u] patch_type:%u\n", i, patch_type);

        if (RTK_PATCH_TYPE_IS_DATA(patch_type))
        {
            ret = _phy_patch_process(phydev, table[i].patch.data.conf, table[i].patch.data.size);

            if (ret == RT_ERR_CHECK_FAILED)
                chk_ret = ret;
            else if (ret  != RT_ERR_OK)
            {
                phydev_info(phydev, "id:%u patch-%u failed. ret:0x%X\n", i, patch_type, ret);
                return ret;
            }
        }
        else if (RTK_PATCH_TYPE_IS_FLOW(patch_type))
        {
            RT_ERR_CHK_EHDL(pPatchDb->fPatch_flow(phydev, table[i].patch.flow_id),
                            ret, phydev_err(phydev, "id:%u patch-%u failed. ret:0x%X\n", i, patch_type, ret););
        }
        else
        {
            break;
        }
    }

    return (chk_ret == RT_ERR_CHECK_FAILED) ? chk_ret : RT_ERR_OK;
}
