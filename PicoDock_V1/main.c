// DEFINIR QUAL TOUCH USAR!!!!
//#define TOUCH_RESISTIVE

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"

// Display ILI9341
#include "ili9341.h"
#include "gfx.h"

//  Touch Resistivo
#include "touch_resistive.h"

//  Touch Capacitivo
#include "touch_capacitive.h"

// Display SSD1306 + Botões
#include "ssd1306.h"
#include "acme_5_outlines_font.h"
#include "bubblesstandard_font.h"
#include "crackers_font.h"
#include "BMSPA_font.h"

// === Definições para ILI9341 ===
const uint LITE = 15;
#define SCREEN_WIDTH 240

// === Definições para FT6206 ===
const int I2C_SCL_GPIO = 21;
const int I2C_SDA_GPIO = 20;

// === Definições para SSD1306 ===
ssd1306_t disp;
const uint8_t *fonts[4] = {acme_font, bubblesstandard_font, crackers_font, BMSPA_font};

const int BTN_PIN_R = 4;
const int BTN_PIN_G = 5;
const int BTN_PIN_B = 6;
const int LED_PIN_R = 7;
const int LED_PIN_G = 8;
const int LED_PIN_B = 9;
const int BUZZER_PIN = 10;

// === Definições para 4051 e ADC ===
const uint SEL_A_4051 = 13;
const uint SEL_B_4051 = 12;
const uint SEL_C_4051 = 11;

void polling_adc_init(void) {
    gpio_init(SEL_A_4051); gpio_set_dir(SEL_A_4051, GPIO_OUT);
    gpio_init(SEL_B_4051); gpio_set_dir(SEL_B_4051, GPIO_OUT);
    gpio_init(SEL_C_4051); gpio_set_dir(SEL_C_4051, GPIO_OUT);
}

void select_4051_channel(uint channel) {
    gpio_put(SEL_A_4051, channel & 0x01);
    gpio_put(SEL_B_4051, (channel >> 1) & 0x01);
    gpio_put(SEL_C_4051, (channel >> 2) & 0x01);
}

volatile int flag_BTN_R = 0;
volatile int flag_BTN_G = 0;
volatile int flag_BTN_B = 0;

void btn_callback(uint gpio, uint32_t events) {
    if (events & 0x4) {
        if (gpio == BTN_PIN_R) flag_BTN_R = 1;
        else if (gpio == BTN_PIN_G) flag_BTN_G = 1;
        else if (gpio == BTN_PIN_B) flag_BTN_B = 1;
    }

    if (events == 0x8) {
        if (gpio == BTN_PIN_R) flag_BTN_R = 0;
        else if (gpio == BTN_PIN_G) flag_BTN_G = 0;
        else if (gpio == BTN_PIN_B) flag_BTN_B = 0;
    }
}

void oled_init(void) {
    i2c_init(i2c1, 400000);
    gpio_set_function(2, GPIO_FUNC_I2C);
    gpio_set_function(3, GPIO_FUNC_I2C);
    gpio_pull_up(2); gpio_pull_up(3);

    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, i2c1);
    ssd1306_clear(&disp); ssd1306_show(&disp);
}

