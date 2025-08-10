#include "veilbit.h"
#include <stdio.h>
#include <string.h>

void print_help() {
    printf("VeilBit - Hide messages in images\n");
    printf("Usage:\n");
    printf("  veilbit hide -i input.bmp -o output.bmp -m \"message\"\n");
    printf("  veilbit extract -i image.bmp\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    if (strcmp(argv[1], "hide") == 0) {
        char *input = NULL, *output = NULL, *message = NULL;
        
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-i") == 0) input = argv[++i];
            else if (strcmp(argv[i], "-o") == 0) output = argv[++i];
            else if (strcmp(argv[i], "-m") == 0) message = argv[++i];
            else if (input == NULL) input = argv[i];
            else if (message == NULL) message = argv[i];
            else if (output == NULL) output = argv[i];  
        }

        if (!input || !output || !message) {
            print_help();
            return 1;
        }

        if (veilbit_hide(input, output, message)) {
            fprintf(stderr, "Error hiding message\n");
            return 1;
        }
        printf("Message hidden in %s\n", output);
    }
    else if (strcmp(argv[1], "extract") == 0) {
        char *input = NULL;
        char message[1024] = {0};

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-i") == 0) input = argv[++i];
        }

        if (!input) {
            print_help();
            return 1;
        }

        if (veilbit_extract(input, message, sizeof(message))) {
            fprintf(stderr, "Error extracting message\n");
            return 1;
        }
        printf("Extracted: %s\n", message);
    }
    else {
        print_help();
        return 1;
    }

    return 0;
}