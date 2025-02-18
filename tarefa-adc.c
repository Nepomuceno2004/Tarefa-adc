#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define joystick_X 26      // GPIO para eixo X
#define joystick_Y 27      // GPIO para eixo Y
#define joystick_Button 22 // GPIO para botão do Joystick

#define LED_RED 13   // LED vermelho (PWM)
#define LED_BLUE 12  // LED azul (PWM)
#define LED_GREEN 11 // LED verde (acionado pelo botão do joystick)

#define button_A 5 // BotÃ£o para ativar/desativar LEDs PWM

#define pwm_wrap 4095

volatile bool pwm_enabled = true; // Estado dos LEDs PWM
volatile uint32_t last_time = 0;

uint pwm_init_gpio(uint gpio, uint wrap);
uint16_t get_center(uint8_t adc_channel);
void gpio_irq_handler(uint gpio, uint32_t events);

int main()
{
    stdio_init_all();

    gpio_init(button_A);
    gpio_set_dir(button_A, GPIO_IN);
    gpio_pull_up(button_A);

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, 0);

    gpio_init(joystick_Button);
    gpio_set_dir(joystick_Button, GPIO_IN);
    gpio_pull_up(joystick_Button);

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

    adc_init();
    adc_gpio_init(joystick_X);
    adc_gpio_init(joystick_Y);

    uint16_t adc_value_x;
    uint16_t adc_value_y;
    uint16_t center_x = get_center(0);
    uint16_t center_y = get_center(1);

    pwm_init_gpio(LED_RED, pwm_wrap);
    pwm_init_gpio(LED_BLUE, pwm_wrap);

    gpio_set_irq_enabled_with_callback(joystick_Button, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    uint32_t last_print_time = 0;
    bool cor = true;
    int borda = 0;
    volatile uint32_t last_press = 0;

    while (true)
    {
        // Leitura do Joystick
        adc_select_input(0); // Eixo X (pino 26)
        adc_value_x = adc_read();
        adc_select_input(1); // Eixo Y (pino 27)
        adc_value_y = adc_read();

        // Mapeamento do ADC para a tela, garantindo limites
        int y = (adc_value_y * 120) / 4095; // Mapeia o eixo X para 120
        y += 3;                             // Ajusta para começar a partir de 3

        if (y > 120)
            y = 120; // Limitação para evitar ultrapassar a borda

        int x = 56 - (adc_value_x * 56) / 4095; // Inverte o eixo X
        x += 3;

        if (x > 56)
            x = 56;
        if (x < 3)
            x = 3;

        if (pwm_enabled)
        {
            int16_t deviation_x = adc_value_x - center_x;
            int16_t deviation_y = adc_value_y - center_y;

            uint16_t pwm_value_red = (abs(deviation_y) > 50) ? abs(deviation_y) * 2 : 0;
            if (pwm_value_red > pwm_wrap)
                pwm_value_red = pwm_wrap;
            pwm_set_gpio_level(LED_RED, pwm_value_red);

            uint16_t pwm_value_blue = (abs(deviation_x) > 50) ? abs(deviation_x) * 2 : 0;
            if (pwm_value_blue > pwm_wrap)
                pwm_value_blue = pwm_wrap;
            pwm_set_gpio_level(LED_BLUE, pwm_value_blue);
        }

        ssd1306_fill(&ssd, !cor); // Limpa o display

        // Desenha a borda de acordo com o estado atual
        if (!gpio_get(LED_GREEN))
        {
            ssd1306_rect(&ssd, 2, 2, 124, 62, cor, !cor); // Borda mais espessa
        }
        else
        {
            // Borda pontilhada
            for (int i = 3; i < 123; i += 4)
            {
                ssd1306_pixel(&ssd, i, 3, cor);
                ssd1306_pixel(&ssd, i, 60, cor);
            }
            for (int i = 3; i < 60; i += 4)
            {
                ssd1306_pixel(&ssd, 3, i, cor);
                ssd1306_pixel(&ssd, 122, i, cor);
            }
        }

        ssd1306_rect(&ssd, x, y, 8, 8, cor, cor); // Desenha o quadrado na posição corrigida
        ssd1306_send_data(&ssd);                  // Atualiza o display

        sleep_ms(100);
    }
}

uint pwm_init_gpio(uint gpio, uint wrap)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);
    return slice_num;
}

uint16_t get_center(uint8_t adc_channel)
{
    uint32_t sum = 0;
    for (int i = 0; i < 100; i++)
    {
        adc_select_input(adc_channel);
        sum += adc_read();
        sleep_ms(5);
    }
    return sum / 100;
}

void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time > 20000)
    { // Debounce

        if (gpio == joystick_Button)
        {
            gpio_put(LED_GREEN, !gpio_get(LED_GREEN));
            printf("LED Verde: %s\n", gpio_get(LED_GREEN) ? "ON" : "OFF");
        }
        else if (gpio == button_A)
        {
            pwm_enabled = !pwm_enabled;
            if (!pwm_enabled)
            {
                pwm_set_gpio_level(LED_RED, 0);
                pwm_set_gpio_level(LED_BLUE, 0);
            }
            printf("PWM: %s\n", pwm_enabled ? "ON" : "OFF");
        }

        last_time = current_time;
    }
}