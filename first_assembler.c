#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"


/**
 * The first assembler
 * In this assembler we create commands, instructions, entries, external abd symbol table
 * Notice we don't insert all address, because we will know them only in the second assembler
 * After we calculate all symbols
 * @param filename The assembler file name
 * @param assembler_tables All Assembler tables (include the macro itself)
 * @return The status code of the pre assembler action
 */
STATUS_CODE first_assembler(char *filename, ASSEMBLER_TABLES *assembler_tables) {
    /* Define the pre assembler file name to read the code */
    char *input_file_name_with_extension = add_suffix_to_string(filename, PRE_ASSEMBLER_FILE_EXTENSION);

    /* Define string contains each line (can't be more than the line max length) */
    char line[LINE_MAX_LENGTH];

    /* Counter for line number in the file */
    int line_number = 0;

    /* IC and DC counter init default values */
    int ic = IC_COUNTER_DEFAULT_VALUE, dc = DC_COUNTER_DEFAULT_VALUE;

    /* Command name (e.g. .data/mov/jmp...) */
    char *command_name;

    /* Pointer to char inside the file line */
    char *line_ptr;

    /* The symbol name of the line (if defined) */
    char *symbol_name;

    /* Entry and External names */
    char *entry_name;
    char *external_name;

    /* If the assembler was successfully (default ok otherwise we find error) */
    STATUS_CODE status_code = OK;

    /* The symboles tables */
    SYMBOL_TABLE *new_symbol;

    /* --------Init all table------ */
    /* Command lines tables */
    COMMAND_BINARY_LINE *command_binary_line_tail = NULL;
    COMMAND_BINARY_LINE *command_binary_line_new = NULL;

    /* Instruction lines tables */
    INSTRUCTION_BINARY_LINE *instruction_binary_line_tail = NULL;
    INSTRUCTION_BINARY_LINE *instruction_binary_line_new = NULL;

    /* Entry lines tables */
    ENTRY_INSTRUCTION *tail_entry_instruction = NULL;
    ENTRY_INSTRUCTION *new_entry_instruction = NULL;

    /* Find details about commands */
    const COMMAND_INFO *command_info;

    /* Open the input file */
    FILE *assembly_file = fopen(input_file_name_with_extension, "r");
    if (assembly_file == NULL) {
        printf("ERROR: Unable to find or open file %s\n", input_file_name_with_extension);
        return ERROR;
    }

    while (fgets(line, sizeof(line), assembly_file) != NULL) {
        line_ptr = line;
        /* Update the line number counter */
        line_number++;

        /* Remove end of line with end of string */
        trim_newline(line_ptr);

        /* Line can be starts with empty spaces */
        skip_empty_spaces(&line_ptr);

        /* Not every ine symbol define, so remove the last line name */
        symbol_name = NULL;
        new_symbol = NULL;

        /* Empty line, go to the next line*/
        if (!*line_ptr) continue;
        /* Line with comment, go to the next line*/
        if (*line_ptr == COMMENT_SYMBOL) continue;

        /* Extract the first word of the line (can be symbol / command / instruction) */
        command_name = coppy_next_command_or_symbol(&line_ptr, line_number);
        if (command_name == NULL) {
            status_code = ERROR;
            continue;
        }

        /* Possibly a symbol definition, Therefore updating the symbol and the command vars */
        if (command_name[strlen(command_name) - 1] == SYMBOL_SUFFIX) {
            /* Replacing symbol name var with command name */
            symbol_name = malloc(strlen(command_name) + 1);
            if (symbol_name == NULL) {
                printf("CRITICAL: Failed to allocate memory for new symbol");
                exit(1);
            }

            /* Update the symbol var with current value */
            strcpy(symbol_name, command_name);
            /* We don't want to save the last char (':') so we remove this */
            symbol_name[strlen(symbol_name) - 1] = END_OF_STRING;

            /* Check symbol length */
            if (strlen(symbol_name) > MAX_SYMBOL_LENGTH) {
                printf("ERROR: (Line %d) Symbol length should be less than %d \n", line_number, MAX_SYMBOL_LENGTH);
                status_code = ERROR;
                continue;
            }
            if (find_symbol_by_name(assembler_tables->symbol_table, symbol_name) != NULL) {
                printf("ERROR: (Line %d) Symbol name already defined (%s) \n", line_number, symbol_name);
                status_code = ERROR;
                continue;
            }
            if (find_macro_by_name(assembler_tables->macro, symbol_name, strlen(symbol_name)) != NULL) {
                printf("ERROR: (Line %d) Symbol name and macro can't share same name (%s) \n", line_number, symbol_name);
                status_code = ERROR;
                continue;
            }

            /* Calculate the new command */
            command_name = coppy_next_command_or_symbol(&line_ptr, line_number);

            if (command_name == NULL) {
                status_code = ERROR;
                continue;
            }
        }

        /* Now line_ptr is after the command and symbol */
        skip_empty_spaces(&line_ptr);

        /* Our command is instruction type */
        if (*command_name == INSTRUCTION_PREFIX) {
            /* Ignore the prefix */
            command_name++;

            /* Entry Type */
            if (strcmp(command_name, ENTRY_INSTRUCTION_NAME) == 0) {
                /* After '.entry' we expected to get the entry name */
                entry_name = get_current_symbol(&line_ptr, line_number);

                /* Symbol name is invalid */
                if (entry_name == NULL) {
                    status_code = ERROR;
                    continue;
                }
                /* Entry Already exist */
                if (find_entry_instruction(assembler_tables->entry_instruction, entry_name) != NULL) {
                    printf("ERROR: (Line %d) Found multi entries with same name (%s) \n", line_number, entry_name);
                    status_code = ERROR;
                    continue;
                }

                /* We still don't know this entry address (only in the second assembler) so we set it as 0 */
                new_entry_instruction = create_entry_instruction(entry_name, 0, line_number);
                /* Check if this is the first entry in the table */
                if (assembler_tables->entry_instruction == NULL) {
                    assembler_tables->entry_instruction = new_entry_instruction;
                } else {
                    /* Add the instruction to the last inserted */
                    tail_entry_instruction->next = new_entry_instruction;
                }

                /*  Update the last entry with the new one */
                tail_entry_instruction = new_entry_instruction;
                /* External Type*/
            } else if (strcmp(command_name, EXTERNAL_INSTRUCTION_NAME) == 0) {
                external_name = get_current_symbol(&line_ptr, line_number);

                /* Symbol name is invalid */
                if (external_name == NULL) {
                    status_code = ERROR;
                    continue;
                }
                /* External Already exist */
                if (find_symbol_by_name(assembler_tables->symbol_table, external_name) != NULL) {
                    printf("ERROR: (Line %d) Found multi externals with same name (%s) \n", line_number, external_name);
                    status_code = ERROR;
                    continue;
                }

                /* In external symbol we don't know the address, so init with 0 */
                new_symbol = create_symbol(external_name, EXTERNAL, 0);
                /* Other instruction (data, mat, string)*/
            } else {
                /* Line with symbol, so save the symal with the current 'dc' address */
                if (symbol_name != NULL) {
                    /* Already exist */
                    if (find_symbol_by_name(assembler_tables->symbol_table, symbol_name) != NULL) {
                        printf("ERROR: (Line %d) Multy symbols with same name (%s) \n", line_number, symbol_name);
                        status_code = ERROR;
                        continue;
                    }

                    new_symbol = create_symbol(symbol_name, DATA, dc);
                }

                /* Data Type */
                if (strcmp(command_name, DATA_INSTRUCTION_NAME) == 0) {
                    instruction_binary_line_new = get_data_machine_codes(&line_ptr, &dc, line_number);
                } else if (strcmp(command_name, MAT_INSTRUCTION_NAME) == 0) {
                    instruction_binary_line_new = get_mat_machine_codes(&line_ptr, &dc, line_number);
                } else if (strcmp(command_name, STRING_INSTRUCTION_NAME) == 0) {
                    instruction_binary_line_new = get_string_machine_codes(&line_ptr, &dc, line_number);
                } else {
                    printf("ERROR: (Line %d) Failed to find instruction with name: %s \n", line_number, command_name);
                    status_code = ERROR;
                    continue;
                }

                /* The data was invalid */
                if (instruction_binary_line_new == NULL) {
                    status_code = ERROR;
                    continue;
                }

                /* We connect the new instruction */
                if (assembler_tables->instruction_binary_line == NULL) {
                    assembler_tables->instruction_binary_line = instruction_binary_line_new;
                } else {
                    instruction_binary_line_tail->next = instruction_binary_line_new;
                }

                /* Can be many instructions (e.g. in .string)
                 * So we want to loop over the instruction and update the latest for the next time */
                while (instruction_binary_line_new->next != NULL)
                    instruction_binary_line_new = instruction_binary_line_new->next;
                instruction_binary_line_tail = instruction_binary_line_new;
            }
            /* Regular command type */
        } else {
            /* Save the symbol with the current ic command */
            if (symbol_name != NULL) {
                new_symbol = create_symbol(symbol_name, CODE, ic);
            }

            /* Find current command info */
            command_info = get_command_info_by_name(command_name);
            if (command_info == NULL) {
                printf("ERROR (Line %d) Flied to find command name (%s) \n", line_number, command_name);
                status_code = ERROR;
                continue;
            }

            command_binary_line_new = get_command_operands_machine_codes(&line_ptr, command_info, &ic, line_number);
            /*  Failed to calculate the command binaries */
            if (command_binary_line_new == NULL) {
                status_code = ERROR;
                continue;
            }

            /* This is the first command, so update the head */
            if (assembler_tables->command_binary_line == NULL) {
                assembler_tables->command_binary_line = command_binary_line_new;
            } else {
                /* Connect the new command to the last commands */
                command_binary_line_tail->next = command_binary_line_new;
            }

            /* Can be many instructions (e.g. in .string)
               So we want to loop over the instruction and update the latest for the next time */
            while (command_binary_line_new->next != NULL)
                command_binary_line_new = command_binary_line_new->next;
            command_binary_line_tail = command_binary_line_new;
        }

        /* Update assembler table with the new  */
        if (new_symbol != NULL) {
            new_symbol->next = assembler_tables->symbol_table;
            assembler_tables->symbol_table = new_symbol;
        }

        /* We expect to this line to be ended, so if it doesn't we get unexpected params */
        if (*line_ptr && *line_ptr != COMMENT_SYMBOL) {
            printf("ERROR (Line: %d): The line contains unexpected params '%s' \n", line_number, line_ptr);
            status_code = ERROR;
            continue;
        }
    }

    /* Save the ic in order to use it in the second assembler */
    assembler_tables->ic = ic;
    assembler_tables->dc = dc;

    /* Close the assembler file */
    fclose(assembly_file);

    return status_code;
}


