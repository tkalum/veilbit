#include "veilbit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>

VeilBitStatus hide_jpeg(const char* input, const char* output, const char* msg) {
    struct jpeg_decompress_struct srcinfo;
    struct jpeg_compress_struct dstinfo;
    struct jpeg_error_mgr jsrcerr, jdsterr;
    jvirt_barray_ptr *src_coef_arrays;
    jvirt_barray_ptr *dst_coef_arrays;
    
    FILE* in = fopen(input, "rb");
    FILE* out = fopen(output, "wb");
    if (!in || !out) {
        if (in) fclose(in);
        if (out) fclose(out);
        return VEILBIT_FILE_ERROR;
    }
    
    // Setup decompression
    srcinfo.err = jpeg_std_error(&jsrcerr);
    jpeg_create_decompress(&srcinfo);
    jpeg_stdio_src(&srcinfo, in);
    jpeg_read_header(&srcinfo, TRUE);
    
    // Read DCT coefficients
    src_coef_arrays = jpeg_read_coefficients(&srcinfo);
    
    // Setup compression with same parameters
    dstinfo.err = jpeg_std_error(&jdsterr);
    jpeg_create_compress(&dstinfo);
    jpeg_stdio_dest(&dstinfo, out);
    jpeg_copy_critical_parameters(&srcinfo, &dstinfo);
    
    // Copy coefficient arrays
    dst_coef_arrays = src_coef_arrays;
    
    // Prepare message with length prefix
    int msg_len = strlen(msg);
    int total_bytes = msg_len + 4; // 4 bytes for length
    unsigned char* data = malloc(total_bytes);
    
    // Store length as 4 bytes (big-endian)
    data[0] = (msg_len >> 24) & 0xFF;
    data[1] = (msg_len >> 16) & 0xFF;
    data[2] = (msg_len >> 8) & 0xFF;
    data[3] = msg_len & 0xFF;
    
    // Copy message
    memcpy(data + 4, msg, msg_len);
    
    int total_bits = total_bytes * 8;
    int bit_index = 0;
    
    // Embed data in DCT coefficients
    for (int ci = 0; ci < srcinfo.num_components && bit_index < total_bits; ci++) {
        JBLOCKARRAY buffer;
        jpeg_component_info *compptr = srcinfo.comp_info + ci;
        
        for (JDIMENSION row = 0; row < compptr->height_in_blocks && bit_index < total_bits; row++) {
            buffer = (srcinfo.mem->access_virt_barray)
                ((j_common_ptr)&srcinfo, src_coef_arrays[ci], row, 1, TRUE);
            
            for (JDIMENSION col = 0; col < compptr->width_in_blocks && bit_index < total_bits; col++) {
                JCOEF *block = buffer[0][col];
                
                // Skip DC coefficient (k=0), modify AC coefficients
                for (int k = 1; k < 64 && bit_index < total_bits; k++) {
                    // Use coefficients that are not -1, 0, or 1
                    if (block[k] != 0 && block[k] != 1 && block[k] != -1) {
                        // Extract bit from data
                        int byte_index = bit_index / 8;
                        int bit_pos = 7 - (bit_index % 8);
                        int bit = (data[byte_index] >> bit_pos) & 1;
                        
                        // Modify LSB while preserving sign
                        if (block[k] > 0) {
                            block[k] = (block[k] & ~1) | bit;
                        } else {
                            block[k] = -(((-block[k]) & ~1) | bit);
                        }
                        bit_index++;
                    }
                }
            }
        }
    }
    
    free(data);
    
    if (bit_index < total_bits) {
        fprintf(stderr, "Warning: Could only embed %d of %d bits\n", bit_index, total_bits);
    }
    
    // Write modified coefficients
    jpeg_write_coefficients(&dstinfo, dst_coef_arrays);
    jpeg_finish_compress(&dstinfo);
    jpeg_finish_decompress(&srcinfo);
    
    jpeg_destroy_compress(&dstinfo);
    jpeg_destroy_decompress(&srcinfo);
    fclose(in);
    fclose(out);
    
    return VEILBIT_OK;
}

char* extract_jpeg(const char* input) {
    struct jpeg_decompress_struct srcinfo;
    struct jpeg_error_mgr jsrcerr;
    jvirt_barray_ptr *src_coef_arrays;
    
    FILE* file = fopen(input, "rb");
    if (!file) return NULL;
    
    srcinfo.err = jpeg_std_error(&jsrcerr);
    jpeg_create_decompress(&srcinfo);
    jpeg_stdio_src(&srcinfo, file);
    jpeg_read_header(&srcinfo, TRUE);
    
    // Read DCT coefficients
    src_coef_arrays = jpeg_read_coefficients(&srcinfo);
    
    // Buffer to store all extracted bits
    unsigned char* extracted_data = calloc(10000, 1);
    int bit_index = 0;
    
    // Extract all data bits sequentially
    for (int ci = 0; ci < srcinfo.num_components; ci++) {
        JBLOCKARRAY buffer;
        jpeg_component_info *compptr = srcinfo.comp_info + ci;
        
        for (JDIMENSION row = 0; row < compptr->height_in_blocks; row++) {
            buffer = (srcinfo.mem->access_virt_barray)
                ((j_common_ptr)&srcinfo, src_coef_arrays[ci], row, 1, FALSE);
            
            for (JDIMENSION col = 0; col < compptr->width_in_blocks; col++) {
                JCOEF *block = buffer[0][col];
                
                for (int k = 1; k < 64; k++) {
                    if (block[k] != 0 && block[k] != 1 && block[k] != -1) {
                        if (bit_index < 80000) { // 10000 bytes * 8 bits
                            // Extract LSB
                            int bit = abs(block[k]) & 1;
                            int byte_index = bit_index / 8;
                            int bit_pos = 7 - (bit_index % 8);
                            
                            extracted_data[byte_index] |= (bit << bit_pos);
                            bit_index++;
                        }
                    }
                }
            }
        }
    }
    
    // Now decode the length from first 4 bytes
    int msg_len = (extracted_data[0] << 24) | (extracted_data[1] << 16) | 
                  (extracted_data[2] << 8) | extracted_data[3];
    
    // Validate length
    if (msg_len <= 0 || msg_len > 10000) {
        fprintf(stderr, "Invalid message length: %d\n", msg_len);
        free(extracted_data);
        jpeg_finish_decompress(&srcinfo);
        jpeg_destroy_decompress(&srcinfo);
        fclose(file);
        char* empty = malloc(1);
        empty[0] = '\0';
        return empty;
    }
    
    // Allocate buffer for message
    char *message = calloc(msg_len + 1, 1);
    
    // Copy the message part (skip the 4-byte length prefix)
    memcpy(message, extracted_data + 4, msg_len);
    
    free(extracted_data);
    jpeg_finish_decompress(&srcinfo);
    jpeg_destroy_decompress(&srcinfo);
    fclose(file);
    
    return message;
}