#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/dac.h"
#include "driver/timer.h"
#include "esp_log.h"
#include "esp_err.h"

/*  The timer ISR has an execution time of 5.5 micro-seconds(us).
    Therefore, a timer period less than 5.5 us will cause trigger the interrupt watchdog.
    7 us is a safe interval that will not trigger the watchdog. No need to customize it.
*/

#define TIMER_INTR_US          7     // Execution time of each ISR interval in micro-seconds
#define TIMER_DIVIDER          16
#define POINT_ARR_LEN          200   // Length of points array
#define AMP_DAC                255   // Amplitude of DAC voltage. If it's more than 256 will causes dac_output_voltage() output 0.
#define VDD                    3300  // VDD is 3.3V, 3300mV
#define CONST_PERIOD_2_PI      6.2832
#define SEC_TO_MICRO_SEC(x)    ((x) / 1000 / 1000)    // Convert second to micro-second
#define TIMER_TICKS            (TIMER_BASE_CLK / TIMER_DIVIDER)     // TIMER_BASE_CLK = APB_CLK = 80MHz
#define ALARM_VAL_US           SEC_TO_MICRO_SEC(TIMER_INTR_US * TIMER_TICKS)     // Alarm value in micro-seconds
#define OUTPUT_POINT_NUM       (int)(1000000 / (TIMER_INTR_US * FREQ) + 0.5)     // The number of output wave points.

#define DAC_CHAN        0  // DAC_CHANNEL_1 (GPIO25) by default
#define FREQ            1000        // 1000 Hz by default
#define EN_AMP          5   //GPIO5 = 1 for power on Amplifier
#define EN_ACCEL 		23  //GPIO23 = 1 for power on Accelerometr
#define PIN_MISO		12
#define PIN_MOSI 		13
#define PIN_CLK  		14
#define PIN_SPI_CS  	15

#define LED1_GPIO 27
#define LED2_GPIO 26
#define LED3_GPIO 33

#define SW1_GPIO 39
#define SW2_GPIO 18
#define TAG "ACCELEROMETER"

/****************** Описание регистров ADXL345 ***********************/

#define DEVICE_ID       0X00 //R Получить идентификатор устройства, 0XE5
#define THRESH_TAP      0X1D // Порог тапа
#define OFSX            0X1E // значение смещения корректировки оси x
#define OFSY            0X1F // значение смещения корректировки оси Y
#define OFSZ            0X20 // значение смещения корректировки оси Z
#define DUR             0X21 // Длительность толчка
#define Latent          0X22 // Задержка перед ожиданием второго толчка
#define Window          0X23 // Время для обнаружения второго толчка
#define THRESH_ACK      0X24 // Значение уставки функции активности
#define THRESH_INACT    0X25 // Значение уставки функции бездействия
#define TIME_INACT      0X26 // Время бездействия
#define ACT_INACT_CTL   0X27 // Привязка событий активности/бездействия на оси
#define THRESH_FF       0X28 // Значение уставки свободного падения
#define TIME_FF         0X29 // Время свободного падения
#define TAP_AXES        0X2A // Привязка событий толчка на оси
#define ACT_TAP_STATUS  0X2B //R Регистр флагов событий толчка или активности
#define BW_RATE         0X2C // Настройка частоты дискретизации
#define POWER_CTL       0X2D // Настройка режима энергосбережения
#define INT_ENABLE      0X2E // Регистр управления прерываниями
#define INT_MAP         0X2F // Назначение выходных линий для прерываний
#define INT_SOURCE      0X30 //R Регистр флагов прерываний
#define DATA_FORMAT     0X31 // Настройка формата выходных данных
#define DATA_X0         0X32 //R Выходные данные для оси X (младший байт)
#define DATA_X1         0X33 //R Выходные данные для оси X (старший байт)
#define DATA_Y0         0X34 //R Выходные данные для оси Y (младший байт)
#define DATA_Y1         0X35 //R Выходные данные для оси Y (старший байт)
#define DATA_Z0         0X36 //R Выходные данные для оси Z (младший байт)
#define DATA_Z1         0X37 //R Выходные данные для оси Z (старший байт)
#define FIFO_CTL        0X38 // Настройка буфера FIFO 
#define FIFO_STATUS     0X39 //R Регистр состояния буфера FIFO
 
#define I_M_DEVID ((uint8_t) 0xE5) // ID устройства = 0XE5
