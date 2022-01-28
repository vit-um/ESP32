#include "sh1106.h"


/* --- Static functions declarations --- */
static void power_on_oled(void);
static void i2c_master_init(void);
static void sh1106_init(void);


/* --- Public functions definitions --- */
void oled_init(void) {
    i2c_master_init();
    power_on_oled();
    sh1106_init();
}

void sh1106_display_clear(void) {
	i2c_cmd_handle_t cmd;

	uint8_t zero[132];
    memset(zero, 0, 132);
	for (uint8_t i = 0; i < 8; i++) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);

		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, zero, 132, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
    i2c_master_write_byte(cmd, 0x00, true); // reset column
    i2c_master_write_byte(cmd, 0x10, true);
    i2c_master_write_byte(cmd, 0xB0, true); // reset page
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}


void oled_clear_page(uint8_t page) {
	i2c_cmd_handle_t cmd;
	uint8_t zero[132];
    memset(zero, 0, 132);
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);

	i2c_master_write_byte(cmd, 0xB0 | page, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
	i2c_master_write(cmd, zero, 132, true);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
}

void oled_set_orientation(oled_orientation_e orient) {
	i2c_cmd_handle_t cmd; 

    if(orient == OLED_NORMAL) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

		i2c_master_write_byte(cmd, SH1106_SEGREMAP_REVERSE, true);
		i2c_master_write_byte(cmd, SH1106_COMSCANDEC, true);
	
		i2c_master_stop(cmd);
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS));
		i2c_cmd_link_delete(cmd);
    }
    if (orient == OLED_REVERT) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

		i2c_master_write_byte(cmd, SH1106_SEGREMAP_NORMAL, true);
		i2c_master_write_byte(cmd, SH1106_COMSCANINC, true);
	
		i2c_master_stop(cmd);
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS));
		i2c_cmd_link_delete(cmd);
    }
}



void sh1106_display_pattern(void *ignore) {
	i2c_cmd_handle_t cmd;

	for (uint8_t i = 0; i < 4; i++) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, SH1106_PAGE_ADDRESS | i, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		for (uint8_t j = 0; j < 132; j++) {
			i2c_master_write_byte(cmd, 0xFF >> (j % 16), true); 
		}
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}
}


void oled_set_cursor(uint8_t page, uint8_t column) {
    uint8_t higher_bits = 0x0F & (column >> 4);
    uint8_t lower_bits = 0x0F & column;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

	i2c_master_write_byte(cmd, SH1106_PAGE_ADDRESS | page, true);
	i2c_master_write_byte(cmd, SH1106_SETHIGHCOLUMN | higher_bits, true);
    i2c_master_write_byte(cmd, SH1106_SETLOWCOLUMN | lower_bits, true);
	i2c_master_stop(cmd);
	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
}

void oled_putchar(char c) {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
    for(uint8_t j = 0; j < 6; j++)
        i2c_master_write_byte(cmd, font6x8[(((uint8_t)c - 0x20) * 6) + j], true);

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}

void oled_puts(char *str) {
    for (uint8_t i = 0; i < strlen(str); i++) {
        oled_putchar(str[i]);
    }
}


void oled_putchar_inv(char c) {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
    for(uint8_t j = 0; j < 6; j++)
        i2c_master_write_byte(cmd, ~font6x8[(((uint8_t)c - 0x20) * 6) + j], true);

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}


void oled_puts_inv(char *str) {
    for (uint8_t i = 0; i < strlen(str); i++) {
        oled_putchar_inv(str[i]);
    }
}


void oled_puthex( uint8_t h) {
    uint8_t packet[] = {0x40, h};

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
	i2c_master_write(cmd, packet, sizeof(packet), true);	
	i2c_master_stop(cmd);
	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS));
	i2c_cmd_link_delete(cmd);
}



void send_display_text(const void *arg_text) {
	char *text = (char*)arg_text;
	uint8_t text_len = strlen(text);

	i2c_cmd_handle_t cmd;

	uint8_t cur_page = 0;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, 0x08, true); // reset column
	i2c_master_write_byte(cmd, 0x10, true);
	i2c_master_write_byte(cmd, 0xB0 | cur_page, true); // reset page

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	for (uint8_t i = 0; i < text_len; i++) {
		if (text[i] == '\n') {
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
			i2c_master_write_byte(cmd, 0x08, true); // reset column
			i2c_master_write_byte(cmd, 0x10, true);
			i2c_master_write_byte(cmd, SH1106_PAGE_ADDRESS | ++cur_page, true); // increment page

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		} 
		else {
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
			for(uint8_t j = 0; j < 6; j++)
				i2c_master_write_byte(cmd, font6x8[(((uint8_t)text[i] - 0x20) * 6) + j], true);
	
			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		}
	}
}


