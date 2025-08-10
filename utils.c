#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"


/**
 * This function coppy all chars from string until it arrive to none digit
 * It also updates the string params with the next address
 * @param str_ptr The pointer to string to copy from
 * @return The new number as int
 */
char *copy_next_number(char **str_ptr) {
    /* When we finish with the function we update the orginal pointer */
    char *src = *str_ptr;

    /* Pointer to final char */
    char *number;

    /* Hold the start address of the number */
    char *start = src;

    /* The new number length */
    int number_len;

    /* Number can start with +/- symbols */
    if (*src == POSITIVE_NUMBER_SYMBOL || *src == NEGATIVE_NUMBER_SYMBOL) src++;

    /* Loop over all the digits and go to next char */
    while (isdigit(*src)) src++;

    /* Calculate number length (include the +/- symbol) */
    number_len = src - start;

    /* We don't find any number, or we find only +/- symbols */
    if (number_len == 0 || (number_len == 1 && (*start == NEGATIVE_NUMBER_SYMBOL || *src == NEGATIVE_NUMBER_SYMBOL))) {
        return NULL;
    }

    /* We add 1 for \0 symbol */
    number = malloc(number_len + 1);
    if (number == NULL) {
        printf("CRITICAL: Failed to allocate memory for new string");
        exit(1);
    }

    /* Update the number with the new value */
    strncpy(number, start, number_len);
    number[number_len] = '\0';

    /* Update the original param with the new address */
    *str_ptr = src;

    return number;
}

/**
 * This function get pointer to string, and skip all spaces and tabs
 * It updates the str with the new pointer position
 * @param str The string pointer to skip spaces
 */
void skip_empty_spaces(char **str) {
    char *str_ptr = *str;

    /* Go to next char, if it is space */
    while (*str_ptr == EMPTY_CHAR || *str_ptr == TAB_CHAR) str_ptr++;

    *str = str_ptr;
}

/**
 * This function extract the current symbol from the string
 * If the symbol name is invalid, it will return null
 * @param string_ptr Pointer to the string which contains the symbol
 * @param line_number The line number of the symbol
 * @return The symbol string
 */
char *get_current_symbol(char **string_ptr, int line_number) {
    /* In the end we will update the pointer with the new address */
    char *str = *string_ptr;

    /* Pointer to final symbol */
    char *symbol;

    /* Count the symbol size (Validate is is not too long) */
    int symbol_size = 0;

    if (!*str) {
        printf("ERROR: (Line %d) Empty symbol name was received \n", line_number);
        return NULL;
    }

    /* Check first char value */
    if (!isalpha(*str)) {
        printf("ERROR: (Line %d) Symbol must start with alphameric char (%s) \n", line_number, str);
        return NULL;
    }


    /* Go until the end of the symbol */
    while (isalnum(*str)) {
        str++;
        symbol_size++;

        /* Check Symbole length */
        if (symbol_size > MAX_SYMBOL_LENGTH) {
            printf("ERROR: (Line %d) Symbol length should not be more than %d \n", line_number, MAX_SYMBOL_LENGTH);
            return NULL;
        }
    }

    /* +1 for \0 */
    symbol = malloc(symbol_size + 1);
    if (symbol == NULL) {
        printf("CRITICAL: Failed to allocate memory for symbol name");
        exit(1);
    }

    /* Create new string with the symbol name */
    strncpy(symbol, *string_ptr, symbol_size);
    symbol[symbol_size] = END_OF_STRING;

    /* Update the pointer with the new address */
    *string_ptr = str;

    return symbol;
}


/**
 * This function get string, and new string ti append
 * It return new string contains the two words together
 * @param base The base string
 * @param suffix The string to add to
 * @return The new string contains both strings
 */
char *add_suffix_to_string(char *base, char *suffix) {
    /* Create new memory for both strings */
    char *result = malloc(strlen(base) + strlen(suffix) + 1);
    if (result == NULL) {
        printf("Failed to allocate memory for string %s with suffix %s\n", base, suffix);
        exit(1);
    }

    /* Combine two strings to one */
    strcpy(result, base);
    strcat(result, suffix);

    return result;
}

/**
 * Array contains all command indo
 */
