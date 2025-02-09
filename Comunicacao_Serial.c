#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "ssd1306.h"
#include "ws2812.pio.h"

#define WS2812_PIN 7
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
int current_message = 0;

void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin, float freq);
void set_matrix(char digito);
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

const uint32_t patterns[10] = {
        0b011101000110011011100,  // 0
        0b001001011000001011100,  // 1
        0b011101000011100011110,  // 2
        0b011101000011100100011,  // 3
        0b000110011111000010000,  // 4
        0b011111000011110100011,  // 5
        0b011111000111110100011,  // 6
        0b011100000110001000010,  // 7
        0b011111000111110111111,  // 8
        0b011111000111110100011   // 9
    };


void button_callback(uint gpio, uint32_t events) {
        static uint32_t last_time = 0;
        uint32_t now = to_ms_since_boot(get_absolute_time());

        if (now - last_time <200) return; //debounce
        last_time = now;
        if (gpio == BUTTON_PIN) {
                static bool green_led_state = false;
                green_led_state = !green_led_state;
                gpio_put(GREEN_PIN, green_led_state);
                update_display(green_led_state ? "Led Verde ON" : "Led Verde OFF");
        }
        if (gpio == BUTTON_PIN2) {
                static bool blue_led_state = false;
                blue_led_state = !blue_led_state;
                gpio_put(BLUE_PIN, blue_led_state);
                update_display(blue_led_state ? "Led Azul ON" : "Led Azul OFF");
        }
}

int main() {
        stdio_init_all();
        uart_init(uart0, 115200); // configurar porta serial
        gpio_set_function(0, GPIO_FUNC_UART);
        gpio_set_function(1, GPIO_FUNC_UART);
         // Configuração de interrupção nos botões
        gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);
        gpio_set_irq_enabled_with_callback(BUTTON_PIN2, GPIO_IRQ_EDGE_FALL, true, &button_callback);

        //inicializar matrix de leds
        uint offset = pio_add_program(pio, &ws2812_program);
        ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000);
        //inicializar leds RGB
        gpio_init(RED_PIN);
        gpio_set_dir(RED_PIN, GPIO_OUT);
        gpio_init(GREEN_PIN);
        gpio_set_dir(GREEN_PIN, GPIO_OUT);
        gpio_init(BLUE_PIN);
        gpio_set_dir(BLUE_PIN, GPIO_OUT);
        //inicializar botoes
        gpio_init(BUTTON_PIN);
        gpio_set_dir(BUTTON_PIN, GPIO_IN);
        gpio_pull_up(BUTTON_PIN);
        gpio_init(BUTTON_PIN2);
        gpio_set_dir(BUTTON_PIN2, GPIO_IN);
        gpio_pull_up(BUTTON_PIN2);

        init_display();
        while (true) {
                if (uart_is_readable(uart0)) {
                    char cc;
                    scanf("%c", &cc);
                    printf("Recebi: %c\n", cc);
                    
                    char buffer[20];
                    sprintf(buffer, "Caracter: %c", cc);
                    update_display(buffer);
            
                    if (cc >= '0' && cc <= '9') {
                        set_matrix(cc);  // Atualiza a matriz WS2812 com base no número recebido
                    }
                }
                check_buttons();  // Atualiza os LEDs dos botões
                
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
    
        // Configuração do programa
        pio_sm_config c = ws2812_program_get_default_config(offset);
        sm_config_set_sideset_pins(&c, pin); // Usa o pino sideset
        sm_config_set_out_shift(&c, true, true, 24); // 24 bits (3 bytes) por LED
        sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX); // Usa apenas o FIFO TX
    
        // Ajuste a frequência do clock
        float div = (float)clock_get_hz(clk_sys) / freq;
        sm_config_set_clkdiv(&c, div);
    
        // Inicializa a state machine
        pio_sm_init(pio, sm, offset, &c);
        pio_sm_set_enabled(pio, sm, true);
    }    
    void set_matrix(char digito) {
        int index = digito - '0';
        // Enviar os dados para a WS2812
        for (int i = 0; i < NUM_LED; i++) {
                uint32_t color = (patterns[index] & (1 << i)) ? 0x00FF00 : 0x000000; // Verde ou apagado
                pio_sm_put_blocking(pio, sm, color << 8);
        }
    
        // Depuração: Exibir no Serial Monitor para ver se o índice está correto
        printf("Número recebido: %c, Índice convertido: %d\n", digito, index);
    }