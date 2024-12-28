#include "HEDER.h"
/**
 * @brief Gets the legal number of operands for a given opcode.
 * @return The number of operands that the given opcode requires.
 */
int getOperandCount(int opcode_value) {
    static const int operand_counts[] = {2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0};
    return operand_counts[opcode_value];
}

/**
 * @brief Converts an integer to a binary string representation.
 *
 * This function takes an integer, masks it to fit within 15 bits,
 * and then converts it to a binary string representation. The function 
 * allocates memory for the binary string, which the caller must free!!!
 * 
 * @param num The integer to be converted to binary. Only the lower 15 bits are used.
 * @return A pointer to a dynamically allocated string containing the binary representation of the number.
 *         The caller is responsible for freeing this memory.
 */
char* printBinary(int num) {
    int i;
    char *binary_str = (char *)malloc(BITS + 1);
    num = num & 0x7FFF;
    if (binary_str == NULL) {
        perror("ERR: Unable to allocate memory for binary string");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < BITS; i++) {
        binary_str[i] = (num & (1 << (BITS - 1 - i))) ? '1' : '0';
    }
    binary_str[BITS] = '\0'; 
    return binary_str;
}

/**
 * @brief Takes the structure table "lines" after the first pass and converts it to proper files.
 * @param name_of_file The name of the file.
 * @param lines The structure of data after the first pass.
 * @param line_count The number of lines.
 * @return 0 if the proccess succeded and 1 otherwise
 */
int secondPass(char *name_of_file ,LineInfo *lines, int line_count){
    char output_filename[MAX_LABEL_LENGTH]; 
    char *dot_pos;

    strcpy(output_filename, name_of_file);

    dot_pos = strrchr(output_filename, '.');
    if (dot_pos && strcmp(dot_pos, ".afp") == 0) {
        strcpy(dot_pos, ".asp");
    } else {
        printf("ERR: no '.afp' file to proceed");
        return 1;
    }

    generateOutput(lines, line_count, output_filename);

    return 0;
}

/**
 * @brief Finds the memory value of a given label.
 *
 * This function searches through a LineInfo struct to find a 
 * specific label. If the label is found, it returns the associated memory value.
 * 
 * @param lines is a LineInfo struct that contains label names and memory values.
 * @param numLines The number of elements in the lines struct.
 * @param label The label name to search for in the lines struct.
 * @return The memory value associated with the label if found, otherwise returns -1.
 */
int findLabelMemory(LineInfo lines[], int numLines, char *label){
    int i;
    for (i = 0; i < numLines; i++) {   
        if(strcmp(lines[i].label_name, label) == 0) {
            return lines[i].memory_value;
        }
    }
    return -1;
}

/**
 * @brief Finds and formats the address associated with a given label.
 *
 * This function searches through a LineInfo struct to find a 
 * specific label and formats its memory address. The function 
 * also checks if the label is an entry or external and formats the address accordingly.
 * 
 * @param lines is a LineInfo struct that contains label names and memory values.
 * @param numLines The number of elements in the lines array.
 * @param label The label name to search for in the lines array.
 * @return The formatted address associated with the label, or 1 if the label is external and not found in the array.
 */
int findLabelAddress(LineInfo lines[], int numLines, char *label){
    int word = -1;
    int i;
    if(isExtern(lines, numLines, label) == 0) {
         return 1;
    }
    
    for (i = 0; i < numLines; i++) {   
        if(strcmp(lines[i].label_name, label) == 0) { 
            word = 0;
            word |= ((lines[i].memory_value) << 3);
            word |= (1 << 1);
        }
    }
    
    return word;
}

/**
 * @brief Generates the output files based on the parsed lines of assembly code.
 *
 * This function processes a LineInfo struct, generating the binary output 
 * for each line and writing it to a specified output file. It handles various types of lines, 
 * including data lines, string lines, entry lines, extern lines and operation lines.
 *
 * @param lines a LineInfo struct that contains the parsed assembly lines.
 * @param numLines The number of elements in the lines struct.
 * @param filename The name of the output file where the binary data will be written.
 */
