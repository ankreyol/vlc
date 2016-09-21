/*****************************************************************************
 * hotkeys.c: Hotkey handling for vlc
 *****************************************************************************
 * Copyright (C) 2005-2016 the VideoLAN team
 * $Id$
 *
 * Authors: Sigmund Augdal Helberg <dnumgis@videolan.org>
 *          Jean-Paul Saman <jpsaman #_at_# m2x.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define VLC_MODULE_LICENSE VLC_LICENSE_GPL_2_PLUS
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_actions.h>
#include <vlc_interface.h>
#include <vlc_input.h>
#include <vlc_playlist.h>

#include <assert.h>

/*****************************************************************************
 * intf_sys_t: description and status of FB interface
 *****************************************************************************/
struct intf_sys_t
{
    vlc_mutex_t         lock;
    vout_thread_t      *p_vout;
    input_thread_t     *p_input;

    struct
    {
        bool b_can_change;
        bool b_button_pressed;
        int x, y;
    } vrnav;
};

/*****************************************************************************
 * Local prototypes
 *****************************************************************************/
static int  Open    ( vlc_object_t * );
static void Close   ( vlc_object_t * );
static int ActionEvent( vlc_object_t *libvlc, char const *psz_var,
                        vlc_value_t oldval, vlc_value_t newval, void *p_data );

/*****************************************************************************
 * Module descriptor
 *****************************************************************************/

vlc_module_begin ()
    set_shortname( N_("Hotkeys") )
    set_description( N_("Hotkeys management interface") )
    set_capability( "interface", 0 )
    set_callbacks( Open, Close )
    set_category( CAT_INTERFACE )
    set_subcategory( SUBCAT_INTERFACE_HOTKEYS )

vlc_module_end ()

static int MovedEvent( vlc_object_t *p_this, char const *psz_var,
                       vlc_value_t oldval, vlc_value_t newval, void *p_data )
{
    intf_thread_t *p_intf = (intf_thread_t *)p_data;
    intf_sys_t    *p_sys = p_intf->p_sys;

    (void) p_this; (void) psz_var; (void) oldval;

    if( p_sys->vrnav.b_button_pressed )
    {
        int i_horizontal = newval.coords.x - p_sys->vrnav.x;
        int i_vertical   = newval.coords.y - p_sys->vrnav.y;

        vlc_viewpoint_t viewpoint = {
            .yaw   = -i_horizontal * 0.05f,
            .pitch = -i_vertical   * 0.05f,
        };

        vlc_actions_do( p_this, ACTIONID_VIEWPOINT_UPDATE, false, &viewpoint );

        p_sys->vrnav.x = newval.coords.x;
        p_sys->vrnav.y = newval.coords.y;
    }

    return VLC_SUCCESS;
}

static int ViewpointMovedEvent( vlc_object_t *p_this, char const *psz_var,
                                vlc_value_t oldval, vlc_value_t newval,
                                void *p_data )
{
    (void) psz_var; (void) oldval; (void) p_data;

    vlc_actions_do( p_this, ACTIONID_VIEWPOINT_UPDATE, false, newval.p_address );

    return VLC_SUCCESS;
}

static int ButtonEvent( vlc_object_t *p_this, char const *psz_var,
                        vlc_value_t oldval, vlc_value_t newval, void *p_data )
{
    intf_thread_t *p_intf = p_data;
    intf_sys_t *p_sys = p_intf->p_sys;

    (void) psz_var; (void) oldval;

    if( newval.i_int & 0x01 )
    {
        if( !p_sys->vrnav.b_button_pressed )
        {
            p_sys->vrnav.b_button_pressed = true;
            var_GetCoords( p_this, "mouse-moved",
                           &p_sys->vrnav.x, &p_sys->vrnav.y );
        }
    }
    else
        p_sys->vrnav.b_button_pressed = false;

    return VLC_SUCCESS;
}

static void ChangeVout( intf_thread_t *p_intf, vout_thread_t *p_vout )
{
    intf_sys_t *p_sys = p_intf->p_sys;

    bool b_vrnav_can_change;
    if( p_vout != NULL )
        b_vrnav_can_change = var_GetBool( p_vout, "viewpoint-changeable" );

    vlc_mutex_lock( &p_sys->lock );
    vout_thread_t *p_old_vout = p_sys->p_vout;
    bool b_vrnav_could_change = p_sys->vrnav.b_can_change;
    p_sys->p_vout = p_vout;
    if( p_vout != NULL )
        p_sys->vrnav.b_can_change = b_vrnav_can_change;
    else
        p_sys->vrnav.b_can_change = false;
    vlc_mutex_unlock( &p_sys->lock );

    if( p_old_vout != NULL )
    {
        if( b_vrnav_could_change )
        {
            var_DelCallback( p_old_vout, "mouse-moved", MovedEvent,
                             p_intf );
            var_DelCallback( p_old_vout, "mouse-button-down", ButtonEvent,
                             p_intf );
            var_DelCallback( p_old_vout, "viewpoint-moved", ViewpointMovedEvent,
                             p_intf );
        }
        vlc_object_release( p_old_vout );
    }

    if( p_sys->vrnav.b_can_change )
    {
        assert( p_sys->p_vout != NULL );
        var_AddCallback( p_sys->p_vout, "mouse-moved", MovedEvent,
                         p_intf );
        var_AddCallback( p_sys->p_vout, "mouse-button-down", ButtonEvent,
                         p_intf );
        var_AddCallback( p_sys->p_vout, "viewpoint-moved", ViewpointMovedEvent,
                         p_intf );
    }
}

