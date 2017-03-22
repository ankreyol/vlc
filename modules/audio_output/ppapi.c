/*****************************************************************************
 * ppapi.c: Audio output using PPAPI on NativeClient
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
#include <vlc_aout.h>
#include <vlc_block_helper.h>
#include <vlc_plugin.h>
#include <vlc_threads.h>

#include <ppapi/c/pp_instance.h>
#include <ppapi/c/ppb_audio_config.h>
#include <ppapi/c/ppb_audio.h>
#include <ppapi/c/ppb_core.h>
#include <ppapi/c/ppb.h>

/*****************************************************************************
 * Module descriptor
 *****************************************************************************/
static int Open(vlc_object_t *);
static void Close(vlc_object_t *);

vlc_module_begin()
    set_shortname("PPAPI aout")
    set_description(N_("PPAPI based audio output"))
    set_category(CAT_AUDIO)
    set_subcategory(SUBCAT_AUDIO_AOUT)

    set_capability("audio output", 50)
    set_callbacks(Open, Close)
vlc_module_end()

/*****************************************************************************
 * Local prototypes.
 *****************************************************************************/
struct aout_sys_t
{
    block_bytestream_t data;

    bool flushing;
    vlc_cond_t flushed_signal;
    vlc_mutex_t mutex;

    float latency;
    unsigned samplerate;

    PP_Instance instance;
    PP_Resource config;
    PP_Resource context;

    PPB_AudioConfig *audioconfig;
    PPB_Audio *audio;
    PPB_Core *core;
};

/*****************************************************************************
 * Implementation
 *****************************************************************************/
static void ppapi_audio_callback(void *sample_buffer,
        uint32_t buffer_size_in_bytes, PP_TimeDelta latency, void* user_data)
{
    audio_output_t *aout = user_data;
    aout_sys_t *sys = aout->sys;

    vlc_mutex_lock(&sys->mutex);
    sys->latency = latency;

    if (block_GetBytes(&sys->data, sample_buffer, buffer_size_in_bytes) != VLC_SUCCESS) {
        if (sys->flushing) {
            block_BytestreamEmpty(&sys->data);
            vlc_cond_signal(&sys->flushed_signal);
        } else {
            memset(sample_buffer, 0x00, buffer_size_in_bytes);
        }
    }

    block_BytestreamFlush(&sys->data);
    vlc_mutex_unlock(&sys->mutex);
}

static int Start(audio_output_t *aout, audio_sample_format_t *restrict fmt)
{
    aout_sys_t *sys = aout->sys;

    sys->samplerate = fmt->i_rate;

    /* Create audio config */
    uint32_t sample_frame_count = sys->audioconfig->RecommendSampleFrameCount(
            sys->instance, sys->samplerate, 512);

    msg_Dbg(aout, "Using sample_frame_count of %u", sample_frame_count);

    sys->config = sys->audioconfig->CreateStereo16Bit(sys->instance,
            sys->samplerate, sample_frame_count);
    if (unlikely(sys->config) == 0) {
        msg_Err(aout, "Could not create PPAPI audio config");
        goto error;
    }

    /* Create audio context */
    sys->context = sys->audio->Create(sys->instance, sys->config,
            ppapi_audio_callback, aout);
    if (unlikely(sys->context) == 0) {
        msg_Err(aout, "Could not create PPAPI audio resource");
        goto error;
    }

    /* Enforce stereo output S16N output */
    fmt->i_format = VLC_CODEC_S16N;
    fmt->i_physical_channels = AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT;

    sys->audio->StartPlayback(sys->context);

    return VLC_SUCCESS;

error:
    if (sys->config != 0)
        sys->core->ReleaseResource(sys->config);

    return VLC_EGENERIC;
}