/**
 * This function calculate command (such as mov) operands
 * It clculate all operands machine codes
 * @param operands_ptr Pointer to operands string
 * @param command_info The current command details
 * @param ic The ic counter (It also updates it)
 * @param line_number The command line number
 * @return All operands machine codes
 */
COMMAND_BINARY_LINE *get_command_operands_machine_codes(char **operands_ptr, const COMMAND_INFO *command_info, int *ic,
                                                        int line_number) {
    /* In the end we will update with the real address */
    char *str = *operands_ptr;

    /* We add +1 for \0 end of string */
    char *command_binary = malloc(ADDRESS_SIZE + 1);

    /* If we create address for operand we save it here */
    char *operand_address;
    /* Hold the command address */
    int command_address;

    /* Init command codes tables */
    COMMAND_BINARY_LINE *head_command_binary_line = NULL;
    COMMAND_BINARY_LINE *tail_command_binary_line = NULL;
    COMMAND_BINARY_LINE *command_binary_line = NULL;

    /* The operands type */
    OPERAND_TYPE first_operand_type;
    OPERAND_TYPE second_operand_type;

    /* How much operands we have */
    int num_of_params;

    /* Set default operand type (0) in binary. if we need, we will update them */
    char *source_operand_binary = decimal_to_binary(0, OPERAND_TYPE_BINARY_SIZE);
    char *des_operand_binary = decimal_to_binary(0, OPERAND_TYPE_BINARY_SIZE);

    /* Find the two operands (if one of them or both are not exist it will set it as undefined) */
    char *first_param = get_next_command_operand(&str);
    char *second_param;

    /* Validate not containing garbage data */
    command_binary[0] = END_OF_STRING;

    /* Insert the command type to the machine code (we will insert the operand type in the next codes) */
    strcat(command_binary, decimal_to_binary(command_info->command_number, COMMAND_NUMBER_BITS_SIZE));
    /* Save current address, we will use it when we create the full command binary table */
    command_address = (*ic)++;

    /* Can be r2  ,*/
    skip_empty_spaces(&str);

    /* If we don't have ',' should not be more operands */
    if (*str != DATA_DELIMITER) {
        if (*str && *str != COMMENT_SYMBOL) {
            printf("ERROR: (Line %d) After one operand should be , to another operand (%s) \n", line_number, str);
            return NULL;
        }

        /* We have only one operand -> set the second as undefined */
        second_param = NULL;
    } else {
        /* Skip the DATA_DELIMITER */
        str++;
        second_param = get_next_command_operand(&str);

        if (second_param == NULL) {
            printf("ERROR: (Line %d) Unexpected second param value \n", line_number);
            return NULL;
        }
    }

    /* Find the operands type (can be undefined for invalid operands or empty one) */
    first_operand_type = find_operand_type(first_param, line_number);
    second_operand_type = find_operand_type(second_param, line_number);

    /* Invalid operands format */
    if (first_operand_type == INVALID_OPERAND || second_operand_type == INVALID_OPERAND) {
        return NULL;
    }


    /* If one of param is none !!param will return 0 so if we combine the sum we get the number of params */
    num_of_params = !!first_param + !!second_param;
    /* Check correct number of operands (for more operands we will deal with in the next) */
    if (num_of_params != command_info->num_of_operands) {
        printf("ERROR: (Line: %d) Unexpected number of operands (Expected: %d, Actual: %d) \n",
               line_number, command_info->num_of_operands, num_of_params
        );
        return NULL;
    }

    /* We don't have any operands (e.g. stop command) */
    if (first_operand_type == UNDEFINED && second_operand_type == UNDEFINED) {
        head_command_binary_line = NULL;
        /* We have only one operand */
    } else if (second_operand_type == UNDEFINED) {
        if (!is_valid_operand_type(first_operand_type, command_info, DES_OPERAND_ORDER)) {
            printf("ERROR: (Line: %d) Unexpected destination operand type \n", line_number);
            return NULL;
        }

        head_command_binary_line = extract_operand_binary(first_param, ic, first_operand_type, line_number);
        des_operand_binary = decimal_to_binary(first_operand_type, OPERAND_TYPE_BINARY_SIZE);
    } else {
        if (!is_valid_operand_type(first_operand_type, command_info, SOURCE_OPERAND_ORDER)) {
            printf("ERROR: (Line: %d) Unexpected source operand type \n", line_number);
            return NULL;
        }
        if (!is_valid_operand_type(second_operand_type, command_info, DES_OPERAND_ORDER)) {
            printf("ERROR: (Line: %d) Unexpected destination operand type \n", line_number);
            return NULL;
        }

        source_operand_binary = decimal_to_binary(first_operand_type, OPERAND_TYPE_BINARY_SIZE);
        des_operand_binary = decimal_to_binary(second_operand_type, OPERAND_TYPE_BINARY_SIZE);
        /* If the both operands are registry thay share the same line */
        if (first_operand_type == REGISTRY && second_operand_type == REGISTRY) {
            /* We add +1 for \0 */
            operand_address = malloc(COMMAND_NUMBER_BITS_SIZE + 1);

            /* Validate the end is in the start*/
            operand_address[0] = END_OF_STRING;

            /* Update the registry address */
            strcat(operand_address, decimal_to_binary(first_param[1] - '0', REGISTRY_BITS_SIZE));
            strcat(operand_address, decimal_to_binary(second_param[1] - '0', REGISTRY_BITS_SIZE));
            /* Update the ERA (in registry 0) */
            strcat(operand_address, decimal_to_binary(0, ERA_BITS_SIZE));

            head_command_binary_line = create_command_binary_line((*ic)++, operand_address, line_number);
        } else {
            head_command_binary_line = extract_operand_binary(first_param, ic, first_operand_type, line_number);
            tail_command_binary_line = head_command_binary_line;

            /* Can be more that one line in same operand */
            while (tail_command_binary_line->next != NULL) tail_command_binary_line = tail_command_binary_line->next;

            /* Added the second operand address */
            tail_command_binary_line->next = extract_operand_binary(second_param, ic, second_operand_type, line_number);
        }
    }

    /* Now that we know the operands types we can create the command address */
    strcat(command_binary, source_operand_binary);
    strcat(command_binary, des_operand_binary);
    strcat(command_binary, decimal_to_binary(COMMAND_ERA_DEFAULT_VALUE, ERA_BITS_SIZE));

    /* Create the command lien itself and add it to the start */
    command_binary_line = create_command_binary_line(command_address, command_binary, line_number);
    command_binary_line->next = head_command_binary_line;

    /* Update the pointer actual address */
    *operands_ptr = str;

    return command_binary_line;
}


