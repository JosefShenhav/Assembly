#pragma once

/* File extensions */
#define ASSEMBLY_FILE_EXTENSION ".as"
#define PRE_ASSEMBLER_FILE_EXTENSION ".am"
#define OBJECT_FILE_EXTENSION ".ob"
#define ENTRY_FILE_EXTENSION ".ent"
#define EXTERNAL_FILE_EXTENSION ".ext"

/* Global Consts */
#define MACRO_NAME "mcro"
#define END_MACRO_NAME "mcroend"
#define COMMENT_SYMBOL ';'
#define SYMBOL_BITS_LENGTH 8
#define SYMBOL_SUFFIX ':'
#define END_OF_STRING '\0'
#define ADDRESS_SIZE 10
#define STRING_SYMBOL '"'
#define POSITIVE_NUMBER_SYMBOL '+'
#define NEGATIVE_NUMBER_SYMBOL '-'
#define DATA_DELIMITER ','
#define MAX_POSITIVE_NUMBER_VALUE 511
#define MIN_NEGATIVE_NUMBER_VALUE -512
#define MAX_SYMBOL_LENGTH 30
#define COMMAND_NUMBER_BITS_SIZE 4
#define NUMBER_PREFIX '#'
#define UNDERSCORE_SYMBOL '_'
#define WINDOWS_END_OF_LINE '\r'
#define END_OF_LINE '\n'
#define EMPTY_CHAR ' '
#define TAB_CHAR '\t'
#define IC_COUNTER_DEFAULT_VALUE 100
#define DC_COUNTER_DEFAULT_VALUE 0

/* Registry */
#define REGISTRY_PREFIX 'r'
#define REGISTRY_MIN_LENGTH 2
#define MIN_REGISTRY_NUMBER 0
#define MAX_REGISTRY_NUMBER 7
#define REGISTRY_BITS_SIZE 4
#define ERA_BITS_SIZE 2

/* Mat */
#define MAT_OPEN_BRACKET '['
#define MAT_CLOSE_BRACKET ']'

/* Instruction consts */
#define INSTRUCTION_PREFIX '.'
#define ENTRY_INSTRUCTION_NAME "entry"
#define EXTERNAL_INSTRUCTION_NAME "extern"
#define MAT_INSTRUCTION_NAME "mat"
#define STRING_INSTRUCTION_NAME "string"
#define DATA_INSTRUCTION_NAME "data"

/* Boolean definition */
typedef int boolean;
#define TRUE 1
#define FALSE 0

/* Commands */

/* We have array of details about source and des operands so this holds the index in the array  */
#define SOURCE_OPERAND_ORDER 0
#define DES_OPERAND_ORDER 1
#define OPERAND_TYPE_BINARY_SIZE 2
#define COMMAND_ERA_DEFAULT_VALUE 0

/* Defaults */
#define MAT_DEFAULT_VALUE 0

/* Length */
/* 80 + '/n' */
#define LINE_MAX_LENGTH 80

typedef enum STATUS_CODE {
    OK = 0,
    ERROR = 1
} STATUS_CODE;

/* Macros Tables */
typedef struct MACRO_CONTENT {
    char *content;
    struct MACRO_CONTENT *next;
} MACRO_CONTENT;

typedef struct MACRO {
    char *name;
    MACRO_CONTENT *content;
    MACRO_CONTENT *last;
    struct MACRO *next;
} MACRO;


/* Global Utils */
/**
 * This function get string, and new string ti append
 * It return new string contains the two words together
 * @param base The base string
 * @param suffix The string to add to
 * @return The new string contains both strings
 */
char *add_suffix_to_string(char *base, char *suffix);

/**
 * This function remove the end symbols from string and replace it with regular end of string
 * @param str String to replace from
 */
void trim_newline(char *str);

/**
 * This function get decimal number and convert this to binary
 * We request the length, so if the binary length is grether we add leading zeros
 * @param decimal The decimal number to convert to binary
 * @param length The binary length
 * @return The new binary
 */
char *decimal_to_binary(int decimal, int length);

