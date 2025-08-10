#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "assembler.h"

/**
 * This function get Macro table and search for a specific table by its name.
 * If it doesn't find any table, this will return NULL
 * @param head The macro table
 * @param name The table name which we want to find
 * @param length The string length to search in
 * @return The table with this name
 */
MACRO *find_macro_by_name(MACRO *head, const char *name, int length) {
    while (head != NULL) {
        /* Check if this table has the name */
        if (strncmp(head->name, name, length) == 0) {
            return head;
        }

        /* We try the next table */
        head = head->next;
    }

    /* We don't find any tale */
    return NULL;
}


/**
 * Add new content line to existing macro
 * @param current_macro The macro to add the new line to
 * @param content The new line content
 */
void add_content_to_macro(MACRO *current_macro, char *content) {
    /* Init the new macro content */
    MACRO_CONTENT *new_content = malloc(sizeof(MACRO_CONTENT));
    if (new_content == NULL) {
        printf("CRITICAL: Failed to create new Content");
        exit(1);
    }

    /* Copy to new string (+1 for \0) */
    new_content->content = malloc(strlen(content) + 1);
    strcpy(new_content->content, content);

    /* Init default values */
    new_content->next = NULL;

    /* Add the content to the table */
    if (current_macro->content == NULL) {
        current_macro->content = new_content;
    } else {
        current_macro->last->next = new_content;
    }
    current_macro->last = new_content;
}

/**
 * This function validate all macros memories are free
 * @param head The head pointer to the macro
 */
void free_macros(MACRO *head) {
    /* We hold two pointer each time, one for the current and one for the previous
     * So we can free the previous and still have the current */
    MACRO *current_macro = head;
    MACRO *prev_macro;

    MACRO_CONTENT *macro_content;
    MACRO_CONTENT *prev_macro_content;

    while (current_macro != NULL) {
        /*  Free firstly the contents */
        macro_content = current_macro->content;

        while (macro_content != NULL) {
            /* Free the content string and */
            free(macro_content->content);
            prev_macro_content = macro_content;
            macro_content = macro_content->next;
            free(prev_macro_content);
        }

        prev_macro = current_macro;
        current_macro = current_macro->next;

        /* Free the macro itself */
        free(prev_macro->name);
        free(prev_macro);
    }
}

/**
 * This function get Symbols table and search for a specific one by its name.
 * If it doesn't find any table, this will return NULL
 * @param head The Symbols table
 * @param name The symbol name which we want to find
 * @return The symbol
 */
SYMBOL_TABLE *find_symbol_by_name(SYMBOL_TABLE *head, const char *name) {
    while (head != NULL) {
        /* Check if this table has the name */
        if (strcmp(head->name, name) == 0) {
            return head;
        }

        /* We try the next table */
        head = head->next;
    }

    /* We don't find any tale */
    return NULL;
}


/**
 * Create new entry instruction
 * @param name The new entry instruction name
 * @param address the entry machine address
 * @param line_number The entry line number
 * @return The new Entry
 */
ENTRY_INSTRUCTION *create_entry_instruction(char *name, int address, int line_number) {
    ENTRY_INSTRUCTION *new_entry_instruction = malloc(sizeof(ENTRY_INSTRUCTION));
    if (new_entry_instruction == NULL) {
        printf("ERROR (Line %d) Failed to allocate memory for entry instruction.", line_number);
        return NULL;
    }

    /* Init values */
    new_entry_instruction->name = name;
    new_entry_instruction->address = address;
    new_entry_instruction->line_number = line_number;

    /* Default values */
    new_entry_instruction->next = NULL;

    return new_entry_instruction;
}


/**
 * Find entry instruction by it name
 * @param name The new entry instruction name
 * @param head The head pointer of the entry instructions table
 * @return The Entry instruction
 */
ENTRY_INSTRUCTION *find_entry_instruction(ENTRY_INSTRUCTION *head, char *name) {
    while (head != NULL) {
        /* Check if this table has the name */
        if (strcmp(head->name, name) == 0) {
            return head;
        }

        /* We try the next table */
        head = head->next;
    }

    /* We don't find any tale */
    return NULL;
}

/**
 * Create new Symbol
 * @param name The new symbol name
 * @param type The symbol type (SYMBOL_TYPE)
 * @param location The symbol machine address
 * @return The new Symbol
 */
SYMBOL_TABLE *create_symbol(char *name, SYMBOL_TYPE type, int location) {
    SYMBOL_TABLE *symbol = malloc(sizeof(SYMBOL_TABLE));
    if (!symbol) {
        printf("CRITICAL: Failed to create symbol table");
        exit(1);
    }

    /* Init the values */
    symbol->type = type;
    symbol->name = name;
    symbol->location = location;

    /* Init with default values */
    symbol->next = NULL;

    return symbol;
}

