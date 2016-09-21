/*****************************************************************************
 * actions.c: handle vlc actions
 *****************************************************************************
 * Copyright (C) 2003-2016 VLC authors and VideoLAN
 *
 * Authors: Sigmund Augdal Helberg <dnumgis@videolan.org>
 *          Jean-Paul Saman <jpsaman #_at_# m2x.nl>
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

/**
 * \file
 * This file defines functions and structures for hotkey handling in vlc
 */

#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#ifdef HAVE_SEARCH_H
# include <search.h>
#endif
#include <errno.h>
#include <math.h>

#include <vlc_common.h>
#include <vlc_actions.h>
#include <vlc_charset.h>
#include <vlc_interface.h>
#include <vlc_input.h>
#include <vlc_playlist.h>
#include <vlc_aout.h>
#include <vlc_vout.h>
#include <vlc_vout_osd.h>
#include "config/configuration.h"
#include "libvlc.h"

static const struct key_descriptor
{
    const char psz[20];
    uint32_t i_code;
} s_keys[] =
{   /* Alphabetical order */
    { N_("Backspace"),         KEY_BACKSPACE         },
    { N_("Brightness Down"),   KEY_BRIGHTNESS_DOWN   },
    { N_("Brightness Up"),     KEY_BRIGHTNESS_UP     },
    { N_("Browser Back"),      KEY_BROWSER_BACK      },
    { N_("Browser Favorites"), KEY_BROWSER_FAVORITES },
    { N_("Browser Forward"),   KEY_BROWSER_FORWARD   },
    { N_("Browser Home"),      KEY_BROWSER_HOME      },
    { N_("Browser Refresh"),   KEY_BROWSER_REFRESH   },
    { N_("Browser Search"),    KEY_BROWSER_SEARCH    },
    { N_("Browser Stop"),      KEY_BROWSER_STOP      },
    { N_("Delete"),            KEY_DELETE            },
    { N_("Down"),              KEY_DOWN              },
    { N_("End"),               KEY_END               },
    { N_("Enter"),             KEY_ENTER             },
    { N_("Esc"),               KEY_ESC               },
    { N_("F1"),                KEY_F1                },
    { N_("F10"),               KEY_F10               },
    { N_("F11"),               KEY_F11               },
    { N_("F12"),               KEY_F12               },
    { N_("F2"),                KEY_F2                },
    { N_("F3"),                KEY_F3                },
    { N_("F4"),                KEY_F4                },
    { N_("F5"),                KEY_F5                },
    { N_("F6"),                KEY_F6                },
    { N_("F7"),                KEY_F7                },
    { N_("F8"),                KEY_F8                },
    { N_("F9"),                KEY_F9                },
    { N_("Home"),              KEY_HOME              },
    { N_("Insert"),            KEY_INSERT            },
    { N_("Left"),              KEY_LEFT              },
    { N_("Media Angle"),       KEY_MEDIA_ANGLE       },
    { N_("Media Audio Track"), KEY_MEDIA_AUDIO       },
    { N_("Media Forward"),     KEY_MEDIA_FORWARD     },
    { N_("Media Menu"),        KEY_MEDIA_MENU        },
    { N_("Media Next Frame"),  KEY_MEDIA_FRAME_NEXT  },
    { N_("Media Next Track"),  KEY_MEDIA_NEXT_TRACK  },
    { N_("Media Play Pause"),  KEY_MEDIA_PLAY_PAUSE  },
    { N_("Media Prev Frame"),  KEY_MEDIA_FRAME_PREV  },
    { N_("Media Prev Track"),  KEY_MEDIA_PREV_TRACK  },
    { N_("Media Record"),      KEY_MEDIA_RECORD      },
    { N_("Media Repeat"),      KEY_MEDIA_REPEAT      },
    { N_("Media Rewind"),      KEY_MEDIA_REWIND      },
    { N_("Media Select"),      KEY_MEDIA_SELECT      },
    { N_("Media Shuffle"),     KEY_MEDIA_SHUFFLE     },
    { N_("Media Stop"),        KEY_MEDIA_STOP        },
    { N_("Media Subtitle"),    KEY_MEDIA_SUBTITLE    },
    { N_("Media Time"),        KEY_MEDIA_TIME        },
    { N_("Media View"),        KEY_MEDIA_VIEW        },
    { N_("Menu"),              KEY_MENU              },
    { N_("Mouse Wheel Down"),  KEY_MOUSEWHEELDOWN    },
    { N_("Mouse Wheel Left"),  KEY_MOUSEWHEELLEFT    },
    { N_("Mouse Wheel Right"), KEY_MOUSEWHEELRIGHT   },
    { N_("Mouse Wheel Up"),    KEY_MOUSEWHEELUP      },
    { N_("Page Down"),         KEY_PAGEDOWN          },
    { N_("Page Up"),           KEY_PAGEUP            },
    { N_("Pause"),             KEY_PAUSE             },
    { N_("Print"),             KEY_PRINT             },
    { N_("Right"),             KEY_RIGHT             },
    { N_("Space"),             ' '                   },
    { N_("Tab"),               KEY_TAB               },
    { N_("Unset"),             KEY_UNSET             },
    { N_("Up"),                KEY_UP                },
    { N_("Volume Down"),       KEY_VOLUME_DOWN       },
    { N_("Volume Mute"),       KEY_VOLUME_MUTE       },
    { N_("Volume Up"),         KEY_VOLUME_UP         },
    { N_("Zoom In"),           KEY_ZOOM_IN           },
    { N_("Zoom Out"),          KEY_ZOOM_OUT          },
};
#define KEYS_COUNT (sizeof(s_keys)/sizeof(s_keys[0]))

static int keystrcmp (const void *key, const void *elem)
{
    const char *sa = key, *sb = elem;
    return strcmp (sa, sb);
}

/* Convert Unicode code point to UTF-8 */
static char *utf8_cp (uint_fast32_t cp, char *buf)
{
    if (cp < (1 << 7))
    {
        buf[1] = 0;
        buf[0] = cp;
    }
    else if (cp < (1 << 11))
    {
        buf[2] = 0;
        buf[1] = 0x80 | (cp & 0x3F);
        cp >>= 6;
        buf[0] = 0xC0 | cp;
    }
    else if (cp < (1 << 16))
    {
        buf[3] = 0;
        buf[2] = 0x80 | (cp & 0x3F);
        cp >>= 6;
        buf[1] = 0x80 | (cp & 0x3F);
        cp >>= 6;
        buf[0] = 0xE0 | cp;
    }
    else if (cp < (1 << 21))
    {
        buf[4] = 0;
        buf[3] = 0x80 | (cp & 0x3F);
        cp >>= 6;
        buf[2] = 0x80 | (cp & 0x3F);
        cp >>= 6;
        buf[1] = 0x80 | (cp & 0x3F);
        cp >>= 6;
        buf[0] = 0xE0 | cp;
    }
    else
        return NULL;
    return buf;
}

/**
 * Parse a human-readable string representation of a VLC key code.
 * @note This only works with the American English representation
 * (a.k.a. C or POSIX), not with the local representation returned from
 * vlc_keycode2str().
 * @return a VLC key code, or KEY_UNSET on failure.
 */
uint_fast32_t vlc_str2keycode (const char *name)
{
    uint_fast32_t mods = 0;
    uint32_t code;

    for (;;)
    {
        size_t len = strcspn (name, "-+");
        if (len == 0 || name[len] == '\0')
            break;

        if (len == 4 && !strncasecmp (name, "Ctrl", 4))
            mods |= KEY_MODIFIER_CTRL;
        if (len == 3 && !strncasecmp (name, "Alt", 3))
            mods |= KEY_MODIFIER_ALT;
        if (len == 5 && !strncasecmp (name, "Shift", 5))
            mods |= KEY_MODIFIER_SHIFT;
        if (len == 4 && !strncasecmp (name, "Meta", 4))
            mods |= KEY_MODIFIER_META;
        if (len == 7 && !strncasecmp (name, "Command", 7))
            mods |= KEY_MODIFIER_COMMAND;

        name += len + 1;
    }

    struct key_descriptor *d = bsearch (name, s_keys, KEYS_COUNT,
                                        sizeof (s_keys[0]), keystrcmp);
    if (d != NULL)
        code = d->i_code;
    else
    if (vlc_towc (name, &code) <= 0)
        code = KEY_UNSET;

    if (code != KEY_UNSET)
        code |= mods;
    return code;
}

static char *nooptext (const char *txt)
{
    return (char *)txt;
}

/**
 * Format a human-readable and unique representation of a VLC key code
 * (including modifiers).
 * @param code key code to translate to a string
 * @param locale true to get a localized string,
 *               false to get a C string suitable for 'vlcrc'
 * @return a heap-allocated string, or NULL on error.
 */
