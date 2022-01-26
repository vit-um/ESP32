#include "buttons.h"

/* --- Static variables --- */
static buttons_ctx_t ctx[BUTTON_MAX];

/* --- Static functions declarations --- */
static void IRAM_ATTR buttons_isr_handler(void* arg);
static bool IRAM_ATTR timer_debounce_isr_handler(void *arg);
static void timer_debounce_init(void);

/* --- Public functions definitions --- */
void buttons_init(void) {
    gpio_config_t buttons_config = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << SW1_GPIO) | (1ULL << SW2_GPIO),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&buttons_config));

    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_EDGE));

    ESP_ERROR_CHECK(gpio_isr_handler_add(SW1_GPIO, buttons_isr_handler, (void*) BUTTON_1));
    ctx[BUTTON_1].gpio = SW1_GPIO;
    ESP_ERROR_CHECK(gpio_isr_handler_add(SW2_GPIO, buttons_isr_handler, (void*) BUTTON_2));
    ctx[BUTTON_2].gpio = SW2_GPIO;

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
    timer_config_t timer_config = {
        .alarm_en = TIMER_ALARM_EN,
        .counter_en = TIMER_PAUSE,
        .intr_type = TIMER_INTR_LEVEL,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = TIMER_AUTORELOAD_EN,
        .divider = TIMER_BASE_CLK / 1000000,    // 1 us per tick
    };
    ESP_ERROR_CHECK(timer_init(DEBOUNCE_TIMER_GROUP, DEBOUNCE_TIMER, &timer_config));
    ESP_ERROR_CHECK(timer_set_counter_value(DEBOUNCE_TIMER_GROUP, DEBOUNCE_TIMER, 0));
    ESP_ERROR_CHECK(timer_set_alarm_value(DEBOUNCE_TIMER_GROUP, DEBOUNCE_TIMER, 1000 * DEBOUNCE_TICK_TIME_MS));
    ESP_ERROR_CHECK(timer_enable_intr(DEBOUNCE_TIMER_GROUP, DEBOUNCE_TIMER));
    ESP_ERROR_CHECK(timer_isr_callback_add(DEBOUNCE_TIMER_GROUP, DEBOUNCE_TIMER, timer_debounce_isr_handler, NULL, 0));
    ESP_ERROR_CHECK(timer_start(DEBOUNCE_TIMER_GROUP, DEBOUNCE_TIMER));
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
