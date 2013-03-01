/* 
 * File:   main.c
 * Author: Jake
 *
 * Created on November 20, 2012, 6:42 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <sol/runtime.h>
#include <solc/solc.h>
#include "solgen.h"

char* file_strip_path(char* file);
char* file_get_name(char* file);
char* file_modify_extension(char* file, char* ext);

/*
 * 
 */
int main(int argc, char** argv) {
    // parse command-line flags
    char* filename;
    bool flag_b, flag_c;
    for (int i = 1; i < argc; i++) {
        char* arg = argv[i];
        if (*arg == '-') {
            if (arg[1] == '-') {
                fprintf(stderr, "Unrecognized flag %s.\n", arg);
            } else {
                while (*++arg != '\0') {
                    switch (*arg) {
                        case 'b':
                            flag_b = true;
                            break;
                        case 'c':
                            flag_c = true;
                            break;
                        default:
                            fprintf(stderr, "Unrecognized flag -%c.\n", *arg);
                            break;
                    }
                }
            }
        } else {
            filename = arg;
        }
    }
    
    // handle invalid input
    if (argc == 0 || !filename) {
        printf("usage:  solc [-b|-c] filename\n");
        return EXIT_FAILURE;
    }
    if (flag_b && flag_c) {
        fprintf(stderr, "Invalid flag combination: -c and -b.\n");
        return EXIT_FAILURE;
    }
    
    // handle sol runtime information
    sol_runtime_init();
    
    // begin compilation
    FILE* in = fopen(filename, "r");
    if (in == NULL) {
        fprintf(stderr, "File '%s' could not be read.\n", filename);
        return EXIT_FAILURE;
    }
    
    // get and write the binary file
    off_t bin_size;
    unsigned char* bin = solc_compile_f(in, &bin_size);
    fclose(in);
    if (!flag_c) {
        char* bin_out_name = file_modify_extension(file_strip_path(filename), "solbin");
        FILE* bin_out = fopen(bin_out_name, "wb");
        fwrite(bin, bin_size, 1, bin_out);
        fclose(bin_out);
        free(bin_out_name);
    }
    
    // write C source file
    if (!flag_b) {
        char* out_name = file_modify_extension(file_strip_path(filename), "c");
        FILE* out = flag_b ? NULL : fopen(out_name, "w");
        solc_generate_c(bin, bin_size, out);
        fclose(out);
        free(out_name);
    }
    
    sol_runtime_destroy();
    
    return EXIT_SUCCESS;
}

char* file_strip_path(char* file) {
    char* slash = strrchr(file, '/');
    if (slash == NULL) return file;
    return slash + 1;
}

char* file_get_name(char* file) {
    const char* dot = strrchr(file, '.');
    if (dot == NULL) dot = file + strlen(file);
    char* name = malloc(dot - file + 1);
    memcpy(name, file, dot - file);
    name[dot - file + 1] = '\0';
    return name;
}

char* file_modify_extension(char* file, char* ext) {
    const char* dot = strrchr(file, '.');
    if (dot == NULL) dot = file + strlen(file);
    char* name = malloc(dot - file + strlen(ext) + 2);
    memcpy(name, file, dot - file);
    name[dot - file] = '.';
    memcpy(name + (dot - file) + 1, ext, strlen(ext) + 1);
    return name;
}

