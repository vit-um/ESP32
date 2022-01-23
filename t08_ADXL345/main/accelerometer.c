#include "ADXL345.h"

_Static_assert(OUTPUT_POINT_NUM <= POINT_ARR_LEN, "The CONFIG_EXAMPLE_WAVE_FREQUENCY is too low and using too long buffer.");
static int raw_val[POINT_ARR_LEN];    // Used to store raw values
static int g_index = 0;
static spi_device_handle_t handle;

/* Timer interrupt service routine */
static void IRAM_ATTR timer0_ISR(void *ptr) {
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);

    int *head = (int*)ptr;

    /* DAC output ISR has an execution time of 4.4 us*/
    if (g_index >= OUTPUT_POINT_NUM) g_index = 0;
    dac_output_voltage(DAC_CHAN, *(head + g_index));
    g_index++;
}

/* Timer group0 TIMER_0 initialization */
static void dac_timer_init(int timer_idx, bool auto_reload) {
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .intr_type = TIMER_INTR_LEVEL,
        .auto_reload = auto_reload,
    };

    ESP_ERROR_CHECK(timer_init(TIMER_GROUP_0, timer_idx, &config));
    ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL));
    ESP_ERROR_CHECK(timer_set_alarm_value(TIMER_GROUP_0, timer_idx, ALARM_VAL_US));
    ESP_ERROR_CHECK(timer_enable_intr(TIMER_GROUP_0, TIMER_0));
    /* Register an ISR handler */
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer0_ISR, (void *)raw_val, ESP_INTR_FLAG_IRAM, NULL);

    timer_pause(TIMER_GROUP_0, TIMER_0);
    for (int i = 0; i < OUTPUT_POINT_NUM; i++)
        raw_val[i] = (int)((sin(i * CONST_PERIOD_2_PI / OUTPUT_POINT_NUM) + 1) * (double)(AMP_DAC) / 2 + 0.5);

    gpio_reset_pin(LED1_GPIO);
    gpio_reset_pin(LED2_GPIO);
	gpio_reset_pin(LED3_GPIO);
    gpio_set_direction(LED1_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(LED2_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(LED3_GPIO, GPIO_MODE_INPUT_OUTPUT);
}

static void power_on(void){
    gpio_reset_pin(EN_AMP);
    gpio_reset_pin(EN_ACCEL);
    gpio_set_direction(EN_AMP, GPIO_MODE_OUTPUT);
    gpio_set_direction(EN_ACCEL, GPIO_MODE_OUTPUT);
    gpio_set_level(EN_AMP, 1);
    gpio_set_level(EN_ACCEL, 1);
	vTaskDelay(10 / portTICK_PERIOD_MS); // After power supply is applied, the LIS3DH
										 // performs a 5 ms boot procedure to load the
										 // trimming parameter
}

static void spi_init_master(void) {
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_MISO,
        .mosi_io_num = PIN_MOSI,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,  // -1 if not used.
        .quadhd_io_num = -1,  // -1 if not used.
        .max_transfer_sz = 32,
    };
    //Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "Initializing bus SPI...");
}

/*Configuration for the SPI device on the other side of the bus*/
static void spi_init_device(void) {
    ESP_LOGI(TAG, "Initializing the SPI device...");
    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,              // Amount of dummy bits to insert between address and data phase.
        .clock_speed_hz = 3000000,    // Max 5 MHz
        .duty_cycle_pos = 128,        // 50% duty cycle (default)
        .mode = 3,                    // clock polarity (CPOL) = 1 and clock phase (CPHA) = 1.
        .spics_io_num = PIN_SPI_CS,   // CS GPIO pin for this device, or -1 if not used.
        .cs_ena_posttrans = 3,        // Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
        .queue_size = 3,              // Transaction queue size = 3
        .input_delay_ns = 10    // The time required between SCLK and MISO valid, including the possible clock delay from slave to master
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &handle));
}

static uint8_t read_acc_byte(uint8_t address) {
    uint8_t sendbuf[2] = {0};
    uint8_t recvbuf[2] = {0};
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    sendbuf[0] = 0x80 | address; // read register
    t.length = sizeof(sendbuf) * 8;
    t.tx_buffer = sendbuf;
    t.rx_buffer = recvbuf;
    //printf("Sent: 0x%02X\n", sendbuf[0]);
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_device_polling_transmit(handle, &t));
    return recvbuf[1];
}

