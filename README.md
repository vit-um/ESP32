# ESP32 Hardware

## Task 00 "Hello, World"

Make a Hello World application using ESP-IDF.   

Project dir and main function [here](t00_HelloWorld/main/hello_world_main.c)

### CONSOLE OUTPUT:
```
Hello world!
This is esp32 chip with 2 CPU cores, WiFi/BT/BLE, silicon revision 1, 4MB external flash
Free heap: 299924
Restarting in 10 seconds...
Restarting in 9 seconds...
Restarting in 8 seconds...
```
## Task 01 "Turn on GPIO LEDs"

Project dir and main function [here](t01_LED/main/gpio_led_on.c)

### CONSOLE OUTPUT:
```
Press SW1: Turn on all LEDs
Don`t press SW2!!!
Turning off all LEDs
Turning on LED1
Turning on LED2
Turning on LED3
Turning off all LEDs
Works in loop mode, restart plz
```

## Task 02 "PWM and DAC"

Make `LED1` and `LED2` pulse slowly. You must use `PWM` for `LED1` and `DAC` for `LED2`.

Project dir and main function [here](t02_PWM_DAC/main/led_control_main.c)

### CONSOLE OUTPUT:
```
LED1: Began fade up to duty 8191
LED1: Began fade down to duty 8
LED2: Achieved maximum voltage on the LED = 3.29V.
LED1: Began fade up to duty 8191
LED1: Began fade down to duty 8
LED2: Achieved minimum voltage on the LED = 1.76V.
```

## Task 03 "Buttons"  

Button `SW1` must affect `LED1`, and button `SW2` must affect `LED2`. The initial state of the `LED` is off. One press switches LED on, and another one switches it off. You must avoid switch bounce and be sure that the single button press always works properly.

Project dir and main function [here](t03_SWITCH/main/sw_led_on.c)

### CONSOLE OUTPUT:
```
Turning off all LEDs
Press SW1: Toggle LED1
Press SW2: Toggle LED2
```

## Task 04 "DHT11"  

Get temperature and humidity from DHT11 and display them on the monitor.

Project dir and main function [here](t04_DHT11/main/esp-dht.c)

### CONSOLE OUTPUT:

```
Temperature: 28 C
Humidity: 19 %
...
Temperature: 29 C
Humidity: 95 %
```
## Task 05 "UART"

Print string "RED GREEN BLUE DEFAULT\n" by using the second UART. Make a background color for words according to their names. Use the default console background for the DEFAULT word.

Project dir and main function [here](t05_UART/main/esp_uart.c)
### CONSOLE OUTPUT:

```
I (305) uart: queue free spaces: 10
I (305) TX_TASK: Wrote 62 bytes
```

Connect UART in second USB port and run this command:
> screen /dev/ttyUSB0 115200

### SCREEN OUTPUT:

```bash
RED GREEN BLUE DEFAULT
```

Before that, I recommend setting it up in .screenrc:
> nano ~/.screenrc 

Copy this code:
```
# ?????????????????? ??????????????????????
startup_message off

# ???????????????? ???????????????????? ????????????
vbell on

# ?????????? ?????? ?????????????????????? ?????????? ???????????? 10000
defscrollback 10000

# ???????????????? ???????????? ??????????????????
hardstatus alwayslastline
hardstatus string '%{= kG}[ %{G}%H %{g}][%= %{= kw}%?%-Lw%?%{r}(%{W}%n*%f%t%?(%u)%?%{r})%{w}%?%+Lw%?%?%= %{g}][%{B} %m-%d %{W}%c %{g}]'
```

Some useful commands:

> screen -list

```
There is a screen on:
        1013200.pts-1.HP-x360   (27.01.22 22:29:08)     (Attached)
1 Socket in /run/screen/S-vit.
```

> kill 1013200
> screen -list
```
No Sockets found in /run/screen/S-vit.
```
## Task 06 "Speaker"

Make any sound using the speaker.

Project dir and main function [here](t06_WAVE_GEN/main/wave_gen.c)

### CONSOLE OUTPUT:

```
Power on the sound amplifier
I (312) wave_gen: Waveform: SINE
I (322) wave_gen: Frequency(Hz): 1000
I (322) wave_gen: Output points num: 142

Press SW1: Mute
Press SW2: Change Waveform
I (17132) wave_gen: Waveform: TRIANGLE
I (17132) wave_gen: Frequency(Hz): 1000
I (17132) wave_gen: Output points num: 142

I (18192) wave_gen: Waveform: SAWTOOTH
I (18192) wave_gen: Frequency(Hz): 1000
I (18192) wave_gen: Output points num: 142

I (20662) wave_gen: Waveform: SQUARE
I (20662) wave_gen: Frequency(Hz): 1000
I (20662) wave_gen: Output points num: 142

Mute
Unmute
```
## Task 07 "OLED"

Display 'Hello, World!' on the OLED. You are able to use font6x8.h from the resources

Project dir and main function [here](t07_OLED/main/oled.c)

### CONSOLE OUTPUT:

```
I (316) SH1106: Power on the OLED
I (316) SH1106: OLED configured successfully
Task 0 completed
Task 1 completed
Task 2 completed
Task 3 completed
Task 4 completed
The End
```

## Task 08 "Accelerometer"

Make beep series (a sequence of sound tones using `DAC`) and enable all LEDs (`LED1`, `LED2`, `LED3`) when you flip the board (flip the board at least on `90` degrees from horizontal position in any direction).

Project dir and main function [here](t08_ADXL345/main/accelerometer.c)

### CONSOLE OUTPUT:

```
Press SW1 for print XYZ-acceleration 
Acceleration:   X=13    Y=-2    Z=-257
Acceleration:   X=-11   Y=-262  Z=12
Acceleration:   X=-239  Y=-2    Z=6
```