const COMMAND_INFO commands[] = {
    {
        "mov", 0, 2,
        {
            {SIMPLE, SYMBOL, MAT, REGISTRY},
            {SYMBOL, MAT, REGISTRY},
        },
        {4, 3}

    },
    {
        "cmp", 1, 2,
        {
            {SIMPLE, SYMBOL, MAT, REGISTRY},
            {SIMPLE, SYMBOL, MAT, REGISTRY},
        },
        {4, 4}
    },
    {
        "add", 2, 2,
        {
            {SIMPLE, SYMBOL, MAT, REGISTRY},
            {SYMBOL, MAT, REGISTRY},
        },
        {4, 3}
    },
    {
        "sub", 3, 2,
        {
            {SIMPLE, SYMBOL, MAT, REGISTRY},
            {SYMBOL, MAT, REGISTRY},
        },
        {4, 3}
    },
    {
        "lea", 4, 2,
        {
            {SYMBOL, MAT},
            {SYMBOL, MAT, REGISTRY},
        },
        {2, 3}
    },
    {
        "clr", 5, 1,
        {
            {UNDEFINED},
            {SYMBOL, MAT, REGISTRY},
        },
        {0, 3}
    },
    {
        "not", 6, 1,
        {
            {UNDEFINED},
            {SYMBOL, MAT, REGISTRY},
        },
        {0, 3}
    },
    {
        "inc", 7, 1,
        {
            {UNDEFINED},
            {SYMBOL, MAT, REGISTRY},
        },
        {0, 3}
    },
    {
        "dec", 8, 1,
        {
            {UNDEFINED},
            {SYMBOL, MAT, REGISTRY},
        },
        {0, 3}
    },
    {
        "jmp", 9, 1,
        {
            {UNDEFINED},
            {SYMBOL, MAT, REGISTRY},
        },
        {0, 3}
    },
    {
        "bne", 10, 1,
        {
            {UNDEFINED},
            {SYMBOL, MAT, REGISTRY},
        },
        {0, 3}
    },
    {
        "jsr", 11, 1,
        {
            {UNDEFINED},
            {SYMBOL, MAT, REGISTRY},
        },
        {0, 3}
    },
    {
        "red", 12, 1,
        {
            {UNDEFINED},
            {SYMBOL, MAT, REGISTRY},
        },
        {0, 3}
    },
    {
        "prn", 13, 1,
        {
            {UNDEFINED},
            {SIMPLE, SYMBOL, MAT, REGISTRY},
        },
        {0, 4}
    },
    {
        "rts", 14, 0,
        {
            {UNDEFINED},
            {UNDEFINED},
        },
        {0, 0}
    },
    {
        "stop", 15, 0,
        {
            {UNDEFINED},
            {UNDEFINED},
        },
        {0, 0}
    }
};

/**
 * Number of available command
 */
const int NUM_COMMANDS = sizeof(commands) / sizeof(commands[0]);

/**
 * Find the command details
 * If it doesn't find command with this name, it will return null
 * @param name The command name
 * @return The command info
 */
const COMMAND_INFO *get_command_info_by_name(const char *name) {
    int i;
    for (i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(commands[i].name, name) == 0) {
            /* Return the command info isnide this index */
            return &commands[i];
        }
    }

    return NULL;
}


/**
 * This function remove the end symbols from string and replace it with regular end of string
 * @param str String to replace from
 */
void trim_newline(char *str) {
    /* Find the string length for check the end string */
    size_t len = strlen(str);

    /* Go until we don't have any more ends of line symbols */
    while (len > 0 && (str[len - 1] == END_OF_LINE || str[len - 1] == WINDOWS_END_OF_LINE)) {
        /* Update the end of string to current position (remove the end of line) */
        str[len - 1] = END_OF_STRING;
        len--;
    }
}

/**
 * This function get decimal number and convert this to binary
 * We request the length, so if the binary length is grether we add leading zeros
 * @param decimal The decimal number to convert to binary
 * @param length The binary length
 * @return The new binary
 */
char *decimal_to_binary(int decimal, int length) {
    /* Loop counter */
    int i;

    /* We create the bitmask for negative for negative number */
    unsigned int mask = (1U << length) - 1;

    /* Applying the bitmask */
    unsigned int value = (unsigned int) decimal & mask;

    /* We add +1 for \0 */
    char *binary = malloc(length + 1);
    if (binary == NULL) {
        printf("ERROR: Failed to allocate binary representation.\n");
        exit(1);
    }

    binary[length] = END_OF_STRING;

    /* Create the binary string */
    for (i = length - 1; i >= 0; i--) {
        /* If the bit is zero it will write 0 otherwise 1 because it is the next char after zero*/
        binary[i] = (value & 1) + '0';

        /* Go to next bit */
        value >>= 1;
    }

    return binary;
}

/**
 * This function get positive decimal number,
 * It will return it base4 value
 * @param value The decimal number
 * @return The base4 number
 */
char *decimal_to_base4(int value) {
    /* The final base4 string length */
    int base4_length = 0;

    /* Only for calculate the final base4 length */
    int temp_value = value;

    /* The final base4 number */
    char *result;

    /* Handle with zero value */
    if (value == 0) {
        /* a + \0 */
        result = malloc(2);
        if (result == NULL) {
            printf("CRITICAL: Failed to allocate memory for base4");
            exit(1);
        }

        result[0] = 'a';
        result[1] = '\0';
        return result;
    }

    /* Calculate the base4 length */
    while (temp_value > 0) {
        temp_value /= 4;
        base4_length++;
    }

    /* +1 for \0 */
    result = malloc(base4_length + 1);
    if (result == NULL) {
        printf("CRITICAL: Failed to allocate binary representation.\n");
        exit(1);
    }

    result[base4_length] = END_OF_STRING;

    /* Update the base4 chars from the end */
    while (value > 0) {
        result[--base4_length] = BASE_4_CHARS[value % 4];
        value /= 4;
    }

    return result;
}

