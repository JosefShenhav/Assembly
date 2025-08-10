#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"


/**
 * In the second assembly we mainly do three things:
 * 1. Update all symbols address
 * 2. Update all entry with their machine code
 * 3. Update externals with their machine code use
 * @param assembler_tables All Assembler tables (include the macro itself)
 * @return The status code of the pre assembler action
 */
STATUS_CODE second_assembler(ASSEMBLER_TABLES *assembler_tables) {
    /*  We assume the program works correctly, otherwise we update it with the error code */
    STATUS_CODE status_code = OK;

    /* External instructions tables */
    EXTERNAL_INSTRUCTION *last_external_instruction = NULL;
    EXTERNAL_INSTRUCTION *new_external_instruction = NULL;

    /* Tables definitions */
    SYMBOL_TABLE *symbol_table;
    ENTRY_INSTRUCTION *entry_instruction;
    COMMAND_BINARY_LINE *command_binary_line;

    /* Hold entry instruction entry */
    SYMBOL_TABLE *entry_symbol;

    /* Calculate all symbols addresses */
    char *symbol_address;

    /* Added to each instruction address the ic value (Data is place directly after the Command) */
    INSTRUCTION_BINARY_LINE *instruction_binary_line = assembler_tables->instruction_binary_line;
    while (instruction_binary_line != NULL) {
        instruction_binary_line->address = instruction_binary_line->address + assembler_tables->ic;
        instruction_binary_line = instruction_binary_line->next;
    }

    /* Also add the ic value for the symbols with type data */
    symbol_table = assembler_tables->symbol_table;
    while (symbol_table != NULL) {
        if (symbol_table->type == DATA) {
            symbol_table->location += assembler_tables->ic;
        }
        symbol_table = symbol_table->next;
    }

    /* Update the value back to the head (for next loops) */
    symbol_table = assembler_tables->symbol_table;

    /* Update the entry instructions with their address */
    entry_instruction = assembler_tables->entry_instruction;
    while (entry_instruction != NULL) {
        /* Find entry symbol */
        entry_symbol = find_symbol_by_name(symbol_table, entry_instruction->name);

        if (entry_symbol == NULL) {
            printf("ERROR: (Line %d) Failed to find symbol with name: %s \n", entry_instruction->line_number,
                   entry_instruction->name
            );
            status_code = ERROR;
        } else {
            entry_instruction->address = entry_symbol->location;
        }
        /* Entry address */
        entry_instruction = entry_instruction->next;
    }

    command_binary_line = assembler_tables->command_binary_line;
    while (command_binary_line != NULL) {
        /* We want to check if we already insert the address, or the symbole
         * if it starts with 0/1 this is address
         * Otherwise, this is symbol we need to update it actual address
         */
        if (*command_binary_line->machine_code != '0' && *command_binary_line->machine_code != '1') {
            SYMBOL_TABLE *command_symbol = find_symbol_by_name(symbol_table, command_binary_line->machine_code);

            if (command_symbol == NULL) {
                printf("ERROR: (Line %d) Failed to find symbol name (%s). \n", command_binary_line->line_number,
                       command_binary_line->machine_code);
                status_code = ERROR;
                command_binary_line = command_binary_line->next;
                continue;
            }

            /* It this is external we also want to save the command address for external file */
            if (command_symbol->type == EXTERNAL) {
                new_external_instruction = create_external_instruction(
                    command_binary_line->machine_code, command_binary_line->address);

                /* In external, we only save the ERA as 1, all other bits as zero */
                command_binary_line->machine_code = decimal_to_binary(EXTERNAL, ADDRESS_SIZE);

                /* This is the first external */
                if (assembler_tables->external_instruction == NULL) {
                    assembler_tables->external_instruction = new_external_instruction;
                    /* Append to the last external */
                } else {
                    last_external_instruction->next = new_external_instruction;
                }
                last_external_instruction = new_external_instruction;
            } else {
                /* ADDRESS_SIZE + \0 */
                symbol_address = malloc(ADDRESS_SIZE + 1);
                symbol_address[0] = END_OF_STRING;
                /* Define the data address */
                strcat(symbol_address, decimal_to_binary(command_symbol->location, SYMBOL_BITS_LENGTH));
                /* Define the ERA */
                strcat(symbol_address, decimal_to_binary(DATA, ERA_BITS_SIZE));
                command_binary_line->machine_code = symbol_address;
            }
        }
        command_binary_line = command_binary_line->next;
    }

    return status_code;
}