char *vlc_keycode2str (uint_fast32_t code, bool locale)
{
    char *(*tr) (const char *) = locale ? vlc_gettext : nooptext;
    const char *name;
    char *str, buf[5];
    uintptr_t key = code & ~KEY_MODIFIER;

    for (size_t i = 0; i < KEYS_COUNT; i++)
        if (s_keys[i].i_code == key)
        {
            name = s_keys[i].psz;
            goto found;
        }

    if (utf8_cp (key, buf) == NULL)
        return NULL;
    name = buf;

found:
    if (asprintf (&str, "%s%s%s%s%s%s",
                  (code & KEY_MODIFIER_CTRL) ? tr(N_("Ctrl+")) : "",
                  (code & KEY_MODIFIER_ALT) ? tr(N_("Alt+")) : "",
                  (code & KEY_MODIFIER_SHIFT) ? tr(N_("Shift+")) : "",
                  (code & KEY_MODIFIER_META) ? tr(N_("Meta+")) : "",
                  (code & KEY_MODIFIER_COMMAND) ? tr(N_("Command+")) : "",
                  tr(name)) == -1)
        return NULL;
    return str;
}


/*** VLC key map ***/

#define MAXACTION 26
static const struct name2action
{
    char psz[MAXACTION];
    vlc_action_id_t id;
} s_names2actions[] =
{
    /* *MUST* be sorted (ASCII order) */
    { "aspect-ratio", ACTIONID_ASPECT_RATIO, },
    { "audio-track", ACTIONID_AUDIO_TRACK, },
    { "audiodelay-down", ACTIONID_AUDIODELAY_DOWN, },
    { "audiodelay-up", ACTIONID_AUDIODELAY_UP, },
    { "audiodevice-cycle", ACTIONID_AUDIODEVICE_CYCLE, },
    { "chapter-next", ACTIONID_CHAPTER_NEXT, },
    { "chapter-prev", ACTIONID_CHAPTER_PREV, },
    { "clear-playlist", ACTIONID_PLAY_CLEAR, },
    { "crop", ACTIONID_CROP, },
    { "crop-bottom", ACTIONID_CROP_BOTTOM, },
    { "crop-left", ACTIONID_CROP_LEFT, },
    { "crop-right", ACTIONID_CROP_RIGHT, },
    { "crop-top", ACTIONID_CROP_TOP, },
    { "decr-scalefactor", ACTIONID_SCALE_DOWN, },
    { "deinterlace", ACTIONID_DEINTERLACE, },
    { "deinterlace-mode", ACTIONID_DEINTERLACE_MODE, },
    { "disc-menu", ACTIONID_DISC_MENU, },
    { "faster", ACTIONID_FASTER, },
    { "frame-next", ACTIONID_FRAME_NEXT, },
    { "incr-scalefactor", ACTIONID_SCALE_UP, },
    { "intf-boss", ACTIONID_INTF_BOSS, },
    { "intf-popup-menu", ACTIONID_INTF_POPUP_MENU, },
    { "intf-show", ACTIONID_INTF_TOGGLE_FSC, },
    { "jump+extrashort", ACTIONID_JUMP_FORWARD_EXTRASHORT, },
    { "jump+long", ACTIONID_JUMP_FORWARD_LONG, },
    { "jump+medium", ACTIONID_JUMP_FORWARD_MEDIUM, },
    { "jump+short", ACTIONID_JUMP_FORWARD_SHORT, },
    { "jump-extrashort", ACTIONID_JUMP_BACKWARD_EXTRASHORT, },
    { "jump-long", ACTIONID_JUMP_BACKWARD_LONG, },
    { "jump-medium", ACTIONID_JUMP_BACKWARD_MEDIUM, },
    { "jump-short", ACTIONID_JUMP_BACKWARD_SHORT, },
    { "leave-fullscreen", ACTIONID_LEAVE_FULLSCREEN, },
    { "loop", ACTIONID_LOOP, },
    { "nav-activate", ACTIONID_NAV_ACTIVATE, },
    { "nav-down", ACTIONID_NAV_DOWN, },
    { "nav-left", ACTIONID_NAV_LEFT, },
    { "nav-right", ACTIONID_NAV_RIGHT, },
    { "nav-up", ACTIONID_NAV_UP, },
    { "next", ACTIONID_NEXT, },
    { "pause", ACTIONID_PAUSE, },
    { "play", ACTIONID_PLAY, },
    { "play-bookmark1", ACTIONID_PLAY_BOOKMARK1, },
    { "play-bookmark10", ACTIONID_PLAY_BOOKMARK10, },
    { "play-bookmark2", ACTIONID_PLAY_BOOKMARK2, },
    { "play-bookmark3", ACTIONID_PLAY_BOOKMARK3, },
    { "play-bookmark4", ACTIONID_PLAY_BOOKMARK4, },
    { "play-bookmark5", ACTIONID_PLAY_BOOKMARK5, },
    { "play-bookmark6", ACTIONID_PLAY_BOOKMARK6, },
    { "play-bookmark7", ACTIONID_PLAY_BOOKMARK7, },
    { "play-bookmark8", ACTIONID_PLAY_BOOKMARK8, },
    { "play-bookmark9", ACTIONID_PLAY_BOOKMARK9, },
    { "play-pause", ACTIONID_PLAY_PAUSE, },
    { "position", ACTIONID_POSITION, },
    { "prev", ACTIONID_PREV, },
    { "program-sid-next", ACTIONID_PROGRAM_SID_NEXT, },
    { "program-sid-prev", ACTIONID_PROGRAM_SID_PREV, },
    { "quit", ACTIONID_QUIT, },
    { "random", ACTIONID_RANDOM, },
    { "rate-faster-fine", ACTIONID_RATE_FASTER_FINE, },
    { "rate-normal", ACTIONID_RATE_NORMAL, },
    { "rate-slower-fine", ACTIONID_RATE_SLOWER_FINE, },
    { "record", ACTIONID_RECORD, },
    { "set-bookmark1", ACTIONID_SET_BOOKMARK1, },
    { "set-bookmark10", ACTIONID_SET_BOOKMARK10, },
    { "set-bookmark2", ACTIONID_SET_BOOKMARK2, },
    { "set-bookmark3", ACTIONID_SET_BOOKMARK3, },
    { "set-bookmark4", ACTIONID_SET_BOOKMARK4, },
    { "set-bookmark5", ACTIONID_SET_BOOKMARK5, },
    { "set-bookmark6", ACTIONID_SET_BOOKMARK6, },
    { "set-bookmark7", ACTIONID_SET_BOOKMARK7, },
    { "set-bookmark8", ACTIONID_SET_BOOKMARK8, },
    { "set-bookmark9", ACTIONID_SET_BOOKMARK9, },
    { "slower", ACTIONID_SLOWER, },
    { "snapshot", ACTIONID_SNAPSHOT, },
    { "stop", ACTIONID_STOP, },
    { "subdelay-down", ACTIONID_SUBDELAY_DOWN, },
    { "subdelay-up", ACTIONID_SUBDELAY_UP, },
    { "subpos-down", ACTIONID_SUBPOS_DOWN, },
    { "subpos-up", ACTIONID_SUBPOS_UP, },
    { "subsync-apply", ACTIONID_SUBSYNC_APPLY, },
    { "subsync-markaudio", ACTIONID_SUBSYNC_MARKAUDIO, },
    { "subsync-marksub", ACTIONID_SUBSYNC_MARKSUB, },
    { "subsync-reset", ACTIONID_SUBSYNC_RESET, },
    { "subtitle-revtrack", ACTIONID_SUBTITLE_REVERSE_TRACK, },
    { "subtitle-text-scale-down", ACTIONID_SUBTITLE_TEXT_SCALE_DOWN, },
    { "subtitle-text-scale-normal", ACTIONID_SUBTITLE_TEXT_SCALE_NORMAL, },
    { "subtitle-text-scale-up", ACTIONID_SUBTITLE_TEXT_SCALE_UP, },
    { "subtitle-toggle", ACTIONID_SUBTITLE_TOGGLE, },
    { "subtitle-track", ACTIONID_SUBTITLE_TRACK, },
    { "title-next", ACTIONID_TITLE_NEXT, },
    { "title-prev", ACTIONID_TITLE_PREV, },
    { "toggle-autoscale", ACTIONID_TOGGLE_AUTOSCALE, },
    { "toggle-fullscreen", ACTIONID_TOGGLE_FULLSCREEN, },
    { "uncrop-bottom", ACTIONID_UNCROP_BOTTOM, },
    { "uncrop-left", ACTIONID_UNCROP_LEFT, },
    { "uncrop-right", ACTIONID_UNCROP_RIGHT, },
    { "uncrop-top", ACTIONID_UNCROP_TOP, },
    { "unzoom", ACTIONID_UNZOOM, },
    { "viewpoint-fov-in", ACTIONID_VIEWPOINT_FOV_IN, },
    { "viewpoint-fov-out", ACTIONID_VIEWPOINT_FOV_OUT, },
    { "viewpoint-roll-anticlock", ACTIONID_VIEWPOINT_ROLL_ANTICLOCK, },
    { "viewpoint-roll-clock", ACTIONID_VIEWPOINT_ROLL_CLOCK, },
    { "vol-down", ACTIONID_VOL_DOWN, },
    { "vol-mute", ACTIONID_VOL_MUTE, },
    { "vol-up", ACTIONID_VOL_UP, },
    { "wallpaper", ACTIONID_WALLPAPER, },
    { "zoom", ACTIONID_ZOOM, },
    { "zoom-double", ACTIONID_ZOOM_DOUBLE, },
    { "zoom-half", ACTIONID_ZOOM_HALF, },
    { "zoom-original", ACTIONID_ZOOM_ORIGINAL, },
    { "zoom-quarter", ACTIONID_ZOOM_QUARTER, },
};
#define ACTIONS_COUNT (sizeof (s_names2actions) / sizeof (s_names2actions[0]))

