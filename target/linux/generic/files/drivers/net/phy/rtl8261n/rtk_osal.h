/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#ifndef __RTK_PHY_OSAL_H
#define __RTK_PHY_OSAL_H

#include <linux/kernel.h>
#include <linux/phy.h>
#include "type.h"
#include "error.h"
#include "phy_patch.h"
#include "rtk_phylib.h"

#ifndef WAIT_COMPLETE_VAR
#define WAIT_COMPLETE_VAR() \
    osal_usecs_t    _t, _now, _t_wait=0, _timeout;  \
    int32           _chkCnt=0;

#define WAIT_COMPLETE(_timeout_us)     \
    _timeout = _timeout_us;  \
    for(osal_time_usecs_get(&_t),osal_time_usecs_get(&_now),_t_wait=0,_chkCnt=0 ; \
        (_t_wait <= _timeout); \
        osal_time_usecs_get(&_now), _chkCnt++, _t_wait += ((_now >= _t) ? (_now - _t) : (0xFFFFFFFF - _t + _now)),_t = _now \
       )

#define WAIT_COMPLETE_IS_TIMEOUT()   (_t_wait > _timeout)
#endif

/* OSAL */
#include <linux/slab.h>
int32 osal_time_usecs_get(osal_usecs_t *pUsec);
void *osal_alloc(uint32 size);

#include <linux/ctype.h>    /* for Kernel Space */
#include <linux/kernel.h>
#include <linux/string.h>

/* reg access */
int32 phy_common_general_reg_mmd_get(struct phy_device *phydev, uint32 mmdAddr, uint32 mmdReg, uint32 *pData);
int32 phy_common_general_reg_mmd_set(struct phy_device *phydev, uint32 mmdAddr, uint32 mmdReg, uint32 data);


#endif /* __RTK_PHY_OSAL_H */