void generateOutput(LineInfo lines[], int numLines, const char *filename) {
    int output[MAX_LINES] = {0};
    int outputIndex = MIN_MEM_VAL;
    int ic = 0, dc = 0;
    int value;
    char *c;
    char *token;
    char *temp;
    int word;
    int address;
    int regWord;
    char *binary_str;
    int i, k;
    FILE *file;
    
    for (i = 0; i < numLines; i++) {
        LineInfo line = lines[i];

        if (line.is_data) {
            token = strtok(line.data_string_value, ",");
            token[strcspn(token, "\r\t\n")] = '\0';
            while (token) {
                temp = token;
                if (*temp == '+' || *temp == '-') {
                    temp++; }
                while (*temp) {
                    if (!isdigit(*temp)) {
                        printf("ERR: '%s' is not a valid data value\n", token);
                        lines[i].flag = true;
			break ;
                    }
		    else {
                    temp++; }
                }
                value = atoi(token);
                if (outputIndex < MAX_LINES) {
                    output[outputIndex++] = (value & 0x7FFF);
                    dc++;
                }
                token = strtok(NULL, ",\t ");
            }
        } else if (line.is_string) {
            for (c = line.data_string_value; *c; c++) {
                if (outputIndex < MAX_LINES) {
                    output[outputIndex++] = (*c & 0x7FFF);
                    dc++;
                }
            }
            if (outputIndex < MAX_LINES) {
                output[outputIndex++] = 0;
                dc++;
            }

        } else if (line.opcode_value != -1) {
            if (isGoodLine(line)==1){
                lines[i].flag=true;
                printf("ERR: the '%s' op code in line %d and method combination is not valid\n",line.opcode_name, i);
            }
            
            word = (line.opcode_value << 11);

            if (line.source_method == IMMEDIATE) {
                word |= (1 << 7);
            } else if (line.source_method == DIRECT) {
                word |= (1 << 8);
            } else if (line.source_method == INDIRECT_REGISTER) {
                word |= (1 << 9);
            } else if (line.source_method == DIRECT_REGISTER) {
                word |= (1 << 10);
            }

            if (line.destination_method == IMMEDIATE) {
                word |= (1 << 3);
            } else if (line.destination_method == DIRECT) {
                word |= (1 << 4);
            } else if (line.destination_method == INDIRECT_REGISTER) {
                word |= (1 << 5);
            } else if (line.destination_method == DIRECT_REGISTER) {
                word |= (1 << 6);
            }

            word |= (1 << 2);

            if (outputIndex < MAX_LINES) {
                output[outputIndex++] = word;
                ic++;
            }
            word = 0;

            if (line.source_method == IMMEDIATE) {
                value = atoi(line.source_method_value + 1);
                if (outputIndex < MAX_LINES) {
                    word = (value << 3);
                    word |= (1 << 2);
                    output[outputIndex++] = word;
                    ic++;
                }
            } else if (line.source_method == DIRECT) {
                address = findLabelAddress(lines, numLines, line.source_method_value);
                if (address == -1) {
                    printf("ERR: the label %s wasn't found\n", line.source_method_value);
                    lines[i].flag = true;
                }
                if (outputIndex < MAX_LINES) {
                    output[outputIndex++] = address;
                    ic++;
                }
            } else if ((line.destination_method == DIRECT_REGISTER || line.destination_method == INDIRECT_REGISTER) &&
                       (line.source_method == DIRECT_REGISTER || line.source_method == INDIRECT_REGISTER)) {
                regWord = 0;
                if (line.source_method == INDIRECT_REGISTER) {
                    regWord |= ((line.source_method_value[2] - '0') << 6);
                } else if (line.source_method == DIRECT_REGISTER) {
                    regWord |= ((line.source_method_value[1] - '0') << 6);
                }

                if (line.destination_method == INDIRECT_REGISTER) {
                    regWord |= ((line.destination_method_value[2] - '0') << 3);
                } else if (line.destination_method == DIRECT_REGISTER) {
                    regWord |= ((line.destination_method_value[1] - '0') << 3);
                }

                regWord |= (1 << 2);
                if (outputIndex < MAX_LINES) {
                    output[outputIndex++] = regWord;
                    ic++;
                }
            } else if (line.source_method == INDIRECT_REGISTER) {
                regWord = 0;
                regWord |= ((line.source_method_value[2] - '0') << 6);
                regWord |= (1 << 2);
                if (outputIndex < MAX_LINES) {
                    output[outputIndex++] = regWord;
                    ic++;
                }
            } else if (line.source_method == DIRECT_REGISTER) {
                regWord = 0;
                regWord |= ((line.source_method_value[1] - '0') << 6);
                regWord |= (1 << 2);
                if (outputIndex < MAX_LINES) {
                    output[outputIndex++] = regWord;
                    ic++;
                }
            }

            if (line.destination_method == INDIRECT_REGISTER && (line.source_method != INDIRECT_REGISTER &&
                                                                 line.source_method != DIRECT_REGISTER)) {
                regWord = 0;
                regWord |= ((line.destination_method_value[2] - '0') << 3);
                regWord |= (1 << 2);
                if (outputIndex < MAX_LINES) {
                    output[outputIndex++] = regWord;
                    ic++;
                }
            } else if (line.destination_method == DIRECT_REGISTER && (line.source_method != INDIRECT_REGISTER &&
                                                                      line.source_method != DIRECT_REGISTER)) {
                regWord = 0;
                regWord |= ((line.destination_method_value[1] - '0') << 3);
                regWord |= (1 << 2);
                if (outputIndex < MAX_LINES) {
                    output[outputIndex++] = regWord;
                    ic++;
                }
            }
            if (line.destination_method == IMMEDIATE) {
                value = atoi(line.destination_method_value + 1);
                if (outputIndex < MAX_LINES) {
                    word = (value << 3);
                    word |= (1 << 2);
                    output[outputIndex++] = word;
                    ic++;
                }
            } else if (line.destination_method == DIRECT) {
                address = findLabelAddress(lines, numLines, line.destination_method_value);
                if (address == -1) {
                    printf("ERR: the label %s wasn't found\n", line.destination_method_value);
                    lines[i].flag = true;
                }
                if (outputIndex < MAX_LINES) {
                    output[outputIndex++] = address;
                    ic++;
                }
            }
        }
    }

    file = fopen(filename, "w");
    if (!file) {
        perror("ERR: Failed to open file");
        return;
    }
    for (k = MIN_MEM_VAL; k < outputIndex; k++) {
        binary_str = printBinary(output[k]);
        fprintf(file, "%s\n", binary_str);
        free(binary_str);
        binary_str = NULL;
    }
    fclose(file);

    if (isFlag(lines, numLines) == false) {
        makeOb(output, filename, dc, ic);
        makeExt(lines, numLines, (char *)filename);
        makeEnt(lines, numLines, (char *)filename);
    } else {
        printf("We didnt make the files (ob/ext/ent) becuse you have errors\n");
    }
}