struct mapping
{
    uint32_t     key; ///< Key code
    vlc_action_id_t action; ///< Action ID
};

static int keycmp (const void *a, const void *b)
{
    const struct mapping *ka = a, *kb = b;

    return (ka->key < kb->key) ? -1 : (ka->key > kb->key) ? +1 : 0;
}

struct vlc_actions_t
{
    /*subtitle_delaybookmarks: placeholder for storing subtitle sync timestamps*/
    struct
    {
        int64_t i_time_subtitle;
        int64_t i_time_audio;
    } subtitle_delaybookmarks;

    vout_thread_t *p_last_vout;
    int slider_chan;

    void *map; /* Key map */
    void *global_map; /* Grabbed/global key map */
    const char *ppsz_keys[];
};

static int vlc_key_to_action (vlc_object_t *obj, const char *varname,
                              vlc_value_t prevkey, vlc_value_t curkey, void *d)
{
    void *const *map = d;
    const struct mapping **pent;
    uint32_t keycode = curkey.i_int;

    pent = tfind (&keycode, map, keycmp);
    if (pent == NULL)
        return VLC_SUCCESS;

    (void) varname;
    (void) prevkey;
    return var_SetInteger (obj, "key-action", (*pent)->action);
}

/**
 * Adds a mapping from a certain key code to a certain action.
 */
static int add_mapping (void **map, uint32_t keycode, vlc_action_id_t action)
{
    struct mapping *entry = malloc (sizeof (*entry));
    if (entry == NULL)
        return ENOMEM;
    entry->key = keycode;
    entry->action = action;

    struct mapping **pent = tsearch (entry, map, keycmp);
    if (unlikely(pent == NULL))
        return ENOMEM;
    if (*pent != entry)
    {
        free (entry);
        return EEXIST;
    }
    return 0;
}

static void add_wheel_mapping (void **map, uint32_t kmore, uint32_t kless,
                                 int mode)
{
    vlc_action_id_t amore = ACTIONID_NONE, aless = ACTIONID_NONE;

    switch (mode)
    {
        case 0: /* volume up/down */
            amore = ACTIONID_COMBO_VOL_FOV_UP;
            aless = ACTIONID_COMBO_VOL_FOV_DOWN;
            break;
        case 2: /* position latter/earlier */
            amore = ACTIONID_JUMP_FORWARD_EXTRASHORT;
            aless = ACTIONID_JUMP_BACKWARD_EXTRASHORT;
            break;
        case 3: /* position earlier/latter */
            amore = ACTIONID_JUMP_BACKWARD_EXTRASHORT;
            aless = ACTIONID_JUMP_FORWARD_EXTRASHORT;
            break;
    }

    if (amore != ACTIONID_NONE)
        add_mapping (map, kmore, amore);
    if (aless != ACTIONID_NONE)
        add_mapping (map, kless, aless);
}


/**
 * Sets up all key mappings for a given action.
 * \param map tree (of struct mapping entries) to write mappings to
 * \param confname VLC configuration item to read mappings from
 * \param action action ID
 */
static void init_action (vlc_object_t *obj, void **map,
                            const char *confname, vlc_action_id_t action)
{
    char *keys = var_InheritString (obj, confname);
    if (keys == NULL)
        return;

    for (char *buf, *key = strtok_r (keys, "\t", &buf);
         key != NULL;
         key = strtok_r (NULL, "\t", &buf))
    {
        uint32_t code = vlc_str2keycode (key);
        if (code == KEY_UNSET)
        {
            msg_Warn (obj, "Key \"%s\" unrecognized", key);
            continue;
        }

        if (add_mapping (map, code, action) == EEXIST)
            msg_Warn (obj, "Key \"%s\" bound to multiple actions", key);
    }
    free (keys);
}

/**
 * Initializes the key map from configuration.
 */
int libvlc_InternalActionsInit (libvlc_int_t *libvlc)
{
    assert(libvlc != NULL);

    vlc_object_t *obj = VLC_OBJECT(libvlc);
    vlc_actions_t *as = malloc (sizeof (*as) + (1 + ACTIONS_COUNT)
                      * sizeof (*as->ppsz_keys));

    if (unlikely(as == NULL))
        return VLC_ENOMEM;
    as->map = NULL;
    as->global_map = NULL;
    as->p_last_vout = NULL;
    as->subtitle_delaybookmarks.i_time_audio = 0;
    as->subtitle_delaybookmarks.i_time_subtitle = 0;

    var_Create (obj, "key-pressed", VLC_VAR_INTEGER);
    var_Create (obj, "global-key-pressed", VLC_VAR_INTEGER);
    var_Create (obj, "key-action", VLC_VAR_INTEGER);

    /* Initialize from configuration */
    for (size_t i = 0; i < ACTIONS_COUNT; i++)
    {
#ifndef NDEBUG
        if (i > 0
         && strcmp (s_names2actions[i-1].psz, s_names2actions[i].psz) >= 0)
        {
            msg_Err (libvlc, "key-%s and key-%s are not ordered properly",
                     s_names2actions[i-1].psz, s_names2actions[i].psz);
            abort ();
        }
#endif
        as->ppsz_keys[i] = s_names2actions[i].psz;

        char name[12 + MAXACTION];

        snprintf (name, sizeof (name), "global-key-%s", s_names2actions[i].psz);
        init_action (obj, &as->map, name + 7, s_names2actions[i].id);
        init_action (obj, &as->global_map, name, s_names2actions[i].id);
    }
    as->ppsz_keys[ACTIONS_COUNT] = NULL;

    /* Initialize mouse wheel events */
    add_wheel_mapping (&as->map, KEY_MOUSEWHEELRIGHT, KEY_MOUSEWHEELLEFT,
                         var_InheritInteger (obj, "hotkeys-x-wheel-mode"));
    add_wheel_mapping (&as->map, KEY_MOUSEWHEELUP, KEY_MOUSEWHEELDOWN,
                         var_InheritInteger (obj, "hotkeys-y-wheel-mode"));

    libvlc_priv(libvlc)->actions = as;
    var_AddCallback (obj, "key-pressed", vlc_key_to_action, &as->map);
    var_AddCallback (obj, "global-key-pressed", vlc_key_to_action,
                     &as->global_map);
    return VLC_SUCCESS;
}

/**
 * Destroys the key map.
 */
void libvlc_InternalActionsClean (libvlc_int_t *libvlc)
{
    assert(libvlc != NULL);

    vlc_actions_t *as = libvlc_priv(libvlc)->actions;
    if (unlikely(as == NULL))
        return;

    var_DelCallback (libvlc, "global-key-pressed", vlc_key_to_action,
                     &as->global_map);
    var_DelCallback (libvlc, "key-pressed", vlc_key_to_action, &as->map);

    tdestroy (as->global_map, free);
    tdestroy (as->map, free);
    free (as);
    libvlc_priv(libvlc)->actions = NULL;
}


static int actcmp(const void *key, const void *ent)
{
    const struct name2action *act = ent;
    return strcmp(key, act->psz);
}

/**
 * Get the action ID from the action name in the configuration subsystem.
 * @return the action ID or ACTIONID_NONE on error.
 */
vlc_action_id_t
vlc_actions_get_id (const char *name)
{
    const struct name2action *act;

    if (strncmp (name, "key-", 4))
        return ACTIONID_NONE;
    name += 4;

    act = bsearch(name, s_names2actions, ACTIONS_COUNT, sizeof(*act), actcmp);
    return (act != NULL) ? act->id : ACTIONID_NONE;
}

#undef vlc_actions_get_keycode
uint_fast32_t
vlc_actions_get_keycode(vlc_object_t *p_obj, const char *psz_key_name,
                        bool b_global)
{
    char varname[12 /* "global-key-" */ + strlen( psz_key_name )];
    sprintf( varname, "%skey-%s", b_global ? "global-" : "", psz_key_name );

    char *psz_key = var_InheritString( p_obj, varname );
    if( psz_key == NULL )
        return KEY_UNSET;

    uint_fast32_t i_key = vlc_str2keycode( psz_key );
    free( psz_key );
    return i_key;
}

