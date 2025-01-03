#!/bin/sh
modprobe bfq
for d in /sys/block/sd?; do
  # HDD (tuned for Seagate SMR drive)
  echo bfq >"$d/queue/scheduler"
  echo 4 >"$d/queue/nr_requests"
  echo 32000 >"$d/queue/iosched/back_seek_max"
  echo 3 >"$d/queue/iosched/back_seek_penalty"
  echo 80 >"$d/queue/iosched/fifo_expire_sync"
  echo 1000 >"$d/queue/iosched/fifo_expire_async"
  echo 5300 >"$d/queue/iosched/slice_idle_us"
  echo 1 >"$d/queue/iosched/low_latency"
  echo 200 >"$d/queue/iosched/timeout_sync"
  echo 0 >"$d/queue/iosched/max_budget"
  echo 1 >"$d/queue/iosched/strict_guarantees"

  # additional tweaks for SSD (tuned for Samsung EVO 850):
  if test $(cat "$d/queue/rotational") = "0"; then
    echo 36 >"$d/queue/nr_requests"
    echo 1 >"$d/queue/iosched/back_seek_penalty"
    # slice_idle_us should be ~ 0.7/IOPS in µs
    echo 16 >"$d/queue/iosched/slice_idle_us"
    echo 10 >"$d/queue/iosched/fifo_expire_sync"
    echo 250 >"$d/queue/iosched/fifo_expire_async"
    echo 10 >"$d/queue/iosched/timeout_sync"
    echo 0 >"$d/queue/iosched/strict_guarantees"
  fi
done