static void write_acc_byte(uint8_t address, uint8_t data) {
    uint8_t sendbuf[2] = {0};
    uint8_t recvbuf[2] = {0};
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    sendbuf[0] = address; // write address
    sendbuf[1] = data; // write data    
    t.length = sizeof(sendbuf) * 8;
    t.tx_buffer = sendbuf;
    t.rx_buffer = recvbuf;
    //printf("Addr: 0x%02X Config: 0x%02X\n", sendbuf[0], sendbuf[1]);
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_device_polling_transmit(handle, &t));
}

static void ADXL345_init(void) {
    if(read_acc_byte(DEVICE_ID) == I_M_DEVID) {
        write_acc_byte(DATA_FORMAT, 0X0b); // BIT6: 4-проводный режим SPI (по умолчанию); 
                                           // BIT5: уровень прерывания 0/1 (допустимый высокий / низкий);
                                           // BIT3: full res = 1 or max 13 dig 
                                           // BIT2: juidtify = 0 right;  BIT0-1: диапазон = 16g
        write_acc_byte(POWER_CTL, 0x08); // D3 = 0/1: (режим измерения / режим ожидания); D2 = 0/1: (работа / спящий режим);
        write_acc_byte(TAP_AXES, 0x07);             // tap axes on    
        write_acc_byte(BW_RATE, 0x08); // Младшие 4 бита: скорость вывода данных = 25 (при скорости 1600 должна быть установлена ​​скорость SPI> = 2M); BIT4 = 0/1 (низкое энергопотребление / нормальное)
        write_acc_byte(INT_ENABLE, 0x00); // Настройка функции прерывания: не включено
        write_acc_byte(INT_MAP, 0x00); // Установить отображение прерывания на вывод INT1 или INT2
        write_acc_byte(FIFO_CTL, 0x00);
        write_acc_byte(OFSX, 0x00); // Регулировка смещения XYZ
        write_acc_byte(OFSY, 0x00);
        write_acc_byte(OFSZ, 0x00);
        //printf("Received: 0x%02X\n", read_acc_byte(DATA_FORMAT));
        ESP_LOGI(TAG, "ADXL345 init ok");
    }
    else
        ESP_LOGI(TAG, "ADXL345 init error!");
}

static void actions(void) {
    timer_start(TIMER_GROUP_0, TIMER_0);
    gpio_set_level(LED1_GPIO, 1);
    gpio_set_level(LED2_GPIO, 1);
    gpio_set_level(LED3_GPIO, 1);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    gpio_set_level(LED1_GPIO, 0);
    gpio_set_level(LED2_GPIO, 0);
    gpio_set_level(LED3_GPIO, 0);
    timer_pause(TIMER_GROUP_0, TIMER_0);
}

void app_main(void) {
    int16_t x;
    int16_t y;
    int16_t z;
    uint8_t buf[6];
    power_on();
    dac_timer_init(TIMER_0, 1);
    ESP_ERROR_CHECK(dac_output_enable(DAC_CHAN));
    spi_init_master();
    spi_init_device();
    ADXL345_init();

    gpio_set_direction(SW1_GPIO, GPIO_MODE_INPUT);
    printf("Press SW1 for print XYZ-acceleration \n");

    while(1) {

        buf[0] = read_acc_byte(DATA_X0);
        buf[1] = read_acc_byte(DATA_X1);

        buf[2] = read_acc_byte(DATA_Y0);
        buf[3] = read_acc_byte(DATA_Y1);
    
        buf[4] = read_acc_byte(DATA_Z0);
        buf[5] = read_acc_byte(DATA_Z1);

        x = ((int16_t)buf[1] << 8) + buf[0];
        y = ((int16_t)buf[3] << 8) + buf[2];
        z = ((int16_t)buf[5] << 8) + buf[4];

        if (gpio_get_level(SW1_GPIO) == 0) {
            
            printf("Acceleration:\tX=%d\tY=%d\tZ=%d\n", x, y, z);
        }
        while (gpio_get_level(SW1_GPIO) == 0)
            vTaskDelay(1 / portTICK_PERIOD_MS);
        if (abs(x) > 240 || abs(y) > 240)   // || abs(z) < 240)
            actions();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    ESP_ERROR_CHECK(spi_bus_remove_device(handle));
}

//    printf("Received: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n", recvbuf[0],recvbuf[1],recvbuf[2],recvbuf[3],recvbuf[4],recvbuf[5],recvbuf[6]);

