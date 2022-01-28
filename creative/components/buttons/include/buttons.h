#include "driver/gpio.h"
#include "driver/timer.h"
#include "esp_err.h"
#include "esp_log.h"


/* --- Buttons pinout --- */
#define BUTTON_1_GPIO           39
#define BUTTON_2_GPIO           18


/* --- Defines --- */
/*
 *  These values can be adjusted to fit buttons sensitivity
*/
#define DEBOUNCE_TICK_TIME_MS   5
#define DEBOUNCE_CONFIDENCE     5


/* --- Enums --- */
typedef enum button_num {
    BUTTON_1 = 0,
    BUTTON_2,
    BUTTON_MAX
}            button_num_e;


/*--- Public functions declarations --- */
void buttons_init(void);
int buttons_get_state(button_num_e button_num);
bool buttons_get_event(button_num_e button_num);
void buttons_set_event(button_num_e button_num, int event);
