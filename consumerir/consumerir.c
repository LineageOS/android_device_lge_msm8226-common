/*
 * Copyright (C) 2013 The Android Open Source Project
 * Copyright (C) 2017 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "LgIrHal"

#include <audio_route/audio_route.h>
#include <cutils/log.h>
#include <errno.h>
#include <fcntl.h>
#include <hardware/consumerir.h>
#include <hardware/hardware.h>
#include <malloc.h>
#include <string.h>
#include <sys/ioctl.h>
#include <system/audio.h>
#include <tinyalsa/asoundlib.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define IRRC_IOCTL_MAGIC 'a'
#define IRRC_START _IOW(IRRC_IOCTL_MAGIC, 0, int)
#define IRRC_STOP _IOW(IRRC_IOCTL_MAGIC, 1, int)

/* from HAL */
#define MIXER_CARD 0
#define MIXER_XML_PATH "/system/etc/mixer_paths.xml"
#define DEFAULT_OUTPUT_SAMPLING_RATE 48000
#define LOW_LATENCY_OUTPUT_PERIOD_SIZE 256 // audio hal 240
#define LOW_LATENCY_OUTPUT_PERIOD_COUNT 2

/* from pwm kernel driver */
struct irrc_compr_params {
    int frequency;
    int duty;
    int length;
};

static const consumerir_freq_range_t consumerir_freqs[] = {
    {.min = 23000, .max = 1200000}, // from pwm kernel driver
};

static int consumerir_transmit(struct consumerir_device *dev __unused,
                               int carrier_freq, const int pattern[],
                               int pattern_len) {
    int irrc_desc = -1;
    int ret = 0;
    int i = 0;
    int buffer_size = 0;
    int max_buffer_size = 0;
    uint16_t *space_buffer = NULL;
    uint16_t *pulse_buffer = NULL;

    struct audio_route *audio_route = NULL;
    struct irrc_compr_params irrc_compr_params;
    unsigned int flags = PCM_OUT;
    struct pcm *pcm = NULL;

    struct pcm_config config = {
        // From audio hal
        .channels = 2,
        .rate = DEFAULT_OUTPUT_SAMPLING_RATE, // 48000
        .period_size = LOW_LATENCY_OUTPUT_PERIOD_SIZE,
        .period_count = LOW_LATENCY_OUTPUT_PERIOD_COUNT,
        .format = PCM_FORMAT_S16_LE,
        .start_threshold = LOW_LATENCY_OUTPUT_PERIOD_SIZE / 4,
        .stop_threshold = INT_MAX,
        .avail_min = LOW_LATENCY_OUTPUT_PERIOD_SIZE / 4,
    };

    /* Set carrier freq and duty. */
    irrc_compr_params.frequency = carrier_freq;
    irrc_compr_params.duty = 50;

    irrc_desc = open("/dev/msm_IRRC_pcm_dec", O_RDWR);
    if (irrc_desc < 0) {
        ALOGE("Error open /dev/msm_IRRC_pcm_dec");
        ret = -2;
        goto out7;
    }

    audio_route = audio_route_init(MIXER_CARD, MIXER_XML_PATH);
    if (!audio_route) {
        ALOGE("Failed to init audio route controls.");
        ret = -1;
        goto out6;
    }

    /* TODO: get AudioFocus */

    /* Apply mixer for route sound to IR blaster */
    audio_route_apply_path(audio_route, "lg-irrc-lineout");
    audio_route_update_mixer(audio_route);
    audio_route_apply_path(audio_route, "lg-irrc-playback");
    audio_route_update_mixer(audio_route);

    /* Open sound device connected to IR blaster */
    flags |= PCM_MONOTONIC;               // From audiohal
    pcm = pcm_open(0, 1, flags, &config); // From audiohal
    if (!pcm || !pcm_is_ready(pcm)) {
        ALOGE("pcm_open failed: %s", pcm_get_error(pcm));
        ret = -3;
        goto out5;
    }

    max_buffer_size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));

    /* Buffer for IR spaces */
    space_buffer = calloc(1, max_buffer_size);
    if (space_buffer == NULL) {
        ALOGE("Failed to allocate space sound buffer");
        ret = -4;
        goto out4;
    }

    /* Buffer for IR pulses */
    pulse_buffer = calloc(1, max_buffer_size);
    if (pulse_buffer == NULL) {
        ALOGE("Failed to allocate pulse sound buffer");
        ret = -5;
        goto out3;
    }

    /* Fill pulses buffer with max volume values */
    for (i = 0; i < max_buffer_size / (int)sizeof(*pulse_buffer); i++)
        pulse_buffer[i] = 32767;

    /* Enable IR PWM with set freq and duty. */
    if (ioctl(irrc_desc, IRRC_START, &irrc_compr_params) < 0) {
        ALOGE("Error call IRRC_START ioctl");
        ret = -6;
        goto out2;
    }

    /* Write empty data to initialize the IR blaster.
     * Don't know how does it work, but LG does this.
     * And without it IR doesn't work properly. */
    for (i = 0; i < 4; i++) {
        if (pcm_write(pcm, space_buffer, max_buffer_size)) {
            ALOGE("pcm_write failed %d", errno);
            ret = -7;
            goto out1;
        }
    }

    /* Lets go... */
    for (i = 0; i < pattern_len; i++) {
        /* Calculate how much we need to write for given amount of microseconds. */
        buffer_size = pattern[i] * DEFAULT_OUTPUT_SAMPLING_RATE / 1000000 * 2 *
                      sizeof(*pulse_buffer);
        if (buffer_size > max_buffer_size) {
            ALOGE("Big (%d) packet not supported, truncated", buffer_size);
            buffer_size = max_buffer_size;
        }
        if ((i + 1) % 2) { // "is alternating series"
            ALOGD("PULSE [%d]: %d , size %d", i, pattern[i], buffer_size);
            /* write needed pulses */
            if (pcm_write(pcm, pulse_buffer, buffer_size)) {
                ALOGE("pcm_write failed %d", errno);
                ret = -8;
                goto out1;
            }
        } else {
            ALOGD("SPACE [%d]: %d , size %d", i, pattern[i], buffer_size);
            /* or write needed spaces */
            if (pcm_write(pcm, space_buffer, buffer_size)) {
                ALOGE("pcm_write failed %d", errno);
                ret = -9;
                goto out1;
            }
        }
    }

    /* Write final space */
    if (pcm_write(pcm, space_buffer, max_buffer_size)) {
        ALOGE("pcm_write failed %d", errno);
        ret = -10;
    }