/**
 * This function coppy all chars from string until it arrive to none digit
 * It also updates the string params with the next address
 * @param str_ptr The pointer to string to copy from
 * @return The new number as int
 */
char *copy_next_number(char **str_ptr);

/**
 * Calculate the mat size (row * col)
 * If we get invalid syntax we return -1
 * It also updates the mat with the new address
 * @param mat The mat size as string
 * @return The actual mat size
 */
int get_mat_instruction_size(char **mat);

/**
 * This function update the r1, r2 var with the mat registries
 * IF the syntax is invalid this will return error status code
 * @param mat_ptr The mat size as string
 * @param r1_out The first registry to update
 * @param r2_out The second registry to update
 * @return The status code
 */
STATUS_CODE get_mat_registries(char **mat_ptr, int *r1_out, int *r2_out);

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
STATUS_CODE get_next_number_from_instruction_params(char **instruction_params, int *out_member, int line_number);

/**
 * This function extract the current symbol from the string
 * If the symbol name is invalid, it will return null
 * @param string_ptr Pointer to the string which contains the symbol
 * @param line_number The line number of the symbol
 * @return The symbol string
 */
char *get_current_symbol(char **string_ptr, int line_number);

/**
 * Extract the current command operand
 * We search until we arrive to end / empty char / comma
 * If the string is empty it will return null
 * @param str_ptr Pointer ot operands string
 * @return String contains only the operand itself
 */
char *get_next_command_operand(char **str_ptr);


/* instruction */
#define NUMBER_OF_COMMANDS 16

typedef enum {
    CODE,
    EXTERNAL,
    DATA
} SYMBOL_TYPE;

typedef struct BASE_TABLE {
    char *name;
    struct BASE_TABLE *next;
} BASE_TABLE;

typedef struct SYMBOL_TABLE {
    char *name;
    SYMBOL_TYPE type;
    int location;
    struct SYMBOL_TABLE *next;
} SYMBOL_TABLE;

typedef struct COMMAND_BINARY_LINE {
    int address;
    char *machine_code;
    int line_number;
    struct COMMAND_BINARY_LINE *next;
} COMMAND_BINARY_LINE;

typedef struct INSTRUCTION_BINARY_LINE {
    int address;
    char *machine_code;

    struct INSTRUCTION_BINARY_LINE *next;
} INSTRUCTION_BINARY_LINE;

/* Entry Instruction */
typedef struct ENTRY_INSTRUCTION {
    char *name;
    int address;
    int line_number;
    struct ENTRY_INSTRUCTION *next;
} ENTRY_INSTRUCTION;


/* External Instruction */
typedef struct EXTERNAL_INSTRUCTION {
    char *name;
    int address;

    struct EXTERNAL_INSTRUCTION *next;
} EXTERNAL_INSTRUCTION;

typedef struct ASSEMBLER_TABLES {
    MACRO *macro;
    SYMBOL_TABLE *symbol_table;
    COMMAND_BINARY_LINE *command_binary_line;
    INSTRUCTION_BINARY_LINE *instruction_binary_line;
    EXTERNAL_INSTRUCTION *external_instruction;
    ENTRY_INSTRUCTION *entry_instruction;

    int ic;
    int dc;
} ASSEMBLER_TABLES;

/********************************************************/
/* Tables Utils */
/********************************************************/

/* Instructions */
/**
 * This function creates new instruction
 * @param address The instruction address (dc almost)
 * @param machine_code The instruction binary machine code (e.g. 1010101010)
 * @return The new Instruction table
 */
INSTRUCTION_BINARY_LINE *create_instruction_binary_line(int address, char *machine_code);

/* Macros */
/**
 * Add new content line to existing macro
 * @param current_macro The macro to add the new line to
 * @param content The new line content
 */
void add_content_to_macro(MACRO *current_macro, char *content);


/**
 * This function get Macro table and search for a specific table by its name.
 * If it doesn't find any table, this will return NULL
 * @param head The macro table
 * @param name The table name which we want to find
 * @param length The string length to search in
 * @return The table with this name
 */
MACRO *find_macro_by_name(MACRO *head, const char *name, int length);

