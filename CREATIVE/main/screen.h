#include "sh1106.h"

/* --- Enums --- */
typedef enum screen_page {
    SCREEN_TEMPERATURE = 0,
    SCREEN_HUMIDITY
}            screen_page_e;

typedef enum screen_orient {
    SCREEN_UP = 0,
    SCREEN_DOWN
}            screen_orient_e;


/* --- Typedefs --- */


/*--- Public functions declarations --- */
void screen_init(void);
void screen_update(void);

void screen_set_page(screen_page_e page);
void screen_set_data(uint8_t temperature, uint8_t humidity);
void screen_set_orientation(screen_orient_e orient);
