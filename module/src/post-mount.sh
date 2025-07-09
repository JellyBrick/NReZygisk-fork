#!/system/bin/sh

DEBUG=@DEBUG@

MODDIR=${0%/*}
if [ "$ZYGISK_ENABLED" ]; then
  exit 0
fi

cd "$MODDIR"

CPU_ABIS=$(getprop ro.product.cpu.abilist)

if [[ "$CPU_ABIS" == *"arm64-v8a"* || "$CPU_ABIS" == *"x86_64"* ]]; then
  ./bin/zygisk-ptrace64 mount_ns_private
else
  # INFO: Device is 32-bit only

  ./bin/zygisk-ptrace32 mount_ns_private
fi