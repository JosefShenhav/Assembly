#include <stdio.h>
#include <stdlib.h>
#include "assembler.h"

int main(int argc, char **argv) {
    /* All the assemblers status code */
    int pre_assembler_status_code, first_assembler_status_code, second_assembler_status_code;

    /* Contain all assembler tables */
    ASSEMBLER_TABLES *assembler_tables;
    char *output_file_name_with_extension;

    /* For loop var */
    int i;

    /* If all assemblies was successfully */
    STATUS_CODE status_code = OK;

    if (argc < 2) {
        fprintf(stderr, "CRITICAL: Found 0 file to assembly \n");
        exit(1);
    }

    for (i = 1; i < argc; i++) {
        /* Assembler filename */
        char *filename = argv[i];

        output_file_name_with_extension = add_suffix_to_string(filename, PRE_ASSEMBLER_FILE_EXTENSION);

        /* Init all assembler tables */
        assembler_tables = malloc(sizeof(ASSEMBLER_TABLES));
        assembler_tables->command_binary_line = NULL;
        assembler_tables->instruction_binary_line = NULL;
        assembler_tables->external_instruction = NULL;
        assembler_tables->entry_instruction = NULL;
        assembler_tables->symbol_table = NULL;

        /* Run pre assembler */
        pre_assembler_status_code = pre_assembler(filename, assembler_tables);
        if (pre_assembler_status_code != OK) {
            printf("WARNING: Pre-assembler failed. Skipping to next file...\n");
            status_code = ERROR;
            free_macros(assembler_tables->macro);
            remove(output_file_name_with_extension);
            continue;
        }

        /* Run main assemblers */
        first_assembler_status_code = first_assembler(filename, assembler_tables);
        second_assembler_status_code = second_assembler(assembler_tables);
        if (first_assembler_status_code != OK || second_assembler_status_code != OK) {
            status_code = ERROR;
            printf("WARNING: Assembler failed. Skipping to next file...\n");
        }
        else {
            /* If assembler was successfully write the output files */
            write_assembler_files(filename, assembler_tables);
        }

        free_assembler_tables(assembler_tables);
    }

    return status_code;
}