/**
 * This function gets even binary number length
 * and return its base number
 * @param binary_str The binary string (in even length)
 * @return The base4 number
 */
char *binary_to_base4(char *binary_str) {
    /* The binary number length */
    int binary_length;

    /* The final base4 number */
    char *result;

    /* For loop counter */
    int i, j = 0;

    /* The current binary number */
    int current_binary_value;

    binary_length = strlen(binary_str);

    /* Half of the binary (from base2 to base4 we divide by two) */
    result = malloc(binary_length / 2 + 1);
    if (result == NULL) {
        printf("CRITICAL: Failed to allocate binary representation.\n");
        exit(1);
    }

    /* Loop over the binary and each two bits replace with 4base */
    for (i = 0; i < binary_length; i += 2) {
        /* The first binary value is *2 and the second is *1 */
        current_binary_value = (binary_str[i] - '0') * 2 + (binary_str[i + 1] - '0');
        result[j++] = BASE_4_CHARS[current_binary_value];
    }

    /* Close the base4 string*/
    result[j] = END_OF_STRING;

    return result;
}


/**
 * This function writes all assembler files
 * Include object, external and entry
 * @param filename The filename of the assembly file
 * @param assembler_tables The assembler tables
 */
void write_assembler_files(char *filename, ASSEMBLER_TABLES *assembler_tables) {
    /* Define all files name */
    char *object_file_name_with_extension = add_suffix_to_string(filename, OBJECT_FILE_EXTENSION);
    char *entry_file_name_with_extension = add_suffix_to_string(filename, ENTRY_FILE_EXTENSION);
    char *external_file_name_with_extension = add_suffix_to_string(filename, EXTERNAL_FILE_EXTENSION);

    /* Init tables pointers */
    COMMAND_BINARY_LINE *command_binary_line = assembler_tables->command_binary_line;
    INSTRUCTION_BINARY_LINE *instruction_binary_line = assembler_tables->instruction_binary_line;
    ENTRY_INSTRUCTION *entry_instruction = assembler_tables->entry_instruction;
    EXTERNAL_INSTRUCTION *external_instruction = assembler_tables->external_instruction;

    /* Output files */
    FILE *entry_file;
    FILE *external_file;
    FILE *object_file = fopen(object_file_name_with_extension, "w");
    if (object_file == NULL) {
        perror("CRITICAL: Failed to create object file!");
        exit(1);
    }

    fprintf(object_file, "\t%s %s\n",
            decimal_to_base4(assembler_tables->ic - IC_COUNTER_DEFAULT_VALUE),
            decimal_to_base4(assembler_tables->dc - DC_COUNTER_DEFAULT_VALUE)
    );
    /* Write commands file */
    while (command_binary_line != NULL) {
        fprintf(object_file, "%s\t%s\n",
                decimal_to_base4(command_binary_line->address),
                binary_to_base4(command_binary_line->machine_code)
        );
        command_binary_line = command_binary_line->next;
    }
    /* Added the instruction section */
    while (instruction_binary_line != NULL) {
        fprintf(object_file, "%s\t%s\n",
                decimal_to_base4(instruction_binary_line->address),
                binary_to_base4(instruction_binary_line->machine_code)
        );
        instruction_binary_line = instruction_binary_line->next;
    }
    fclose(object_file);

    /* Write entry file only if we define entries */
    if (entry_instruction != NULL) {
        entry_file = fopen(entry_file_name_with_extension, "w");
        if (entry_file == NULL) {
            perror("CRITICAL: Failed to create entry file!");
            exit(1);
        }

        while (entry_instruction != NULL) {
            fprintf(entry_file, "%s\t%s\n",
                    entry_instruction->name,
                    decimal_to_base4(entry_instruction->address)
            );
            entry_instruction = entry_instruction->next;
        }
        fclose(entry_file);
    }

    /* Write externals only if it was define */
    if (external_instruction != NULL) {
        external_file = fopen(external_file_name_with_extension, "w");
        if (external_file == NULL) {
            perror("CRITICAL: Failed to create external file!");
            exit(1);
        }

        while (external_instruction != NULL) {
            fprintf(external_file, "%s\t%s\n",
                    external_instruction->name,
                    decimal_to_base4(external_instruction->address)
            );
            external_instruction = external_instruction->next;
        }
        fclose(external_file);
    }
}


/**
 * This function return the length until the next space or tab
 * @param str The string to search in
 * @return The word length
 */
int get_word_length_until_space(char *str) {
    /* Hold the word length */
    int len = 0;

    /* Go until the next space */
    while (str[len] && str[len] != EMPTY_CHAR && str[len] != TAB_CHAR) {
        len++;
    }

    return len;
}
