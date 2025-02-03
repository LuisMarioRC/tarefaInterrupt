# BitDogLab - Controle de LEDs com Interrupções


# Controle de LEDs WS2812 e RGB com Botões - RP2040

## Tarefa: Interrupções e Controle de LEDs

### Descrição

Este repositório contém o código-fonte de um projeto desenvolvido em Linguagem C, utilizando o Pico SDK, para controlar LEDs WS2812 e um LED RGB conectados a um microcontrolador Raspberry Pi Pico (BitDogLab). O projeto implementa funcionalidades utilizando interrupções (IRQs) e debouncing via software, explorando conceitos de hardware e software de maneira integrada.

A solução implementa estratégias para:  
1. Controle de LEDs RGB com diferentes efeitos.  
2. Incremento e decremento de números exibidos na matriz 5x5 de LEDs WS2812.  
3. Tratamento do bouncing de botões com debouncing via software.

## Funcionalidades

1. **Controle do LED RGB**:
   - Pisca o LED vermelho continuamente 5 vezes por segundo.

2. **Controle da Matriz de LEDs WS2812**:
   - Exibe números de 0 a 9 em formato fixo, com estilo digital ou criativo.  
   - **Botão A**: Incrementa o número exibido na matriz.  
   - **Botão B**: Decrementa o número exibido na matriz.  

3. **Debouncing via Software**:
   - Tratamento do bouncing para garantir que cada botão responda de forma precisa.

4. **Interrupções**:
   - As funcionalidades dos botões foram implementadas utilizando rotinas de interrupção (IRQs).

## Pré-requisitos

1. Ambiente de Desenvolvimento VS Code.  
2. Pico SDK instalado.  
3. Compilador de C, como GCC.  
4. Ferramentas de gravação e execução na placa BitDogLab.

## Como Usar

1. Clone o repositório:

    ```bash
    git clone https://github.com/LuisMarioRC/tarefaInterrupt.git
    ```

2. Navegue até o diretório do projeto:

    ```bash
    cd tarefaInterrupt
    ```

3. Compile o código com o compilador de C:

    ```bash
    gcc -o main tarefaInterrupt.c
    ```

4. Grave e execute o programa na placa BitDogLab seguindo os procedimentos padrões.

## Vídeo

Confira a demonstração prática no link: [Link do vídeo](o).