out1:
    /* Turn off PWM */
    if (ioctl(irrc_desc, IRRC_STOP, &irrc_compr_params) < 0) {
        ALOGE("Error call IRRC_STOP ioctl");
        ret = -11;
    }

out2:
    free(pulse_buffer);
out3:
    free(space_buffer);
out4:
    pcm_close(pcm);
out5:
    audio_route_free(audio_route);
out6:
    close(irrc_desc);
out7:
    return ret;
}

static int
consumerir_get_num_carrier_freqs(struct consumerir_device *dev __unused) {
    return ARRAY_SIZE(consumerir_freqs);
}

static int consumerir_get_carrier_freqs(struct consumerir_device *dev __unused,
                                        size_t len,
                                        consumerir_freq_range_t *ranges) {
    size_t to_copy = ARRAY_SIZE(consumerir_freqs);

    to_copy = len < to_copy ? len : to_copy;
    memcpy(ranges, consumerir_freqs, to_copy * sizeof(consumerir_freq_range_t));
    return to_copy;
}

static int consumerir_close(hw_device_t *dev) {
    free(dev);
    return 0;
}

/*
 * Generic device handling
 */
static int consumerir_open(const hw_module_t *module, const char *name,
                           hw_device_t **device) {
    if (strcmp(name, CONSUMERIR_TRANSMITTER) != 0) {
        return -EINVAL;
    }
    if (device == NULL) {
        ALOGE("NULL device on open");
        return -EINVAL;
    }

    consumerir_device_t *dev = malloc(sizeof(consumerir_device_t));
    memset(dev, 0, sizeof(consumerir_device_t));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t *)module;
    dev->common.close = consumerir_close;

    dev->transmit = consumerir_transmit;
    dev->get_num_carrier_freqs = consumerir_get_num_carrier_freqs;
    dev->get_carrier_freqs = consumerir_get_carrier_freqs;

    *device = (hw_device_t *)dev;
    return 0;
}

static struct hw_module_methods_t consumerir_module_methods = {
    .open = consumerir_open,
};

consumerir_module_t HAL_MODULE_INFO_SYM = {
    .common =
        {
            .tag = HARDWARE_MODULE_TAG,
            .module_api_version = CONSUMERIR_MODULE_API_VERSION_1_0,
            .hal_api_version = HARDWARE_HAL_API_VERSION,
            .id = CONSUMERIR_HARDWARE_MODULE_ID,
            .name = "LG Audio IRRC IR HAL",
            .author = "The LineageOS Project",
            .methods = &consumerir_module_methods,
        },
};
