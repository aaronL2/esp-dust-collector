#pragma once
#include <stdint.h>

#define MODE_NUMERIC 1
#define MODE_ALPHANUMERIC 2
#define MODE_BYTE 4

typedef struct {
    uint8_t version;
    uint8_t size;
    uint8_t ecc;      // <- add this
    uint8_t mode;     // <- add this
    uint8_t mask;     // <- add this
    uint8_t *modules;
    uint8_t *is_function;
} QRCode;

int8_t qrcode_initBytes(QRCode *qrcode, uint8_t *modules, uint8_t version, uint8_t ecc, uint8_t *data, uint16_t length);
int8_t qrcode_initText(QRCode *qrcode, uint8_t *modules, uint8_t version, uint8_t ecc, const char *data);
bool qrcode_getModule(QRCode *qrcode, uint8_t x, uint8_t y);
uint16_t qrcode_getBufferSize(uint8_t version);
