/* My ESP debug functions
 * File: me_debug.h
 * Started: Wed Jun 16 13:38:07 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#ifndef _ME_DEBUG_H
#define _ME_DEBUG_H

#include <esp_log.h>
#include <esp_err.h>

#if CONFIG_ME_DEBUG
#define me_debug(T, ...) ESP_LOGI( "ME_DBG-" T, __VA_ARGS__ )
#define me_debug_assert(E) do { if( !(E) ) abort(); } while(0)
#else
#define me_debug(...)
#define me_debug_assert(...)
#endif

#endif