#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HEDER.h"

/**
 * @brief Initializes a LineInfo struct with default values.
 *
 * This function sets default values for all fields in a LineInfo struct.
 * String fields are initialized to empty strings, integer and boolean fields 
 * are initialized to zero or false, and certain fields are set to specific 
 * default values, such as `-1` for invalid or uninitialized states.
 *
 * @param lineInfo A pointer to the `LineInfo` structure to be initialized.
 */
void initializeLineInfo(LineInfo *lineInfo) {
    /* Set all string fields to empty strings */
    strcpy(lineInfo->label_name, "");
    strcpy(lineInfo->opcode_name, "");
    strcpy(lineInfo->source_method_value, "");
    strcpy(lineInfo->destination_method_value, "");
    strcpy(lineInfo->data_string_value, "");

    /* Initialize all integer and boolean fields to zero or false */
    lineInfo->source_method = -1;
    lineInfo->destination_method = -1;
    lineInfo->memory_cells = 0;
    lineInfo->memory_value = 100;
    lineInfo->opcode_value = -1; /* Invalid value as default */
    lineInfo->is_data = false;
    lineInfo->is_string = false;
    lineInfo->is_entry = false;
    lineInfo->is_extern = false;
    lineInfo->count_op = -1;
    lineInfo->flag = 0;
}

/**
 * @brief Maps an opcode name to its corresponding numeric value.
 *
 * This function takes an opcode name as a string and returns the corresponding numeric value.
 * It checks the opcode name against a list of known opcodes and returns the appropriate value.
 * If the opcode name is not recognized, the function returns -1.
 *
 * @param opcode_name A string representing the name of the opcode.
 * @return The numeric value of the opcode if found, otherwise -1 for an invalid opcode.
 */
int getOpcodeValue(char *opcode_name) {
    if (strcmp(opcode_name, "mov") == 0) return 0;
    if (strcmp(opcode_name, "cmp") == 0) return 1;
    if (strcmp(opcode_name, "add") == 0) return 2;
    if (strcmp(opcode_name, "sub") == 0) return 3;
    if (strcmp(opcode_name, "lea") == 0) return 4;
    if (strcmp(opcode_name, "clr") == 0) return 5;
    if (strcmp(opcode_name, "not") == 0) return 6;
    if (strcmp(opcode_name, "inc") == 0) return 7;
    if (strcmp(opcode_name, "dec") == 0) return 8;
    if (strcmp(opcode_name, "jmp") == 0) return 9;
    if (strcmp(opcode_name, "bne") == 0) return 10;
    if (strcmp(opcode_name, "red") == 0) return 11;
    if (strcmp(opcode_name, "prn") == 0) return 12;
    if (strcmp(opcode_name, "jsr") == 0) return 13;
    if (strcmp(opcode_name, "rts") == 0) return 14;
    if (strcmp(opcode_name, "stop") == 0) return 15;
    return -1; /* Invalid opcode */
}

/**
 * @brief Calculates the number of data elements in a comma-separated string.
 *
 * This function takes a comma-separated string and counts the number of data elements 
 * by counting the commas and adding one. It returns the total number of elements found.
 *
 * @param str A string containing comma-separated values.
 * @return The number of data elements in the string.
 */
int calcData(char *str){
    int count = 1;
    while (*str) {
        if (*str == ',') {
            count++;
        }
        str++;
    }
    return count;
}

/**
 * @brief Calculates the length of a string including the null terminator.
 *
 * This function takes a string and counts the number of characters, including the null terminator.
 * It starts counting from 1 to account for the null terminator at the end of the string.
 *
 * @param str A pointer to the string whose length is to be calculated.
 * @return The length of the string including the null terminator.
 */
int calcString(char *str){
    int count = 1; /*we start at 1 becuse the null line at the end of the string*/
    while (*str != '\0') {
        count++;
        str++;
    }
    return count;
}

/**
 * @brief Calculates the number of memory cells required by a given line of code.
 *
 * This function determines the number of memory cells used by a line based on its type.
 * It accounts for data lines, string lines, and operation lines, adjusting the count 
 * based on the addressing methods used. For data lines, it delegates to `calcData`, and 
 * for string lines, it delegates to `calcString`.
 *
 * @param lineInfo A pointer to a `LineInfo` structure containing information about the line.
 * @return The number of memory cells required by the line.
 */