static void Stop(audio_output_t *aout)
{
    aout_sys_t *sys = aout->sys;

    if (unlikely(sys->context == 0))
        return;

    if (sys->audio->StopPlayback(sys->context) != PP_TRUE)
        msg_Warn(aout, "Failed to stop playback");

    sys->core->ReleaseResource(sys->context);
    sys->context = 0;
}

static int TimeGet(audio_output_t *aout, mtime_t *delay)
{
    aout_sys_t *sys = aout->sys;

    vlc_mutex_lock(&sys->mutex);
    *delay = sys->latency * CLOCK_FREQ +
        (block_BytestreamRemaining(&sys->data) / 4) * CLOCK_FREQ / sys->samplerate;
    vlc_mutex_unlock(&sys->mutex);

    return VLC_SUCCESS;
}

static void Play(audio_output_t *aout, block_t *block)
{
    aout_sys_t *sys = aout->sys;

    vlc_mutex_lock(&sys->mutex);
    block_BytestreamPush(&sys->data, block);
    vlc_mutex_unlock(&sys->mutex);
}

static void Flush(audio_output_t *aout, bool wait)
{
    aout_sys_t *sys = aout->sys;

    vlc_mutex_lock(&sys->mutex);
    if (wait) {
        sys->flushing = true;
        vlc_cond_wait(&sys->flushed_signal, &sys->mutex);
        sys->flushing = false;
    } else {
        block_BytestreamEmpty(&sys->data);
    }
    vlc_mutex_unlock(&sys->mutex);
}

static int Open(vlc_object_t *obj)
{
    audio_output_t *aout = (audio_output_t *)obj;
    aout_sys_t *sys;

    sys = calloc(1, sizeof(*sys));
    if (unlikely(sys == NULL))
        return VLC_ENOMEM;
    aout->sys = sys;

    vlc_mutex_init(&sys->mutex);
    vlc_cond_init(&sys->flushed_signal);

    PPB_GetInterface ppb_get_interface = var_InheritAddress(obj, "ppapi-ppb-get-interface");
    if (ppb_get_interface == NULL) {
        msg_Err(aout, "Variable ppapi-ppb-get-interface is not set");
        goto error;
    }

    sys->instance = (int)var_InheritInteger(obj, "ppapi-instance");
    if (unlikely(sys->instance == 0)) {
        msg_Err(aout, "Variable ppapi-instance is not set");
        goto error;
    }

    sys->core = (PPB_Core*)ppb_get_interface(PPB_CORE_INTERFACE);
    if (sys->core == NULL) {
        msg_Err(aout, "Failed to get PPB_CORE_INTERFACE");
        goto error;
    }

    sys->audio = (PPB_Audio*)ppb_get_interface(PPB_AUDIO_INTERFACE);
    if (unlikely(sys->audio == NULL)) {
        msg_Err(aout, "Failed to get PPB_AUDIO_INTERFACE");
        goto error;
    }

    sys->audioconfig = (PPB_AudioConfig*)ppb_get_interface(PPB_AUDIO_CONFIG_INTERFACE);
    if (unlikely(sys->audioconfig == NULL)) {
        msg_Err(aout, "Failed to get PPB_AUDIO_CONFIG_INTERFACE");
        goto error;
    }

    block_BytestreamInit(&sys->data);

    aout->start = Start;
    aout->stop = Stop;
    aout->time_get = TimeGet;
    aout->play = Play;
    aout->flush = Flush;

    return VLC_SUCCESS;

error:
    Close(obj);
    return VLC_EGENERIC;
}

static void Close(vlc_object_t *obj)
{
    audio_output_t *aout = (audio_output_t *)obj;
    aout_sys_t *sys = aout->sys;

    block_BytestreamRelease(&sys->data);

    vlc_cond_destroy(&sys->flushed_signal);
    vlc_mutex_destroy(&sys->mutex);

    if (sys->context != 0)
        sys->core->ReleaseResource(sys->context);

    if (sys->config != 0)
        sys->core->ReleaseResource(sys->config);

    free(sys);
}