int main(void) {
    stdio_init_all();

    // Inicialização do display LCD + Touch
    LCD_initDisplay();
    LCD_setRotation(0);
    GFX_createFramebuf();

#ifdef TOUCH_RESISTIVE
    configure_touch(); // Inicializa touch resistivo
#else
    touch_init_i2c(I2C_SDA_GPIO, I2C_SCL_GPIO, DEFAULT_I2C_PORT);
    if (!touch_capacitive_begin(128)) {
        printf("Falha ao inicializar o sensor de toque FT6206.\n");
        while (1) tight_loop_contents();
    }
    printf("Sensor de toque iniciado com sucesso!\n");
#endif

    gpio_init(LITE); gpio_set_dir(LITE, GPIO_OUT); gpio_put(LITE, 0);

    oled_init();

    // Inicialização dos botões e LEDs
    gpio_init(BTN_PIN_R); gpio_set_dir(BTN_PIN_R, GPIO_IN); gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_init(BTN_PIN_G); gpio_set_dir(BTN_PIN_G, GPIO_IN); gpio_pull_up(BTN_PIN_G);
    gpio_set_irq_enabled(BTN_PIN_G, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_init(BTN_PIN_B); gpio_set_dir(BTN_PIN_B, GPIO_IN); gpio_pull_up(BTN_PIN_B);
    gpio_set_irq_enabled(BTN_PIN_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

    gpio_init(LED_PIN_R); gpio_set_dir(LED_PIN_R, GPIO_OUT); gpio_put(LED_PIN_R, 1);
    gpio_init(LED_PIN_G); gpio_set_dir(LED_PIN_G, GPIO_OUT); gpio_put(LED_PIN_G, 1);
    gpio_init(LED_PIN_B); gpio_set_dir(LED_PIN_B, GPIO_OUT); gpio_put(LED_PIN_B, 1);

    gpio_init(BUZZER_PIN); gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    // Inicialização ADC e 4051
    polling_adc_init();
    adc_init();
    adc_gpio_init(28); // GPIO28 = ADC2
    adc_select_input(2);

    int px, py;

    while (true) {
        // === Parte LCD Touch ===
        GFX_clearScreen();
        GFX_setCursor(0, 10);
        GFX_printf("Touch Demo\n");

#ifdef TOUCH_RESISTIVE
        if (readPoint(&px, &py)) {
            gpio_put(LITE, 1);
            px = SCREEN_WIDTH - px;
            GFX_printf("X:%03d Y:%03d\n", px, py);
            gpio_put(BUZZER_PIN, 1); sleep_us(3000);
            gpio_put(BUZZER_PIN, 0); sleep_us(3000);
        } else {
            gpio_put(LITE, 0);
            GFX_printf("Sem toque\n");
        }
#else
        TS_Point points[2];
        uint8_t n = touch_capacitive_getPoints(points);
        if (n > 0) {
            gpio_put(LITE, 1);
            int adjustedX = 240 - points[0].x;
            GFX_printf("X:%03d Y:%03d\n", adjustedX, points[0].y);
            gpio_put(BUZZER_PIN, 1); sleep_us(3000);
            gpio_put(BUZZER_PIN, 0); sleep_us(3000);
        } else {
            gpio_put(LITE, 0);
            GFX_printf("Sem toque\n");
        }
        //sleep_ms(1000);
#endif


         // === Leitura dos canais do 4051 no LCD ===
        for (uint channel = 0; channel < 8; channel++) {
            select_4051_channel(channel);
            sleep_ms(5); // estabilização
            uint16_t result = adc_read();
            GFX_setCursor(0, 40 + channel * 10);
            GFX_printf("Canal %d: %4d\n", channel, result);
        }

        sleep_ms(50);
        GFX_flush();

        // === Parte OLED Botões ===
        if (flag_BTN_R == 1) {
            gpio_put(LED_PIN_R, 0);
            ssd1306_draw_string(&disp, 8, 24, 2, "RED");
            ssd1306_show(&disp);
            while (flag_BTN_R == 1) {
                gpio_put(BUZZER_PIN, 1); sleep_us(100);
                gpio_put(BUZZER_PIN, 0); sleep_us(100);
            }
            ssd1306_clear(&disp); ssd1306_show(&disp);
            gpio_put(LED_PIN_R, 1);
        }

        if (flag_BTN_G == 1) {
            gpio_put(LED_PIN_G, 0);
            ssd1306_draw_string(&disp, 8, 24, 2, "GREEN");
            ssd1306_show(&disp);
            while (flag_BTN_G == 1) {
                gpio_put(BUZZER_PIN, 1); sleep_us(750);
                gpio_put(BUZZER_PIN, 0); sleep_us(750);
            }
            ssd1306_clear(&disp); ssd1306_show(&disp);
            gpio_put(LED_PIN_G, 1);
        }

        if (flag_BTN_B == 1) {
            gpio_put(LED_PIN_B, 0);
            ssd1306_draw_string(&disp, 8, 24, 2, "BLUE");
            ssd1306_show(&disp);
            while (flag_BTN_B == 1) {
                gpio_put(BUZZER_PIN, 1); sleep_us(1500);
                gpio_put(BUZZER_PIN, 0); sleep_us(1500);
            }
            ssd1306_clear(&disp); ssd1306_show(&disp);
            gpio_put(LED_PIN_B, 1);
        }

       
        sleep_ms(1);
    }
}
