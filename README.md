
# Simple heartbeat detector
This project uses a [DOIT Esp32 DevKit v1 board](https://docs.zerynth.com/latest/reference/boards/doit_esp32/docs/) and the [AD8232 Heart Rate Monitor module](https://www.sparkfun.com/products/12650?_ga=2.125800706.943048900.1602183273-943862590.1599672290) from Sparkfun to detect heartbeats.
# Hardware
### Connections
| AD8232 Pin | ESP32 Pin |
|--|--|
| GND | GND |
| 3.3V| 3V3 |
| LO+| D18 |
| LO-| D19 |

### Schematic
![Connections diagram](https://i.imgur.com/F4L8DsZ.png)

### Pads placement
![Pad placement](https://cdn.sparkfun.com/r/600-600/assets/learn_tutorials/2/5/0/body.png)
<center><h6>Source: Sparkfun </h6></center>

# Firmware
The firmware was written in C using the ESP-IDF framework with [PlatformIO](https://platformio.org/) in Visual Studio Code. You should be able to open the repository as a PlatformIO project. If you have any problems, please feel free to contact me.
 
The sensor is read every 10 milliseconds from the timer ISR and a task waits for the data. Once it gets the sensor value from a [FreeRTOS](https://www.freertos.org/) queue, it applies the _z-score_ algorithm to find its peaks, that should correspond to the R waves from the ECG. This algorithm is not perfect and still need improvements in accuracy. You can adjust the parameters (lag, threshold and influence) to change the sensitivity of the algorithm based on your results.