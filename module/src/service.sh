#!/system/bin/sh

DEBUG=@DEBUG@

MODDIR=${0%/*}
if [ "$ZYGISK_ENABLED" ]; then
  exit 0
fi

cd "$MODDIR"

if [ "$(which magisk)" ]; then
  for file in ../*; do
    if [ -d "$file" ] && [ -d "$file/zygisk" ] && ! [ -f "$file/disable" ]; then
      if [ -f "$file/service.sh" ]; then
        cd "$file"
        log -p i -t "zygisk-sh" "Manually trigger service.sh for $file"
        sh "$(realpath ./service.sh)" &
        cd "$MODDIR"
      fi
    fi
  done
fi

p_s() {
  curr="$(resetprop "$1")"
  if [ -n "$curr" ] && [ "$curr" != "$2" ]; then
    resetprop -n "$1" "$2"
  fi
}

p_r() {
  case "$(resetprop "$1")" in
    *"$2"*) resetprop -n "$1" "$3" ;;
  esac
}

resetprop -w sys.boot_completed 0

p_s ro.build.type user
p_s ro.build.tags release-keys
p_s ro.boot.vbmeta.device_state locked
p_s ro.boot.verifiedbootstate green
p_s vendor.boot.vbmeta.device_state locked
p_s vendor.boot.verifiedbootstate green
p_s sys.oem_unlock_allowed 0
p_s ro.boot.veritymode enforcing
p_s ro.boot.veritymode.managed yes
p_s ro.boot.flash.locked 1
p_s ro.boot.warranty_bit 0
p_s ro.warranty_bit 0
p_s ro.vendor.boot.warranty_bit 0
p_s ro.vendor.warranty_bit 0
p_s ro.debuggable 0
p_s ro.force.debuggable 0
p_s ro.secure 1
p_s ro.adb.secure 1
p_s ro.boot.realmebootstate green
p_s ro.boot.realme.lockstate 1
p_s ro.secureboot.lockstate locked
p_s ro.is_ever_orange 0

p_r ro.bootmode recovery unknown
p_r ro.boot.mode recovery unknown
p_r ro.boot.bootmode recovery unknown
p_r vendor.boot.mode recovery unknown
p_r vendor.boot.bootmode recovery unknown
