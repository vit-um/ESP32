#include "sh1106.h"


void app_main(void) {
    uint8_t offset = 0;
	oled_init(); 
    sh1106_display_clear(NULL);
	sh1106_display_pattern(NULL);
	printf("\033[34mTask 0 completed\033[0m\n");
	vTaskDelay(2000/portTICK_PERIOD_MS);
	sh1106_display_clear(NULL);
    oled_set_cursor(1, 10 * 6);
    oled_putchar('C');
	printf("\033[33mTask 1 completed\033[0m\n");
	vTaskDelay(2000/portTICK_PERIOD_MS);
	sh1106_display_clear(NULL);
    oled_set_cursor(2, 18 * 6);
    oled_puts("Hi!");
	printf("\033[35mTask 2 completed\033[0m\n");
	vTaskDelay(2000/portTICK_PERIOD_MS);
    sh1106_display_clear(NULL);
  	send_display_text("\n   Hello, World!\n-------------------");
	printf("\033[36mTask 3 completed\033[0m\n");
	vTaskDelay(2000/portTICK_PERIOD_MS);
	sh1106_display_text2("\n\n\nAlternate font");
	printf("\033[32mTask 4 completed\033[0m\n");
	vTaskDelay(3000/portTICK_PERIOD_MS);
	printf("\033[31mThe End\033[0m\n");
    while(1) {
        oled_set_offset(offset++);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
