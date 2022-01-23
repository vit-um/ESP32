#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "string.h"
#include "driver/uart.h"
#include "driver/gpio.h"

static const int RX_BUF_SIZE = 1024;
static const int uart_num = UART_NUM_2;

static void setup_uart(void) {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .rx_flow_ctrl_thresh = 122,
    };

    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    
    // Set UART2 pins(TX: IO17, RX: IO16, RTS: IO18, CTS: IO19)
    ESP_ERROR_CHECK(uart_set_pin(uart_num, 17, 16, 18, 19));

    // Setup UART buffered IO with event queue
    const int uart_buffer_size = (RX_BUF_SIZE * 2);
    QueueHandle_t uart_queue;
    // Install UART driver using an event queue here
    ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size, \
                                        uart_buffer_size, 10, &uart_queue, 0));

}

static int sendData(const char* logName, const char* data) {
    const int len = strlen(data);
    // const int txBytes = uart_write_bytes_with_break(uart_num, data, len, 100);
    const int txBytes = uart_write_bytes(uart_num, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

static void tx_task(void *arg) {
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1) {
        sendData(TX_TASK_TAG, "\033[H\033[2J\033[41mRED\033[0m \033[42mGREEN\033[0m \033[44mBLUE\033[0m DEFAULT\033[?25l");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    setup_uart();
    xTaskCreate(tx_task, "uart_tx_task", RX_BUF_SIZE * 2, NULL, configMAX_PRIORITIES, NULL);
}
