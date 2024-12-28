#ifndef HEDER_H
#define HEDER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
 
/*preProc*/
#define MAX_MACROS 100
#define MAX_MACRO_NAME 50
#define MAX_MACRO_BODY 50
#define MAX_LINE_LENGTH 256

/*firstPass and second pass*/
#define MAX_LABEL_LENGTH 31
#define MAX_OPCODE_LENGTH 10
#define MAX_METHOD_LENGTH 10
#define MIN_MEM_VAL 100
#define MAX_LINES 4096
#define BITS 15


typedef struct {
    char name[MAX_MACRO_NAME];
    char body[MAX_MACRO_BODY][MAX_LINE_LENGTH];
    int body_lines;
} Macro;

typedef struct {
    char label_name[MAX_LABEL_LENGTH];
    char opcode_name[MAX_OPCODE_LENGTH];
    int source_method; /* Numeric representation of source method */
    int destination_method; /* Numeric representation of destination method */
    char source_method_value[MAX_METHOD_LENGTH]; /* Actual value for source method (e.g., "r3") */
    char destination_method_value[MAX_METHOD_LENGTH]; /* Actual value for destination method */
    int memory_cells;
    int memory_value; /* Address in memory */
    int opcode_value; /* Numeric value for opcode */
    bool is_data;
    bool is_string;
    bool is_entry;
    bool is_extern;
    char data_string_value[MAX_LINE_LENGTH]; /* Actual value for data or string */
    int count_op;/*count how many opcode is there in the line*/
    bool flag; /*Tracks errors in first and second pass*/
} LineInfo;

typedef struct {
    char label_name[MAX_LABEL_LENGTH];
} ExternLabels;

typedef enum {
    IMMEDIATE = 0, 
    DIRECT = 1,    
    INDIRECT_REGISTER = 2,
    DIRECT_REGISTER = 3   
} AddressingMethod;

typedef enum {
    A = 4, 
    R = 2, 
    E = 1  
} FieldType;



extern Macro macros[MAX_MACROS];
extern int macro_count;

/*Stating the prototype of the pre assembler functions*/
int preAss(char *name_of_file);
void trim_whitespace(char* str);
void remove_blank_lines(const char *input_file);
void add_macro(const char* name, char body[][MAX_LINE_LENGTH], int body_lines);
Macro* get_macro(const char* name);
void process_file(const char* input_file, const char* output_file);
int preAss(char *name_of_file);

/*Stating the prototype of the first pass functions*/
int firstPass(char *name_of_file,LineInfo *lines ,int line_count);
void initializeLineInfo(LineInfo *lineInfo);
int getOpcodeValue(char *opcode_name);
int calcData(char *str);
int calcString(char *str);
int calculateMemoryCells(LineInfo *lineInfo);
void processLine(char *line, LineInfo *lineInfo);
void parseMethod(const char *method_name, int *method, char *value);
void processInputFile(FILE *file, LineInfo *lines, int *line_count);

/*Stating the prototype of the second pass functions*/
int secondPass(char *name_of_file ,LineInfo *lines, int line_count);
int getOperandCount(int opcode_value);
char* printBinary(int num);
int findLabelMemory(LineInfo lines[], int numLines, char *label);
int findLabelAddress(LineInfo lines[], int numLines, char *label);
void generateOutput(LineInfo lines[], int numLines, const char *filename);
void makeOb(int machine[], const char *filename, int dc, int ic);
int isExtern(LineInfo *lines,int num_of_lines,char *label);
void makeExt(LineInfo *lines,int num_of_lines, char *filename);
void makeEnt(LineInfo *lines,int num_of_lines, char *filename);
int isFlag(LineInfo lines[], int numLines);
int isGoodLine(LineInfo line);


#endif