/**
 * @brief a function that checks if the opcode and operand are valid
 * @param lines gets line info structure
 * @return true if the line is not valid and false otherwise
 */
int isGoodLine(LineInfo line){
    int opNum=line.opcode_value;
    if ((opNum==0 || opNum==2 || opNum==3) && line.destination_method==0){
        return true;
    }
    if (opNum==4  && (line.source_method!=1 || line.destination_method==0)){
        return true;
    }
    if (((opNum>=5  && opNum<=8) || opNum==11) && (line.source_method!=-1 || line.destination_method==0)){
        return true;
    }
    if ((opNum==9 || opNum==10 || opNum==13) && (line.source_method!=-1 || line.destination_method==0 || line.destination_method==3)){
        return true;
    }
    if ((opNum==14 || opNum==15 ) && (line.source_method!=-1 || line.destination_method!=-1)){
        return true;
    }
    if ((opNum==12) && (line.source_method!=-1 )){
        return true;
    }
    return false;
}

/**
 * @brief a function that checks if there is error lines int the process
 * @param lines gets line info structure
 * @param numLines gets the number of lines
 * @return false if there is no errors an true if there is errors
 */
int isFlag(LineInfo lines[], int numLines){
    int i;
    for (i = 0; i < numLines; i++) {
        if (lines[i].flag == true) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Generates the object file (.ob) based on the machine code array.
 *
 * This function creates an object file with the extension ".ob" based on the provided filename.
 * It processes the input filename to replace its extension with ".ob" and prepares the object file for writing.
 *
 * @param machine An array of integers representing the machine code.
 * @param filename The original filename to which the ".ob" extension will be applied.
 * @param dc The data counter representing the amount of data stored.
 * @param ic The instruction counter representing the number of instructions processed.
 */
void makeOb(int machine[], const char *filename, int dc, int ic){
    char *dot_pos;
    int k;
    FILE *file;
    char *object_file_name = (char *)malloc(strlen(filename) + 4);

    if (object_file_name == NULL) {
        perror("ERR: Unable to allocate memory for object file name");
        exit(EXIT_FAILURE);
    }

    strcpy(object_file_name, filename);
    dot_pos = strrchr(object_file_name, '.');
    if (dot_pos) {
        strcpy(dot_pos, ".ob");
    } else {
        printf("ERR: no .asp file to proceed\n");
        free(object_file_name);
        return;
    }

    file = fopen(object_file_name, "w");
    if (!file) {
        perror("ERR: Failed to open file");
        free(object_file_name);
        return;
    }

    fprintf(file, "%d %d\n", ic, dc);
    for (k = MIN_MEM_VAL; k < MIN_MEM_VAL + ic + dc; k++) {
        fprintf(file, "%04d %05o\n", k, ((machine[k]) & 077777));
    }
    fclose(file);
    free(object_file_name);
}

/**
 * @brief Checks if a given label is external.
 *
 * This function iterates through a LineInfo struct to determine 
 * if a specific label is marked as external. It returns 0 if the label is found 
 * and is external, otherwise, it returns 1.
 *
 * @param lines of LineInfo struct containing the parsed assembly lines.
 * @param num_of_lines The number of elements in the `lines` array.
 * @param label The label name to check if it is external.
 * @return 0 if the label is external, otherwise 1.
 */
void makeExt(LineInfo *lines, int num_of_lines, char *filename){
    int flag = 0;
    int i;
    int k;
    char *dot_pos;
    FILE *file;
    char *extern_file_name;

    for (i = 0; i < num_of_lines; i++) {
        if (lines[i].is_extern) {
            flag = 1;
        }
    }
    if (flag == 0) {
        return;
    }

    extern_file_name = (char *)malloc(strlen(filename) + 5);

    if (extern_file_name == NULL) {
        perror("ERR: Unable to allocate memory for extern file name");
        exit(EXIT_FAILURE);
    }

    strcpy(extern_file_name, filename);
    dot_pos = strrchr(extern_file_name, '.');
    if (dot_pos) {
        strcpy(dot_pos, ".ext");
    } else {
        printf("ERR: no .asp file to proceed\n");
        free(extern_file_name);
        return;
    }

    file = fopen(extern_file_name, "w");
    if (!file) {
        perror("ERR: Failed to open file");
        free(extern_file_name);
        return;
    }
    for (k = 0; k < num_of_lines; k++) {
        if (isExtern(lines, num_of_lines, lines[k].source_method_value) == 0) {
            fprintf(file, "%s %d\n", lines[k].source_method_value, lines[k].memory_value + 1);
        }
        if (isExtern(lines, num_of_lines, lines[k].destination_method_value) == 0) {
            if (lines[k].source_method == -1) {
                fprintf(file, "%s %d\n", lines[k].destination_method_value, lines[k].memory_value + 1);
            } else {
                fprintf(file, "%s %d\n", lines[k].destination_method_value, lines[k].memory_value + 2);
            }
        }
    }
    fclose(file);
    free(extern_file_name);
}

/**
 * @brief Generates the entry file (.ent) based on the parsed assembly lines.
 *
 * This function creates an entry file with the extension ".ent" based on the provided filename.
 * It processes the input filename to replace its extension with ".ent" and writes all the labels
 * marked as entry points along with their memory addresses into the entry file.
 *
 * @param lines a LineInfo struct containing the parsed assembly lines.
 * @param num_of_lines The number of elements in the lines struct.
 * @param filename The original filename to which the ".ent" extension will be applied.
 */
void makeEnt(LineInfo *lines, int num_of_lines, char *filename){
    int flag = 0;
    int i, k;
    char *dot_pos;
    char *entry_file_name;
    FILE *file;

    for (i = 0; i < num_of_lines; i++) {
        if (lines[i].is_entry) {
            flag = 1;
        }
    }
    if (flag == 0) {
        return;
    }

    entry_file_name = (char *)malloc(strlen(filename) + 5);

    if (entry_file_name == NULL) {
        perror("ERR: Unable to allocate memory for entry file name");
        exit(EXIT_FAILURE);
    }

    strcpy(entry_file_name, filename);
    dot_pos = strrchr(entry_file_name, '.');
    if (dot_pos) {
        strcpy(dot_pos, ".ent");
    } else {
        printf("ERR: no .asp file to proceed\n");
        free(entry_file_name);
        return;
    }

    file = fopen(entry_file_name, "w");
    if (!file) {
        perror("ERR: Failed to open file");
        free(entry_file_name);
        return;
    }
    for (k = 0; k < num_of_lines; k++) {
        if (strcmp(lines[k].label_name, "") != 0 && lines[k].is_entry) {
            fprintf(file, "%s %d\n", lines[k].label_name, lines[k].memory_value);
        }
    }
    fclose(file);
    free(entry_file_name);
}

int isExtern(LineInfo *lines, int num_of_lines, char *label){
    int i;
    for (i = 0; i < num_of_lines; i++) {
        if (lines[i].is_extern == 1 && lines[i].opcode_value == -1) {
            if (strcmp(label, lines[i].data_string_value) == 0) {
                return 0;
            }
        }
    }
    return 1;
}

