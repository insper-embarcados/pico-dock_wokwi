#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "touch_capacitive.h"

#define TOUCH_BUF_SIZE 16

typedef struct {
    uint16_t x[2];
    uint16_t y[2];
    uint8_t id[2];
    uint8_t count;
} touch_data_t;

static i2c_inst_t *touch_i2c_port = DEFAULT_I2C_PORT;
static touch_data_t current_touch_data;

// -----------------------------------------------------------------------------
// I2C Communication - Read and Write Register
// -----------------------------------------------------------------------------

// Leitura de registrador com verificação de falha
uint8_t touch_read_register(uint8_t reg) {
    uint8_t data = 0;
    if (i2c_write_blocking(touch_i2c_port, FT6206_ADDRESS, &reg, 1, true) != 1) {
        printf("Erro ao acessar registrador 0x%02X (i2c_write_blocking falhou)\n", reg);
        return 0;
    }
    if (i2c_read_blocking(touch_i2c_port, FT6206_ADDRESS, &data, 1, false) != 1) {
        printf("Erro ao ler registrador 0x%02X (i2c_read_blocking falhou)\n", reg);
        return 0;
    }
    return data;
}

// Escrita de registrador com verificação de falha
void touch_write_register(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    if (i2c_write_blocking(touch_i2c_port, FT6206_ADDRESS, buf, 2, false) != 2) {
        printf("Erro ao escrever 0x%02X no registrador 0x%02X\n", value, reg);
    }
}

// -----------------------------------------------------------------------------
// Inicialização
// -----------------------------------------------------------------------------

void touch_init_i2c(uint sda_pin, uint scl_pin, i2c_inst_t *i2c_port) {
    if (i2c_port) touch_i2c_port = i2c_port;
    i2c_init(touch_i2c_port, 100 * 1000);  // Configura a taxa de 100kHz para o I2C
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);  // Habilita o pull-up nos pinos I2C
    gpio_pull_up(scl_pin);
}

bool touch_capacitive_begin(uint8_t thresh) {
    uint8_t vend_id = touch_read_register(FT62XX_REG_VENDID);
    if (vend_id != FT62XX_VENDID) {
        printf("ID de fornecedor inválido: 0x%02X\n", vend_id);
        return false;
    }

    uint8_t chip_id = touch_read_register(FT62XX_REG_CHIPID);
    if (chip_id != FT6206_CHIPID && chip_id != FT6236_CHIPID &&
        chip_id != FT6236U_CHIPID && chip_id != FT6336U_CHIPID) {
        printf("ID do chip inválido: 0x%02X\n", chip_id);
        return false;
    }

    if (thresh != 0) {
        touch_write_register(FT62XX_REG_THRESHHOLD, thresh);
    }

    return true;
}

// -----------------------------------------------------------------------------
// Leitura de Dados
// -----------------------------------------------------------------------------

// Leitura otimizada do dado de toque
static bool read_touch_data(uint8_t *buf, size_t len) {
    uint8_t reg = 0x00;
    if (i2c_write_blocking(touch_i2c_port, FT6206_ADDRESS, &reg, 1, true) != 1) {
        printf("Erro ao iniciar leitura de dados de toque\n");
        return false;
    }

    if (i2c_read_blocking(touch_i2c_port, FT6206_ADDRESS, buf, len, false) != len) {
        printf("Erro ao ler dados de toque (tamanho de leitura: %d)\n", len);
        return false;
    }
    return true;
}

// Parsea os dados de toque
static void parse_touch_data(uint8_t *buf, touch_data_t *data) {
    data->count = buf[2] & 0x0F;
    if (data->count == 0 || data->count > 2) {
        data->count = 0;
        return;
    }

    for (uint8_t i = 0; i < data->count; i++) {
        data->x[i] = ((buf[3 + 6 * i] & 0x0F) << 8) | buf[4 + 6 * i];
        data->y[i] = ((buf[5 + 6 * i] & 0x0F) << 8) | buf[6 + 6 * i];
        data->id[i] = buf[5 + 6 * i] >> 4;
    }
}

void touch_read_data(void) {
    uint8_t buf[TOUCH_BUF_SIZE] = {0};
    if (read_touch_data(buf, TOUCH_BUF_SIZE)) {
        parse_touch_data(buf, &current_touch_data);
    } else {
        current_touch_data.count = 0;
    }
}

// -----------------------------------------------------------------------------
// API de Leitura de Toques
// -----------------------------------------------------------------------------

// Retorna o número de toques
uint8_t touch_capacitive_getPoints(TS_Point *points) {
    touch_read_data();  // Atualiza os dados de toque

    for (uint8_t i = 0; i < current_touch_data.count; i++) {
        points[i].x = current_touch_data.x[i];
        points[i].y = current_touch_data.y[i];
        points[i].id = current_touch_data.id[i];
    }

    return current_touch_data.count;
}

TS_Point touch_capacitive_getPoint(uint8_t index) {
    TS_Point p = {0, 0, 0};
    if (index >= current_touch_data.count) return p;  // Se índice inválido, retorna ponto nulo
    p.x = current_touch_data.x[index];
    p.y = current_touch_data.y[index];
    p.id = current_touch_data.id[index];
    return p;
}
