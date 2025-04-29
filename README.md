# Tarefa Resistores
# Leitor de Resistores com Raspberry Pi Pico

Este projeto implementa um sistema de medição de resistores com Interface Humano–Máquina (IHM) utilizando Raspberry Pi Pico, display OLED SSD1306, matriz de LEDs e LED RGB. A seguir, uma visão geral, instruções de uso, descrição das funções e legendas de cores.

---

## 📋 Visão Geral do Projeto

- **Leitura** da resistência por ADC  
- **Cálculo** do valor comercial (série E24)  
- **Cálculo** do percentual de erro  
- **Exibição** no display OLED:  
  - Valor medido  
  - Valor comercial mais próximo  
  - Percentual de erro  
  - Código de cores dos três anéis  
- **Matriz de LEDs**: visualiza as faixas de cores  

---

## 🛠️ Hardware Necessário

- Raspberry Pi Pico  
- Display OLED SSD1306 (I²C, 128×64)  
- Matriz de LEDs (5×5) controlada por PIO  
- LED RGB externo  
- Resistores de referência, fios e jumpers  

---

## 🔌 Conexões

| Sinal          | Pico GPIO | Periférico        |
| -------------- | --------- | ----------------- |
| I2C SDA        | GP14      | OLED SDA          |
| I2C SCL        | GP15      | OLED SCL          |
| ADC de entrada | GP28      | Divider de teste  |
| LED Red        | GP13      | LED RGB – canal R |
| LED Green      | GP11      | LED RGB – canal G |
| LED Matrix     | GP7       | Data da PIO       |
| Botão BOOTSEL  | GP6       | Pulldown BOOT     |

---

## ⚙️ Software e Bibliotecas

- **SDK Pico C/C++**  
- `hardware/adc.h`, `hardware/i2c.h`, `hardware/clocks.h`  
- `lib/ssd1306.h`, `lib/font.h`  
- `lib/matrix.h`, `Tarefa_Resistores.pio.h`  
- `<math.h>`  

---

## 🚀 Como Compilar e Gravar

1. Clone o repositório e entre na pasta do projeto.  
2. Configure o ambiente Pico SDK.  
3. Compile com CMake:  
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```
4. Com o Pico em modo BOOTSEL (segure o botão GP6 ao conectar via USB), copie o arquivo `.uf2` gerado para gravar o firmware.

---

## 🔍 Descrição das Funções

### `int ValorComercias(float R);`
Percorre o vetor de valores E24 (`valores[]`) e retorna o resistor comercial mais próximo de `R`.

### `void AtualizaDisplay(void);`
Atualiza o OLED com:
- **VALOR** (comercial)
- **MEDIDA** (calculada)
- **ERRO (%)**
- **CORES** (letras)

### `void codigo(void);`
Calcula os dois primeiros dígitos significativos e o expoente (quantas casas decimais).  
Gera o vetor `cores[3]` com letras que representam cada faixa.  
Configura `cor_matriz[3]` para a matriz de LEDs.

### `void exibir_matriz(PIO pio, uint sm, uint32_t cor[3]);`
Desenha um ícone 5×5 na matriz de LEDs alternando as três cores fornecidas em `cor[3]`.

---

## 📊 Tabela de Cores no OLED

Cada letra exibida no campo **CORES** do OLED representa uma cor:

| Letra | Cor      |
|-------|----------|
| P     | Preto    |
| M     | Marrom   |
| V     | Vermelho |
| L     | Laranja  |
| Y     | Amarelo  |
| G     | Verde    |
| A     | Azul     |
| X     | Violeta  |
| C     | Cinza    |
| W     | Branco   |
| J     | Overflow |


