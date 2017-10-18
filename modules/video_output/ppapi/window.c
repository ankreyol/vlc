/**
 * @file window.c
 * @brief PPAPI native window provider module for VLC media player
 */
/*****************************************************************************
 * Copyright © 2017 VLC authors and VideoLAN
 *
 * Author: Julian Scheel <julian@jusst.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdarg.h>

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_vout_window.h>

#include <ppapi/c/ppb.h>
#include <ppapi/c/ppb_core.h>
#include <ppapi/c/ppb_graphics_3d.h>
#include <ppapi/c/ppb_instance.h>
#include <ppapi/c/pp_instance.h>

static int Open(vout_window_t*, const vout_window_cfg_t*);
static void Close(vout_window_t*);
static int Control(vout_window_t*, int, va_list ap);

/*
 * Module descriptor
 */
vlc_module_begin()
    set_shortname(N_("PPAPI Window"))
    set_description(N_("PPAPI drawing area"))
    set_category(CAT_VIDEO)
    set_subcategory(SUBCAT_VIDEO_VOUT)
    set_capability("vout window", 10)
    set_callbacks(Open, Close)
vlc_module_end()

struct vout_window_sys_t {
    PP_Resource context;
    PPB_Graphics3D *graphics3d;
    PPB_Core *core;
};

static int Open(vout_window_t *wnd, const vout_window_cfg_t *cfg)
{
    if (cfg->type != VOUT_WINDOW_TYPE_INVALID &&
            cfg->type != VOUT_WINDOW_TYPE_PPAPI)
        return VLC_EGENERIC;

    vout_window_sys_t *sys = calloc(1, sizeof (*sys));
    if (sys == NULL)
        return VLC_ENOMEM;

    wnd->sys = sys;

    PP_Instance pp_instance = (int)var_InheritInteger(wnd, "ppapi-instance");
    if (pp_instance == 0)
        goto error;

    PPB_GetInterface ppb_get_interface = var_InheritAddress(wnd, "ppapi-ppb-get-interface");
    if (ppb_get_interface == NULL) {
        msg_Err(wnd, "Variable ppapi-ppb-get-interface is not set");
        goto error;
    }

    sys->core = (PPB_Core*)ppb_get_interface(PPB_CORE_INTERFACE);
    if (sys->core == NULL) {
        msg_Err(wnd, "Failed to get PPB_CORE_INTERFACE");
        goto error;
    }

    sys->graphics3d = (PPB_Graphics3D*)ppb_get_interface(PPB_GRAPHICS_3D_INTERFACE);
    if (sys->graphics3d == NULL) {
        msg_Err(wnd, "Failed to get PPB_GRAPHICS_3D_INTERFACE");
        goto error;
    }

    int32_t attr[] = {
        PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 0,
        PP_GRAPHICS3DATTRIB_BLUE_SIZE, 8,
        PP_GRAPHICS3DATTRIB_GREEN_SIZE, 8,
        PP_GRAPHICS3DATTRIB_RED_SIZE, 8,
        PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 0,
        PP_GRAPHICS3DATTRIB_STENCIL_SIZE, 0,
        PP_GRAPHICS3DATTRIB_WIDTH, (int32_t)cfg->width,
        PP_GRAPHICS3DATTRIB_HEIGHT, (int32_t)cfg->height,
        PP_GRAPHICS3DATTRIB_GPU_PREFERENCE, PP_GRAPHICS3DATTRIB_GPU_PREFERENCE_LOW_POWER,
        PP_GRAPHICS3DATTRIB_NONE
    };
    sys->context = sys->graphics3d->Create(pp_instance, 0, attr);
    if(!sys->graphics3d->IsGraphics3D(sys->context)) {
        msg_Err(wnd, "Failed to create context");
        goto error;
    }

    PPB_Instance *ppb_instance = (PPB_Instance*)ppb_get_interface(PPB_INSTANCE_INTERFACE);
    if (ppb_instance == NULL) {
        msg_Err(wnd, "Failed to get PPB_INSTANCE_INTERFACE");
        goto error;
    }

    if (ppb_instance->BindGraphics(pp_instance, sys->context) != PP_TRUE) {
        msg_Err(wnd, "Binding PPAPI graphics context to instance failed");
        goto error;
    }

    wnd->type = VOUT_WINDOW_TYPE_PPAPI;
    wnd->handle.pp_context = sys->context;
    wnd->control = Control;

    return VLC_SUCCESS;

error:
    Close(wnd);
    return VLC_EGENERIC;
}

static void Close(vout_window_t *wnd)
{
    vout_window_sys_t *sys = wnd->sys;
    if (sys->graphics3d &&
            sys->graphics3d->IsGraphics3D(sys->context) &&
            sys->core)
        sys->core->ReleaseResource(sys->context);
    free(sys);
}

static int Control(vout_window_t *wnd, int cmd, va_list ap)
{
    VLC_UNUSED(cmd);
    VLC_UNUSED(ap);
    msg_Err(wnd, "control requests not supported");
    return VLC_EGENERIC;
}
