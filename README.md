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

Project dir and main function [here](t04_ВРЕ11/main/уыз-вре.c)

### CONSOLE OUTPUT:

```
...
Temperature: 28 C
Humidity: 19 %
...
Temperature: 29 C
Humidity: 95 %
...

```
