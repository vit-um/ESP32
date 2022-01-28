#include "screen.h"


/* --- Typedefs --- */
typedef struct screen_ctx {
    screen_page_e page;
    screen_orient_e orientation;
    char temperature[21];
    char humidity[21];
    bool flip_event;
    bool page_event;
    bool value_update;
}              screen_ctx_t;


/* --- Static variables --- */
static screen_ctx_t ctx;


/* --- Static functions declarations --- */
static void draw_button_a(void);
static void draw_button_b(void);
static void draw_buttons(void);
static void draw_value(void);
static void draw_box(screen_page_e page);
static void change_orient(void);


/* --- Public functions definitions --- */
void screen_init(void) {
    oled_init();
    memset(&ctx, 0, sizeof(screen_ctx_t));
    ctx.page = SCREEN_TEMPERATURE;
    ctx.orientation = SCREEN_UP;

    sh1106_display_clear();
    draw_buttons();
    draw_value();
}

void screen_update(void) {
    if(ctx.flip_event) {
        sh1106_display_clear();
        change_orient();
        draw_buttons();
        draw_value();
    }
    if (ctx.page_event) {
        draw_buttons();
        draw_value();
    }
    if (ctx.value_update) {
        draw_value();
    }
}

void screen_set_page(screen_page_e page) {
    screen_page_e prev_page = ctx.page;
    if (prev_page != page) {
        ctx.page = page;
        ctx.page_event = true;
    }
}

void screen_set_data(uint8_t temperature, uint8_t humidity) {
    snprintf(ctx.temperature, 21, "Temperature: %dC", temperature);
    snprintf(ctx.humidity, 21, "Humidity: %d%%", humidity);
    ctx.value_update = true;
}

void screen_set_orientation(screen_orient_e orient) {
    screen_orient_e prev_orient = ctx.orientation;

    if (prev_orient != orient) {
        ctx.orientation = orient;
        ctx.flip_event = true;
    }
}


/* --- Static functions definitions --- */
static void draw_button_a(void) {
    uint8_t button_page = ctx.orientation == SCREEN_UP ? 7 : 0;
    uint8_t button_offset = ctx.orientation == SCREEN_UP ? 64 : 0;

    oled_set_cursor(button_page, button_offset + 8);
    ctx.page == SCREEN_TEMPERATURE ? oled_puts_inv("Temp") : oled_puts("Temp");  //1 _inv
}

static void draw_button_b(void) {
    uint8_t button_page = ctx.orientation == SCREEN_UP ? 7 : 0;
    uint8_t button_offset = ctx.orientation == SCREEN_UP ? 0 : 64;

    oled_set_cursor(button_page, button_offset + 8);
    ctx.page == SCREEN_HUMIDITY ? oled_puts_inv("Humidity") : oled_puts("Humidity"); //1 _inv
}

static void draw_value(void) {
    uint8_t value_page = ctx.orientation == SCREEN_UP ? 3 : 4;

    oled_clear_page(value_page);
    oled_set_cursor(value_page, 10);
    if (ctx.page == SCREEN_TEMPERATURE) {
        oled_puts(ctx.temperature);
        ctx.value_update = false;
    } else if (ctx.page == SCREEN_HUMIDITY) {
        oled_puts(ctx.humidity);
        ctx.value_update = false;
    } else {
        return;
    }
}

static void draw_box(screen_page_e page) {
    uint8_t button_page = ctx.orientation == SCREEN_UP ? 7 : 0;
    uint8_t line_pos = ctx.orientation == SCREEN_UP ? 55 : 8;

    uint8_t offset;
    if (page == SCREEN_TEMPERATURE) {
        offset = ctx.orientation == SCREEN_UP ? 64 : 0;;
    } else if (page == SCREEN_HUMIDITY) {
        offset = ctx.orientation == SCREEN_UP ? 0 : 64;
    } else {
        return;
    }

    oled_draw_hline(line_pos, offset, 64);
    oled_set_cursor(button_page, offset);
    for(int i = 0; i < 64; i++) {
        oled_puthex(0xFF);
    }
}

static void draw_buttons(void) {
    if(ctx.orientation == SCREEN_UP) {
        oled_clear_page(6);
        oled_clear_page(7);
    } else if (ctx.orientation == SCREEN_DOWN) {
        oled_clear_page(0);
        oled_clear_page(1);
    }

    draw_box(ctx.page);
    draw_button_a();
    draw_button_b();
    ctx.page_event = false;
}

static void change_orient(void) {
    if(ctx.orientation == SCREEN_UP) {
        oled_set_orientation(OLED_NORMAL);
    } else if (ctx.orientation == SCREEN_DOWN) {
        oled_set_orientation(OLED_REVERT);
    } else {
        return;
    }
    ctx.flip_event = false;
}
