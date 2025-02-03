#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"

// Incluir cabeçalho do programa PIO
#include "tarefaInterrupt.pio.h"

// Variáveis globais para armazenar PIO e máquina de estado
static PIO pio = pio0;
static uint sm;

// Constantes de configuração
#define LED_MATRIX_SIZE 25
#define BUTTON_DEBOUNCE_MS 75
#define CLOCK_FREQUENCY 100000

// Definições de pinos
typedef struct {
    uint red;
    uint green;
    uint blue;
    uint buttonA;
    uint buttonB;
    uint matrixOutput;
} PinConfiguration;

// Representações de números na matriz de LEDs
typedef struct {
    double pattern[LED_MATRIX_SIZE];
} NumberDisplay;

// Estado da aplicação
typedef struct {
    int currentValue;
    uint32_t lastInterruptTime;
} AppState;

// Configuração global
static PinConfiguration pins = {
    .red = 13,
    .green = 11,
    .blue = 12,
    .buttonA = 5,
    .buttonB = 6,
    .matrixOutput = 7
};

// Padrões de números predefinidos
static const NumberDisplay numberPatterns[10] = {
    { // desenho do número 0
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // desenho do número 1
        0, 0, 1, 0, 0,
        0, 0, 1, 1, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 1, 1, 1, 0
    },
    { // desenho do número 2
        0, 0, 1, 0, 0,
        0, 1, 0, 1, 0,
        0, 0, 1, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // desenho do número 3
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0
    },
    { // desenho do número 4
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0
    },
    { // desenho do número 5
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0
    },
    { // desenho do número 6
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // desenho do número 7
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0
    },
    { // desenho do número 8
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // desenho do número 9
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0
    }
};

// Protótipos de funções
void inicializarHardware(PinConfiguration* config);
void configurarRelogioSistema(void);
void configurarPIOParaMatrizLED(PIO* pio, uint* offset, uint* stateMachine);
uint32_t converterCorRGB(double brilho, double vermelho, double verde);
void exibirNumeroNaMatriz(int numero, PIO pio, uint stateMachine);
void manipuladorInterrupcaoBotao(uint gpio, uint32_t eventos);

// Inicializar pinos GPIO
void inicializarHardware(PinConfiguration* config) {
    gpio_init(config->red);
    gpio_set_dir(config->red, GPIO_OUT);
    gpio_put(config->red, 0);

    gpio_init(config->green);
    gpio_set_dir(config->green, GPIO_OUT);
    gpio_put(config->green, 0);

    gpio_init(config->blue);
    gpio_set_dir(config->blue, GPIO_OUT);
    gpio_put(config->blue, 0);

    gpio_init(config->buttonA);
    gpio_set_dir(config->buttonA, GPIO_IN);
    gpio_pull_up(config->buttonA);

    gpio_init(config->buttonB);
    gpio_set_dir(config->buttonB, GPIO_IN);
    gpio_pull_up(config->buttonB);
}

// Configuração do relógio do sistema
void configurarRelogioSistema(void) {
    if (!set_sys_clock_khz(CLOCK_FREQUENCY, false)) {
        printf("Falha na configuração do relógio\n");
    }
}

// Configuração do PIO para comunicação com a matriz de LEDs
void configurarPIOParaMatrizLED(PIO* pio, uint* offset, uint* stateMachine) {
    *pio = pio0;
    *offset = pio_add_program(*pio, &tarefaInterrupt_program);
    *stateMachine = pio_claim_unused_sm(*pio, true);
    tarefaInterrupt_program_init(*pio, *stateMachine, *offset, pins.matrixOutput);
}

// Conversão de cor RGB
uint32_t converterCorRGB(double brilho, double vermelho, double verde) {
    unsigned char R = vermelho * 255;
    unsigned char G = verde * 255;
    unsigned char B = brilho * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

// Exibir número na matriz de LEDs
void exibirNumeroNaMatriz(int numero, PIO pio, uint stateMachine) {
    uint32_t ledValue;
    if (numero >= 0 && numero <= 9) {
        for (int i = 0; i < LED_MATRIX_SIZE; i++) {
            ledValue = converterCorRGB(0.0, 0.1 * numberPatterns[numero].pattern[LED_MATRIX_SIZE - 1 - i], 0.0);
            pio_sm_put_blocking(pio, stateMachine, ledValue);
        }
    }
}

// Manipulador de interrupção do botão
void manipuladorInterrupcaoBotao(uint gpio, uint32_t eventos) {
    static AppState appState = {0, 0};
    uint32_t currentTime = to_us_since_boot(get_absolute_time());

    if (currentTime - appState.lastInterruptTime > BUTTON_DEBOUNCE_MS * 1000) {
        appState.lastInterruptTime = currentTime;

        if (gpio == pins.buttonA && appState.currentValue < 9) {
            appState.currentValue++;
            exibirNumeroNaMatriz(appState.currentValue, pio0, sm);
            printf("Contador: %d\n", appState.currentValue);
        }
        else if (gpio == pins.buttonB && appState.currentValue > 0) {
            appState.currentValue--;
            exibirNumeroNaMatriz(appState.currentValue, pio0, sm);
            printf("Contador: %d\n", appState.currentValue);
        }
    }
}

int main() {
    uint offset;

    stdio_init_all();
    inicializarHardware(&pins);
    configurarRelogioSistema();
    configurarPIOParaMatrizLED(&pio, &offset, &sm);

    printf("Sistema inicializado. Aguardando entrada...\n");

    // Configurar interrupções dos botões
    gpio_set_irq_enabled_with_callback(pins.buttonA, GPIO_IRQ_EDGE_FALL, true, &manipuladorInterrupcaoBotao);
    gpio_set_irq_enabled_with_callback(pins.buttonB, GPIO_IRQ_EDGE_FALL, true, &manipuladorInterrupcaoBotao);

    // Exibição inicial do zero
    exibirNumeroNaMatriz(0, pio, sm);

    // Indicador de inicialização
    gpio_put(pins.red, 1);
    sleep_ms(300);
    gpio_put(pins.red, 0);

    while (true) {
        // Rotina de piscar LED
        for (int i = 0; i < 5; i++) {
            gpio_put(pins.red, 1);
            sleep_ms(100);
            gpio_put(pins.red, 0);
            sleep_ms(100);
        }
    }

    return 0;
}
