# PicoDock Simulator (Wokwi)

Simulação da **PicoDock** utilizando a **Raspberry Pi Pico (RP2040)** na plataforma **Wokwi**.

Este repositório permite executar e validar o firmware utilizado na PicoDock em ambiente virtual, possibilitando testes de periféricos, validação de mapeamento de GPIOs e desenvolvimento embarcado sem necessidade de hardware físico.

---

## 📌 Objetivo

Reproduzir virtualmente os principais recursos da PicoDock:

- 3x Botões
- 1x LED RGB
- 1x Buzzer (sem oscilador interno)
- 1x Display OLED 128x32 (I2C)
~~ - 1x Multiplexador ADC 8x1 (CD4051) ~~
- 1x Interface para TFT LCD (SPI)
- 1x Interface Touch (Resistivo / Capacitivo)
~~- Alimentação externa (simulada)~~

---

## 🔌 Mapeamento de GPIOs

| Componente                                   | GPIO |
|----------------------------------------------|------|
| OLED - SDA                                   | 2    |
| OLED - SCK (SCL)                             | 3    |
| Botão B1                                     | 4    |
| Botão B2                                     | 5    |
| Botão B3                                     | 6    |
| LED RGB - Vermelho                           | 7    |
| LED RGB - Verde                              | 8    |
| LED RGB - Azul                               | 9    |
| Buzzer                                       | 10   |
~~| Sel_A_4051 (MUX)                             | 11   |~~
~~| Sel_B_4051 (MUX)                             | 12   |~~
~~| Sel_C_4051 (MUX)                             | 13   |~~
~~| ADC_IN_4051 (MUX)                            | 28   |~~
| LCD_TOUCH_X+ (Resistivo)                     | 14   |
| LCD_TOUCH_X- (Resistivo)                     | 26   |
| LCD_TOUCH_Y+ (Resistivo)                     | 27   |
| LCD_TOUCH_Y-/SDA (Resistivo / Capacitivo)    | 20   |
| LCD_TOUCH_SCL (Capacitivo)                   | 21   |
| LCD_RESET                                    | 16   |
| LCD_SPI_0_CS                                 | 17   |
| LCD_SPI_0_SCK                                | 18   |
| LCD_SPI_0_TX                                 | 19   |
| LCD_DATA_CMD_SEL                             | 22   |
| LCD_LITE                                     | 15   |

---