/**
 * Create new macro
 * @param macro_name The new macro name
 * @return The new Macro
 */
MACRO *create_macro(char *macro_name);

/* Symbols */
/**
 * This function get Symbols table and search for a specific one by its name.
 * If it doesn't find any table, this will return NULL
 * @param head The Symbols table
 * @param name The symbol name which we want to find
 * @return The symbol
 */
SYMBOL_TABLE *find_symbol_by_name(SYMBOL_TABLE *head, const char *name);

/**
 * Create new Symbol
 * @param name The new symbol name
 * @param type The symbol type (SYMBOL_TYPE)
 * @param location The symbol machine address
 * @return The new Symbol
 */
SYMBOL_TABLE *create_symbol(char *name, SYMBOL_TYPE type, int location);

/* Entries */
/**
 * Create new entry instruction
 * @param name The new entry instruction name
 * @param address the entry machine address
 * @param line_number The entry line number
 * @return The new Entry
 */
ENTRY_INSTRUCTION *create_entry_instruction(char *name, int address, int line_number);

/**
 * Find entry instruction by it name
 * @param name The new entry instruction name
 * @param head The head pointer of the entry instructions table
 * @return The Entry instruction
 */
ENTRY_INSTRUCTION *find_entry_instruction(ENTRY_INSTRUCTION *head, char *name);

/* Commands */

/**
 * This function creates new command binary address and machine code
 * @param address The command address (ic)
 * @param machine_code The command machine code (Binary representation)
 * @param line_number The machine code line number
 * @return The new command binary table
 */
COMMAND_BINARY_LINE *create_command_binary_line(int address, char *machine_code, int line_number);


/* Externals */
/**
 * This function creates new external instruction
 * @param name The external instruction name
 * @param address The external address
 * @return The new external instruction table
 */
EXTERNAL_INSTRUCTION *create_external_instruction(char *name, int address);


/**
 * This function validate all macros memories are free
 * @param head The head pointer to the macro
 */
void free_macros(MACRO *head);

/**
 * This functions frees all assembler tables
 * @param assembler_tables The assembler tables to free its address
 */
void free_assembler_tables(ASSEMBLER_TABLES *assembler_tables);


/* All operands types */
typedef enum {
    SIMPLE,
    SYMBOL,
    MAT,
    REGISTRY,
    UNDEFINED,
    INVALID_OPERAND
} OPERAND_TYPE;

/* Struct contains command info */
typedef struct {
    const char *name;
    const int command_number;
    const int num_of_operands;
    const OPERAND_TYPE allowed_operands[2][4];
    const int allowed_operand_number[2];
} COMMAND_INFO;

/**
 * Array contains all command indo
 */
extern const COMMAND_INFO commands[NUMBER_OF_COMMANDS];

/**
 * Find the command details
 * If it doesn't find command with this name, it will return null
 * @param name The command name
 * @return The command info
 */
const COMMAND_INFO *get_command_info_by_name(const char *name);

/* Assembler */
/* First Assembler functions */
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
                                                        int line_number);

/**
 * This calculates mat instruction type data machine codes
 * Every cell in the mat is specific machine code,
 * So our final machine codes will be row*col
 * @param mat_instruction_ptr Pointer to mat data
 * @param dc The dc count (It also update it with the new value)
 * @param line_number The mat line number in the assembly file
 * @return All mat instructions machine codes
 */
INSTRUCTION_BINARY_LINE *get_mat_machine_codes(char **mat_instruction_ptr, int *dc, int line_number);

/**
 * It gets regular command operand and try to find its type (Mat, symbol, ect...)
 * @param operand The operand string
 * @param line_number The line number of the operand
 * @return The operand type
 */
OPERAND_TYPE find_operand_type(const char *operand, int line_number);

/**
 * Calculate the string instruction machine codes
 * It adds zero to the string end (to indicate the end of the string)
 * It also update the string to the next word (skip the string itself)
 * @param str_ptr Pointer to the string
 * @param dc The dc counter
 * @return All string machine codes
 */
INSTRUCTION_BINARY_LINE *get_string_machine_codes(char **str_ptr, int *dc, int line_number);