/**
 * This calculates mat instruction type data machine codes
 * Every cell in the mat is specific machine code,
 * So our final machine codes will be row*col
 * @param mat_instruction_ptr Pointer to mat data
 * @param dc The dc count (It also update it with the new value)
 * @param line_number The mat line number in the assembly file
 * @return All mat instructions machine codes
 */
INSTRUCTION_BINARY_LINE *get_mat_machine_codes(char **mat_instruction_ptr, int *dc, int line_number) {
    /* In the function end we want to update to the new address */
    char *mat_instruction = *mat_instruction_ptr;

    /* Init instruction tables */
    INSTRUCTION_BINARY_LINE *head_instruction_binary_line = NULL;
    INSTRUCTION_BINARY_LINE *last_instruction_binary_line = NULL;
    INSTRUCTION_BINARY_LINE *new_instruction_binary_line = NULL;

    /* We need to define num_of_params in the address */
    int num_of_params = get_mat_instruction_size(&mat_instruction);

    /* Check how much actually params we got */
    int actual_params_number = 0;

    /* The current number value */
    int current_number;

    /* Loop counter */
    int i;

    /* Error in mat definition size */
    if (num_of_params == -1) {
        printf("ERROR: (Line %d) Invalid mat definition syntax \n", line_number);
        return NULL;
    }

    /* We can define .mat[1][2]    3 -> so skip all these empty spaces */
    skip_empty_spaces(&mat_instruction);

    /* If the first value starts with invalid char (e.g. '.mat ,') */
    if (*mat_instruction && !isdigit(*mat_instruction) && *mat_instruction != POSITIVE_NUMBER_SYMBOL && *mat_instruction
        !=
        NEGATIVE_NUMBER_SYMBOL) {
        printf("ERROR: (Line %d) Number must start with a valid number or +/- symbols \n", line_number);
        return NULL;
    }

    while (*mat_instruction) {
        /* We update the current number and if we get error we return null */
        if (get_next_number_from_instruction_params(&mat_instruction, &current_number, line_number) == ERROR) {
            return NULL;
        }

        /* We have new number, so update the counter */
        actual_params_number++;

        /* We got more than expected params */
        if (actual_params_number > num_of_params) {
            printf("ERROR: (Line %d) Number of params should not be more than %d \n", line_number, num_of_params);
            return NULL;
        }

        /* Update the new number value with current number */
        new_instruction_binary_line = create_instruction_binary_line(
            (*dc)++, decimal_to_binary(current_number, ADDRESS_SIZE));
        if (head_instruction_binary_line == NULL) {
            head_instruction_binary_line = new_instruction_binary_line;;
        } else {
            last_instruction_binary_line->next = new_instruction_binary_line;
        }
        last_instruction_binary_line = new_instruction_binary_line;
    }

    /* Added default values for other mat cells */
    for (i = 0; i < num_of_params - actual_params_number; i++) {
        /* We set default value for empty cells */
        new_instruction_binary_line = create_instruction_binary_line(
            (*dc)++, decimal_to_binary(MAT_DEFAULT_VALUE, ADDRESS_SIZE));
        if (head_instruction_binary_line == NULL) {
            head_instruction_binary_line = new_instruction_binary_line;;
        } else {
            last_instruction_binary_line->next = new_instruction_binary_line;
        }
        last_instruction_binary_line = new_instruction_binary_line;
    }

    /* Update the new address */
    *mat_instruction_ptr = mat_instruction;

    return head_instruction_binary_line;
}


