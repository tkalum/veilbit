#ifndef VEILBIT_H
#define VEILBIT_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Error codes
typedef enum {
    VEILBIT_OK = 0,
    VEILBIT_FILE_ERROR,
    VEILBIT_UNSUPPORTED_FORMAT,
    VEILBIT_INVALID_INPUT
} VeilBitStatus;

// Image formats
typedef enum {
    VEILBIT_BMP,
    VEILBIT_PNG,
    VEILBIT_UNKNOWN
} VeilBitFormat;

// Core functions
VeilBitStatus veilbit_hide(const char *input, const char *output, const char *message);
char* veilbit_extract(const char* input, const char* output);
// Format detection
VeilBitFormat veilbit_detect_format(const char *filename);

#endif // VEILBIT_H