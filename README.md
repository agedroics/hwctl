# hwctl - system device control

---
## Supported devices

* Nvidia GPUs (with nvidia plugin)
  * Read core temperature (degrees C)
  * Read fan speed (%)


* NZXT Kraken X42/X52/X62/X72 (with kraken-xx2 plugin)
  * Read liquid temperature (degrees C)
  * Read fan speed (RPM)
  * Set fan speed (%)
  * Read pump speed (RPM)
  * Set pump speed (%)

## Installing

### Requirements

* CMake
* hidapi (for kraken-xx2 plugin)
* CUDA Toolkit (for nvidia plugin)

### Instructions

* Generate makefiles using CMake
* Run `make install` with root privileges

Note: PKGBUILD scripts are also available for generating pacman packages.

## Configuration

Configuration is done via profile files in `/etc/hwctld/profiles`.

Syntax:

```
<interval>

<path>
<to>
<input>
;

<path>
<to>
<output>
;

<input value> <output value>
<input value> <output value>
...
```

`<interval>` - time in milliseconds between executions. If set to 0, the profile will execute only once.  
`<path>` - device ID path. List available devices with `hwctld list`.  
`<input value> <output value>` - mappings between values read from input device and values written to output device.

Linear interpolation is used between specified value pairs.  
Pairs are ordered based on the input value.

Example:

`/etc/hwctld/profiles/gpu_aio_fan`

```
1000

GPU-71e222c9-58be-f856-8b9c-7bb4729a0962
core
;

HID-1e71:170e-6D8E4583495
fan
;

0 25
45 25
50 30
55 37
60 46
65 57
70 70
75 85
80 100
```

The example profile will apply a fan curve based on GPU temperature with an update interval of 1000 ms.

After configuration, the daemon can be started by running `hwctld` or by starting the included systemd service (if installed).