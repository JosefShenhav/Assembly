#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"


/**
 * This funvtion get line contains macro, and return its name
 * @param line Pointer to macro line
 * @param line_number The line number of the macro
 * @return Teh macro name
 */
char *extract_macro_name(char **line, int line_number) {
    /* Pointer to line start */
    char *start = *line;

    /* Macro name length (end - start) */
    int length;

    /* Pinter to macro name end */
    char *end;

    /* Holds the new macro name */
    char *macro_name;

    /* After 'mcro' can be multi spaces */
    skip_empty_spaces(&start);

    /* Update the end pointer */
    end = start;

    /* Macro name must start with letters */
    if (!*end || !isalpha(*end)) {
        printf("ERROR: (Line %d) Macro name should start only with letters: (%s) \n", line_number, end);
        return NULL;
    }

    /* Increase the end until we arrive to the space */
    while (*end && *end != EMPTY_CHAR && *end != TAB_CHAR) {
        /* Validate correct chars */
        if (!isalnum(*end) && *end != UNDERSCORE_SYMBOL) {
            printf("ERROR: (Line %d) Macro should contains only letters / numbers / underscore (%s)", line_number, end);
            return NULL;
        }

        end++;
    }

    /* Update the length (this is end minus start) */
    length = end - start;

    /* Add +1 for \0 */
    macro_name = malloc(length + 1);
    if (macro_name == NULL) {
        printf("Failed to allocate memory for macro name");
        exit(1);
    }

    /* Copy the macro name itself */
    strncpy(macro_name, start, length);

    /* Close the string */
    macro_name[length] = END_OF_STRING;

    /* Update the original pointer */
    *line = end;

    return macro_name;
}


/* Assemblers */
/**
 * This function is the pre assembler
 * It only saves all the macros and when it seems it in the code it replaces it with the actual macro
 * e.g.
 * mcro a
 *    prn AS
 * mcroend
 *
 * a
 * stop
 * It will be replaced with:
 * prn AS
 * stop
 * @param filename The assembler file name
 * @param assembler_tables All Assembler tables (include the macro itself)
 * @return The status code of the pre assembler action
 */
