#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "ws2812.pio.h"

#define MATRIX_PIN 7
#define NUM_LED 25
#define RED_PIN 13
#define GREEN_PIN 11
#define BLUE_PIN 12
#define BUTTON_PIN 5
#define BUTTON_PIN2 6

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;
PIO pio = pio0;
uint sm = 0;
void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin, float freq);
void init_display();

int main() {
        stdio_init_all();
        uint offset = pio_add_program(pio, &ws2812_program);
        ws2812_program_init(pio, sm, offset, MATRIX_PIN, 800000);
        init_display();
        while (true)
        {
                sleep_ms(1000);
        }
        return 0;
        
}

void init_display() {
        i2c_init(i2c1, ssd1306_i2c_clock * 1000);
        gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
        gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
        gpio_pull_up(I2C_SDA);
        gpio_pull_up(I2C_SCL);

        ssd1306_init();
        struct render_area frame_area = {
                start_column : 0,
                end_column : ssd1306_width - 1,
                start_page : 0,
                end_page : ssd1306_n_pages - 1
        };
        calculate_render_area_buffer_length(&frame_area);
        uint8_t ssd[ssd1306_buffer_length];
        memset(ssd, 0, ssd1306_buffer_length);
        render_on_display(ssd, &frame_area);

        restart:
        char *text[] = {"Hello", "World", "Pico", "Raspberry", "Pi"};
        int y = 0;
        for (uint i = 0; i < count_of(text); i++) {
                ssd1306_draw_string(ssd, 5, y, text[i]);
                y += 8;
        }
        render_on_display(ssd, &frame_area);
}

void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin, float freq) {
        pio_gpio_init(pio, pin);
        pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);

}