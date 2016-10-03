/*
 * Copyright (C) 2014 The CyanogenMod Project
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

public class DisplayColorCalibration {
    private static final String COLOR_FILE = "/sys/class/graphics/fb0/rgb";
    private static final String KCAL_FILE = "/sys/class/graphics/fb0/kcal";
    private static int control_type;

    public static boolean isSupported() {

        if (FileUtils.isFileWritable(COLOR_FILE)) {
                control_type = 1;
                return true;
        } else if (FileUtils.isFileWritable(KCAL_FILE)) {
                control_type = 2;
                return true;
        } else {
                return false;
        }

    }

    public static int getMaxValue()  {
        if (control_type == 1) {
                return 32768;
        } else {
                return 255;
        }
    }

    public static int getMinValue()  {
        if (control_type == 1) {
                return 255;
        } else {
                return 35;
        }
    }

    public static int getDefValue() {
        return getMaxValue();
    }

    public static String getCurColors()  {
        if (control_type == 1) {
                return FileUtils.readOneLine(COLOR_FILE);
        } else {
                return FileUtils.readOneLine(KCAL_FILE);
        }
    }

    public static boolean setColors(String colors) {
        if (control_type == 1) {
                return FileUtils.writeLine(COLOR_FILE, colors);
        } else {
                return FileUtils.writeLine(KCAL_FILE, colors);
        }
    }
}