static int InputEvent( vlc_object_t *p_this, char const *psz_var,
                       vlc_value_t oldval, vlc_value_t val, void *p_data )
{
    input_thread_t *p_input = (input_thread_t *)p_this;
    intf_thread_t *p_intf = p_data;

    (void) psz_var; (void) oldval;

    if( val.i_int == INPUT_EVENT_VOUT )
        ChangeVout( p_intf, input_GetVout( p_input ) );

    return VLC_SUCCESS;
}

static void ChangeInput( intf_thread_t *p_intf, input_thread_t *p_input )
{
    intf_sys_t *p_sys = p_intf->p_sys;

    input_thread_t *p_old_input = p_sys->p_input;
    vout_thread_t *p_old_vout = NULL;
    if( p_old_input != NULL )
    {
        /* First, remove callbacks from previous input. It's safe to access it
         * unlocked, since it's written from this thread */
        var_DelCallback( p_old_input, "intf-event", InputEvent, p_intf );

        p_old_vout = p_sys->p_vout;
        /* Remove mouse events before setting new input, since callbacks may
         * access it */
        if( p_old_vout != NULL && p_sys->vrnav.b_can_change )
        {
            var_DelCallback( p_old_vout, "mouse-moved", MovedEvent,
                             p_intf );
            var_DelCallback( p_old_vout, "mouse-button-down", ButtonEvent,
                             p_intf );
            var_DelCallback( p_old_vout, "viewpoint-moved", ViewpointMovedEvent,
                             p_intf );
        }
    }

    /* Replace input and vout locked */
    vlc_mutex_lock( &p_sys->lock );
    p_sys->p_input = p_input ? vlc_object_hold( p_input ) : NULL;
    p_sys->p_vout = NULL;
    p_sys->vrnav.b_can_change = false;
    vlc_mutex_unlock( &p_sys->lock );

    /* Release old input and vout objects unlocked */
    if( p_old_input != NULL )
    {
        if( p_old_vout != NULL )
            vlc_object_release( p_old_vout );
        vlc_object_release( p_old_input );
    }

    /* Register input events */
    if( p_input != NULL )
        var_AddCallback( p_input, "intf-event", InputEvent, p_intf );
}

static int PlaylistEvent( vlc_object_t *p_this, char const *psz_var,
                          vlc_value_t oldval, vlc_value_t val, void *p_data )
{
    intf_thread_t *p_intf = p_data;

    (void) p_this; (void) psz_var; (void) oldval;

    ChangeInput( p_intf, val.p_address );

    return VLC_SUCCESS;
}

/*****************************************************************************
 * Open: initialize interface
 *****************************************************************************/
static int Open( vlc_object_t *p_this )
{
    intf_thread_t *p_intf = (intf_thread_t *)p_this;
    intf_sys_t *p_sys;
    p_sys = malloc( sizeof( intf_sys_t ) );
    if( !p_sys )
        return VLC_ENOMEM;

    p_intf->p_sys = p_sys;

    p_sys->p_vout = NULL;
    p_sys->p_input = NULL;
    p_sys->vrnav.b_can_change = false;
    p_sys->vrnav.b_button_pressed = false;

    vlc_mutex_init( &p_sys->lock );

    var_AddCallback( p_intf->obj.libvlc, "key-action", ActionEvent, p_intf );

    var_AddCallback( pl_Get(p_intf), "input-current", PlaylistEvent, p_intf );

    return VLC_SUCCESS;
}

/*****************************************************************************
 * Close: destroy interface
 *****************************************************************************/
static void Close( vlc_object_t *p_this )
{
    intf_thread_t *p_intf = (intf_thread_t *)p_this;
    intf_sys_t *p_sys = p_intf->p_sys;

    var_DelCallback( pl_Get(p_intf), "input-current", PlaylistEvent, p_intf );

    var_DelCallback( p_intf->obj.libvlc, "key-action", ActionEvent, p_intf );

    ChangeInput( p_intf, NULL );

    vlc_mutex_destroy( &p_sys->lock );

    /* Destroy structure */
    free( p_sys );
}

/*****************************************************************************
 * ActionEvent: callback for hotkey actions
 *****************************************************************************/
static int ActionEvent( vlc_object_t *libvlc, char const *psz_var,
                        vlc_value_t oldval, vlc_value_t newval, void *p_data )
{
    (void)libvlc;
    (void)psz_var;
    (void)oldval;

    vlc_object_t *p_this = p_data;
    return vlc_actions_do( p_this, newval.i_int, true );
}