/**
 * It gets regular command operand and try to find its type (Mat, symbol, ect...)
 * @param operand The operand string
 * @param line_number The line number of the operand
 * @return The operand type
 */
OPERAND_TYPE find_operand_type(const char *operand, int line_number) {
    /* We hold the registry number (e.g. r4 -> 4)*/
    int registry_number;

    /* This is empty string */
    if (operand == NULL || !*operand) {
        return UNDEFINED;
    }

    /* This is symple number */
    if (operand[0] == NUMBER_PREFIX) {
        return SIMPLE;
    }

    /* Check if this is registry */
    if (operand[0] == REGISTRY_PREFIX) {
        /* Registry syntax should be length two with 'r' and after that digit, otherwise this is symbol */
        if (strlen(operand) == 2 && isdigit(operand[1])) {
            /* Convert the registry number to int */
            registry_number = operand[1] - '0';

            /* Invalid registry syntax */
            if (registry_number < MIN_REGISTRY_NUMBER || registry_number > MAX_REGISTRY_NUMBER) {
                printf("ERROR: (Line %d) Invalid registry number (%s) \n", line_number, operand);
                return INVALID_OPERAND;
            }

            return REGISTRY;
        }
    }

    /* Operands can be any letter, member, [ */
    while (isalnum(*operand) || *operand == MAT_OPEN_BRACKET) {
        if (*operand == MAT_OPEN_BRACKET)
            return MAT;
        operand++;
    }

    /* We find all other type so it must be symbol */
    return SYMBOL;
}

