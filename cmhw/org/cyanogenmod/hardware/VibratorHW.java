/*
 * Copyright (C) 2016 The CyanogenMod Project
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

package org.cyanogenmod.hardware;

import java.util.Scanner;
import org.cyanogenmod.internal.util.FileUtils;

public class VibratorHW {

    private static String TSPDRV_NFORCE_PATH = "/sys/devices/platform/tspdrv/nforce_timed";
    private static String QPNP_NFORCE_PATH = "/sys/devices/virtual/timed_output/vibrator/vtg_level";
    private static int control_type;

    public static boolean isSupported() {
        if (FileUtils.isFileWritable(TSPDRV_NFORCE_PATH)) {
                control_type = 1;
                return true;
        } else if (FileUtils.isFileWritable(QPNP_NFORCE_PATH)) {
                control_type = 2;
                return true;
        } else {
                return false;
        }
    }

    public static int getMaxIntensity()  {
        if (control_type == 1) {
                return 127;
        } else {
                return 31;
        }

    }
    public static int getMinIntensity()  {
        if (control_type == 1) {
                return 1;
        } else {
                return 12;
        }
    }
    public static int getWarningThreshold()  {
        if (control_type == 1) {
                return 90;
        } else {
                return -1;
        }
    }
    public static int getCurIntensity()  {
        if (control_type == 1) {
                return Integer.parseInt(FileUtils.readOneLine(TSPDRV_NFORCE_PATH));
        } else {
                return Integer.parseInt(FileUtils.readOneLine(QPNP_NFORCE_PATH));
        }
    }
    public static int getDefaultIntensity()  {
        if (control_type == 1) {
                return 65;
        } else {
                return 28;
        }
    }
    public static boolean setIntensity(int intensity)  {
        if (control_type == 1) {
                return FileUtils.writeLine(TSPDRV_NFORCE_PATH, String.valueOf(intensity));
        } else {
                return FileUtils.writeLine(QPNP_NFORCE_PATH, String.valueOf(intensity));
        }
    }
}
