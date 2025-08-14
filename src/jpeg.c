#include "veilbit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JPEG_HEADER_SIZE 500

// Helper function to hide one byte in 8 pixels
static void hide_byte(uint8_t secret, FILE* in, FILE* out) {
    for (int i = 0; i < 8; i++) {
        uint8_t pixel;
        if (fread(&pixel, 1, 1, in) != 1) return;
        pixel = (pixel & 0xFE) | ((secret >> (7 - i)) & 1);
        fwrite(&pixel, 1, 1, out);
    }
}

// Helper function to extract one byte from 8 pixels
static uint8_t extract_byte(FILE* file) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        uint8_t pixel;
        if (fread(&pixel, 1, 1, file) != 1) return 0;
        byte = (byte << 1) | (pixel & 1);
    }
    return byte;
}

// Hide message in jpeg file
VeilBitStatus hide_jpeg(const char* input, const char* output, const char* msg) {
    FILE* in = fopen(input, "rb");
    FILE* out = fopen(output, "wb");
    if (!in || !out) return VEILBIT_FILE_ERROR;

    // 1. Copy jpeg header
    uint8_t header[JPEG_HEADER_SIZE];
    if (fread(header, 1, JPEG_HEADER_SIZE, in) != JPEG_HEADER_SIZE) {
        fclose(in); fclose(out);
        return VEILBIT_FILE_ERROR;
    }
    fwrite(header, 1, JPEG_HEADER_SIZE, out);

    // 2. Hide message length (4 bytes)
    size_t len = strlen(msg);
    for (int i = 0; i < 4; i++) {
        hide_byte((len >> (8 * (3 - i))) & 0xFF, in, out);
    }

    // 3. Hide message content
    for (size_t i = 0; i < len; i++) {
        hide_byte(msg[i], in, out);
    }

    // 4. Copy remaining pixels
    uint8_t byte;
    while (fread(&byte, 1, 1, in) == 1) {
        fwrite(&byte, 1, 1, out);
    }

    fclose(in);
    fclose(out);
    return VEILBIT_OK;
}

// Extract message from jpeg file
char* extract_jpeg(const char* input) {
    FILE* file = fopen(input, "rb");
    if (!file) return NULL;

    // 1. Skip jpeg header
    if (fseek(file, JPEG_HEADER_SIZE, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    // 2. Extract message length (4 bytes)
    size_t len = 0;
    for (int i = 0; i < 4; i++) {
        uint8_t byte = extract_byte(file);
        if (feof(file)) {
            fclose(file);
            return NULL;
        }
        len = (len << 8) | byte;
    }

    // 3. Validate reasonable length
    if (len > 1024*1024) {  // 1MB max
        fclose(file);
        return NULL;
    }

    // 4. Allocate and extract message
    char* message = malloc(len + 1);
    if (!message) {
        fclose(file);
        return NULL;
    }

    for (size_t i = 0; i < len; i++) {
        message[i] = extract_byte(file);
        if (feof(file)) {
            free(message);
            fclose(file);
            return NULL;
        }
    }
    message[len] = '\0';

    fclose(file);
    return message;
}