void sh1106_display_text2(const void *arg_text) {
	char *text = (char*)arg_text;
	uint8_t text_len = strlen(text);

	i2c_cmd_handle_t cmd;

	uint8_t cur_page = 0;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, 0x08, true); // reset column
	i2c_master_write_byte(cmd, 0x10, true);
	i2c_master_write_byte(cmd, 0xB0 | cur_page, true); // reset page

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	for (uint8_t i = 0; i < text_len; i++) {
		if (text[i] == '\n') {
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
			i2c_master_write_byte(cmd, 0x08, true); // reset column
			i2c_master_write_byte(cmd, 0x10, true);
			i2c_master_write_byte(cmd, SH1106_PAGE_ADDRESS | ++cur_page, true); // increment page

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		} 
		else {
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
			i2c_master_write(cmd, font8x8_basic_tr[(uint8_t)text[i]], 8, true);	
			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		}
	}
}

void oled_set_offset(uint8_t offset) {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

	i2c_master_write_byte(cmd, SH1106_SETDISPLAYOFFSET, true);
	i2c_master_write_byte(cmd, offset, true);
	i2c_master_stop(cmd);
	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
}


void oled_draw_hline(uint8_t x_start, uint8_t y_start, uint8_t len) {
    uint8_t page = x_start / 8;
    uint8_t x_page = x_start % 8;

    oled_set_cursor(page, y_start);
    for (uint8_t pos = 0; pos < len; pos++) {
        oled_puthex(1 << x_page);
    }
}



/* --- Static functions definitions --- */
static void power_on_oled(void) {
    
    gpio_config_t en_pin_config = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << EN_OLED),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&en_pin_config));

	ESP_LOGI(tag, "Power on the OLED");
    ESP_ERROR_CHECK(gpio_set_level(EN_OLED, 1));
    vTaskDelay(500 / portTICK_PERIOD_MS);
}

static void i2c_master_init(void) {
	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = SDA_PIN,
		.scl_io_num = SCL_PIN,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = OLED_I2C_SPEED,    
	};
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_config));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
}


static void sh1106_init(void) {
	esp_err_t espRc;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//1 DISPLAY OFF
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_OFF, true);
//2 SET_DISPLAY_CLK_DIV	
	i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_CLK_DIV, true);
	i2c_master_write_byte(cmd, 0x80, true); // DIVIDE RATION = 1, oscillator frequency +15%  
//3 SH1106_SETMULTIPLEX
	i2c_master_write_byte(cmd, SH1106_SETMULTIPLEX, true);
	i2c_master_write_byte(cmd, 0x1F, true); // Multiplex Ratio = 32  
//4 OLED_CMD_SET_DISPLAY_OFFSET
	i2c_master_write_byte(cmd, SH1106_SETDISPLAYOFFSET, true);
	i2c_master_write_byte(cmd, 0x00, true); // OFFSET COM0 
//5 Change mode
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
//6 OLED_CMD_SET_DISPLAY_OFFSET
	i2c_master_write_byte(cmd, SH1106_DC_CMD, true);
	i2c_master_write_byte(cmd, SH1106_DC_ON, true); 
//7 OLED_CMD_SET_SEGMENT_REMAP_INVERSE
	i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP_INVERSE, true);
//8 OLED_CMD_SET_COM_SCAN_MODE_REVERSE
	i2c_master_write_byte(cmd, SH1106_COMSCANDEC, true);
//9 SH1106_SETCOMPINS_SEQUENTIAL
	i2c_master_write_byte(cmd, SH1106_SETCOMPINS, true);
	i2c_master_write_byte(cmd, SH1106_SETCOMPINS_SEQUENTIAL, true); 
//10 OLED_CMD_SET_CONTRAST 
	i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true);
	i2c_master_write_byte(cmd, 0x8F, true); // midle contrast
//11 OLED_CMD_SET_PRECHARGE
	i2c_master_write_byte(cmd, OLED_CMD_SET_PRECHARGE, true);
	i2c_master_write_byte(cmd, 0xF1, true); // pre-change period 1 DCLK / dis-change period 15 DCLKs
//12 OLED_CMD_SET_VCOMH_DESELCT
	i2c_master_write_byte(cmd, OLED_CMD_SET_VCOMH_DESELCT, true);
	i2c_master_write_byte(cmd, 0x40, true); // 1
//13 OLED_CMD_ENTIRE_DISPLAY_OFF
	i2c_master_write_byte(cmd, OLED_CMD_ENTIRE_DISPLAY_OFF, true);
//14 OLED_CMD_DISPLAY_INVERT_NORMAL
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_INVERT_NORMAL, true);
//17 DISPLAY ON
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		ESP_LOGI(tag, "OLED configured successfully");
	} else {
		ESP_LOGE(tag, "OLED configuration failed. code: 0x%.2X", espRc);
	}
	i2c_cmd_link_delete(cmd);
}

