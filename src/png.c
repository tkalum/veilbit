#include "veilbit.h"
#include <png.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

VeilBitStatus hide_png(const char* input, const char* output, const char* msg) {
    FILE *fp_in = fopen(input, "rb");
    if (!fp_in) return VEILBIT_FILE_ERROR;
    
    // Check PNG signature
    unsigned char header[8];
    fread(header, 1, 8, fp_in);
    if (png_sig_cmp(header, 0, 8)) {
        fclose(fp_in);
        return VEILBIT_FILE_ERROR;
    }
    
    // Create read struct
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp_in);
        return VEILBIT_FILE_ERROR;
    }
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp_in);
        return VEILBIT_FILE_ERROR;
    }
    
    // Error handling
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp_in);
        return VEILBIT_FILE_ERROR;
    }
    
    // Initialize IO and read info
    png_init_io(png_ptr, fp_in);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    
    // Get image info
    png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    
    // Update info based on transformations
    png_read_update_info(png_ptr, info_ptr);
    
    // Allocate memory for image data
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*)malloc(rowbytes);
    }
    
    // Read image data
    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, NULL);
    
    // Clean up read structures
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp_in);
    
    // Now hide the message
    int msg_len = strlen(msg);
    int total_bits = (msg_len + 4) * 8; // 4 bytes for length
    int bit_counter = 0;
    
    // Determine bytes per pixel based on color type
    int bytes_per_pixel = 0;
    switch (color_type) {
        case PNG_COLOR_TYPE_GRAY:       bytes_per_pixel = 1; break;
        case PNG_COLOR_TYPE_GRAY_ALPHA: bytes_per_pixel = 2; break;
        case PNG_COLOR_TYPE_RGB:        bytes_per_pixel = 3; break;
        case PNG_COLOR_TYPE_RGB_ALPHA:  bytes_per_pixel = 4; break;
    }
    
    // If 16-bit depth, double the bytes per pixel
    if (bit_depth == 16) {
        bytes_per_pixel *= 2;
    }
    
    // Hide data - iterate through pixels properly
    for (int y = 0; y < height && bit_counter < total_bits; y++) {
        png_byte* row = row_pointers[y];
        
        for (int x = 0; x < width && bit_counter < total_bits; x++) {
            // Process each byte of the pixel
            for (int b = 0; b < bytes_per_pixel && bit_counter < total_bits; b++) {
                int byte_pos = x * bytes_per_pixel + b;
                
                uint8_t bit_to_hide;
                if (bit_counter < 32) {
                    // Hide message length
                    bit_to_hide = (msg_len >> (31 - bit_counter)) & 1;
                } else {
                    // Hide message data
                    int msg_bit = bit_counter - 32;
                    int byte_index = msg_bit / 8;
                    int bit_index = 7 - (msg_bit % 8);
                    bit_to_hide = (msg[byte_index] >> bit_index) & 1;
                }
                
                // Modify LSB
                row[byte_pos] = (row[byte_pos] & 0xFE) | bit_to_hide;
                bit_counter++;
            }
        }
    }
    
    // Write output file
    FILE *fp_out = fopen(output, "wb");
    if (!fp_out) {
        for (int y = 0; y < height; y++) free(row_pointers[y]);
        free(row_pointers);
        return VEILBIT_FILE_ERROR;
    }
    
    png_structp png_ptr_write = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr_write) {
        fclose(fp_out);
        for (int y = 0; y < height; y++) free(row_pointers[y]);
        free(row_pointers);
        return VEILBIT_FILE_ERROR;
    }
    
    png_infop info_ptr_write = png_create_info_struct(png_ptr_write);
    if (!info_ptr_write) {
        png_destroy_write_struct(&png_ptr_write, NULL);
        fclose(fp_out);
        for (int y = 0; y < height; y++) free(row_pointers[y]);
        free(row_pointers);
        return VEILBIT_FILE_ERROR;
    }
    
    if (setjmp(png_jmpbuf(png_ptr_write))) {
        png_destroy_write_struct(&png_ptr_write, &info_ptr_write);
        fclose(fp_out);
        for (int y = 0; y < height; y++) free(row_pointers[y]);
        free(row_pointers);
        return VEILBIT_FILE_ERROR;
    }
    
    png_init_io(png_ptr_write, fp_out);
    
    // Set image info
    png_set_IHDR(png_ptr_write, info_ptr_write, width, height,
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    
    png_write_info(png_ptr_write, info_ptr_write);
    png_write_image(png_ptr_write, row_pointers);
    png_write_end(png_ptr_write, NULL);
    
    // Clean up
    png_destroy_write_struct(&png_ptr_write, &info_ptr_write);
    fclose(fp_out);
    
    for (int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);
    
    return VEILBIT_OK;
}

