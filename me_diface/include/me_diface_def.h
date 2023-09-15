/* My ESP HMI defaults
 * File: me_diface_def.h
 * Started: Thu Aug 31 14:41:14 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */

#ifndef _ME_DIFACE_DEF_H
#define _ME_DIFACE_DEF_H

#include <me_diface.h>

me_diface_range_t me_diface_range_0_100[1] = {
    {
        .min = 0,
        .max = 100,
        .step = 0.1,
        .stepk = 5
    }
};

me_str_t    me_diface_def_str_zero   = me_str("zero");
me_str_t    me_diface_def_str_span   = me_str("span");
me_str_t    me_diface_def_str_set    = me_str("set ");
me_str_t    me_diface_def_str_cancel = me_str("cncl");
me_str_t    me_diface_def_str_ok     = me_str(" ok ");

#endif

