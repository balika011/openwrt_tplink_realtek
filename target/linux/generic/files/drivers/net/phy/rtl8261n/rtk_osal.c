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

    RT_PARAM_CHK((NULL == pUsec), -EINVAL);

    ktime_get_ts64(&ts);
    *pUsec = (u32)((ts.tv_sec * USEC_PER_SEC) + (ts.tv_nsec / NSEC_PER_USEC));
    return 0;
}

void *
osal_alloc(u32 size)
{
    return kmalloc((size_t)size, GFP_ATOMIC);
}