int calculateMemoryCells(LineInfo *lineInfo) {
    if (lineInfo->is_data){
        return calcData(lineInfo->data_string_value);
    }else if (lineInfo->is_string) {
        return calcString(lineInfo->data_string_value);
    }
    else if(!lineInfo->is_extern && !lineInfo->is_entry){
        if (lineInfo->source_method == -1 && lineInfo->destination_method == -1){
            return 1;
        }else if (lineInfo->source_method == -1){
            return 2;
        }else if ((lineInfo->destination_method == DIRECT_REGISTER || lineInfo->destination_method == INDIRECT_REGISTER) &&
                (lineInfo->source_method == DIRECT_REGISTER || lineInfo->source_method == INDIRECT_REGISTER)){
            return 2;
        }else{
            return 3;
        }
    }
    return 0; /* Return a default value in case none of the conditions are met */
}

/**
 *@brief check_label_validity Function to check label validity and print appropriate message
 *
 *@param line Pointer on the 'i' line in the struct
 *
 */

int badLabel(char *token) {
    int i;
    const char *registers[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};        
    /* Check if the first character is an English letter */
    if (!isalpha(token[0])) {
        printf("Invalid label: %s (must start with a letter)\n", token);
        return 1;
    }

    /* Check if the remaining characters are letters or digits */
    for (i = 1; token[i] != '\0'; i++) {
        if (!isalnum(token[i])) {
            printf("Invalid label: %s (must contain only letters and digits)\n", token);
            return 1;
        }
    }

    /* Check if the label matches one of the register names */
    for (i = 0; i < 8; i++) {
        if (strcmp(token, registers[i]) == 0) {
            printf("Invalid label: %s (cannot be a register name)\n", token);
            return 1;
        }
    }
    /* If all checks pass, the label is valid */
    return 0;
}

/**
 * @brief Processes a single line of assembly code and populates the LineInfo structure.
 *
 * This function parses a line of assembly code, extracting the label (if present), opcode, and operands.
 * It determines whether the line is a directive (.data, .string, .entry, .extern) or an instruction,
 * and populates the corresponding fields in the LineInfo struct.
 *
 * @param line A string containing the line of assembly code to process.
 * @param lineInfo A pointer to a LineInfo struct that will be populated with the parsed information.
 */
void processLine(char *line, LineInfo *lineInfo) {
    char *token;
    char *operands = NULL;
    char *data_token; 
    char *string_token; 
    char *entry_token; 
    char *extern_token;
    char *src_operand; 
    char *dest_operand; 
    char *error_operand;

    /* Initialize the LineInfo structure */
    initializeLineInfo(lineInfo);

    /* Check for label: a token ending with ':' */
    token = strtok(line, " \r\t\n");
    
    if (token != NULL && token[strlen(token) - 1] == ':') {
        token[strlen(token) - 1] = '\0'; /* Remove the colon */
        if (badLabel(token)==1) {
            printf("ERR: label %s not legal\n", token);
            lineInfo->flag = true;
            return;
        }
        strcpy(lineInfo->label_name, token);
        token = strtok(NULL, " \t"); /* Move to the next token */
    }

    /* Process opcode or directive */
    if (token) {
        token[strcspn(token, "\r\t\n")] = '\0'; /* Canceling extra trims or spaces from token */

        strcpy(lineInfo->opcode_name, token);
        lineInfo->opcode_value = getOpcodeValue(token);

        /* Handle special directives */
        if (lineInfo->opcode_value == -1) {
            if (strcmp(token, ".data") == 0) {
                lineInfo->is_data = true;
                /* Process subsequent tokens as data values */
                data_token = strtok(NULL, "\n");
                if (data_token) {
                    strcpy(lineInfo->data_string_value, data_token);
                }
		        strcpy(lineInfo->opcode_name, ""); /* Set opcode name to NULL */

            } else if (strcmp(token, ".string") == 0) {
                lineInfo->is_string = true;
                /* Process subsequent tokens as string values */
                string_token = strtok(NULL, "\n");
                if (string_token) {
                    size_t len = strlen(string_token);

                    /* Check if the string starts and ends with a double quote */
                    if (string_token[0] == '"' && string_token[len - 1] == '"') {
                        string_token[len - 1] = '\0';  /* Remove the closing quote */
                        string_token++;  /* Skip the opening quote */
                    }

                    strcpy(lineInfo->data_string_value, string_token);
                }
                strcpy(lineInfo->opcode_name, ""); /* Set opcode name to NULL */
                
            } else if (strcmp(token, ".entry") == 0) {
                lineInfo->is_entry = true;

                /* Process subsequent tokens as entry values */
                entry_token = strtok(NULL, "\n");
                if (entry_token) {
                    strcpy(lineInfo->data_string_value, entry_token);
                }
                strcpy(lineInfo->opcode_name, ""); /* Set opcode name to NULL */

            } else if (strcmp(token, ".extern") == 0) {
                lineInfo->is_extern = true;
                /* Process subsequent tokens as extern values */
                extern_token = strtok(NULL, "\r\t\n");
                if (extern_token) {
                    strcpy(lineInfo->data_string_value, extern_token);
                }
                strcpy(lineInfo->opcode_name, ""); /* Set opcode name to NULL */

            }

        } else {
            /* If it's a valid opcode, process operands */
            operands = strtok(NULL, "\n");

            if (operands) {
                src_operand = strtok(operands, " , ");
                if (src_operand) {
                    parseMethod(src_operand, &lineInfo->source_method, lineInfo->source_method_value);
                }
                dest_operand = strtok(NULL, " , ");
                if (dest_operand) {
                    parseMethod(dest_operand, &lineInfo->destination_method, lineInfo->destination_method_value);
                    lineInfo->count_op = 2;
                    error_operand = strtok(NULL, " , ");
                    if (error_operand) {
                        printf("ERR: there are too many operands\n");
                        lineInfo->flag = true;
                        return;
                    }
                } else if (!dest_operand && src_operand) {
                    /* If there's only one operand and no second operand put at destination blocks */
                    parseMethod(src_operand, &lineInfo->destination_method, lineInfo->destination_method_value);
                    lineInfo->source_method = -1;
                    strcpy(lineInfo->source_method_value, "");
                    lineInfo->count_op = 1;
                }
            }
        }
        lineInfo->memory_cells = calculateMemoryCells(lineInfo);
    }
}