char* extract_png(const char* input) {
    FILE *fp = fopen(input, "rb");
    if (!fp) return NULL;
    
    // Check PNG signature
    unsigned char header[8];
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)) {
        fclose(fp);
        return NULL;
    }
    
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp);
        return NULL;
    }
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return NULL;
    }
    
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }
    
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    
    png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    
    png_read_update_info(png_ptr, info_ptr);
    
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*)malloc(rowbytes);
    }
    
    png_read_image(png_ptr, row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    
    // Determine bytes per pixel
    int bytes_per_pixel = 0;
    switch (color_type) {
        case PNG_COLOR_TYPE_GRAY:       bytes_per_pixel = 1; break;
        case PNG_COLOR_TYPE_GRAY_ALPHA: bytes_per_pixel = 2; break;
        case PNG_COLOR_TYPE_RGB:        bytes_per_pixel = 3; break;
        case PNG_COLOR_TYPE_RGB_ALPHA:  bytes_per_pixel = 4; break;
    }
    
    if (bit_depth == 16) {
        bytes_per_pixel *= 2;
    }
    
    int msg_len = 0;
    int bit_counter = 0;
    
    // Extract length (first 32 bits)
    for (int y = 0; y < height && bit_counter < 32; y++) {
        png_byte* row = row_pointers[y];
        
        for (int x = 0; x < width && bit_counter < 32; x++) {
            for (int b = 0; b < bytes_per_pixel && bit_counter < 32; b++) {
                int byte_pos = x * bytes_per_pixel + b;
                int bit = row[byte_pos] & 1;
                msg_len |= (bit << (31 - bit_counter));
                bit_counter++;
            }
        }
    }
    
    // Validate length
    if (msg_len <= 0 || msg_len > 10000) {
        for (int y = 0; y < height; y++) free(row_pointers[y]);
        free(row_pointers);
        return NULL;
    }
    
    // Extract message
    char* message = (char*)calloc(msg_len + 1, 1);
    int msg_bit_counter = 0;
    
    // Continue from beginning but skip first 32 bits
    bit_counter = 0;
    
    for (int y = 0; y < height && msg_bit_counter < msg_len * 8; y++) {
        png_byte* row = row_pointers[y];
        
        for (int x = 0; x < width && msg_bit_counter < msg_len * 8; x++) {
            for (int b = 0; b < bytes_per_pixel && msg_bit_counter < msg_len * 8; b++) {
                int byte_pos = x * bytes_per_pixel + b;
                
                if (bit_counter >= 32) {  // Skip length bits
                    int bit = row[byte_pos] & 1;
                    int byte_index = msg_bit_counter / 8;
                    int bit_index = 7 - (msg_bit_counter % 8);
                    
                    if (bit) {
                        message[byte_index] |= (1 << bit_index);
                    }
                    
                    msg_bit_counter++;
                }
                bit_counter++;
            }
        }
    }
    
    message[msg_len] = '\0';
    
    // Clean up
    for (int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);
    
    return message;
}