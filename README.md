# Tarefa sobre Conversor Analógico Digital
 
Este projeto utiliza um joystick analógico para controlar a intensidade luminosa de LEDs RGB e a movimentação de um elemento visual em um display SSD1306. Os LEDs são controlados via PWM para permitir uma transição suave de brilho. Além disso, o botão do joystick e um botão adicional são utilizados para alternar estados dos LEDs e modificar a aparência do display.

## Funcionalidades

1. Controle da intensidade dos LEDs RGB com o joystick.
2. Movimento de um quadrado no display SSD1306 baseado nos valores do joystick.
3. Alternância do LED Verde ao pressionar o botão do joystick.
4. Modificação da borda do display ao pressionar o botão do joystick.
5. Liga/desliga dos LEDs PWM através do botão A.

## Como Usar

1. Conecte o joystick e os LEDs RGB à placa microcontroladora.
2. Utilize o eixo Y do joystick para ajustar a intensidade do LED Azul:
   - Posição central: LED apagado.
   - Movimento para cima ou para baixo: aumenta a intensidade do LED.
3. Utilize o eixo X do joystick para ajustar a intensidade do LED Vermelho:
   - Posição central: LED apagado.
   - Movimento para a esquerda ou direita: aumenta a intensidade do LED.
4. Observe o quadrado de 8x8 pixels no display SSD1306, que se moverá proporcionalmente aos valores do joystick.
5. Utilize o botão do joystick para:
   - Alternar o estado do LED Verde (ligar/desligar).
   - Modificar a borda do display a cada pressão.
6. Utilize o botão A para ativar ou desativar os LEDs PWM.

## Vídeo de funcionamento
[DRIVE]()

## AUTOR
### Matheus Nepomuceno Souza
