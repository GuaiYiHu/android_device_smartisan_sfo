# SFO thermal reporting HAL

This vendor AIDL `IThermal/default` service implements frozen interface version 1.
Android's AIDL thermal wrapper supports this version. It reports temperatures,
static hot/cold thresholds, cooling states and filtered change callbacks. It
does not write sysfs or replace the stock `thermal-engine`, QPNP protection or
`msm_thermal` controller.

## Sensor policy

Thermal zones are resolved by their `type`, not their probe-dependent index.
Temperatures and thresholds returned to Android are in Celsius.

| HAL name | Source | Units at source | Reported policy |
| --- | --- | --- | --- |
| `cpu0`–`cpu3` | `tsens_tz_sensor5`–`8` | C | LIGHT 70/55 C, SEVERE 110/90 C, SHUTDOWN 115/110 C |
| `pop-mem` | `tsens_tz_sensor3` | C | LIGHT 70/55 C; type UNKNOWN |
| `main-battery` | `power_supply/main_battery/temp` | deci-C | Charger `health`: Warm/Cool → LIGHT, Overheat/Cold → SEVERE, Good → NONE |
| `pm8941` | `pm8941_tz` | milli-C | Actual boot-time QPNP trip2/1/0 → LIGHT/SEVERE/SHUTDOWN; 2 C clear hysteresis |
| `pm8841-alarm` | `pm8841_tz` | milli-C | Same alarm policy; driver estimates temperature from alarm stage rather than using an ADC |
| `pa-therm0` | `pa_therm0` | C | POWER_AMPLIFIER; no invented protection thresholds |

CPU and memory steady-state points and CPU shutdown points come from
`vendor/smartisan/sfo/proprietary/vendor/etc/thermal-engine-8974.conf`.
CPU mapping and 110/90 C mitigation points come from
`kernel/smartisan/msm8974/arch/arm/boot/dts/qcom/msm8974.dtsi`;
the mapping also matches the stock thermal-engine aliases. A CPU actually
offlined by `msm_thermal/core_control/cpus_offlined` is reported at least SEVERE.
QPNP PMIC trip ordering and 2 C stage hysteresis come from
`drivers/thermal/qpnp-temp-alarm.c` in that kernel. Battery warm/cool thresholds
are read once from `temp_warm`/`temp_cool`; actual severity follows charger health,
so this service does not duplicate or override charger hysteresis.

The absent `back_battery` and `stc3115` return -30 C and present=0. Unconnected
thermistors return -40 C. These are not exposed as healthy cold sensors.
`pa_therm0` is included only when readable and above its -40 C floor at startup.
Other TSENS channels have no verified component mapping/policy and are omitted.

There is currently **no calibrated SKIN sensor**. CPU, battery, memory and PA
temperatures must not be relabeled as physical surface temperature. Android's
global app thermal status is aggregated from SKIN sensors, so it can remain
NONE even while CPU/BATTERY thermal events indicate mitigation. Skin headroom
and forecast remain unavailable. Surface-temperature support needs either a
working surface thermistor or a separately calibrated virtual-skin model.

## Cooling devices and failure behavior

The kernel registers no `cooling_device*` nodes on this device. Instead, the HAL
reports the actual CPU thermal-offline mask (number of affected cores), CPU0's
`scaling_max_freq` and KGSL `max_gpuclk`. The other CPUs' cpufreq directories
disappear during normal hotplug, so they are not advertised as independent
cooling devices. Frequency cap state is the
index of the highest permitted frequency in the boot-time descending frequency
table; zero means the maximum frequency is allowed. These caps can also reflect
power-policy constraints. Current DVFS frequency is not a cooling state.

Sensor and cooling-device names/order are fixed at service startup. A failed
temperature read returns NaN in its original list slot and preserves the last
known protection severity; it does not fabricate 0 C or erase a sensor.
Thresholds remain static regardless of read failures. If a discovered cooling
device cannot be read, the cooling query returns a descriptive EX_ILLEGAL_STATE
instead of dropping that device or claiming it is unrestricted.

Clients receive an initial snapshot after registration, then severity changes,
readability changes and accumulated temperature changes of at least 0.5 C at
two-second polling intervals. Registration wakes the monitor immediately.
Client calls run outside registration/sensor locks. Null, duplicate and unknown
unregister requests fail; dead callbacks are removed when delivery fails.

## Validation

Build `android.hardware.thermal-service.sfo`, `sfo_thermal_hal_test`, the vendor
SELinux outputs and the device compatibility matrix. The test binary operates
only on a temporary directory under `/data/local/tmp`: shuffled thermal indices,
mixed units, absent sensors, original-policy hysteresis including shutdown,
read failures, fixed thresholds/order, charger health, cooling caps and filtered
initial/transition callbacks. It never writes the real `/sys` or registers a HAL
instance with Android. It is not included in `PRODUCT_PACKAGES`.

The installed service supports `--dump-sensors` without registering a Binder
service. A running registered instance also supports
`dumpsys android.hardware.thermal.IThermal/default`. Full VTS, real high-temperature
behavior, suspend/resume and enforcing-mode runtime validation are separate checks.

Reference: [AOSP thermal mitigation](https://source.android.com/docs/core/power/thermal-mitigation).
