#!/system/bin/sh
export PATH=/system/xbin:$PATH

setprop gsm.version.baseband `strings /firmware/image/modem.b21 | grep "^M8626A-" | head -1`

multisim=`getprop persist.radio.multisim.config`

if [ "$multisim" = "dsds" ] || [ "$multisim" = "dsda" ]; then
    start ril-daemon1
fi