/**
 * @brief Parses an addressing method and assigns the appropriate method type and value.
 *
 * This function determines the addressing method based on the first character of the operand string 
 * and assigns the corresponding method type to the provided pointer. It also copies the operand value 
 * into the provided value string.
 *
 * @param method_name A string representing the operand, which could be an immediate value, register, 
 * indirect register, or direct address.
 * @param method A pointer to an integer where the method type will be stored (0 for immediate, 1 for direct, 
 * 2 for indirect register, 3 for register).
 * @param value A string where the operand's value will be copied.
 */
void parseMethod(const char *method_name, int *method, char *value) {
    if (method_name[0] == '#') {
        *method = 0;  /* Immediate method */
        strcpy(value, method_name);  /* Copy the full value, including the minus sign */
    } else if (method_name[0] == 'r') {
        *method = 3;  /* Register method */
        strcpy(value, method_name);
    } else if (method_name[0] == '*') {
        *method = 2;  /* Indirect register method */
        strcpy(value, method_name);
    } else {
        *method = 1;  /* Direct method */
        strcpy(value, method_name);
    }
}

/**
 * @brief Processes the input assembly file and fills an array of LineInfo structures.
 *
 * This function reads each line from the provided file, processes it to extract relevant information, 
 * and fills the corresponding LineInfo struct. It also assigns memory addresses to each line, 
 * and handles `.entry` and `.extern` directives by marking the corresponding lines accordingly.
 *
 * @param file A pointer to the input file to be processed.
 * @param lines An array of `LineInfo` structures that will be populated with the parsed data from the file.
 * @param line_count A pointer to an integer where the total number of processed lines will be stored.
 */
