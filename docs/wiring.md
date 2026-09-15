# Wiring guide

The MAX30100 and SH1106 OLED share one I²C bus. Connect every ground before applying power.

| ESP32 | MAX30100 module | SH1106 OLED |
|---|---|---|
| 3V3 | VIN/VCC* | VCC* |
| GND | GND | GND |
| GPIO 21 | SDA | SDA |
| GPIO 22 | SCL | SCL |

\* Check the markings and regulator arrangement on your exact breakout module. The ESP32 uses 3.3 V logic and its GPIO pins are not 5 V tolerant. Do not feed 5 V I²C pull-ups into SDA or SCL.

Typical I²C addresses are `0x57` for the MAX30100 and `0x3C` for the OLED. If initialization fails:

1. Run an I²C scanner and confirm both addresses appear.
2. Check that SDA and SCL are not swapped.
3. Use short wires and a common ground.
4. Check the module's required supply voltage and onboard pull-ups.
5. Keep the sensor covered by a steady fingertip and away from strong ambient light.

The firmware uses the hardware I²C pins explicitly:

```cpp
Wire.begin(21, 22);  // SDA, SCL
```

No interrupt wire is required by this implementation.

