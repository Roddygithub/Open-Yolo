/*
 * Copyright © 2014      Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef INPUT_INHIBIT_UNSTABLE_V1_CLIENT_PROTOCOL_H
#define INPUT_INHIBIT_UNSTABLE_V1_CLIENT_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "wayland-client.h"

#define WL_HIDE_DEPRECATED

struct wl_surface;

#ifndef ZWP_INPUT_INHIBIT_MANAGER_V1_INTERFACE
#define ZWP_INPUT_INHIBIT_MANAGER_V1_INTERFACE
/**
 * @page page_iface_zwp_input_inhibit_manager_v1 zwp_input_inhibit_manager_v1
 * @section page_iface_zwp_input_inhibit_manager_v1_desc Description
 *
 * This interface is exposed as a global to inhibit input events while a
 * surface is in focus.
 * @section page_iface_zwp_input_inhibit_manager_v1_api API
 * See @ref iface_zwp_input_inhibit_manager_v1.
 */
/**
 * @defgroup iface_zwp_input_inhibit_manager_v1 The zwp_input_inhibit_manager_v1 interface
 *
 * This interface is exposed as a global to inhibit input events while a
 * surface is in focus.
 */
extern const struct wl_interface zwp_input_inhibit_manager_v1_interface;
#endif
#ifndef ZWP_INPUT_INHIBITOR_V1_INTERFACE
#define ZWP_INPUT_INHIBITOR_V1_INTERFACE
/**
 * @page page_iface_zwp_input_inhibitor_v1 zwp_input_inhibitor_v1
 * @section page_iface_zwp_input_inhibitor_v1_desc Description
 *
 * An input inhibitor prevents the compositor from processing input events
 * normally. Instead, the compositor will send them to the client holding the
 * inhibitor.
 * @section page_iface_zwp_input_inhibitor_v1_api API
 * See @ref iface_zwp_input_inhibitor_v1.
 */
/**
 * @defgroup iface_zwp_input_inhibitor_v1 The zwp_input_inhibitor_v1 interface
 *
 * An input inhibitor prevents the compositor from processing input events
 * normally. Instead, the compositor will send them to the client holding the
 * inhibitor.
 */
extern const struct wl_interface zwp_input_inhibitor_v1_interface;
#endif

#define ZWP_INPUT_INHIBIT_MANAGER_V1_GET_INHIBITOR 0

/**
 * @ingroup iface_zwp_input_inhibit_manager_v1
 */
#define ZWP_INPUT_INHIBIT_MANAGER_V1_GET_INHIBITOR_SINCE_VERSION 1

/** @ingroup iface_zwp_input_inhibit_manager_v1 */
static inline void zwp_input_inhibit_manager_v1_set_user_data(
    struct zwp_input_inhibit_manager_v1* zwp_input_inhibit_manager_v1, void* user_data) {
    wl_proxy_set_user_data((struct wl_proxy*)zwp_input_inhibit_manager_v1, user_data);
}

/** @ingroup iface_zwp_input_inhibit_manager_v1 */
static inline void* zwp_input_inhibit_manager_v1_get_user_data(
    struct zwp_input_inhibit_manager_v1* zwp_input_inhibit_manager_v1) {
    return wl_proxy_get_user_data((struct wl_proxy*)zwp_input_inhibit_manager_v1);
}

static inline uint32_t zwp_input_inhibit_manager_v1_get_version(
    struct zwp_input_inhibit_manager_v1* zwp_input_inhibit_manager_v1) {
    return wl_proxy_get_version((struct wl_proxy*)zwp_input_inhibit_manager_v1);
}

/**
 * @ingroup iface_zwp_input_inhibit_manager_v1
 *
 * Create a new input inhibitor.
 */
static inline struct zwp_input_inhibitor_v1* zwp_input_inhibit_manager_v1_get_inhibitor(
    struct zwp_input_inhibit_manager_v1* zwp_input_inhibit_manager_v1, struct wl_surface* surface) {
    struct wl_proxy* id;

    id = wl_proxy_marshal_constructor((struct wl_proxy*)zwp_input_inhibit_manager_v1,
                                      ZWP_INPUT_INHIBIT_MANAGER_V1_GET_INHIBITOR,
                                      &zwp_input_inhibitor_v1_interface, NULL, surface);

    return (struct zwp_input_inhibitor_v1*)id;
}

/**
 * @ingroup iface_zwp_input_inhibitor_v1
 */
#define ZWP_INPUT_INHIBITOR_V1_DESTROY_SINCE_VERSION 1

/** @ingroup iface_zwp_input_inhibitor_v1 */
static inline void zwp_input_inhibitor_v1_set_user_data(
    struct zwp_input_inhibitor_v1* zwp_input_inhibitor_v1, void* user_data) {
    wl_proxy_set_user_data((struct wl_proxy*)zwp_input_inhibitor_v1, user_data);
}

/** @ingroup iface_zwp_input_inhibitor_v1 */
static inline void* zwp_input_inhibitor_v1_get_user_data(
    struct zwp_input_inhibitor_v1* zwp_input_inhibitor_v1) {
    return wl_proxy_get_user_data((struct wl_proxy*)zwp_input_inhibitor_v1);
}

static inline uint32_t zwp_input_inhibitor_v1_get_version(
    struct zwp_input_inhibitor_v1* zwp_input_inhibitor_v1) {
    return wl_proxy_get_version((struct wl_proxy*)zwp_input_inhibitor_v1);
}

/**
 * @ingroup iface_zwp_input_inhibitor_v1
 *
 * Destroy the input inhibitor.
 */
static inline void zwp_input_inhibitor_v1_destroy(
    struct zwp_input_inhibitor_v1* zwp_input_inhibitor_v1) {
    wl_proxy_marshal((struct wl_proxy*)zwp_input_inhibitor_v1, ZWP_INPUT_INHIBITOR_V1_DESTROY);

    wl_proxy_destroy((struct wl_proxy*)zwp_input_inhibitor_v1);
}

#ifdef __cplusplus
}
#endif

#endif
