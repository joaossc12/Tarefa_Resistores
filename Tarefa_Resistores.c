#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "Tarefa_Resistores.pio.h"
#include <math.h>
#include "./lib/matrix.h"


//---------------------------------------------------
// DEFINES
//---------------------------------------------------
#define I2C_PORT        i2c1
#define I2C_SDA         14
#define I2C_SCL         15
#define ENDERECO        0x3C
#define LED_RED         13
#define LED_GREEN        11
#define LED_MATRIX      7
#define ADC_PIN 28
#define MAX_ADC 4095
//---------------------------------------------------
// VARIÁVEIS GLOBAIS
//---------------------------------------------------

uint16_t adc_value;
float Resistor_Medido;
int Resistor_Comercial;
float R_ref = 9780;
char cores[3];
uint32_t cor_matriz[3];
float erro;
int N = 58;
int valores[] = {
    510, 560, 620, 680, 750, 820, 910,
    1000, 1100, 1200, 1300, 1500, 1600, 1800,
    2000, 2200, 2400, 2700, 3000, 3300, 3600, 3900,
    4300, 4700, 5100, 5600, 6200, 6800, 7500, 8200, 9100,
    10000, 11000, 12000, 13000, 15000, 16000, 18000,
    20000, 22000, 24000, 27000, 30000, 33000, 36000, 39000,
    43000, 47000, 51000, 56000, 62000, 68000, 75000, 82000, 91000,
    100000
};


// Display SSD1306
ssd1306_t ssd; // Display de 128 x 64

//---------------------------------------------------
// PROTOTIPAÇÃO
//---------------------------------------------------
int ValorComercias(float R);
void AtualizaDisplay();
void codigo();
void exibir_matriz(PIO pio, uint sm, uint32_t cor[3]);


// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
  reset_usb_boot(0, 0);
}

int main()
{
  // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    stdio_init_all();

    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);

    // Configura I2C para o display e inicia
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    i2c_init(I2C_PORT, 400000); // 0.4 MHz
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicializa o ADC e configura os pinos correspondentes
    adc_init();
    adc_gpio_init(ADC_PIN);

    // Configura o PIO para controlar a matriz de LEDs
    PIO pio = pio0;
    bool clk = set_sys_clock_khz(128000, false);
    uint offset = pio_add_program(pio, &Tarefa_Resistores_program);
    uint sm_pio = pio_claim_unused_sm(pio, true);
    Tarefa_Resistores_program_init(pio, sm_pio, offset, LED_MATRIX);

    while (true) {
        adc_select_input(2);
        float soma = 0.0f;
        for (int i = 0; i < 500; i++){
            soma += (float)adc_read();
            sleep_ms(1);}
        float media = soma / 500.0f;
        // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado /(ADC_RESOLUTION - adc_encontrado)
        Resistor_Medido = (float)(R_ref * media) / (MAX_ADC - media);
        Resistor_Comercial = ValorComercias(Resistor_Medido);
        AtualizaDisplay();
        erro = (float)abs(Resistor_Comercial - Resistor_Medido) / Resistor_Comercial * 100;
        printf("Erro %f \n", erro);
        codigo();
        exibir_matriz(pio, sm_pio, cor_matriz);
    }
}

int ValorComercias(float R){
    float erro_min = abs(R - valores[0]);
    int Resistor = valores[0];
    for(int i = 1; i < N; i++){
        float erro = abs(R - valores[i]);
        if (erro < erro_min){
            erro_min = erro;
            Resistor = valores[i];
        } 
    }
    return Resistor;

}
void AtualizaDisplay(){

    char string_medida[5];
    char string_comercial[5];
    char string_erro[5];
    sprintf(string_medida, "%1.0f", Resistor_Medido);
    sprintf(string_comercial, "%d", Resistor_Comercial);
    sprintf(string_erro, "%1.1f", erro);


    ssd1306_fill(&ssd, false);
    //Borda grossa
    ssd1306_rect(&ssd, 1, 1, 126, 62, 1, 1); // Desenha um retângulo
    ssd1306_rect(&ssd, 4, 4, 120, 56, 0, 1); // Desenha um retângulo
    ssd1306_draw_string(&ssd, "VALOR = ", 6, 7);
    ssd1306_draw_string(&ssd, string_comercial, 75, 7);
    ssd1306_draw_string(&ssd, "MEDIDA = ", 6, 22);
    ssd1306_draw_string(&ssd, string_medida, 75, 22);
    ssd1306_draw_string(&ssd, "ERRO(%)=", 6, 37);
    ssd1306_draw_string(&ssd, string_erro, 75, 37);
    ssd1306_draw_string(&ssd, "CORES = ", 6, 50);
    ssd1306_draw_string(&ssd, cores, 75, 50);
    ssd1306_send_data(&ssd);
}
void codigo(){
    int valores[3];
    int valor_R = Resistor_Comercial;

    while(valor_R >= 100){
        valor_R /=10;
    }
    valores[0] = valor_R / 10;
    valores[1] = valor_R % 10;
    valores[2] = Resistor_Comercial / valor_R;
    printf(" Valores: %d, %d, %d \n", valores[0],valores[1],valores[2]);
    
    int v_aux = 0;

    while(valores[2] >= 10){
        v_aux ++;
        valores[2] /=10;
    }
    valores[2] = v_aux;
    for (int i = 0; i < 3; i ++){
        int faixa = valores[i];
         switch (faixa)
         {
         case 0:
            cores[i] = 'P';//Preto
            cor_matriz[i] = PRETO;
            break;
        case 1:
            cores[i] = 'M';//Marron
            cor_matriz[i] = MARRON;
            break;
        case 2:
            cores[i] = 'V';//Vermelho
            cor_matriz[i] = VERMELHO;
            break;
        case 3:
            cores[i] = 'L';//Laranja
            cor_matriz[i] = LARANJA;
            break;
        case 4:
            cores[i] = 'Y';//Amarelo
            cor_matriz[i] = AMARELO;
            break;
        case 5:
            cores[i] = 'G';//Verde
            cor_matriz[i] = VERDE;
            break;
        case 6:
            cores[i] = 'A';//Azul
            cor_matriz[i] = AZUL;
            break;
        case 7:
            cores[i] = 'X';//Violeta
            cor_matriz[i] = ROXO;
            break;
        case 8:
            cores[i] = 'C';//Cinza
            cor_matriz[i] = CINZA;
            break;
        case 9:
            cores[i] = 'W'; //Branco
            cor_matriz[i] = BRANCO;
            break;
        case 10:
            cores[i] = 'J';
            break;
         default:
            printf("ERRO\n");
            break;
         }
    }
}
void exibir_matriz(PIO pio, uint sm, uint32_t cor[3]){
    uint32_t icone[25] = {
         cor[2],     0,  cor[1],    0,     cor[0],
         cor[0],     0,  cor[1],    0,     cor[2],
         cor[2],     0,  cor[1],    0,     cor[0],
         cor[0],     0,  cor[1],    0,     cor[2],
         cor[2],     0,  cor[1],    0,     cor[0]
    };
        for (int i = 0; i < 25; i++)
        {
            pio_sm_put_blocking(pio, sm, icone[i]);
        }
}
