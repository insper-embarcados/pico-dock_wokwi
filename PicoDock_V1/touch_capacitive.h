#ifndef TOUCH_CAPACITIVE_H
#define TOUCH_CAPACITIVE_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

// -----------------------------------------------------------------------------
// Endereço I2C e constantes do FT6206/FT6236
// -----------------------------------------------------------------------------

#define FT6206_ADDRESS         (0x38)

// IDs esperados
#define FT62XX_VENDID          0x11
#define FT6206_CHIPID          0x06
#define FT6236_CHIPID          0x36
#define FT6236U_CHIPID         0x64
#define FT6336U_CHIPID         0x71

// Registradores
#define FT62XX_REG_VENDID      0xA8
#define FT62XX_REG_CHIPID      0xA3
#define FT62XX_REG_NUMTOUCHES  0x02
#define FT62XX_REG_MODE        0x00
#define FT62XX_REG_CALIBRATE   0x02
#define FT62XX_REG_WORKMODE    0x00
#define FT62XX_REG_FACTORYMODE 0x40
#define FT62XX_REG_THRESHHOLD  0x80
#define FT62XX_REG_FIRMVERS    0xA6

// Número máximo de toques suportados
#define FT6206_MAX_TOUCHES     2

// I2C default (pode ser alterado com touch_init_i2c)
#ifndef DEFAULT_I2C_PORT
#define DEFAULT_I2C_PORT i2c0
#endif

// -----------------------------------------------------------------------------
// Estrutura para representar um ponto de toque
// -----------------------------------------------------------------------------

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t id;
} TS_Point;

// -----------------------------------------------------------------------------
// Inicialização
// -----------------------------------------------------------------------------

void touch_init_i2c(uint sda_pin, uint scl_pin, i2c_inst_t *i2c_port);
bool touch_capacitive_begin(uint8_t thresh);

// -----------------------------------------------------------------------------
// Leitura de dados de toque
// -----------------------------------------------------------------------------

/// Retorna a quantidade de toques atuais (0, 1 ou 2)
uint8_t touch_num_touches(void);

/// Retorna true se houver ao menos um toque
uint8_t touch_touched(void);

/// Lê os dados atuais e preenche o vetor de pontos
uint8_t touch_capacitive_getPoints(TS_Point *points);

/// Retorna o ponto do índice especificado (0 ou 1)
TS_Point touch_capacitive_getPoint(uint8_t index);

/// Atualiza os dados internos de toque
void touch_read_data(void);

/// Acesso individual a ponto, retorna true se índice válido
bool touch_get_point(uint8_t index, uint16_t *x, uint16_t *y, uint8_t *id);

// -----------------------------------------------------------------------------
// Acesso a registradores internos do chip
// -----------------------------------------------------------------------------

uint8_t touch_read_register(uint8_t reg);
void touch_write_register(uint8_t reg, uint8_t value);
uint8_t touch_vendor_id(void);
uint8_t touch_firmware_version(void);
void touch_dump_registers(void);

#endif // TOUCH_CAPACITIVE_H
