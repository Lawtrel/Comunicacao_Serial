# Comunicação Serial com Raspberry Pi Pico

Este projeto implementa uma comunicação serial com o Raspberry Pi Pico, utilizando LEDs WS2812, um display SSD1306 e botões para interação.

## Estrutura do Projeto
. ├── .gitignore ├── .vscode/ │ ├── c_cpp_properties.json │ ├── cmake-kits.json │ ├── extensions.json │ ├── launch.json │ ├── settings.json │ └── tasks.json ├── blink.pio ├── build/ │ ├── .ninja_deps │ ├── .ninja_log │ ├── bs2_default.elf.map │ ├── build.ninja │ ├── cmake_install.cmake │ ├── CMakeCache.txt │ ├── CMakeFiles/ │ ├── compile_commands.json │ ├── Comunicacao_Serial.bin │ ├── Comunicacao_Serial.dis │ ├── Comunicacao_Serial.elf │ ├── Comunicacao_Serial.elf.map │ └── ... ├── CMakeLists.txt ├── Comunicacao_Serial.c ├── font.h ├── pico_sdk_import.cmake ├── ssd1306_i2c.c ├── ssd1306_i2c.h ├── ssd1306.h ├── ws2812.pio └── ws2818b.pio


## Funcionalidades

- Comunicação serial via UART.
- Controle de LEDs WS2812.
- Exibição de mensagens em um display SSD1306.
- Interação com botões para alterar o estado dos LEDs e exibir mensagens.

## Dependências

- Raspberry Pi Pico SDK
- Biblioteca para o display SSD1306
- Biblioteca para os LEDs WS2812

## Configuração

1. Clone o repositório.
2. Certifique-se de que o Raspberry Pi Pico SDK está configurado corretamente.
3. Compile o projeto utilizando CMake.

## Compilação e Execução

Para compilar o projeto, execute os seguintes comandos no terminal:

```sh
mkdir build
cd build
cmake ..
make

## Video
