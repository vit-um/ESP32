#include "driver/gpio.h"
#include "driver/timer.h"
#include "esp_err.h"
#include "esp_log.h"


/* --- Buttons pinout --- */
#define SW1_GPIO 39
#define SW2_GPIO 18


/* --- Defines --- */
/*
 *  These values can be adjusted to fit buttons sensitivity
*/
#define DEBOUNCE_TICK_TIME_MS   5
#define DEBOUNCE_CONFIDENCE     5

#define DEBOUNCE_TIMER_GROUP    TIMER_GROUP_1
#define DEBOUNCE_TIMER          TIMER_1

#define TAG "BUTTONS"

/* --- Enums --- */
typedef enum button_num {
    BUTTON_1 = 0,
    BUTTON_2,
    BUTTON_MAX
}            button_num_e;


/* --- Typedefs --- */
typedef struct buttons_ctx {
    int state;
    bool event;
    bool debouncing;
    uint32_t gpio;
    int debounce_ctr;
}              buttons_ctx_t;

/*--- Public functions declarations --- */
void buttons_init(void);
int buttons_get_state(button_num_e button_num);
bool buttons_get_event(button_num_e button_num);
void buttons_set_event(button_num_e button_num, int event);