/**
 * Get the data instruction number machine codes
 * If the data is invalid this function prints the error and return null
 * This also updates the string with the new address
 * @param str_ptr Pointer to data string contains the numbers
 * @param dc The dc counter
 * @param line_number The data instructions line number
 * @return The data instruction machine code
 */
INSTRUCTION_BINARY_LINE *get_data_machine_codes(char **str_ptr, int *dc, int line_number);

/**
 * This function get pointer to string, and skip all spaces and tabs
 * It updates the str with the new pointer position
 * @param str The string pointer to skip spaces
 */
void skip_empty_spaces(char **str);

/**
 * This function copy to new string the current symbol or command
 * IF it is invalid, null will be returned
 * @param str_ptr The string contains the command or symbol
 * @param line_mumber The line number of the command / symbol
 * @return The new string ot symbole
 */
char *coppy_next_command_or_symbol(char **str_ptr, int line_mumber);

/**
 * Calculate the operands binary codes
 * If it finds invalid operand will return null
 * @param operand The operand as string
 * @param ic The ic counter
 * @param operand_type The operand type
 * @param line_number The line which the operand exist
 * @return Tables of the binaries operands
 */
COMMAND_BINARY_LINE *extract_operand_binary(char *operand, int *ic, OPERAND_TYPE operand_type, int line_number);

/**
 * This function gets pointer to mat instruction,
 * It will return the mat symbol
 * @param input Pointer to the matb instruction
 * @return The mat symbol
 */
char *extract_mat_symbol(char **input);

/* Base4 chars */
static const char BASE_4_CHARS[] = {'a', 'b', 'c', 'd'};

/**
 * This function get positive decimal number,
 * It will return it base4 value
 * @param value The decimal number
 * @return The base4 number
 */
char *decimal_to_base4(int value);

/**
 * This function gets even binary number length
 * and return its base number
 * @param binary_str The binary string (in even length)
 * @return The base4 number
 */
char *binary_to_base4(char *binary_str);

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
STATUS_CODE pre_assembler(char *filename, ASSEMBLER_TABLES *assembler_tables);


/**
 * This function validates the macro has a valid name
 * for rxample it doesn't save word
 * @param macro_name
 * @param line_number
 * @return Is the macro valid
 */
boolean validate_macro_name(char *macro_name, int line_number);

/**
 * This funvtion get line contains macro, and return its name
 * @param line Pointer to macro line
 * @param line_number The line number of the macro
 * @return Teh macro name
 */
char *extract_macro_name(char **line, int line_number);

/**
 * The first assembler!
 * In this assembler we create commands, instructions, entries, external abd symbol table
 * Notice we don't insert all address, because we will know them only in the second assembler
 * After we calculate all symbols
 * @param filename The assembler file name
 * @param assembler_tables All Assembler tables (include the macro itself)
 * @return The status code of the pre assembler action
 */
STATUS_CODE first_assembler(char *filename, ASSEMBLER_TABLES *assembler_tables);

/**
 * In the second assembly we mainly do three things:
 * 1. Update all symbols address
 * 2. Update all entry with their machine code
 * 3. Update externals with their machine code use
 * @param assembler_tables All Assembler tables (include the macro itself)
 * @return The status code of the pre assembler action
 */
STATUS_CODE second_assembler(ASSEMBLER_TABLES *assembler_tables);

/* Operands Utils */
/**
 * Check if the operand of the command is correct and allowed
 * @param type The operand type
 * @param command_info All commands info details
* @param opernad_order Can be one of SOURCE_OPERAND_ORDER / DES_OPERAND_ORDER
 * @return If this is valid operand
 */
boolean is_valid_operand_type(OPERAND_TYPE type, const COMMAND_INFO *command_info, int opernad_order);

/**
 * This function writes all assembler files
 * Include object, external and entry
 * @param filename The filename of the assembly file
 * @param assembler_tables The assembler tables
 */
void write_assembler_files(char *filename, ASSEMBLER_TABLES *assembler_tables);

/**
 * This function return the length until the next space or tab
 * @param str The string to search in
 * @return The word length
 */
int get_word_length_until_space(char *str);