/**
 * Calculate the operands binary codes
 * If it finds invalid operand will return null
 * @param operand The operand as string
 * @param ic The ic counter
 * @param operand_type The operand type
 * @param line_number The line which the operand exist
 * @return Tables of the binaries operands
 */
COMMAND_BINARY_LINE *extract_operand_binary(char *operand, int *ic, OPERAND_TYPE operand_type, int line_number) {
    /* The binary address in bits */
    char *address;
    /* The registry values as int */
    int first_registry_num = 0, second_registry_num = 0;

    /* When we have more than one binary lines we should save these here */
    COMMAND_BINARY_LINE *additional_binary_lines = NULL;
    COMMAND_BINARY_LINE *new_binary_lines = NULL;

    if (operand_type == SIMPLE) {
        /* We have only one binary -> the number itself */
        return create_command_binary_line((*ic)++, decimal_to_binary(atoi(++operand), ADDRESS_SIZE), line_number);
    }
    if (operand_type == SYMBOL) {
        /* We still don't know the address so we put the symbol name, and in the second assembly we will update it */
        return create_command_binary_line((*ic)++, operand, line_number);
    }

    /* Additional +1 for \0 */
    address = malloc(COMMAND_NUMBER_BITS_SIZE + 1);

    /* Validate empty string */
    address[0] = END_OF_STRING;

    if (operand_type == MAT) {
        /* Added the mat symbol to binary line (Will update in the second) */
        additional_binary_lines = create_command_binary_line((*ic)++, extract_mat_symbol(&operand), line_number);
        /* Extract the two registries, and also check for error */
        if (get_mat_registries(&operand, &first_registry_num, &second_registry_num) == ERROR) {
            printf("ERROR: (Line %d) Invalid mat syntax \n", line_number);
            return NULL;
        }
    }

    if (operand_type == REGISTRY && strlen(operand) >= REGISTRY_MIN_LENGTH) {
        /* Registry second char is the registry number */
        first_registry_num = operand[1] - '0';
        /* We don't touch the second 4 bits */
        second_registry_num = 0;
    }

    /* Insert to binary the registries addresses */
    strcat(address, decimal_to_binary(first_registry_num, REGISTRY_BITS_SIZE));
    strcat(address, decimal_to_binary(second_registry_num, REGISTRY_BITS_SIZE));

    /* This is registry so the ERA is 0 */
    strcat(address, decimal_to_binary(0, ERA_BITS_SIZE));

    /* Init the registry addresses */
    new_binary_lines = create_command_binary_line((*ic)++, address, line_number);

    /* We have only one line so return this */
    if (additional_binary_lines == NULL) return new_binary_lines;

    /* Added the additional to the new */
    additional_binary_lines->next = new_binary_lines;

    return additional_binary_lines;
}

/**
 * This function gets pointer to mat instruction,
 * It will return the mat symbol
 * @param input Pointer to the matb instruction
 * @return The mat symbol
 */
char *extract_mat_symbol(char **input) {
    /* Additional pointer, for real update location */
    char *str = *input;

    /* For loop counter */
    int i = 0;

    /* The final mat symbol */
    char *result;

    /* Count until we arrive to the end of the symbol */
    while (isalnum(str[i])) i++;

    /* Init new address for the symbol (+1 for \0) */
    result = malloc(i + 1);
    if (result == NULL) {
        printf("CRITICAL: Failed to allocate memory for mat symbol.");
        exit(1);
    }

    /* Copy the new symbol name */
    strncpy(result, str, i);
    result[i] = END_OF_STRING;

    /* Update the original pointer */
    *input = str + i;

    return result;
}