STATUS_CODE pre_assembler(char *filename, ASSEMBLER_TABLES *assembler_tables) {
    /* Init file names */
    char *input_file_name_with_extension = add_suffix_to_string(filename, ASSEMBLY_FILE_EXTENSION);
    char *output_file_name_with_extension = add_suffix_to_string(filename, PRE_ASSEMBLER_FILE_EXTENSION);

    /* Open files */
    FILE *assembly_file = fopen(input_file_name_with_extension, "r");
    FILE *output_file = fopen(output_file_name_with_extension, "w");

    /* One line can not be more than line max  */
    char line[LINE_MAX_LENGTH + 1];

    /* The pre assembler status code */
    STATUS_CODE status_code = OK;

    /* Hold current macro name */
    char *macro_name;

    /* Current line number */
    unsigned int line_number = 0;

    /* Should we save current log in current macro */
    boolean inside_macro = FALSE;

    /* Mcro command length */
    int macro_len = strlen(MACRO_NAME);

    /* Macro tables initialization */
    MACRO *current_macro = NULL;
    MACRO *last_macro = NULL;

    /* If we inside line contains macro this will hold this macro */
    MACRO *current_line_macro;

    /* Pointer to the current char inside the line */
    char *current_line_ptr;

    /* Failed to open the files */
    if (assembly_file == NULL) {
        printf("CRITICAL: Unable to find or open file %s\n", input_file_name_with_extension);
        return ERROR;
    }
    if (output_file == NULL) {
        printf("CRITICAL: Unable to open file %s\n", output_file_name_with_extension);
        return ERROR;
    }

    while (fgets(line, sizeof(line), assembly_file) != NULL) {
        line_number++;
        /* Update the current char to point the the start */
        current_line_ptr = line;

        /* Validate the line is in correct length (-1 ignore \0) */
        if (strlen(line) > LINE_MAX_LENGTH - 1) {
            printf("ERROR: (Line %d) line length should not be more than %d \n", line_number, LINE_MAX_LENGTH);
            status_code = ERROR;
            continue;
        }

        /* Validate the line ends with \0 */
        trim_newline(line);

        /* Remove leading space (e.g. ' mcro') */
        skip_empty_spaces(&current_line_ptr);

        if (strncmp(current_line_ptr, END_MACRO_NAME, strlen(END_MACRO_NAME)) == 0) {
            /* Update the pointer to be after the mcroend command */
            current_line_ptr += strlen(END_MACRO_NAME);
            /* After mcroend can be many spaces */
            skip_empty_spaces(&current_line_ptr);

            /* After mcroend should not be more letters */
            if (*current_line_ptr) {
                printf("ERROR: (Line %d) Macro end should not contain spam letters\n", line_number);
                status_code = ERROR;
                continue;
            }

            /* Stop added code to macro*/
            inside_macro = FALSE;
        }
        /* Check if we have 'mcro ' */
        else if (strncmp(current_line_ptr, MACRO_NAME, macro_len) == 0) {
            /* Update the pointer to be after the mcro command */
            current_line_ptr += macro_len;

            /* After macro definition can be many spaces */
            skip_empty_spaces(&current_line_ptr);

            /* There is no any macro name */
            if (!*current_line_ptr) {
                printf("ERROR (Line %d) You must define macro name \n", line_number);
                status_code = ERROR;
                continue;
            }

            /* Extract the macro name */
            macro_name = extract_macro_name(&current_line_ptr, line_number);

            if (macro_name == NULL) {
                status_code = ERROR;
                continue;
            }

            if (find_macro_by_name(assembler_tables->macro, macro_name, strlen(macro_name)) != NULL) {
                printf("ERROR: (Line: %d) found multi macros with same name (%s) \n", line_number, macro_name);
                status_code = ERROR;
                continue;
            }

            if (!validate_macro_name(macro_name, line_number)) {
                status_code = ERROR;
                continue;
            }

            skip_empty_spaces(&current_line_ptr);
            if (*current_line_ptr) {
                printf("ERROR: (Line: %d) Macro should not contain spam letters\n", line_number);
                status_code = ERROR;
                continue;
            }

            /* Create new macro */
            current_macro = create_macro(macro_name);

            /* Added the macro to the tables */
            if (assembler_tables->macro == NULL) {
                assembler_tables->macro = current_macro;
            } else {
                last_macro->next = current_macro;
            }
            last_macro = current_macro;

            /* Added the flag for next loop add the code */
            inside_macro = TRUE;
            /* End of the macro */
        } else if (inside_macro) {
            /* Added the current line to the macro tables */
            add_content_to_macro(current_macro, line);
        } else {
            /* Regular line check if we call for specific macro */
            skip_empty_spaces(&current_line_ptr);

            current_line_macro = find_macro_by_name(assembler_tables->macro, current_line_ptr, get_word_length_until_space(current_line_ptr));

            /* Replace macro name #1# */
            if (current_line_macro != NULL) {
                MACRO_CONTENT *content = current_line_macro->content;

                /* Write the all the macro codes #1# */
                while (content != NULL) {
                    fprintf(output_file, "%s\n", content->content);
                    content = content->next;
                }

                continue;
            }

            /* Otherwise write regular line */
            fprintf(output_file, "%s\n", line);
        }
    }

    /* Close the files */
    fclose(assembly_file);
    fclose(output_file);

    return status_code;
}


/**
 * This function validates the macro has a valid name
 * for rxample it doesn't save word
 * @param macro_name
 * @param line_number
 * @return Is the macro valid
 */
boolean validate_macro_name(char *macro_name, int line_number) {
    /* Check the name is not command name (e.g. mov) */
    const COMMAND_INFO *command_info = get_command_info_by_name(macro_name);
    if (command_info != NULL) {
        printf("ERROR: (Line %d) Macro name should not be a command name (%s) \n", line_number, macro_name);
        return FALSE;
    }

    /* Ignore the . and check only the macro itself */
    if (*macro_name == INSTRUCTION_PREFIX) macro_name++;

    /* Validate not another instruction (like .data) */
    if (strcmp(macro_name, ENTRY_INSTRUCTION_NAME) == 0 || strcmp(macro_name, EXTERNAL_INSTRUCTION_NAME) == 0 ||
        strcmp(macro_name, DATA_INSTRUCTION_NAME) == 0 || strcmp(macro_name, MAT_INSTRUCTION_NAME) == 0 ||
        strcmp(macro_name, STRING_INSTRUCTION_NAME) == 0 || strcmp(macro_name, MACRO_NAME) == 0 ||
        strcmp(macro_name, MACRO_NAME) == 0) {
        printf("ERROR: (Line %d) Macro name should not be an instruction name (%s) \n", line_number, macro_name);
        return FALSE;
    }

    return TRUE;
}