void processInputFile(FILE *file, LineInfo *lines, int *line_count) {
    char line[MAX_LINE_LENGTH];
    int current_address = 100; /* starting point address in memory */
    int k, j;

    *line_count = 0;

    /* Read each line from the file */
    while (fgets(line, sizeof(line), file)) {
        /* Initialize the current LineInfo struct */
        initializeLineInfo(&lines[*line_count]);

        /* Process the line and fill the struct */
        processLine(line, &lines[*line_count]);

        /* Set the memory address for this line */
        lines[*line_count].memory_value = current_address;

        /* Increment the current address by the number of memory cells used by this line */
        current_address += lines[*line_count].memory_cells;

        /* Increment the line count */
        (*line_count)++;
    }

    for (k = 0; k < *line_count; k++) {
        if (lines[k].is_entry && lines[k].opcode_value == -1) { /*if entry statement*/
            for (j = 0; j < *line_count; j++) {
                if (strcmp(lines[k].data_string_value, lines[j].label_name) == 0) {
                    lines[j].is_entry = 1;
                }
            }
        } 
        else if (lines[k].is_extern && lines[k].opcode_value == -1) { /*if extern statement*/
            for (j = 0; j < *line_count; j++) {
                if (strcmp(lines[k].data_string_value, lines[j].label_name) == 0) {
                    if (lines[j].is_entry == 1) { /*return error if label is entry and extern*/
                        printf("ERR: label '%s' is stated entry and extern\n", lines[j].label_name);
                        lines[j].flag = true;
                        return; /*stopping the code because of a non-handled input*/
                    }
                    lines[j].is_extern = 1;
                }
            }
        }
    }
}

/**
 * @brief Performs the first pass on an assembly source file, generating an intermediate file and preparing for the second pass.
 *
 * This function processes the input assembly file during the first pass. It reads and parses the file, 
 * filling a LineInfo struct with relevant information. The function then generates an 
 * intermediate file (.afp) that includes a detailed breakdown of each line, such as labels, opcodes, 
 * memory addresses, and directive flags. After generating the .afp file, the function initiates the 
 * second pass by calling `secondPass`.
 *
 * @param name_of_file The name of the input assembly file to be processed.
 * @param lines A LineInfo struct that will be populated during the first pass.
 * @param line_count The initial count of lines, which will be updated during processing.
 * @return 0 on success, or 1 if an error occurs.
 */
int firstPass(char *name_of_file, LineInfo *lines, int line_count) {
    FILE *file;
    int j;
    char *dot_pos;
    char output_filename[80];
    FILE *outputFile;

    if (!name_of_file)
        return 1;

    file = fopen(name_of_file, "r");
    if (!file) {
        perror("ERR: Error opening file");
        return 1;
    }

    processInputFile(file, lines, &line_count);
    fclose(file);

    strcpy(output_filename, name_of_file);
    dot_pos = strrchr(output_filename, '.');
    if (dot_pos && strcmp(dot_pos, ".am") == 0) {
        strcpy(dot_pos, ".afp");
    } else {
        printf("ERR: no '.am' file to proceed");
        return 1;
    }

    outputFile = fopen(output_filename, "a"); /* Open in append mode */
    if (!outputFile) {
        perror("ERR: Error creating output file");
        return 1;
    }

    fprintf(outputFile, "File: %s\n", name_of_file);
    fprintf(outputFile, "----------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    fprintf(outputFile, "| %-22s | %-22s | %-22s | %-22s | %-22s | %-22s | %-22s | %-22s | %-22s | %-22s | %-22s | %-22s | %-22s | %-22s | %-22s|%-22s\n", 
            "Line", "Label", "Opcode", "Opcode Value", "Source Method", "Source Method Value", 
            "Destination Method", "Destination Method Value","count op", "Mem. Cells", "Mem. Value", "Is Data", 
            "Is String", "Is Entry", "Is Extern", "Data/String Value");
    fprintf(outputFile, "----------------------------------------------------------------------------------------------------------------------------------------------------------\n");

    for (j = 0; j < line_count; j++) {
        fprintf(outputFile, "| %-22d | %-22s | %-22s | %-22d | %-22d | %-22s | %-22d | %-22s | %-22d | %-22d | %-22d | %-22d | %-22d | %-22d | %-22d|%-22s\n",
                j + 1, lines[j].label_name, lines[j].opcode_name, lines[j].opcode_value,
                lines[j].source_method, lines[j].source_method_value, lines[j].destination_method,
                lines[j].destination_method_value, lines[j].count_op, lines[j].memory_cells, lines[j].memory_value,
                lines[j].is_data, lines[j].is_string, lines[j].is_entry, lines[j].is_extern,
                lines[j].data_string_value);
    }
    fprintf(outputFile, "----------------------------------------------------------------------------------------------------------------------------------------------------------\n\n");

    fclose(outputFile);

    dot_pos = strrchr(output_filename, '.');
    if (dot_pos) {
        strcpy(dot_pos, ".afp");
    }

    /* Call secondPass */
    if (secondPass(output_filename, lines, line_count) == 1) {
        printf("ERR: Error at second pass processing\n");
        return 1;
    }
    return 0;
}