/**
 * Calculate the string instruction machine codes
 * It adds zero to the string end (to indicate the end of the string)
 * It also update the string to the next word (skip the string itself)
 * @param str_ptr Pointer to the string
 * @param dc The dc counter
 * @return All string machine codes
 */
INSTRUCTION_BINARY_LINE *get_string_machine_codes(char **str_ptr, int *dc, int line_number) {
    /* New string to go ever the string and update in the end */
    char *str = *str_ptr;

    /* Init all our tables (We create tail to avoid loop all over the tables again and again) */
    INSTRUCTION_BINARY_LINE *head_instruction_binary_line = NULL;
    INSTRUCTION_BINARY_LINE *tail_instruction_binary_line = NULL;
    INSTRUCTION_BINARY_LINE *new_instruction_binary_line = NULL;

    /* The 0 in the end pf the string */
    INSTRUCTION_BINARY_LINE *end_of_the_string;


    /* We don't have any string as parameter */
    if (*str == END_OF_STRING || *str != STRING_SYMBOL) {
        printf("ERROR: (Line %d) Failed to find any actual string. \n", line_number);
        return NULL;
    }

    /* Skip the string start symbol ('"') */
    str++;

    /* Loop until the end of string or the line end (and after the while throw error)*/
    while (*str && *str != STRING_SYMBOL) {
        /* create the current char string machine code */
        new_instruction_binary_line = create_instruction_binary_line((*dc)++, decimal_to_binary(*str, 10));

        /* Update the table with the new char machine code */
        if (head_instruction_binary_line == NULL) {
            head_instruction_binary_line = new_instruction_binary_line;
        } else {
            tail_instruction_binary_line->next = new_instruction_binary_line;
        }
        tail_instruction_binary_line = new_instruction_binary_line;

        /* Go to next char */
        str++;
    }

    /* Wo don't close our string with the symbol */
    if (*str != STRING_SYMBOL) {
        printf("ERROR: (Line %d) Forget to close your string. \n", line_number);
        return NULL;
    }

    /* Create the last 0 char in the end */
    end_of_the_string = create_instruction_binary_line((*dc)++, decimal_to_binary(0, 10));

    /* Our string is not empty -> so add to the last one */
    if (tail_instruction_binary_line != NULL) {
        tail_instruction_binary_line->next = end_of_the_string;
    }
    /* Our string is empty, so only update the head */
    else {
        head_instruction_binary_line = end_of_the_string;
    }

    /* Skip last '"' char */
    str++;

    /* Update the string param with the new address */
    *str_ptr = str;

    return head_instruction_binary_line;
}

/**
 * Get the data instruction number machine codes
 * If the data is invalid this function prints the error and return null
 * This also updates the string with the new address
 * @param str_ptr Pointer to data string contains the numbers
 * @param dc The dc counter
 * @return The data instruction machine code
 */
INSTRUCTION_BINARY_LINE *get_data_machine_codes(char **str_ptr, int *dc, int line_number) {
    /* Create new pointer, and in the end update the param with that value */
    char *str = *str_ptr;

    /* Init the tables */
    INSTRUCTION_BINARY_LINE *head_instruction_binary_line = NULL;
    INSTRUCTION_BINARY_LINE *tail_instruction_binary_line = NULL;
    INSTRUCTION_BINARY_LINE *new_instruction_binary_line = NULL;

    /* Hold current number value */
    int current_number;

    /* We can define .data    3 -> so skip all these empty spaces */
    skip_empty_spaces(&str);

    /* If the first value starts with invalid char (e.g. '.data ,') */
    if (!isdigit(*str) && *str != POSITIVE_NUMBER_SYMBOL && *str != NEGATIVE_NUMBER_SYMBOL) {
        printf("ERROR: (Line %d) Number must start with a valid number or +/- symbols \n", line_number);
        return NULL;
    }

    while (*str) {
        if (get_next_number_from_instruction_params(&str, &current_number, line_number) == ERROR) {
            return NULL;
        }

        /* Create new machine code for current number */
        new_instruction_binary_line = create_instruction_binary_line(
            (*dc)++, decimal_to_binary(current_number, ADDRESS_SIZE));

        /* Update the instructions tables with the new value */
        if (head_instruction_binary_line == NULL) {
            head_instruction_binary_line = new_instruction_binary_line;
        } else {
            tail_instruction_binary_line->next = new_instruction_binary_line;
        }
        tail_instruction_binary_line = new_instruction_binary_line;
    }

    /* Update the original string with the new address */
    *str_ptr = str;

    return head_instruction_binary_line;
}


/**
 * Calculate the mat size (row * col)
 * If we get invalid syntax we return -1
 * It also updates the mat with the new address
 * @param mat_ptr The mat size as string
 * @return The actual mat size
 */