#undef vlc_actions_get_key_names
const char* const*
vlc_actions_get_key_names(vlc_object_t *p_obj)
{
    vlc_actions_t *as = libvlc_priv(p_obj->obj.libvlc)->actions;
    return as->ppsz_keys;
}

#define display_message(vout, ...) \
    do { \
        if (vout) \
            vout_OSDMessage(vout, VOUT_SPU_CHANNEL_OSD, __VA_ARGS__); \
    } while(0)
#define display_icon(vout, icon) \
    do { if(vout) vout_OSDIcon(vout, VOUT_SPU_CHANNEL_OSD, icon); } while(0)

static void do_play_bookmark( playlist_t *p_playlist, int i_num )
{
    char *psz_bookmark_name;
    if( asprintf( &psz_bookmark_name, "bookmark%i", i_num ) == -1 )
        return;

    char *psz_bookmark = var_CreateGetString( p_playlist->obj.libvlc,
                                              psz_bookmark_name );

    PL_LOCK;
    FOREACH_ARRAY( playlist_item_t *p_item, p_playlist->items )
        char *psz_uri = input_item_GetURI( p_item->p_input );
        if( !strcmp( psz_bookmark, psz_uri ) )
        {
            free( psz_uri );
            playlist_ViewPlay( p_playlist, NULL, p_item );
            break;
        }
        else
            free( psz_uri );
    FOREACH_END();
    PL_UNLOCK;

    free( psz_bookmark );
    free( psz_bookmark_name );
}

static void do_set_bookmark( playlist_t *p_playlist, int i_num )
{
    char *psz_bookmark_name;
    char *psz_uri = NULL;
    if( asprintf( &psz_bookmark_name, "bookmark%i", i_num ) == -1 )
        return;

    var_Create( p_playlist->obj.libvlc, psz_bookmark_name,
                VLC_VAR_STRING|VLC_VAR_DOINHERIT );

    PL_LOCK;
    playlist_item_t * p_item = playlist_CurrentPlayingItem( p_playlist );
    if( p_item ) psz_uri = input_item_GetURI( p_item->p_input );
    PL_UNLOCK;

    if( p_item )
    {
        config_PutPsz( p_playlist->obj.libvlc, psz_bookmark_name, psz_uri);
        msg_Info( p_playlist, "setting playlist bookmark %i to %s", i_num, psz_uri);
    }

    free( psz_uri );
    free( psz_bookmark_name );
}

static void clear_channels( vlc_actions_t *p_as, vout_thread_t *p_vout  )
{
    if( p_vout )
    {
        vout_FlushSubpictureChannel( p_vout, VOUT_SPU_CHANNEL_OSD );
        vout_FlushSubpictureChannel( p_vout, p_as->slider_chan );
    }
}

static void display_position( vlc_actions_t *p_as, vout_thread_t *p_vout,
                              input_thread_t *p_input )
{
    char psz_duration[MSTRTIME_MAX_SIZE];
    char psz_time[MSTRTIME_MAX_SIZE];

    if( p_vout == NULL ) return;

    clear_channels( p_as, p_vout );

    int64_t t = var_GetInteger( p_input, "time" ) / CLOCK_FREQ;
    int64_t l = var_GetInteger( p_input, "length" ) / CLOCK_FREQ;

    secstotimestr( psz_time, t );

    if( l > 0 )
    {
        secstotimestr( psz_duration, l );
        display_message( p_vout, "%s / %s", psz_time, psz_duration );
    }
    else if( t > 0 )
    {
        display_message( p_vout, "%s", psz_time );
    }

    if( var_GetBool( p_vout, "fullscreen" ) )
    {
        vlc_value_t pos;
        var_Get( p_input, "position", &pos );
        vout_OSDSlider( p_vout, p_as->slider_chan,
                        pos.f_float * 100, OSD_HOR_SLIDER );
    }
}

static void display_volume( vlc_actions_t *p_as, vout_thread_t *p_vout, float vol )
{
    if( p_vout == NULL )
        return;
    clear_channels( p_as, p_vout );

    if( var_GetBool( p_vout, "fullscreen" ) )
        vout_OSDSlider( p_vout, p_as->slider_chan,
                        lroundf(vol * 100.f), OSD_VERT_SLIDER );
    display_message( p_vout, _( "Volume %ld%%" ), lroundf(vol * 100.f) );
}

static void display_rate( vout_thread_t *p_vout, float f_rate )
{
    display_message( p_vout, _("Speed: %.2fx"), (double) f_rate );
}

static float adjust_rate_fine( playlist_t *p_playlist, const int i_dir )
{
    const float f_rate_min = (float)INPUT_RATE_DEFAULT / INPUT_RATE_MAX;
    const float f_rate_max = (float)INPUT_RATE_DEFAULT / INPUT_RATE_MIN;
    float f_rate = var_GetFloat( p_playlist, "rate" );

    int i_sign = f_rate < 0 ? -1 : 1;

    f_rate = floor( fabs(f_rate) / 0.1 + i_dir + 0.05 ) * 0.1;

    if( f_rate < f_rate_min )
        f_rate = f_rate_min;
    else if( f_rate > f_rate_max )
        f_rate = f_rate_max;
    f_rate *= i_sign;

    return f_rate;
}

