#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define JOYSTICK_X_PIN 26 // GPIO para eixo X
#define JOYSTICK_Y_PIN 27 // GPIO para eixo Y
#define JOYSTICK_PB 22    // GPIO para botão do Joystick

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

int main()
{
    stdio_init_all();

    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB);

    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    uint16_t adc_value_x;
    uint16_t adc_value_y;

    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    bool cor = true;

    while (true)
    {
        // Leitura do Joystick
        adc_select_input(0); // Eixo X (pino 26)
        adc_value_x = adc_read();
        adc_select_input(1); // Eixo Y (pino 27)
        adc_value_y = adc_read();

        // Mapeia os valores do ADC (0-4095) para a tela (3-120 para X e 3-56 para Y)
        // Mapeia os valores do ADC (0-4095) para a tela, garantindo limites
        int x = (adc_value_x * (128 - 8)) / 2000;
        int y = (adc_value_y * (64 - 8)) / 2034;

        ssd1306_fill(&ssd, !cor);                                        // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);                    // Desenha o retângulo principal
        ssd1306_rect(&ssd, 2, 2, 124, 62, !gpio_get(JOYSTICK_PB), !cor); // Segunda camada para engrossar a borda

        // Desenha o quadrado de 8x8 pixels no display na posição calculada
        ssd1306_rect(&ssd, x, y, 8, 8, cor, cor);

        ssd1306_send_data(&ssd); // Atualiza o display

        sleep_ms(50);
    }
}
