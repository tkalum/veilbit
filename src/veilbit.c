#include "veilbit.h"
#include <stdio.h>
#include <string.h>

// Forward declarations
VeilBitStatus hide_bmp(const char *input, const char *output, const char *msg);
VeilBitStatus extract_bmp(const char *input, char *msg, size_t max_len);

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

VeilBitStatus veilbit_extract(const char *input, char *message, size_t max_len) {
    VeilBitFormat format = veilbit_detect_format(input);
    
    switch (format) {
        case VEILBIT_BMP:
            return extract_bmp(input, message, max_len);
        case VEILBIT_PNG:
            return VEILBIT_UNSUPPORTED_FORMAT;
        default:
            return VEILBIT_UNSUPPORTED_FORMAT;
    }
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
    // PNG detection would go here
    
    return VEILBIT_UNKNOWN;
}