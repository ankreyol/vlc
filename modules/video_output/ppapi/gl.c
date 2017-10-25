/*****************************************************************************
 * gl.c: GL extension for PPAPI/NaCl
 *****************************************************************************
 * Copyright © 2017 VLC authors and VideoLAN
 *
 * Authors: Julian Scheel <julian@jusst.de>
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
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_vout_display.h>
#include <vlc_opengl.h>

#include "../opengl/vout_helper.h"

#include <ppapi/gles2/gl2ext_ppapi.h>
#include <ppapi/c/ppb_graphics_3d.h>
#include <ppapi/c/ppb_view.h>
#include <ppapi/c/pp_completion_callback.h>
#include <ppapi/c/pp_errors.h>

/*****************************************************************************
 * Module descriptor
 *****************************************************************************/
static int Open(vlc_object_t *);
static void Close(vlc_object_t *);

vlc_module_begin()
    set_shortname("PPAPI GL")
    set_description(N_("PPAPI extension for OpenGL"))
    set_category(CAT_VIDEO)
    set_subcategory(SUBCAT_VIDEO_VOUT)

    set_capability("opengl es2", 50)
    set_callbacks(Open, Close)
    add_shortcut("ppapi-gl", "gles2")
vlc_module_end()

/*****************************************************************************
 * Local prototypes.
 *****************************************************************************/

struct vout_display_sys_t
{
    unsigned width;
    unsigned height;

    PP_Resource context;
    PP_Resource viewport;
    PPB_Graphics3D *graphics3d;
    PPB_View *view;
};

static int GetViewSize(vlc_gl_t *gl, int32_t *width, int32_t *height)
{
    vout_display_sys_t *sys = gl->sys;

    struct PP_Rect rect;
    if (sys->viewport &&
            sys->view->IsView(sys->viewport) &&
            sys->view->GetRect(sys->viewport, &rect)) {
        *width = rect.size.width;
        *height = rect.size.height;
        return VLC_SUCCESS;
    }

    return VLC_EGENERIC;
}

static void Resize(vlc_gl_t *gl, unsigned w, unsigned h)
{
    vout_display_sys_t *sys = gl->sys;

    if (sys->graphics3d->ResizeBuffers(sys->context, w, h) != PP_OK)
        msg_Err(gl, "Resizing buffers failed");
}

static void Swap(vlc_gl_t *gl)
{
    vout_display_sys_t *sys = gl->sys;

    sys->viewport = var_InheritInteger(gl, "ppapi-view");

    int32_t width, height;
    if (GetViewSize(gl, &width, &height) == VLC_SUCCESS)
    {
        if (((unsigned)width != sys->width) ||
                ((unsigned)height != sys->height)) {
            vout_window_ReportSize(gl->surface, width, height);

            sys->width = width;
            sys->height = height;
        }
    }

    sys->graphics3d->SwapBuffers(sys->context, PP_BlockUntilComplete());
}

static void *GetProcAddress(vlc_gl_t *gl, const char *name)
{
    VLC_UNUSED(gl);
    VLC_UNUSED(name);
    return NULL;
}

static int MakeCurrent(vlc_gl_t *gl)
{
    vout_display_sys_t *sys = gl->sys;

    glSetCurrentContextPPAPI(sys->context);
    return VLC_SUCCESS;
}

static void ReleaseCurrent(vlc_gl_t *gl)
{
    VLC_UNUSED(gl);
    glSetCurrentContextPPAPI(0);
}

static int Open(vlc_object_t *obj)
{
    vlc_gl_t *gl = (vlc_gl_t *)obj;
    vout_display_sys_t *sys;

    /* Allocate structure */
    gl->sys = sys = calloc(1, sizeof(*sys));
    if (!sys)
        return VLC_ENOMEM;

    vout_window_t *wnd = gl->surface;
    if (wnd->type != VOUT_WINDOW_TYPE_PPAPI)
        goto error;

    sys->context = wnd->handle.pp_context;

    PPB_GetInterface ppb_get_interface = var_InheritAddress(obj, "ppapi-ppb-get-interface");
    if (ppb_get_interface == NULL) {
        msg_Err(gl, "Variable ppapi-ppb-get-interface is not set");
        goto error;
    }

    sys->view = (PPB_View*)ppb_get_interface(PPB_VIEW_INTERFACE);
    if (sys->view == NULL) {
        msg_Err(gl, "Failed to get PPB_VIEW_INTERFACE");
        goto error;
    }

    sys->graphics3d = (PPB_View*)ppb_get_interface(PPB_GRAPHICS_3D_INTERFACE);
    if (sys->view == NULL) {
        msg_Err(gl, "Failed to get PPB_GRAPHICS_3D_INTERFACE");
        goto error;
    }

    gl->makeCurrent = MakeCurrent;
    gl->releaseCurrent = ReleaseCurrent;
    gl->resize = Resize;
    gl->swap = Swap;
    gl->getProcAddress = GetProcAddress;

    return VLC_SUCCESS;

error:
    Close(obj);
    return VLC_EGENERIC;
}

static void Close(vlc_object_t *obj)
{
    vlc_gl_t *gl = (vlc_gl_t *)obj;
    free(gl->sys);
}
