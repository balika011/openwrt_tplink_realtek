/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#ifndef __COMMON_ERROR_H__
#define __COMMON_ERROR_H__

/*
 * Macro Definition
 */
#define RT_PARAM_CHK(expr, errCode)\
do {\
    if ((int)(expr)) {\
        return errCode; \
    }\
} while (0)

#define RT_PARAM_CHK_EHDL(expr, errCode, err_hdl)\
do {\
    if ((int)(expr)) {\
        {err_hdl}\
        return errCode; \
    }\
} while (0)

#define RT_INIT_CHK(state)\
do {\
    if (INIT_COMPLETED != (state)) {\
        return RT_ERR_NOT_INIT;\
    }\
} while (0)

#define RT_INIT_REENTRY_CHK(state)\
do {\
    if (INIT_COMPLETED == (state)) {\
        printk("%s had already been initialized!\n", __FUNCTION__);\
        return 0;\
    }\
} while (0)

#define RT_INIT_REENTRY_CHK_NO_WARNING(state)\
    do {\
        if (INIT_COMPLETED == (state)) {\
            return 0;\
        }\
    } while (0)

#define RT_ERR_CHK(op, ret)\
do {\
    if ((ret = (op)) < 0)\
        return ret;\
} while(0)

#define RT_ERR_HDL(op, errHandle, ret)\
do {\
    if ((ret = (op)) < 0)\
        goto errHandle;\
} while(0)

#define RT_ERR_CHK_EHDL(op, ret, err_hdl)\
do {\
    if ((ret = (op)) < 0)\
    {\
        {err_hdl}\
        return ret;\
    }\
} while(0)

#define RT_NULL_HDL(pointer, err_label)\
do {\
    if (NULL == (pointer)) {\
        goto err_label;\
    }\
} while (0)

#define RT_ERR_VOID_CHK(op, ret)\
do {\
    if ((ret = (op)) < 0) {\
        printk("Fail in %s %d, ret %x!\n", __FUNCTION__, __LINE__, ret);\
        return ;}\
} while(0)

#endif /* __COMMON_ERROR_H__ */

