/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#ifndef __RTK_PHY_OSAL_H
#define __RTK_PHY_OSAL_H

#include <linux/kernel.h>
#include <linux/phy.h>
#include "error.h"
#include "phy_patch.h"
#include "rtk_phylib.h"

#ifndef WAIT_COMPLETE_VAR
#define WAIT_COMPLETE_VAR() \
    u32          _t, _now, _t_wait=0, _timeout;  \
    int          _chkCnt=0;

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
int osal_time_usecs_get(u32 *pUsec);
void *osal_alloc(u32 size);

#include <linux/ctype.h>    /* for Kernel Space */
#include <linux/kernel.h>
#include <linux/string.h>


#endif /* __RTK_PHY_OSAL_H */
