#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "esp_err.h"

#include "font8x8_basic.h"
#include "font6x8.h"
#include "sh1106.h"

#define EN_OLED 32  //GPIO32
#define SDA_PIN 21
#define SCL_PIN 22
#define tag "SH1106"

static void power_on_oled(void) {
    gpio_reset_pin(EN_OLED);  
    gpio_set_direction(EN_OLED, GPIO_MODE_OUTPUT);
	ESP_LOGI(tag, "Power on the OLED");
    gpio_set_level(EN_OLED, 1);
}

void i2c_master_init(void) {
	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = SDA_PIN,
		.scl_io_num = SCL_PIN,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = 1000000    
	};
	i2c_param_config(I2C_NUM_0, &i2c_config);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

void task_sh1106_display_clear(void *ignore) {
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


void sh1106_init() {
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
	i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_OFFSET, true);
	i2c_master_write_byte(cmd, 0x00, true); // OFFSET COM0 
//5 Change mode
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
//6 OLED_CMD_SET_DISPLAY_OFFSET
	i2c_master_write_byte(cmd, SH1106_DC_CMD, true);
	i2c_master_write_byte(cmd, SH1106_DC_ON, true); 
//7 OLED_CMD_SET_SEGMENT_REMAP_INVERSE
	i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP_INVERSE, true);
//8 OLED_CMD_SET_COM_SCAN_MODE_REVERSE
	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE_REVERSE, true);
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

void task_sh1106_display_pattern(void *ignore) {
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


void task_sh1106_display_text(const void *arg_text) {
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


void task_sh1106_display_text2(const void *arg_text) {
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

void app_main(void) {
    power_on_oled();
    i2c_master_init();
	sh1106_init();
	task_sh1106_display_clear(NULL);
	printf("\033[34mTask 0 completed\033[0m\n");
	task_sh1106_display_pattern(NULL);
	printf("\033[33mTask 1 completed\033[0m\n");
	vTaskDelay(5000/portTICK_PERIOD_MS);
	task_sh1106_display_clear(NULL);
  	task_sh1106_display_text("\n   Hello, World!\n-------------------");
	vTaskDelay(1000/portTICK_PERIOD_MS);
	task_sh1106_display_text2("\n\n\nAlternate font");
	printf("\033[31mThe End\033[0m\n");
}