static int
vlc_actions_do_va( vlc_object_t *p_obj, vlc_action_id_t i_action, bool b_notify,
                   va_list args )
{
    libvlc_int_t *p_libvlc = p_obj->obj.libvlc;
    vlc_actions_t *p_as = libvlc_priv(p_libvlc)->actions;
    playlist_t *p_playlist = libvlc_priv(p_libvlc)->playlist;
    assert(p_as != NULL);

    if( !p_playlist )
        return VLC_EGENERIC;

    /* Update the input */
    input_thread_t *p_input = playlist_CurrentInput( p_playlist );

    /* Update the vout */
    vout_thread_t *p_vout = p_input ? input_GetVout( p_input ) : NULL;
    vout_thread_t *p_notify_vout = b_notify ? p_vout : NULL;

    /* Register OSD channels */
    /* FIXME: this check can fail if the new vout is reallocated at the same
     * address as the old one... We should rather listen to vout events.
     * Alternatively, we should keep a reference to the vout thread. */
    if( p_vout && p_vout != p_as->p_last_vout )
        p_as->slider_chan = vout_RegisterSubpictureChannel( p_vout );
    p_as->p_last_vout = p_vout;

    int i_ret = VLC_SUCCESS;
    /* Quit */
    switch( i_action )
    {
        /* Libvlc / interface actions */
        case ACTIONID_QUIT:
            libvlc_Quit( p_libvlc );

            clear_channels( p_as, p_notify_vout );
            display_message( p_notify_vout, _( "Quit" ) );
            break;

        case ACTIONID_INTF_TOGGLE_FSC:
        case ACTIONID_INTF_HIDE:
            var_TriggerCallback( p_libvlc, "intf-toggle-fscontrol" );
            break;
        case ACTIONID_INTF_BOSS:
            var_TriggerCallback( p_libvlc, "intf-boss" );
            break;
        case ACTIONID_INTF_POPUP_MENU:
            var_TriggerCallback( p_libvlc, "intf-popupmenu" );
            break;

        /* Playlist actions (including audio) */
        case ACTIONID_LOOP:
        {
            /* Toggle Normal -> Loop -> Repeat -> Normal ... */
            const char *mode;
            if( var_GetBool( p_playlist, "repeat" ) )
            {
                var_SetBool( p_playlist, "repeat", false );
                mode = N_("Off");
            }
            else
            if( var_GetBool( p_playlist, "loop" ) )
            { /* FIXME: this is not atomic, we should use a real tristate */
                var_SetBool( p_playlist, "loop", false );
                var_SetBool( p_playlist, "repeat", true );
                mode = N_("One");
            }
            else
            {
                var_SetBool( p_playlist, "loop", true );
                mode = N_("All");
            }
            display_message( p_notify_vout, _("Loop: %s"), vlc_gettext(mode) );
            break;
        }

        case ACTIONID_RANDOM:
        {
            const bool state = var_ToggleBool( p_playlist, "random" );
            display_message( p_notify_vout, _("Random: %s"),
                            vlc_gettext( state ? N_("On") : N_("Off") ) );
            break;
        }

        case ACTIONID_NEXT:
            display_message( p_notify_vout, _("Next") );
            playlist_Next( p_playlist );
            break;
        case ACTIONID_PREV:
            display_message( p_notify_vout, _("Previous") );
            playlist_Prev( p_playlist );
            break;

        case ACTIONID_STOP:
            playlist_Stop( p_playlist );
            break;

        case ACTIONID_RATE_NORMAL:
            var_SetFloat( p_playlist, "rate", 1.f );
            display_rate( p_notify_vout, 1.f );
            break;
        case ACTIONID_FASTER:
            var_TriggerCallback( p_playlist, "rate-faster" );
            display_rate( p_notify_vout, var_GetFloat( p_playlist, "rate" ) );
            break;
        case ACTIONID_SLOWER:
            var_TriggerCallback( p_playlist, "rate-slower" );
            display_rate( p_notify_vout, var_GetFloat( p_playlist, "rate" ) );
            break;
        case ACTIONID_RATE_FASTER_FINE:
        case ACTIONID_RATE_SLOWER_FINE:
        {
            const int i_dir = i_action == ACTIONID_RATE_FASTER_FINE ? 1 : -1;
            float rate = adjust_rate_fine( p_playlist, i_dir );

            var_SetFloat( p_playlist, "rate", rate );
            display_rate( p_notify_vout, rate );
            break;
        }

        case ACTIONID_PLAY_BOOKMARK1:
        case ACTIONID_PLAY_BOOKMARK2:
        case ACTIONID_PLAY_BOOKMARK3:
        case ACTIONID_PLAY_BOOKMARK4:
        case ACTIONID_PLAY_BOOKMARK5:
        case ACTIONID_PLAY_BOOKMARK6:
        case ACTIONID_PLAY_BOOKMARK7:
        case ACTIONID_PLAY_BOOKMARK8:
        case ACTIONID_PLAY_BOOKMARK9:
        case ACTIONID_PLAY_BOOKMARK10:
            do_play_bookmark( p_playlist, i_action - ACTIONID_PLAY_BOOKMARK1 + 1 );
            break;

        case ACTIONID_SET_BOOKMARK1:
        case ACTIONID_SET_BOOKMARK2:
        case ACTIONID_SET_BOOKMARK3:
        case ACTIONID_SET_BOOKMARK4:
        case ACTIONID_SET_BOOKMARK5:
        case ACTIONID_SET_BOOKMARK6:
        case ACTIONID_SET_BOOKMARK7:
        case ACTIONID_SET_BOOKMARK8:
        case ACTIONID_SET_BOOKMARK9:
        case ACTIONID_SET_BOOKMARK10:
            do_set_bookmark( p_playlist, i_action - ACTIONID_SET_BOOKMARK1 + 1 );
            break;
        case ACTIONID_PLAY_CLEAR:
        {
            playlist_Clear( p_playlist, pl_Unlocked );
            break;
        }
        case ACTIONID_VOL_UP:
        {
            float vol;
            if( playlist_VolumeUp( p_playlist, 1, &vol ) == 0 )
                display_volume( p_as, p_notify_vout, vol );
            break;
        }
        case ACTIONID_VOL_DOWN:
        {
            float vol;
            if( playlist_VolumeDown( p_playlist, 1, &vol ) == 0 )
                display_volume( p_as, p_notify_vout, vol );
            break;
        }
        case ACTIONID_VOL_MUTE:
        {
            int mute = playlist_MuteGet( p_playlist );
            if( mute < 0 )
                break;
            mute = !mute;
            if( playlist_MuteSet( p_playlist, mute ) )
                break;

            float vol = playlist_VolumeGet( p_playlist );
            if( mute || vol == 0.f )
            {
                clear_channels( p_as, p_notify_vout );
                display_icon( p_notify_vout, OSD_MUTE_ICON );
            }
            else
                display_volume( p_as, p_notify_vout,  vol );
            break;
        }

        case ACTIONID_AUDIODEVICE_CYCLE:
        {
            audio_output_t *p_aout = playlist_GetAout( p_playlist );
            if( p_aout == NULL )
                break;

            char **ids, **names;
            int n = aout_DevicesList( p_aout, &ids, &names );
            if( n == -1 )
                break;

            char *dev = aout_DeviceGet( p_aout );
            const char *devstr = (dev != NULL) ? dev : "";

            int idx = 0;
            for( int i = 0; i < n; i++ )
            {
                if( !strcmp(devstr, ids[i]) )
                    idx = (i + 1) % n;
            }
            free( dev );

            if( !aout_DeviceSet( p_aout, ids[idx] ) )
                display_message( p_notify_vout, _("Audio Device: %s"), names[idx] );
            vlc_object_release( p_aout );

            for( int i = 0; i < n; i++ )
            {
                free( ids[i] );
                free( names[i] );
            }
            free( ids );
            free( names );
            break;
        }

        /* Playlist + input actions */
        case ACTIONID_PLAY_PAUSE:
            if( p_input )
            {
                clear_channels( p_as, p_notify_vout );

                int state = var_GetInteger( p_input, "state" );
                display_icon( p_notify_vout, state != PAUSE_S ? OSD_PAUSE_ICON : OSD_PLAY_ICON );
            }
            playlist_TogglePause( p_playlist );
            break;

        case ACTIONID_PLAY:
            if( p_input && var_GetFloat( p_input, "rate" ) != 1.f )
                /* Return to normal speed */
                var_SetFloat( p_input, "rate", 1.f );
            else
            {
                clear_channels( p_as, p_notify_vout );
                display_icon( p_notify_vout, OSD_PLAY_ICON );
                playlist_Play( p_playlist );
            }
            break;

        /* Playlist + video output actions */
        case ACTIONID_WALLPAPER:
        {
            bool wp = var_ToggleBool( p_playlist, "video-wallpaper" );
            if( p_vout )
                var_SetBool( p_vout, "video-wallpaper", wp );
            break;
        }

        /* Input actions */
        case ACTIONID_PAUSE:
            if( p_input && var_GetInteger( p_input, "state" ) != PAUSE_S )
            {
                clear_channels( p_as, p_notify_vout );
                display_icon( p_notify_vout, OSD_PAUSE_ICON );
                var_SetInteger( p_input, "state", PAUSE_S );
            }
            break;

        case ACTIONID_RECORD:
            if( p_input && var_GetBool( p_input, "can-record" ) )
            {
                const bool on = var_ToggleBool( p_input, "record" );
                display_message( p_notify_vout, vlc_gettext(on
                                   ? N_("Recording") : N_("Recording done")) );
            }
            break;

        case ACTIONID_FRAME_NEXT:
            if( p_input )
            {
                var_TriggerCallback( p_input, "frame-next" );
                display_message( p_notify_vout, _("Next frame") );
            }
            break;

        case ACTIONID_SUBSYNC_MARKAUDIO:
        {
            p_as->subtitle_delaybookmarks.i_time_audio = mdate();
            display_message( p_notify_vout, _("Sub sync: bookmarked audio time"));
            break;
        }
        case ACTIONID_SUBSYNC_MARKSUB:
            if( p_input )
            {
                vlc_value_t val, list, list2;
                int i_count;
                var_Get( p_input, "spu-es", &val );

                var_Change( p_input, "spu-es", VLC_VAR_GETCHOICES,
                            &list, &list2 );
                i_count = list.p_list->i_count;
                if( i_count < 1 || val.i_int < 0 )
                {
                    display_message( p_notify_vout, _("No active subtitle") );
                    var_FreeList( &list, &list2 );
                    break;
                }
                p_as->subtitle_delaybookmarks.i_time_subtitle = mdate();
                display_message( p_notify_vout,
                                _("Sub sync: bookmarked subtitle time"));
                var_FreeList( &list, &list2 );
            }
            break;
        case ACTIONID_SUBSYNC_APPLY:
        {
            /* Warning! Can yield a pause in the playback.
             * For example, the following succession of actions will yield a 5 second delay :
             * - Pressing Shift-H (ACTIONID_SUBSYNC_MARKAUDIO)
             * - wait 5 second
             * - Press Shift-J (ACTIONID_SUBSYNC_MARKSUB)
             * - Press Shift-K (ACTIONID_SUBSYNC_APPLY)
             * --> 5 seconds pause
             * This is due to var_SetTime() (and ultimately UpdatePtsDelay())
             * which causes the video to pause for an equivalent duration
             * (This problem is also present in the "Track synchronization" window) */
            if ( p_input )
            {
                if ( (p_as->subtitle_delaybookmarks.i_time_audio == 0)
                  || (p_as->subtitle_delaybookmarks.i_time_subtitle == 0) )
                {
                    display_message( p_notify_vout, _( "Sub sync: set bookmarks first!" ) );
                }
                else
                {
                    int64_t i_current_subdelay = var_GetInteger( p_input, "spu-delay" );
                    int64_t i_additional_subdelay =
                        p_as->subtitle_delaybookmarks.i_time_audio
                        - p_as->subtitle_delaybookmarks.i_time_subtitle;
                    int64_t i_total_subdelay = i_current_subdelay + i_additional_subdelay;
                    var_SetInteger( p_input, "spu-delay", i_total_subdelay);
                    clear_channels( p_as, p_notify_vout );
                    display_message( p_notify_vout, _( "Sub sync: corrected %i ms (total delay = %i ms)" ),
                                            (int)(i_additional_subdelay / 1000),
                                            (int)(i_total_subdelay / 1000) );
                    p_as->subtitle_delaybookmarks.i_time_audio = 0;
                    p_as->subtitle_delaybookmarks.i_time_subtitle = 0;
                }
            }
            break;
        }
        case ACTIONID_SUBSYNC_RESET:
        {
            var_SetInteger( p_input, "spu-delay", 0);
            clear_channels( p_as, p_notify_vout );
            display_message( p_notify_vout, _( "Sub sync: delay reset" ) );
            p_as->subtitle_delaybookmarks.i_time_audio = 0;
            p_as->subtitle_delaybookmarks.i_time_subtitle = 0;
            break;
        }

        case ACTIONID_SUBDELAY_DOWN:
        case ACTIONID_SUBDELAY_UP:
        {
            int diff = (i_action == ACTIONID_SUBDELAY_UP) ? 50000 : -50000;
            if( p_input )
            {
                vlc_value_t val, list, list2;
                int i_count;
                var_Get( p_input, "spu-es", &val );

                var_Change( p_input, "spu-es", VLC_VAR_GETCHOICES,
                            &list, &list2 );
                i_count = list.p_list->i_count;
                if( i_count < 1 || val.i_int < 0 )
                {
                    display_message( p_notify_vout, _("No active subtitle") );
                    var_FreeList( &list, &list2 );
                    break;
                }
                int64_t i_delay = var_GetInteger( p_input, "spu-delay" ) + diff;

                var_SetInteger( p_input, "spu-delay", i_delay );
                clear_channels( p_as, p_notify_vout );
                display_message( p_notify_vout, _( "Subtitle delay %i ms" ),
                                (int)(i_delay/1000) );
                var_FreeList( &list, &list2 );
            }
            break;
        }
        case ACTIONID_AUDIODELAY_DOWN:
        case ACTIONID_AUDIODELAY_UP:
        {
            int diff = (i_action == ACTIONID_AUDIODELAY_UP) ? 50000 : -50000;
            if( p_input )
            {
                int64_t i_delay = var_GetInteger( p_input, "audio-delay" )
                                  + diff;

                var_SetInteger( p_input, "audio-delay", i_delay );
                clear_channels( p_as, p_notify_vout );
                display_message( p_notify_vout, _( "Audio delay %i ms" ),
                                 (int)(i_delay/1000) );
            }
            break;
        }

        case ACTIONID_AUDIO_TRACK:
            if( p_input )
            {
                vlc_value_t val, list, list2;
                int i_count, i;
                var_Get( p_input, "audio-es", &val );
                var_Change( p_input, "audio-es", VLC_VAR_GETCHOICES,
                            &list, &list2 );
                i_count = list.p_list->i_count;
                if( i_count > 1 )
                {
                    for( i = 0; i < i_count; i++ )
                    {
                        if( val.i_int == list.p_list->p_values[i].i_int )
                        {
                            break;
                        }
                    }
                    /* value of audio-es was not in choices list */
                    if( i == i_count )
                    {
                        msg_Warn( p_input,
                                  "invalid current audio track, selecting 0" );
                        i = 0;
                    }
                    else if( i == i_count - 1 )
                        i = 1;
                    else
                        i++;
                    var_Set( p_input, "audio-es", list.p_list->p_values[i] );
                    display_message( p_notify_vout, _("Audio track: %s"),
                                    list2.p_list->p_values[i].psz_string );
                }
                var_FreeList( &list, &list2 );
            }
            break;
        case ACTIONID_SUBTITLE_TRACK:
        case ACTIONID_SUBTITLE_REVERSE_TRACK:
            if( p_input )
            {
                vlc_value_t val, list, list2;
                int i_count, i;
                var_Get( p_input, "spu-es", &val );

                var_Change( p_input, "spu-es", VLC_VAR_GETCHOICES,
                            &list, &list2 );
                i_count = list.p_list->i_count;
                if( i_count <= 1 )
                {
                    display_message( p_notify_vout, _("Subtitle track: %s"),
                                    _("N/A") );
                    var_FreeList( &list, &list2 );
                    break;
                }
                for( i = 0; i < i_count; i++ )
                {
                    if( val.i_int == list.p_list->p_values[i].i_int )
                    {
                        break;
                    }
                }
                /* value of spu-es was not in choices list */
                if( i == i_count )
                {
                    msg_Warn( p_input,
                              "invalid current subtitle track, selecting 0" );
                    i = 0;
                }
                else if ((i == i_count - 1) && (i_action == ACTIONID_SUBTITLE_TRACK))
                    i = 0;
                else if ((i == 0) && (i_action == ACTIONID_SUBTITLE_REVERSE_TRACK))
                    i = i_count - 1;
                else
                    i = (i_action == ACTIONID_SUBTITLE_TRACK) ? i+1 : i-1;
                var_SetInteger( p_input, "spu-es", list.p_list->p_values[i].i_int );
                var_SetInteger( p_input, "spu-choice", list.p_list->p_values[i].i_int );
                display_message( p_notify_vout, _("Subtitle track: %s"),
                                list2.p_list->p_values[i].psz_string );
                var_FreeList( &list, &list2 );
            }
            break;
        case ACTIONID_SUBTITLE_TOGGLE:
            if( p_input )
            {
                vlc_value_t list, list2;
                int i_count, i_sel_index, i_sel_id, i_old_id, i_new_index;
                i_old_id = var_GetInteger( p_input, "spu-es" );
                i_sel_id = var_GetInteger( p_input, "spu-choice" );

                var_Change( p_input, "spu-es", VLC_VAR_GETCHOICES,
                            &list, &list2 );
                i_count = list.p_list->i_count;
                if( i_count <= 1 )
                {
                    display_message( p_notify_vout, _("Subtitle track: %s"),
                                    _("N/A") );
                    var_FreeList( &list, &list2 );
                    break;
                }
                for( i_sel_index = 0; i_sel_index < i_count; i_sel_index++ )
                {
                    if( i_sel_id == list.p_list->p_values[i_sel_index].i_int )
                    {
                        break;
                    }
                }
                /* if there is nothing to toggle choose the first track */
                if( !i_sel_index ) {
                    i_sel_index = 1;
                    i_sel_id = list.p_list->p_values[1].i_int;
                    var_SetInteger( p_input, "spu-choice", i_sel_id );
                }

                i_new_index = 0;
                if( i_old_id != i_sel_id )
                {
                    if( i_sel_index >= i_count )
                    {
                        var_SetInteger( p_input, "spu-choice", list.p_list->p_values[0].i_int );
                    }
                    else
                    {
                        i_new_index = i_sel_index;
                    }
                }
                var_SetInteger( p_input, "spu-es", list.p_list->p_values[i_new_index].i_int );
                display_message( p_notify_vout, _("Subtitle track: %s"),
                                list2.p_list->p_values[i_new_index].psz_string );
                var_FreeList( &list, &list2 );
            }
            break;
        case ACTIONID_PROGRAM_SID_NEXT:
        case ACTIONID_PROGRAM_SID_PREV:
            if( p_input )
            {
                vlc_value_t val, list, list2;
                int i_count, i;
                var_Get( p_input, "program", &val );

                var_Change( p_input, "program", VLC_VAR_GETCHOICES,
                            &list, &list2 );
                i_count = list.p_list->i_count;
                if( i_count <= 1 )
                {
                    display_message( p_notify_vout, _("Program Service ID: %s"),
                                    _("N/A") );
                    var_FreeList( &list, &list2 );
                    break;
                }
                for( i = 0; i < i_count; i++ )
                {
                    if( val.i_int == list.p_list->p_values[i].i_int )
                    {
                        break;
                    }
                }
                /* value of program sid was not in choices list */
                if( i == i_count )
                {
                    msg_Warn( p_input,
                              "invalid current program SID, selecting 0" );
                    i = 0;
                }
                else if( i_action == ACTIONID_PROGRAM_SID_NEXT ) {
                    if( i == i_count - 1 )
                        i = 0;
                    else
                        i++;
                    }
                else { /* ACTIONID_PROGRAM_SID_PREV */
                    if( i == 0 )
                        i = i_count - 1;
                    else
                        i--;
                    }
                var_Set( p_input, "program", list.p_list->p_values[i] );
                display_message( p_notify_vout, _("Program Service ID: %s"),
                                list2.p_list->p_values[i].psz_string );
                var_FreeList( &list, &list2 );
            }
            break;

        case ACTIONID_JUMP_BACKWARD_EXTRASHORT:
        case ACTIONID_JUMP_FORWARD_EXTRASHORT:
        case ACTIONID_JUMP_BACKWARD_SHORT:
        case ACTIONID_JUMP_FORWARD_SHORT:
        case ACTIONID_JUMP_BACKWARD_MEDIUM:
        case ACTIONID_JUMP_FORWARD_MEDIUM:
        case ACTIONID_JUMP_BACKWARD_LONG:
        case ACTIONID_JUMP_FORWARD_LONG:
        {
            if( p_input == NULL || !var_GetBool( p_input, "can-seek" ) )
                break;

            const char *varname;
            int sign = +1;
            switch( i_action )
            {
                case ACTIONID_JUMP_BACKWARD_EXTRASHORT:
                    sign = -1;
                    /* fall through */
                case ACTIONID_JUMP_FORWARD_EXTRASHORT:
                    varname = "extrashort-jump-size";
                    break;
                case ACTIONID_JUMP_BACKWARD_SHORT:
                    sign = -1;
                    /* fall through */
                case ACTIONID_JUMP_FORWARD_SHORT:
                    varname = "short-jump-size";
                    break;
                case ACTIONID_JUMP_BACKWARD_MEDIUM:
                    sign = -1;
                    /* fall through */
                case ACTIONID_JUMP_FORWARD_MEDIUM:
                    varname = "medium-jump-size";
                    break;
                case ACTIONID_JUMP_BACKWARD_LONG:
                    sign = -1;
                    /* fall through */
                case ACTIONID_JUMP_FORWARD_LONG:
                    varname = "long-jump-size";
                    break;
                default:
                    vlc_assert_unreachable();
            }

            mtime_t it = var_InheritInteger( p_input, varname );
            if( it < 0 )
                break;
            var_SetInteger( p_input, "time-offset", it * sign * CLOCK_FREQ );
            display_position( p_as, p_notify_vout, p_input );
            break;
        }

        /* Input navigation */
        case ACTIONID_TITLE_PREV:
            if( p_input )
                var_TriggerCallback( p_input, "prev-title" );
            break;
        case ACTIONID_TITLE_NEXT:
            if( p_input )
                var_TriggerCallback( p_input, "next-title" );
            break;
        case ACTIONID_CHAPTER_PREV:
            if( p_input )
                var_TriggerCallback( p_input, "prev-chapter" );
            break;
        case ACTIONID_CHAPTER_NEXT:
            if( p_input )
                var_TriggerCallback( p_input, "next-chapter" );
            break;
        case ACTIONID_DISC_MENU:
            if( p_input )
                var_SetInteger( p_input, "title  0", 2 );
            break;
        case ACTIONID_NAV_ACTIVATE:
            if( p_input )
                input_Control( p_input, INPUT_NAV_ACTIVATE, NULL );
            break;
        case ACTIONID_NAV_UP:
            if( p_vout )
                input_UpdateViewpoint( p_input,
                                       &(vlc_viewpoint_t) { .pitch = -1.f },
                                       false );
            if( p_input )
                input_Control( p_input, INPUT_NAV_UP, NULL );
            break;
        case ACTIONID_NAV_DOWN:
            if( p_vout )
                input_UpdateViewpoint( p_input,
                                       &(vlc_viewpoint_t) { .pitch = 1.f },
                                       false );
            if( p_input )
                input_Control( p_input, INPUT_NAV_DOWN, NULL );
            break;
        case ACTIONID_NAV_LEFT:
            if( p_vout )
                input_UpdateViewpoint( p_input,
                                       &(vlc_viewpoint_t) { .yaw = -1.f },
                                       false );
            if( p_input )
                input_Control( p_input, INPUT_NAV_LEFT, NULL );
            break;
        case ACTIONID_NAV_RIGHT:
            if( p_vout )
                input_UpdateViewpoint( p_input,
                                       &(vlc_viewpoint_t) { .yaw = 1.f },
                                       false );
            if( p_input )
                input_Control( p_input, INPUT_NAV_RIGHT, NULL );
            break;

        /* Video Output actions */
        case ACTIONID_SNAPSHOT:
            if( p_vout )
                var_TriggerCallback( p_vout, "video-snapshot" );
            break;

        case ACTIONID_TOGGLE_FULLSCREEN:
        {
            if( p_vout )
            {
                bool fs = var_ToggleBool( p_vout, "fullscreen" );
                var_SetBool( p_playlist, "fullscreen", fs );
            }
            else
                var_ToggleBool( p_playlist, "fullscreen" );
            break;
        }

        case ACTIONID_LEAVE_FULLSCREEN:
            if( p_vout )
                var_SetBool( p_vout, "fullscreen", false );
            var_SetBool( p_playlist, "fullscreen", false );
            break;

        case ACTIONID_ASPECT_RATIO:
            if( p_vout )
            {
                vlc_value_t val={0}, val_list, text_list;
                var_Get( p_vout, "aspect-ratio", &val );
                if( var_Change( p_vout, "aspect-ratio", VLC_VAR_GETCHOICES,
                                &val_list, &text_list ) >= 0 )
                {
                    int i;
                    for( i = 0; i < val_list.p_list->i_count; i++ )
                    {
                        if( !strcmp( val_list.p_list->p_values[i].psz_string,
                                     val.psz_string ) )
                        {
                            i++;
                            break;
                        }
                    }
                    if( i == val_list.p_list->i_count ) i = 0;
                    var_SetString( p_vout, "aspect-ratio",
                                   val_list.p_list->p_values[i].psz_string );
                    display_message( p_notify_vout, _("Aspect ratio: %s"),
                                    text_list.p_list->p_values[i].psz_string );

                    var_FreeList( &val_list, &text_list );
                }
                free( val.psz_string );
            }
            break;

        case ACTIONID_CROP:
            if( p_vout )
            {
                vlc_value_t val={0}, val_list, text_list;
                var_Get( p_vout, "crop", &val );
                if( var_Change( p_vout, "crop", VLC_VAR_GETCHOICES,
                                &val_list, &text_list ) >= 0 )
                {
                    int i;
                    for( i = 0; i < val_list.p_list->i_count; i++ )
                    {
                        if( !strcmp( val_list.p_list->p_values[i].psz_string,
                                     val.psz_string ) )
                        {
                            i++;
                            break;
                        }
                    }
                    if( i == val_list.p_list->i_count ) i = 0;
                    var_SetString( p_vout, "crop",
                                   val_list.p_list->p_values[i].psz_string );
                    display_message( p_notify_vout, _("Crop: %s"),
                                    text_list.p_list->p_values[i].psz_string );

                    var_FreeList( &val_list, &text_list );
                }
                free( val.psz_string );
            }
            break;
        case ACTIONID_CROP_TOP:
            if( p_vout )
                var_IncInteger( p_vout, "crop-top" );
            break;
        case ACTIONID_UNCROP_TOP:
            if( p_vout )
                var_DecInteger( p_vout, "crop-top" );
            break;
        case ACTIONID_CROP_BOTTOM:
            if( p_vout )
                var_IncInteger( p_vout, "crop-bottom" );
            break;
        case ACTIONID_UNCROP_BOTTOM:
            if( p_vout )
                var_DecInteger( p_vout, "crop-bottom" );
            break;
        case ACTIONID_CROP_LEFT:
            if( p_vout )
                var_IncInteger( p_vout, "crop-left" );
            break;
        case ACTIONID_UNCROP_LEFT:
            if( p_vout )
                var_DecInteger( p_vout, "crop-left" );
            break;
        case ACTIONID_CROP_RIGHT:
            if( p_vout )
                var_IncInteger( p_vout, "crop-right" );
            break;
        case ACTIONID_UNCROP_RIGHT:
            if( p_vout )
                var_DecInteger( p_vout, "crop-right" );
            break;
        case ACTIONID_VIEWPOINT_FOV_IN:
            if( p_vout )
                input_UpdateViewpoint( p_input,
                                       &(vlc_viewpoint_t) { .fov = -1.f },
                                       false );
            break;
        case ACTIONID_VIEWPOINT_FOV_OUT:
            if( p_vout )
                input_UpdateViewpoint( p_input,
                                       &(vlc_viewpoint_t) { .fov = 1.f },
                                       false );
            break;

        case ACTIONID_VIEWPOINT_ROLL_CLOCK:
            if( p_vout )
                input_UpdateViewpoint( p_input,
                                       &(vlc_viewpoint_t) { .roll = -1.f },
                                       false );
            break;
        case ACTIONID_VIEWPOINT_ROLL_ANTICLOCK:
            if( p_vout )
                input_UpdateViewpoint( p_input,
                                       &(vlc_viewpoint_t) { .roll = 1.f },
                                       false );
            break;
        case ACTIONID_VIEWPOINT_UPDATE:
        {
            if ( p_vout )
            {
                vlc_viewpoint_t* viewpoint = va_arg( args, vlc_viewpoint_t* );
                input_UpdateViewpoint( p_input, viewpoint, false );
            }
            break;
        }


         case ACTIONID_TOGGLE_AUTOSCALE:
            if( p_vout )
            {
                float f_scalefactor = var_GetFloat( p_vout, "zoom" );
                if ( f_scalefactor != 1.f )
                {
                    var_SetFloat( p_vout, "zoom", 1.f );
                    display_message( p_notify_vout, _("Zooming reset") );
                }
                else
                {
                    bool b_autoscale = !var_GetBool( p_vout, "autoscale" );
                    var_SetBool( p_vout, "autoscale", b_autoscale );
                    if( b_autoscale )
                        display_message( p_notify_vout, _("Scaled to screen") );
                    else
                        display_message( p_notify_vout, _("Original Size") );
                }
            }
            break;
        case ACTIONID_SCALE_UP:
            if( p_vout )
            {
               float f_scalefactor = var_GetFloat( p_vout, "zoom" );

               if( f_scalefactor < 10.f )
                   f_scalefactor += .1f;
               var_SetFloat( p_vout, "zoom", f_scalefactor );
            }
            break;
        case ACTIONID_SCALE_DOWN:
            if( p_vout )
            {
               float f_scalefactor = var_GetFloat( p_vout, "zoom" );

               if( f_scalefactor > .3f )
                   f_scalefactor -= .1f;
               var_SetFloat( p_vout, "zoom", f_scalefactor );
            }
            break;

        case ACTIONID_ZOOM_QUARTER:
        case ACTIONID_ZOOM_HALF:
        case ACTIONID_ZOOM_ORIGINAL:
        case ACTIONID_ZOOM_DOUBLE:
            if( p_vout )
            {
                float f;
                switch( i_action )
                {
                    case ACTIONID_ZOOM_QUARTER:  f = 0.25; break;
                    case ACTIONID_ZOOM_HALF:     f = 0.5;  break;
                    case ACTIONID_ZOOM_ORIGINAL: f = 1.;   break;
                     /*case ACTIONID_ZOOM_DOUBLE:*/
                    default:                     f = 2.;   break;
                }
                var_SetFloat( p_vout, "zoom", f );
            }
            break;
        case ACTIONID_ZOOM:
        case ACTIONID_UNZOOM:
            if( p_vout )
            {
                vlc_value_t val={0}, val_list, text_list;
                var_Get( p_vout, "zoom", &val );
                if( var_Change( p_vout, "zoom", VLC_VAR_GETCHOICES,
                                &val_list, &text_list ) >= 0 )
                {
                    int i;
                    for( i = 0; i < val_list.p_list->i_count; i++ )
                    {
                        if( val_list.p_list->p_values[i].f_float
                           == val.f_float )
                        {
                            if( i_action == ACTIONID_ZOOM )
                                i++;
                            else /* ACTIONID_UNZOOM */
                                i--;
                            break;
                        }
                    }
                    if( i == val_list.p_list->i_count ) i = 0;
                    if( i == -1 ) i = val_list.p_list->i_count-1;
                    var_SetFloat( p_vout, "zoom",
                                  val_list.p_list->p_values[i].f_float );
                    display_message( p_notify_vout, _("Zoom mode: %s"),
                                    text_list.p_list->p_values[i].psz_string );

                    var_FreeList( &val_list, &text_list );
                }
            }
            break;

        case ACTIONID_DEINTERLACE:
            if( p_vout )
            {
                int i_deinterlace = var_GetInteger( p_vout, "deinterlace" );
                if( i_deinterlace != 0 )
                {
                    var_SetInteger( p_vout, "deinterlace", 0 );
                    display_message( p_notify_vout, _("Deinterlace off") );
                }
                else
                {
                    var_SetInteger( p_vout, "deinterlace", 1 );

                    char *psz_mode = var_GetString( p_vout, "deinterlace-mode" );
                    vlc_value_t vlist, tlist;
                    if( psz_mode && !var_Change( p_vout, "deinterlace-mode", VLC_VAR_GETCHOICES, &vlist, &tlist ) )
                    {
                        const char *psz_text = NULL;
                        for( int i = 0; i < vlist.p_list->i_count; i++ )
                        {
                            if( !strcmp( vlist.p_list->p_values[i].psz_string, psz_mode ) )
                            {
                                psz_text = tlist.p_list->p_values[i].psz_string;
                                break;
                            }
                        }
                        display_message( p_notify_vout, "%s (%s)", _("Deinterlace on"),
                                        psz_text ? psz_text : psz_mode );

                        var_FreeList( &vlist, &tlist );
                    }
                    free( psz_mode );
                }
            }
            break;
        case ACTIONID_DEINTERLACE_MODE:
            if( p_vout )
            {
                char *psz_mode = var_GetString( p_vout, "deinterlace-mode" );
                vlc_value_t vlist, tlist;
                if( psz_mode && !var_Change( p_vout, "deinterlace-mode", VLC_VAR_GETCHOICES, &vlist, &tlist ))
                {
                    const char *psz_text = NULL;
                    int i;
                    for( i = 0; i < vlist.p_list->i_count; i++ )
                    {
                        if( !strcmp( vlist.p_list->p_values[i].psz_string, psz_mode ) )
                        {
                            i++;
                            break;
                        }
                    }
                    if( i == vlist.p_list->i_count ) i = 0;
                    psz_text = tlist.p_list->p_values[i].psz_string;
                    var_SetString( p_vout, "deinterlace-mode", vlist.p_list->p_values[i].psz_string );

                    int i_deinterlace = var_GetInteger( p_vout, "deinterlace" );
                    if( i_deinterlace != 0 )
                    {
                      display_message( p_notify_vout, "%s (%s)", _("Deinterlace on"),
                                      psz_text ? psz_text : psz_mode );
                    }
                    else
                    {
                      display_message( p_notify_vout, "%s (%s)", _("Deinterlace off"),
                                      psz_text ? psz_text : psz_mode );
                    }

                    var_FreeList( &vlist, &tlist );
                }
                free( psz_mode );
            }
            break;

        case ACTIONID_SUBPOS_DOWN:
        case ACTIONID_SUBPOS_UP:
        {
            if( p_input && p_vout )
            {
                vlc_value_t val, list, list2;
                int i_count;
                var_Get( p_input, "spu-es", &val );

                var_Change( p_input, "spu-es", VLC_VAR_GETCHOICES,
                            &list, &list2 );
                i_count = list.p_list->i_count;
                if( i_count < 1 || val.i_int < 0 )
                {
                    display_message( p_notify_vout,
                                    _("Subtitle position: no active subtitle") );
                    var_FreeList( &list, &list2 );
                    break;
                }

                int i_pos;
                if( i_action == ACTIONID_SUBPOS_DOWN )
                    i_pos = var_DecInteger( p_vout, "sub-margin" );
                else
                    i_pos = var_IncInteger( p_vout, "sub-margin" );

                clear_channels( p_as, p_notify_vout );
                display_message( p_notify_vout, _( "Subtitle position %d px" ), i_pos );
                var_FreeList( &list, &list2 );
            }
            break;
        }

        case ACTIONID_SUBTITLE_TEXT_SCALE_DOWN:
        case ACTIONID_SUBTITLE_TEXT_SCALE_UP:
        case ACTIONID_SUBTITLE_TEXT_SCALE_NORMAL:
        {
            if( p_vout )
            {
                int i_scale;
                if( i_action == ACTIONID_SUBTITLE_TEXT_SCALE_NORMAL )
                {
                    i_scale = 100;
                }
                else
                {
                    i_scale = var_GetInteger( p_playlist, "sub-text-scale" );
                    i_scale += ((i_action == ACTIONID_SUBTITLE_TEXT_SCALE_UP) ? 1 : -1) * 25;
                    i_scale = VLC_CLIP( i_scale, 10, 500 );
                }
                var_SetInteger( p_playlist, "sub-text-scale", i_scale );
                display_message( p_notify_vout, _( "Subtitle text scale %d%%" ), i_scale );
            }
            break;
        }

        /* Input + video output */
        case ACTIONID_POSITION:
            if( p_vout && vout_OSDEpg( p_vout, input_GetItem( p_input ) ) )
                display_position( p_as, p_notify_vout, p_input );
            break;
        case ACTIONID_COMBO_VOL_FOV_UP:
        {
            bool b_vrnav = p_vout && var_GetBool( p_vout,
                                                  "viewpoint-changeable" );
            if( b_vrnav )
                vlc_actions_do( p_obj, ACTIONID_VIEWPOINT_FOV_IN, b_notify );
            else
                vlc_actions_do( p_obj, ACTIONID_VOL_UP, b_notify );
            break;
        }
        case ACTIONID_COMBO_VOL_FOV_DOWN:
        {
            bool b_vrnav = p_vout && var_GetBool( p_vout,
                                              "viewpoint-changeable" );
            if( b_vrnav )
                vlc_actions_do( p_obj, ACTIONID_VIEWPOINT_FOV_OUT, b_notify );
            else
                vlc_actions_do( p_obj, ACTIONID_VOL_DOWN, b_notify );
            break;
        }
        case ACTIONID_NONE:
            i_ret = VLC_EGENERIC;
            break;
    }

    if( p_vout )
        vlc_object_release( p_vout );
    if( p_input )
        vlc_object_release( p_input );
    return i_ret;
}

#undef vlc_actions_do
int vlc_actions_do(vlc_object_t *p_obj, vlc_action_id_t i_action,
                   bool b_notify, ...)
{
    va_list args;
    int     i_result;

    va_start( args, b_notify );
    i_result = vlc_actions_do_va( p_obj, i_action, b_notify, args );
    va_end( args );
    return i_result;
}
