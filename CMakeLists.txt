# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# Definição do projeto
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configuração do ambiente do Raspberry Pi Pico
set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(Comunicacao_Serial C CXX ASM)

pico_sdk_init()

# Adiciona os arquivos do projeto
add_executable(Comunicacao_Serial 
    Comunicacao_Serial.c 
    ssd1306_i2c.c
)

# Define nome e versão do programa
pico_set_program_name(Comunicacao_Serial "Comunicacao_Serial")
pico_set_program_version(Comunicacao_Serial "0.1")
pico_generate_pio_header(Comunicacao_Serial ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)


# Configura a comunicação serial (habilita UART e desabilita USB)
pico_enable_stdio_uart(Comunicacao_Serial 1)
pico_enable_stdio_usb(Comunicacao_Serial 0)

# Inclui bibliotecas padrão
target_link_libraries(Comunicacao_Serial 
    pico_stdlib 
    hardware_pio
    hardware_i2c
)

# Adiciona diretórios de inclusão
target_include_directories(Comunicacao_Serial PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
)

# Gera os binários finais
pico_add_extra_outputs(Comunicacao_Serial)
