/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#include "error.h"
#include "rtk_phylib_def.h"
#include "rtk_osal.h"

#include <linux/version.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/sched/signal.h>
#include <linux/phy.h>

int
osal_time_usecs_get(u32 *pUsec)
{
    struct timespec64 ts;

    RT_PARAM_CHK((NULL == pUsec), RT_ERR_NULL_POINTER);

    ktime_get_ts64(&ts);
	*pUsec = (u32)((ts.tv_sec * USEC_PER_SEC) + (ts.tv_nsec / NSEC_PER_USEC));
	return RT_ERR_OK;
}

void *
osal_alloc(u32 size)
{
    void *p;
    p = kmalloc((size_t)size, GFP_ATOMIC);
    return p;
}

int
phy_common_general_reg_mmd_get(struct phy_device *phydev, u32 mmdAddr, u32 mmdReg, u32 *pData)
{
    int rData = 0;
    rData = phy_read_mmd(phydev, mmdAddr, mmdReg);
    if (rData < 0)
        return RT_ERR_FAILED;
    *pData = (u32)rData;
    return RT_ERR_OK;
}

int
phy_common_general_reg_mmd_set(struct phy_device *phydev, u32 mmdAddr, u32 mmdReg, u32 data)
{
    int ret = phy_write_mmd(phydev, mmdAddr, mmdReg, data);
    return (ret < 0) ? RT_ERR_FAILED : RT_ERR_OK;
}
