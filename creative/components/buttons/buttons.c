#include "buttons.h"


/* --- Defines --- */
#define DEBOUNCE_TIMER_GROUP    TIMER_GROUP_1
#define DEBOUNCE_TIMER          TIMER_1


/* --- Typedefs --- */
typedef struct buttons_ctx {
    int state;
    bool event;
    bool debouncing;
    uint32_t gpio;
    int debounce_ctr;
}              buttons_ctx_t;


/* --- Static variables --- */
static const char *TAG = "BUTTONS";
static buttons_ctx_t ctx[BUTTON_MAX];


/* --- Static functions declarations --- */
static void IRAM_ATTR buttons_isr_handler(void* arg);
static bool IRAM_ATTR timer_debounce_isr_handler(void *arg);
static void timer_debounce_init(void);


/* --- Public functions definitions --- */
void buttons_init(void) {
    gpio_config_t buttons_config;
    buttons_config.intr_type = GPIO_INTR_ANYEDGE;
    buttons_config.mode = GPIO_MODE_INPUT;
    buttons_config.pin_bit_mask = (1ULL << BUTTON_1_GPIO) |
                                  (1ULL << BUTTON_2_GPIO);
    buttons_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    buttons_config.pull_up_en = GPIO_PULLUP_DISABLE;

    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&buttons_config));

    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_install_isr_service(ESP_INTR_FLAG_EDGE));

    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_isr_handler_add(BUTTON_1_GPIO, buttons_isr_handler, (void*) BUTTON_1));
    ctx[BUTTON_1].gpio = BUTTON_1_GPIO;
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_isr_handler_add(BUTTON_2_GPIO, buttons_isr_handler, (void*) BUTTON_2));
    ctx[BUTTON_2].gpio = BUTTON_2_GPIO;

    for(int i = 0; i < BUTTON_MAX; i++) {
        ctx[i].state = 1;
        ctx[i].event = 0;
    }

    timer_debounce_init();
}

int buttons_get_state(button_num_e button_num) {
    return ctx[button_num].state;
}

bool buttons_get_event(button_num_e button_num) {
    return ctx[button_num].event;
}

void buttons_set_event(button_num_e button_num, int event) {
    ctx[button_num].event = event;
}


/* --- Static functions definitions --- */
static void timer_debounce_init(void) {
    timer_config_t timer_config;
    timer_config.alarm_en = TIMER_ALARM_EN;
    timer_config.counter_en = TIMER_PAUSE;
    timer_config.intr_type = TIMER_INTR_LEVEL;
    timer_config.counter_dir = TIMER_COUNT_UP;
    timer_config.auto_reload = TIMER_AUTORELOAD_EN;
    timer_config.divider = TIMER_BASE_CLK / 1000000;    // 1 us per tick

    ESP_ERROR_CHECK_WITHOUT_ABORT(timer_init(DEBOUNCE_TIMER_GROUP, DEBOUNCE_TIMER, &timer_config));
    ESP_ERROR_CHECK_WITHOUT_ABORT(timer_set_counter_value(DEBOUNCE_TIMER_GROUP, DEBOUNCE_TIMER, 0));
    ESP_ERROR_CHECK_WITHOUT_ABORT(timer_set_alarm_value(DEBOUNCE_TIMER_GROUP, DEBOUNCE_TIMER, 1000 * DEBOUNCE_TICK_TIME_MS));
    ESP_ERROR_CHECK_WITHOUT_ABORT(timer_enable_intr(DEBOUNCE_TIMER_GROUP, DEBOUNCE_TIMER));
    ESP_ERROR_CHECK_WITHOUT_ABORT(timer_isr_callback_add(DEBOUNCE_TIMER_GROUP, DEBOUNCE_TIMER, timer_debounce_isr_handler, NULL, 0));
    ESP_ERROR_CHECK_WITHOUT_ABORT(timer_start(DEBOUNCE_TIMER_GROUP, DEBOUNCE_TIMER));
}


/* --- Interrupt handlers --- */
static void IRAM_ATTR buttons_isr_handler(void* arg) {
    button_num_e button = (button_num_e)arg;

    ctx[button].debouncing = true;
    // gpio_intr_disable(ctx[button].gpio);
}


static bool IRAM_ATTR timer_debounce_isr_handler(void *arg) {
    for (button_num_e button = 0; button < BUTTON_MAX; button++) {
        if (ctx[button].debouncing) {
            int res = gpio_get_level(ctx[button].gpio);
            if (res == 0) {
                ctx[button].debounce_ctr++;
                if(ctx[button].debounce_ctr >= DEBOUNCE_CONFIDENCE) {       // level is zero
                    int prev_state = ctx[button].state;
                    ctx[button].state = 0;
                    if(prev_state == 1 && ctx[button].state == 0) {
                        ctx[button].event = true;
                    }
                    ctx[button].debounce_ctr = 0;
                    ctx[button].debouncing = false;
                    // gpio_intr_enable(ctx[button].gpio);
                }
            } else {                                                        // level is one
                ctx[button].debounce_ctr = 0;
                ctx[button].state = 1;
                //restart detecting
                ctx[button].debouncing = false;
                // gpio_intr_enable(ctx[button].gpio);
            }
        }
    }

    return 0;
}
