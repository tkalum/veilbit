#include "veilbit.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Forward declarations
VeilBitStatus hide_bmp(const char *input, const char *output, const char *msg);
char* extract_bmp(const char *input);

VeilBitStatus veilbit_hide(const char *input, const char *output, const char *message) {
    VeilBitFormat format = veilbit_detect_format(input);
    
    switch (format) {
        case VEILBIT_BMP:
            return hide_bmp(input, output, message);
        case VEILBIT_PNG:
            return VEILBIT_UNSUPPORTED_FORMAT;
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
 
    return VEILBIT_UNKNOWN;
}