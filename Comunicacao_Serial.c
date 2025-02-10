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
uint8_t ssd[ssd1306_buffer_length];

typedef struct {
        uint8_t G, R, B;
    } led_matrix;
led_matrix matrix[NUM_LED];

struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
};
void init_matrix(uint pin);
void setLedMatrix(uint index, uint8_t R, uint8_t G, uint8_t B);
void clearMatrix();
void renderMatrix();
void set_matrix(char digito);
void init_display();
void update_display(const char *message);
void check_buttons();

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
        init_matrix(WS2812_PIN);
        clearMatrix();
        renderMatrix();

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
            
                    if (cc >= '0' && cc <= '9' || cc == 'c') {
                        set_matrix(cc);  // Atualiza a matriz WS2812 com base no número recebido
                    }
                }
                check_buttons();  // Atualiza os LEDs dos botões
                
            }            
        return 0;
        
}
//inicializar led matrix
void init_matrix(uint pin) {
        uint offset  = pio_add_program(pio0, &ws2812_program);
        sm = pio_claim_unused_sm(pio0, true);
        if (sm < 0) {
                sm = pio_claim_unused_sm(pio1, true);
        }
        ws2812_program_init(pio0, sm, offset, pin, 800000.f);
        clearMatrix();
    }
// seta as cores do led da matrix 
void setLedMatrix(uint index, uint8_t R, uint8_t G, uint8_t B) {
        if (index < NUM_LED) {
                matrix[index].R = R;
                matrix[index].G = G;
                matrix[index].B = B;
        }

    }
//limpar todos led da matrix
void clearMatrix() {
        for (int i = 0; i < NUM_LED; i++) {
            setLedMatrix(i, 0, 0, 0);
        }
    }
//enviar dados para a matrix
void renderMatrix() {
        for (int i = 0; i < NUM_LED; i++) {
                pio_sm_put_blocking(pio, sm, matrix[i].G);
                pio_sm_put_blocking(pio, sm, matrix[i].R);
                pio_sm_put_blocking(pio, sm, matrix[i].B);
        }
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

    void set_matrix(char digito) {
        const uint32_t numeros[10] = {
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
        if (digito == 'c') {
                clearMatrix();
                renderMatrix();
                printf("Matriz de LEDs apagada\n");
                return;
            }
        int index = digito - '0';
        if (index < 0 || index > 9) return;
        clearMatrix();
        setLedMatrix(index, 0, 255, 0);
        renderMatrix();        
        // Depuração: Exibir no Serial Monitor para ver se o índice está correto
        printf("Número recebido: %c, Índice convertido: %d\n", digito, index);
    }