int get_mat_instruction_size(char **mat_ptr) {
    /* In the end we update the new address */
    char *mat = *mat_ptr;

    /* The row and col size as string */
    char *row_size_as_string, *col_size_as_string;

    /* The row and col size as int */
    int row_size, col_size;

    /* Can be lead by spaces e.g. '.mat   []' */
    skip_empty_spaces(&mat);

    /* We should start with [ */
    if (*mat != MAT_OPEN_BRACKET) return -1;

    /* Go to actual row size */
    mat++;
    skip_empty_spaces(&mat);

    row_size_as_string = copy_next_number(&mat);

    if (row_size_as_string == NULL) return -1;

    skip_empty_spaces(&mat);

    if (*mat != MAT_CLOSE_BRACKET) return -1;
    mat++;
    skip_empty_spaces(&mat);

    if (*mat != MAT_OPEN_BRACKET) return -1;
    mat++;

    skip_empty_spaces(&mat);
    col_size_as_string = copy_next_number(&mat);

    if (col_size_as_string == NULL) return -1;

    skip_empty_spaces(&mat);
    if (*mat != MAT_CLOSE_BRACKET) return -1;
    mat++;

    /* Update the new address */
    *mat_ptr = mat;

    row_size = atoi(row_size_as_string);
    col_size = atoi(col_size_as_string);

    return row_size * col_size;
}

/**
 * This function update the r1, r2 var with the mat registries
 * IF the syntax is invalid this will return error status code
 * @param mat_ptr The mat size as string
 * @param r1_out The first registry to update
 * @param r2_out The second registry to update
 * @return The status code
 */
STATUS_CODE get_mat_registries(char **mat_ptr, int *r1_out, int *r2_out) {
    /* In the end we update the new address */
    char *mat = *mat_ptr;

    /* The row and col size as int */
    int row_size, col_size;

    /* Can be lead by spaces e.g. '.mat   []' */
    skip_empty_spaces(&mat);

    /* We should start with [ */
    if (*mat != MAT_OPEN_BRACKET) return ERROR;

    /* Go to actual row size */
    mat++;
    skip_empty_spaces(&mat);
    if (!*mat || *mat != REGISTRY_PREFIX) return ERROR;
    mat++;
    if (!*mat || !isdigit(*mat)) return ERROR;
    row_size = *mat - '0';
    mat++;
    skip_empty_spaces(&mat);

    if (*mat != MAT_CLOSE_BRACKET) return ERROR;
    mat++;
    skip_empty_spaces(&mat);
    if (*mat != MAT_OPEN_BRACKET) return ERROR;
    mat++;
    skip_empty_spaces(&mat);
    if (!*mat || *mat != REGISTRY_PREFIX) return ERROR;
    mat++;
    if (!*mat || !isdigit(*mat)) return ERROR;
    col_size = *mat - '0';
    mat++;
    skip_empty_spaces(&mat);

    if (*mat != MAT_CLOSE_BRACKET) return ERROR;
    mat++;
    /* Update the new address */
    *mat_ptr = mat;

    *r1_out = row_size;
    *r2_out = col_size;

    return OK;
}

/**
 * In data and mat we pass numbers (.data 1, 2, 3)
 * This function update the out member with the int value
 * We must pass the string of the current number
 * If the number is invalid we return ERROR status code
 * @param instruction_params The instruction params
 * @param out_member Pointer to int which we want to update the integre value
 * @param line_number The instrunction line in the assembler file
 * @return Pointer to the number value
 */
STATUS_CODE get_next_number_from_instruction_params(char **instruction_params, int *out_member, int line_number) {
    /* In the end we update the new address */
    char *str = *instruction_params;
    /* Hold current number value as string */
    char *current_number;
    /* Hold current number value as int */
    int number_as_int;

    current_number = copy_next_number(&str);

    /* We don't find number, instead we find another char (e.g. adsad) */
    if (current_number == NULL) {
        printf("ERROR: (Line %d) Expected number values after data instruction \n", line_number);
        return ERROR;
    }

    /* Convert to actual number */
    number_as_int = atoi(current_number);

    /* We only have 10 bits so we have to check we don't get number with more bits */
    if (number_as_int > MAX_POSITIVE_NUMBER_VALUE || number_as_int < MIN_NEGATIVE_NUMBER_VALUE) {
        printf("ERROR: (Line %d) Number in data instruction should be between %d<=x<=%d (Got: %d) \n",
               line_number, MIN_NEGATIVE_NUMBER_VALUE, MAX_POSITIVE_NUMBER_VALUE, number_as_int);
        return ERROR;
    }

    /* After number, we can find spaces (.data 23 , 324) */
    skip_empty_spaces(&str);

    /* If we have ',' we want to validate the next value is real number and not end of line
     * Avoid '.data 234, '
     */
    if (*str == DATA_DELIMITER) {
        str++;

        /* After ',' we can have spaces */
        skip_empty_spaces(&str);

        /* We find ',' without any number */
        if (*str == END_OF_STRING) {
            printf("ERROR: (Line %d) After ',' we expect number and not end of the row \n", line_number);
            return ERROR;
        }
    } else {
        /* Validate there is no more numbers */
        skip_empty_spaces(&str);

        if (*str && *str != COMMENT_SYMBOL) {
            printf("ERROR (Line %d) After one number expected comma before another number (%s)\n", line_number, str);
            return ERROR;
        }
    }

    /* Update with new address */
    *instruction_params = str;

    /* Free our macros */
    free(current_number);

    /* Update the outcome value*/
    *out_member = number_as_int;

    return OK;
}


