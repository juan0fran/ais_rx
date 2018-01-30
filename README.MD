## GNURadio Scheme to get AIS beacons from ships/stations

This module has two parts, the GNURadio scheme (grc/py) and a C program that manages the output socket from GNURadio.
The C module is compiled as:

```
gcc *.c -o ais_rx
```

## Dependances

In order to get this module working this module is required: https://github.com/daniestevez/gr-kiss
