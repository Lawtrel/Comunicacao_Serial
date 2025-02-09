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
void update_display(const char *message);
void check_buttons();
uint8_t ssd[ssd1306_buffer_length];
struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
};

int current_message = 0;

int main() {
        stdio_init_all();
        uint offset = pio_add_program(pio, &ws2812_program);
        ws2812_program_init(pio, sm, offset, MATRIX_PIN, 800000);

        gpio_init(RED_PIN);
        gpio_set_dir(RED_PIN, GPIO_OUT);
        gpio_init(GREEN_PIN);
        gpio_set_dir(GREEN_PIN, GPIO_OUT);
        gpio_init(BLUE_PIN);
        gpio_set_dir(BLUE_PIN, GPIO_OUT);
        
        gpio_init(BUTTON_PIN);
        gpio_set_dir(BUTTON_PIN, GPIO_IN);
        gpio_pull_up(BUTTON_PIN);
        gpio_init(BUTTON_PIN2);
        gpio_set_dir(BUTTON_PIN2, GPIO_IN);
        gpio_pull_up(BUTTON_PIN2);

        init_display();

        while (true)
        {
                check_buttons();
                sleep_ms(100);
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
        calculate_render_area_buffer_length(&frame_area);
        memset(ssd, 0, ssd1306_buffer_length);
        render_on_display(ssd, &frame_area);
        
        // Mensagem inicial
        update_display("Ola, mundo");

}

void update_display(const char *message) {
        memset(ssd, 0, ssd1306_buffer_length);
        int max_char_por_linha = ssd1306_width / 9;
        char linha[20] = {0};
        char linha2[20] = {0};

        if (strlen(message) > max_char_por_linha) {
                strncpy(linha, message, max_char_por_linha);
                linha[max_char_por_linha] = '\0';
                strncpy(linha2, message + max_char_por_linha, max_char_por_linha);
                linha2[max_char_por_linha] = '\0';
        } else {
                strcpy(linha, message);
        }
        ssd1306_draw_string(ssd, 5, 10, linha);
        ssd1306_draw_string(ssd, 5, 20, linha2);
        render_on_display(ssd, &frame_area);
}

void check_buttons() {
        bool button1_pressed = !gpio_get(BUTTON_PIN);
        bool button2_pressed = !gpio_get(BUTTON_PIN2);
    
        if (button1_pressed) {
                gpio_put(RED_PIN, 1);
                gpio_put(GREEN_PIN, 0);
                gpio_put(BLUE_PIN, 0);
                current_message = 1;
                update_display("Botao A pressionado!");
        }
    
        else if (button2_pressed) {
                gpio_put(RED_PIN, 0);
                gpio_put(GREEN_PIN, 1);
                gpio_put(BLUE_PIN, 0);
                current_message = 2;
                update_display("Botao B pressionado!");
        } else {
           gpio_put(RED_PIN, 0);
           gpio_put(GREEN_PIN, 0);
           gpio_put(BLUE_PIN, 1);
        }
}

void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin, float freq) {
        pio_gpio_init(pio, pin);
        pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);

}