/**
 * This function copy to new string the current symbol or command
 * IF it is invalid, null will be returned
 * @param str_ptr The string contains the command or symbol
 * @param line_mumber The line number of the command / symbol
 * @return The new string ot symbole
 */
char *coppy_next_command_or_symbol(char **str_ptr, int line_mumber) {
    /* In the end we update the pointer to the new address */
    char *current_ptr = *str_ptr;

    /* Save the first char */
    char *start;

    /* Length of the whole word */
    int command_len;

    /* Pointer to new string*/
    char *new_command;

    /* Symbol or command can be lead by spaces (e.g. ' mov') */
    skip_empty_spaces(&current_ptr);
    start = current_ptr;

    /* If this is instruction prefix, it can be start with this symbol */
    if (*current_ptr == INSTRUCTION_PREFIX) current_ptr++;

    /* Must start with letter */
    if (!isalpha(*current_ptr)) {
        printf("ERROR: (Line %d) Command or Symbol must start with letter (%s) \n", line_mumber, current_ptr);
        return NULL;
    }

    /* Could be cany letter or number so go to next char */
    while (isalnum(*current_ptr)) current_ptr++;

    /* Added the symbol suffix itself */
    if (*current_ptr == SYMBOL_SUFFIX) current_ptr++;

    /* Calculate the command length */
    command_len = current_ptr - start;

    /* We don't find any command or symbol */
    if (command_len == 0) {
        printf("ERROR: (Line %d) Command or Symbol was not found (%s) \n", line_mumber, current_ptr);
        return NULL;
    }

    /* +1 -> /0 */
    new_command = malloc(command_len + 1);
    if (new_command == NULL) {
        printf("CRITICAL: Failed to allocate memory for coping new word");
        exit(1);
    }

    /* Coppy the new string */
    strncpy(new_command, start, command_len);
    new_command[command_len] = '\0';

    /* Update the pointer to the new location */
    *str_ptr = current_ptr;

    return new_command;
}

/**
 * Extract the current command operand
 * We search until we arrive to end / empty char / comma
 * If the string is empty it will return null
 * @param str_ptr Pointer ot operands string
 * @return String contains only the operand itself
 */
char *get_next_command_operand(char **str_ptr) {
    /* We will update the real address in the end */
    char *str = *str_ptr;

    /* Save the operand start char */
    char *start;

    /* In mat can be space between the symbol and the [] so we save another pointer to check if this is mat */
    char *mat_str;

    /* The operand size */
    int new_string_len;

    /* The operand as string */
    char *new_word;

    /* Before the operand can be spaces (e.g. 'mov  r2') */
    skip_empty_spaces(&str);
    start = str;

    if (*str == END_OF_STRING) {
        return NULL;
    }

    /* Number operand starts with # */
    if (*str == NUMBER_PREFIX) {
        str++;

        /* Number can start with + / - or not */
        if (*str == NEGATIVE_NUMBER_SYMBOL || *str == POSITIVE_NUMBER_SYMBOL) str++;
    }

    /* Can be any letter or number */
    while (isalnum(*str)) str++;

    /* Check if this is mat, and if it is the length is greater than regular operand */
    mat_str = str;
    skip_empty_spaces(&mat_str);
    if (*mat_str == MAT_OPEN_BRACKET) {
        /* Go until the first close bracket */
        while (*mat_str && *mat_str != MAT_CLOSE_BRACKET) mat_str++;
        if (*mat_str && *mat_str == MAT_CLOSE_BRACKET) mat_str++;
        /* Go until the final mat string */
        while (*mat_str && *mat_str != MAT_CLOSE_BRACKET) mat_str++;
    }
    if (*mat_str == MAT_CLOSE_BRACKET) {
        str = mat_str + 1;
    }

    /* Calculate the operand size */
    new_string_len = str - start;

    /* Invalid operand length or undined number (only #) */
    if (new_string_len == 0 || (new_string_len == 1 && *start == NUMBER_PREFIX)) return NULL;

    /* We add +1 for \0 */
    new_word = malloc(new_string_len + 1);
    if (!new_word) {
        printf("CRITICAL: Failed to allocate memory for operand string \n");
        exit(1);
    }

    /* Copy the new word */
    strncpy(new_word, start, new_string_len);
    new_word[new_string_len] = END_OF_STRING;

    /* Update the pointer with the new address */
    *str_ptr = str;

    return new_word;
}


/**
 * Check if the operand of the command is correct and allowed
 * @param type The operand type
 * @param command_info All commands info details
* @param opernad_order Can be one of SOURCE_OPERAND_ORDER / DES_OPERAND_ORDER
 * @return If this is valid operand
 */
boolean is_valid_operand_type(OPERAND_TYPE type, const COMMAND_INFO *command_info, int opernad_order) {
    /* For counter */
    int i;

    /* Find number of allow operands type in this command position */
    int count = command_info->allowed_operand_number[opernad_order];

    /* We find the type in the command allow types */
    for (i = 0; i < count; i++) {
        if (command_info->allowed_operands[opernad_order][i] == type)
            return TRUE;
    }

    /* This type is not allowed */
    return FALSE;
}
