#include "veilbit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BMP_HEADER_SIZE 54

static void hide_byte(uint8_t secret, FILE *in, FILE *out) {
    for (int i = 0; i < 8; i++) {
        uint8_t pixel;
        fread(&pixel, 1, 1, in);
        pixel = (pixel & 0xFE) | ((secret >> (7 - i)) & 1);
        fwrite(&pixel, 1, 1, out);
    }
}

VeilBitStatus hide_bmp(const char *input, const char *output, const char *msg) {
    FILE *in = fopen(input, "rb");
    FILE *out = fopen(output, "wb");
    if (!in || !out) return VEILBIT_FILE_ERROR;

    // Copy header
    uint8_t header[BMP_HEADER_SIZE];
    fread(header, 1, BMP_HEADER_SIZE, in);
    fwrite(header, 1, BMP_HEADER_SIZE, out);

    // Hide message length (4 bytes)
    size_t len = strlen(msg);
    for (int i = 0; i < 4; i++) {
        hide_byte((len >> (8 * (3 - i))) & 0xFF, in, out);
    }

    // Hide message
    for (size_t i = 0; i < len; i++) {
        hide_byte(msg[i], in, out);
    }

    // Copy remaining data
    uint8_t byte;
    while (fread(&byte, 1, 1, in)) {
        fwrite(&byte, 1, 1, out);
    }

    fclose(in);
    fclose(out);
    return VEILBIT_OK;
}

static uint8_t extract_byte(FILE *file) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        uint8_t pixel;
        fread(&pixel, 1, 1, file);
        byte = (byte << 1) | (pixel & 1);
    }
    return byte;
}

VeilBitStatus extract_bmp(const char *input, char *message, size_t max_len) {
    FILE *file = fopen(input, "rb");
    if (!file) return VEILBIT_FILE_ERROR;

    // Skip header
    fseek(file, BMP_HEADER_SIZE, SEEK_SET);

    // Extract message length
    size_t len = 0;
    for (int i = 0; i < 4; i++) {
        len = (len << 8) | extract_byte(file);
    }

    // Check if message fits in buffer
    if (len >= max_len) {
        fclose(file);
        return VEILBIT_INVALID_INPUT;
    }

    // Extract message
    for (size_t i = 0; i < len; i++) {
        message[i] = extract_byte(file);
    }
    message[len] = '\0';

    fclose(file);
    return VEILBIT_OK;
}