# SFO sensor compatibility

`sensors.msm8974.so` is the device MultiHAL wrapper. It loads the proprietary
`libsensors.msm8974.so`, which talks to `sensors.qcom`.

## Timestamp clock

Android sensor event timestamps use the boot clock, including time in suspend
(`CLOCK_BOOTTIME`, the basis of `elapsedRealtimeNanos()`). The legacy SFO time
service instead aligns DSP ticks to `CLOCK_REALTIME` and returns realtime AP
timestamps. The old HAL adds that offset directly to DSP sample timestamps.

The vendor `sensors.qcom` blob switches the two sensor time-service clock
arguments to `CLOCK_BOOTTIME`: the DSP/AP offset calculation and the AP timestamp
generation. Both must change together. DSP ticks, sample intervals, rollover
handling, and unrelated realtime deadlines stay intact. The corrected blob is
pinned in `proprietary-files.txt`; the vendor commit documents the original
hash, instruction offsets and replacement bytes.

This fixes the clock at its source, so MultiHAL does not subtract a changing
wall-clock offset or replace sample timestamps with delivery times. Queued
samples retain their original sampling times.

When several fusion algorithms run together, the legacy HAL can also replay
an older magnetic batch after newer magnetic samples. MultiHAL discards only
magnetic-field samples whose timestamps do not advance for that sensor handle.
Other event types, metadata, values, and timestamps pass through unchanged.

## Magnetic calibration

The BMC050/BMM050 configuration loads `/vendor/lib/libbmc050.so` dynamically.
The matching original firmware library must be included even though it is not
a `DT_NEEDED` entry of `sensors.qcom`. Without it MAG_CAL is unavailable and
rotation-vector, orientation, and geomagnetic fusion activation can fail.
