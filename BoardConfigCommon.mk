#
# Copyright (C) 2015 The CyanogenMod Project
# Copyright (C) 2017 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# This file sets variables that control the way modules are built
# thorughout the system. It should not be used to conditionally
# disable makefiles (the proper mechanism to control what gets
# included in a build is to use PRODUCT_PACKAGES in a product
# definition file).
#
# inherit from the proprietary version
VENDOR_PATH := device/lge/msm8226-common

TARGET_SPECIFIC_HEADER_PATH += $(VENDOR_PATH)/include

# Bootloader
TARGET_NO_BOOTLOADER := true
TARGET_NO_RADIOIMAGE := true

# Platform
TARGET_BOARD_PLATFORM := msm8226
TARGET_BOARD_PLATFORM_GPU := qcom-adreno305

# Architecture
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_MEMCPY_BASE_OPT_DISABLE := true
TARGET_CPU_VARIANT := krait
MALLOC_SVELTE := true
USE_CLANG_PLATFORM_BUILD := true
BOARD_GLOBAL_CFLAGS += -mfpu=neon -mfloat-abi=softfp
BOARD_GLOBAL_CPPFLAGS += -mfpu=neon -mfloat-abi=softfp

# Kernel image
BOARD_KERNEL_SEPARATED_DT := true
TARGET_KERNEL_SOURCE := kernel/lge/msm8226
BOARD_KERNEL_CMDLINE := console=ttyHSL0,115200,n8 androidboot.console=ttyHSL0 user_debug=31 msm_rtb.filter=0x37 androidboot.hardware=qcom androidboot.selinux=enforcing
BOARD_KERNEL_BASE := 0x00000000
BOARD_KERNEL_PAGESIZE := 2048
BOARD_MKBOOTIMG_ARGS := --ramdisk_offset 0x01000000 --tags_offset 0x00000100
TARGET_KERNEL_CROSS_COMPILE_PREFIX := arm-linux-androideabi-

# Keymaster
TARGET_KEYMASTER_WAIT_FOR_QSEE := true

# Offmode Charging
BOARD_GLOBAL_CFLAGS += \
    -DBOARD_CHARGING_CMDLINE_NAME='"androidboot.mode"' \
    -DBOARD_CHARGING_CMDLINE_VALUE='"chargerlogo"'

# Global flags
BOARD_GLOBAL_CFLAGS += -DLG_CAMERA_HARDWARE -DCAMERA_VENDOR_L_COMPAT

# QCOM hardware
BOARD_USES_QCOM_HARDWARE := true
TARGET_RIL_VARIANT := caf
BOARD_GLOBAL_CFLAGS += -DUSE_RIL_VERSION_10
BOARD_GLOBAL_CPPFLAGS += -DUSE_RIL_VERSION_10
TARGET_TAP_TO_WAKE_NODE := "/sys/devices/virtual/input/lge_touch/dt2w_enable"
TARGET_POWERHAL_VARIANT = qcom

BOARD_HAVE_BLUETOOTH := true

# Wifi
ifeq ($(BOARD_HAS_QCOM_WCNSS),true)
BOARD_HAS_QCOM_WLAN := true
BOARD_WLAN_DEVICE := qcwcn
WPA_SUPPLICANT_VERSION := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
BOARD_HOSTAPD_DRIVER := NL80211
BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
TARGET_USES_WCNSS_CTRL := true
TARGET_USES_QCOM_WCNSS_QMI := true
WIFI_DRIVER_FW_PATH_STA := "sta"
WIFI_DRIVER_FW_PATH_AP := "ap"
WLAN_PATH = wlan-caf

# Bluetooth
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(VENDOR_PATH)/bluetooth_qcom
BOARD_HAVE_BLUETOOTH_QCOM := true
BLUETOOTH_HCI_USE_MCT := true

# FM Radio
BOARD_HAVE_QCOM_FM := true
TARGET_QCOM_NO_FM_FIRMWARE := true
endif

# Nfc
ifeq ($(BOARD_HAS_NFC),true)
BOARD_NFC_CHIPSET := pn547
BOARD_NFC_DEVICE := "/dev/pn547"
endif

# Lights
TARGET_PROVIDES_LIBLIGHT := true

# Audio
BOARD_USES_ALSA_AUDIO := true
AUDIO_FEATURE_ENABLED_HWDEP_CAL := true
AUDIO_FEATURE_ENABLED_MULTI_VOICE_SESSIONS := true
AUDIO_FEATURE_ENABLED_NEW_SAMPLE_RATE := true
USE_CUSTOM_AUDIO_POLICY := 1

# Graphics
HAVE_ADRENO_SOURCE := false
MAX_EGL_CACHE_KEY_SIZE := 12*1024
MAX_EGL_CACHE_SIZE := 2048*1024
OVERRIDE_RS_DRIVER := libRSDriver_adreno.so
TARGET_CONTINUOUS_SPLASH_ENABLED := true
TARGET_USES_C2D_COMPOSITION := true
TARGET_USES_ION := true
USE_OPENGL_RENDERER := true
VSYNC_EVENT_PHASE_OFFSET_NS := 2500000
SF_VSYNC_EVENT_PHASE_OFFSET_NS := 0000000

# Hardware tunables framework
BOARD_HARDWARE_CLASS := $(VENDOR_PATH)/cmhw/

# GPS
TARGET_NO_RPC := true

# Camera
USE_DEVICE_SPECIFIC_CAMERA := true
TARGET_NEEDS_PLATFORM_TEXT_RELOCATIONS := true
TARGET_HAS_LEGACY_CAMERA_HAL1 := true

# Filesystem,
TARGET_USERIMAGES_USE_EXT4 := true
TARGET_USERIMAGES_USE_F2FS := true
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_FLASH_BLOCK_SIZE := 131072
BOARD_CANT_BUILD_RECOVERY_FROM_BOOT_PATCH := true 

# Recovery
TARGET_RECOVERY_FSTAB := $(VENDOR_PATH)/rootdir/fstab.qcom

# Include an expanded selection of fonts
EXTENDED_FONT_FOOTPRINT := true

# Time services
BOARD_USES_QC_TIME_SERVICES := true

# Use HW crypto for ODE
TARGET_HW_DISK_ENCRYPTION := false

# SELinux policies
# qcom sepolicy
include device/qcom/sepolicy/sepolicy.mk

BOARD_SEPOLICY_DIRS += $(VENDOR_PATH)/sepolicy

