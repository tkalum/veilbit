#include "veilbit.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Forward declarations
VeilBitStatus hide_bmp(const char *input, const char *output, const char *msg);
char* extract_bmp(const char *input);

VeilBitStatus hide_jpeg(const char *input, const char *output, const char *msg);
char* extract_jpeg(const char *input);

VeilBitStatus hide_png(const char* input, const char* output, const char* msg);
char* extract_png(const char* input);


char*veilbit_read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = malloc(size + 1);
    if (!buffer) return NULL;
    if (fread(buffer, 1, size, file) != size) {
        free(buffer);
        return NULL;
    }
    buffer[size] = '\0';
    fclose(file);
    return buffer;}

VeilBitStatus veilbit_hide(const char *input, const char *output, const char *message) {
    VeilBitFormat format = veilbit_detect_format(input);
    
    switch (format) {
        case VEILBIT_BMP:
            return hide_bmp(input, output, message);
        case VEILBIT_JPEG:
            return hide_jpeg(input, output, message);
        case VEILBIT_PNG:
            return hide_png(input, output, message);
        default:
            return VEILBIT_UNSUPPORTED_FORMAT;
    }
}

char* veilbit_extract(const char* input, const char* output) {
    VeilBitFormat format = veilbit_detect_format(input);
    char* message = NULL;

    switch (format) {
        case VEILBIT_BMP:
            message = extract_bmp(input); 
            break;
        case VEILBIT_JPEG:
            message = extract_jpeg(input);
            break;
        case VEILBIT_PNG:
            message = extract_png(input);
            break;
        default:
            return NULL;
    }

    if (message && output) {
        FILE* out = fopen(output, "w");
        if (out) {
            fprintf(out, "%s", message);
            fclose(out);
        } else {
            free(message);
            return NULL;
        }
    }

    return message;
}

VeilBitFormat veilbit_detect_format(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) return VEILBIT_UNKNOWN;
    
    uint8_t header[8];
    fread(header, 1, 8, file);
    fclose(file);
    
    // BMP detection
    if (header[0] == 'B' && header[1] == 'M') {
        return VEILBIT_BMP;
    }
    // JPEG detection
    if (header[0] == 0xFF && header[1] == 0xD8 && header[2] == 0xFF) {
        return VEILBIT_JPEG;
    }
    // PNG detection
    if (header[0] == 0x89 && header[1] == 'P' && header[2] == 'N' && header[3] == 'G' &&
        header[4] == 0x0D && header[5] == 0x0A && header[6] == 0x1A && header[7] == 0x0A) {
        return VEILBIT_PNG;
    }
 
    return VEILBIT_UNKNOWN;
}