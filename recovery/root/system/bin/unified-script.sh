#!/system/bin/sh
# This script is needed to automatically set device props.

prjname=$(getprop ro.boot.prjname)

resetprop ro.separate.soft "$prjname"

set_props() {
    local model="$1"
    local device="$2"

    resetprop ro.build.product "$model"
    resetprop ro.product.model "$model"
    resetprop ro.product.device "$device"
    resetprop ro.product.product.device "$device"
    resetprop ro.commonsoft.ota "$device"
}

case "$prjname" in
    "21615")
      set_props "RMX3366" "RE546F"
      ;;            
    "21619"|"2161A")
      set_props "RMX3370" "RE5473"
      ;;
    "2169A"|"2169B")
      set_props "RMX3370" "RE879AL1"
      ;;
esac