/**
 * Create new macro
 * @param macro_name The new macro name
 * @return The new Macro
 */
MACRO *create_macro(char *macro_name) {
    MACRO *new_macro = malloc(sizeof(MACRO));
    if (new_macro == NULL) {
        printf("CRITICAL: Failed to create new Macro");
        exit(1);
    }

    /* Int the values */
    new_macro->name = macro_name;

    /* Init default values */
    new_macro->next = NULL;
    new_macro->content = NULL;

    return new_macro;
}


/**
 * This function creates new command binary address and machine code
 * @param address The command address (ic)
 * @param machine_code The command machine code (Binary representation)
 * @param line_number The machine code line number
 * @return The new command binary table
 */
COMMAND_BINARY_LINE *create_command_binary_line(int address, char *machine_code, int line_number) {
    COMMAND_BINARY_LINE *new_binary_line = malloc(sizeof(COMMAND_BINARY_LINE));
    if (new_binary_line == NULL) {
        printf("CRITICAL, Failed to allocate memory for binary line.");
        exit(1);
    }

    /* Init the values */
    new_binary_line->address = address;
    new_binary_line->machine_code = machine_code;
    new_binary_line->line_number = line_number;

    /* Init with the default values */
    new_binary_line->next = NULL;

    return new_binary_line;
}


/* External */

/**
 * This function creates new external instruction
 * @param name The external instruction name
 * @param address The external address
 * @return The new external instruction table
 */
EXTERNAL_INSTRUCTION *create_external_instruction(char *name, int address) {
    /* Init external table */
    EXTERNAL_INSTRUCTION *external_instruction = malloc(sizeof(EXTERNAL_INSTRUCTION));
    if (external_instruction == NULL) {
        printf("CRITICAL: Failed to allocate external instruction.\n");
        exit(1);
    }

    /* Init the value */
    external_instruction->address = address;
    external_instruction->name = name;

    /* Init the default value */
    external_instruction->next = NULL;

    return external_instruction;
}


/* Instructions */
/**
 * This function creates new instruction
 * @param address The instruction address (dc almost)
 * @param machine_code The instruction binary machine code (e.g. 1010101010)
 * @return The new Instruction table
 */
INSTRUCTION_BINARY_LINE *create_instruction_binary_line(int address, char *machine_code) {
    /* Allocate memory for this instruction */
    INSTRUCTION_BINARY_LINE *new_instruction_binary_line = malloc(sizeof(INSTRUCTION_BINARY_LINE));
    if (new_instruction_binary_line == NULL) {
        printf("CRITICAL: Failed to allocate binary represent.");
        exit(1);
    }

    /* Init the value */
    new_instruction_binary_line->address = address;
    new_instruction_binary_line->machine_code = machine_code;

    /* Init default values */
    new_instruction_binary_line->next = NULL;

    return new_instruction_binary_line;
}


/**
 * This functions frees all assembler tables
 * @param assembler_tables The assembler tables to free its address
 */
void free_assembler_tables(ASSEMBLER_TABLES *assembler_tables) {
    /* Always we hold two pinters, one for current macro
     * Another one for hold the preivouse so we can free without lose our pointer
     */
    COMMAND_BINARY_LINE *current_command;
    COMMAND_BINARY_LINE *prev_command;

    INSTRUCTION_BINARY_LINE *current_instruction;
    INSTRUCTION_BINARY_LINE *prev_instruction;

    ENTRY_INSTRUCTION *entry_instruction;
    ENTRY_INSTRUCTION *prev_entry_instruction;

    EXTERNAL_INSTRUCTION *external_instruction;
    EXTERNAL_INSTRUCTION *prev_external_instruction;

    /* Free Macros */
    free_macros(assembler_tables->macro);

    /* Free commands */
    current_command = assembler_tables->command_binary_line;
    while (current_command != NULL) {
        prev_command = current_command;
        current_command = current_command->next;
        free(prev_command->machine_code);
        free(prev_command);
    }

    /* Free instructions */
    current_instruction = assembler_tables->instruction_binary_line;
    while (current_instruction != NULL) {
        prev_instruction = current_instruction;
        current_instruction = current_instruction->next;
        free(prev_instruction->machine_code);
        free(prev_instruction);
    }

    /* Free entries */
    entry_instruction = assembler_tables->entry_instruction;
    while (entry_instruction != NULL) {
        prev_entry_instruction = entry_instruction;
        entry_instruction = entry_instruction->next;
        free(prev_entry_instruction->name);
        free(prev_entry_instruction);
    }

    /* Free externals */
    external_instruction = assembler_tables->external_instruction;
    while (external_instruction != NULL) {
        prev_external_instruction = external_instruction;
        external_instruction = external_instruction->next;
        free(prev_external_instruction->name);
        free(prev_external_instruction);
    }

    /* Free the table itself */
    free(assembler_tables);
}
