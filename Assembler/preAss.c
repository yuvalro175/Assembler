#include <stdio.h>
#include "HEDER.h"

Macro macros[MAX_MACROS];  
int macro_count;   

/**
 * @brief Trims leading and trailing whitespace from a string and reduces multiple spaces to a single space.
 * @param str A pointer to the string to be processed.
 * @returns void
 */
void trim_whitespace(char* str) {
    char* dest = str;  /* Destination pointer for modifying the string in place*/
    char* src = str;   /* Source pointer for iterating over the string*/
    int in_word = 0;

    /*Trim leading spaces*/
    while (isspace((unsigned char)*src)) src++;

    while (*src != '\0') {
        if (isspace((unsigned char)*src)) {
            if (in_word) {
                *dest++ = ' ';  /* Replace multiple spaces with a single space*/
                in_word = 0;
            }
        } else {
            *dest++ = *src;  /* Copy non-space characters*/
            in_word = 1;
        }
        src++;
    }

    /*Remove trailing space*/
    if (dest > str && isspace((unsigned char)*(dest - 1))) {
        dest--;
    }

    *dest = '\0';  /* Null-terminate the modified string*/
}


/**
 * @brief Trims lines that contain null only.
 * @param input_file is the name of the input file.
 * @returns void
 */
void remove_blank_lines(const char *input_file) {
    FILE *fin, *temp, *fout;
    char trimmed_line[MAX_LINE_LENGTH];
    char line[MAX_LINE_LENGTH];

    fin = fopen(input_file, "r");
    if (!fin) {
        perror("ERR:Error opening input file");
        return;
    }

    temp = tmpfile();
    if (!temp) {
        perror("Error creating temporary file");
        fclose(fin);
        return;
    }

    while (fgets(line, sizeof(line), fin)) {
        strcpy(trimmed_line, line);
        trim_whitespace(trimmed_line);

        if (*trimmed_line != '\0' && *trimmed_line != '\n') {
            fputs(line, temp);
        }
    }

    fclose(fin);

    fout = fopen(input_file, "w");
    if (!fout) {
        perror("Error opening output file");
        fclose(temp);
        return;
    }

    rewind(temp);
    while (fgets(line, sizeof(line), temp)) {
        fputs(line, fout);
    }

    fclose(fout);
    fclose(temp);
}

/**
 * @brief Adds a new macro to the macro databank.
 * @param name name of macro.
 * @param body A 2D array containing the lines that form the body of the macro.
 * @param body_lines The number of lines in the macro's body. 
 * @returns void
 */
void add_macro(const char* name, char body[][MAX_LINE_LENGTH], int body_lines) {
    int i;
    if (macro_count < MAX_MACROS) {
        strcpy(macros[macro_count].name, name);
        macros[macro_count].body_lines = body_lines;
        for (i = 0; i < body_lines; ++i) {
            strcpy(macros[macro_count].body[i], body[i]);
        }
        macro_count++;
    } else {
        fprintf(stderr, "ERR:Maximum number of macros exceeded.\n");
    }
}

/**
 * @brief Gives back the body of a macro that was callen.
 * @param name is the name of the macro called.
 * @returns Macro structure.
 */
Macro* get_macro(const char* name) {
    int i;
    for (i = 0; i < macro_count; ++i) {
        if (strcmp(macros[i].name, name) == 0) {
            return &macros[i];
        }
    }
    return NULL;
}

/**
 * @brief Makes a new file without macros and without comments ";"
 * @param input_file name of file with macro.
 * @param output_file name of file after the macro process.
 * @returns void
 */
void process_file(const char* input_file, const char* output_file) {
    FILE *fin, *fout;
    char line[MAX_LINE_LENGTH];
    int i, in_macro_definition, first_token, body_line_count;
    char current_macro_name[MAX_MACRO_NAME];
    char macro_body[MAX_MACRO_BODY][MAX_LINE_LENGTH];
    char *token;

    fin = fopen(input_file, "r");
    fout = fopen(output_file, "w");

    if (!fin || !fout) {
        perror("ERR: Error opening file PrePass");
        if (fin) fclose(fin);
        if (fout) fclose(fout);
        return;
    }

    in_macro_definition = 0;
    body_line_count = 0;

    while (fgets(line, sizeof(line), fin)) {
        trim_whitespace(line); /*triming the blanks that could cause an error*/

        /* Skip lines that start with ';' */
        if (line[0] == ';') {
            continue;
        }

        if (in_macro_definition) {
            if (strcmp(line, "endmacr") == 0) {
                add_macro(current_macro_name, macro_body, body_line_count);
                in_macro_definition = 0;
            } else {
                trim_whitespace(line);
                strcpy(macro_body[body_line_count++], line);
            }
        } else {
            if (strncmp(line, "macr", 4) == 0) {
                sscanf(line, "macr %[^\n]", current_macro_name);
                in_macro_definition = 1;
                body_line_count = 0;
            } else {
                token = strtok(line, " ");
                first_token = 1;

                while (token) {
                    Macro* macro = get_macro(token);
                    if (macro) {
                        for (i = 0; i < macro->body_lines; i++) {
                            if (i == macro->body_lines - 1)
                                fprintf(fout, "%s", macro->body[i]);
                            else
                                fprintf(fout, "%s\n", macro->body[i]);
                        }
                    } else {
                        if (!first_token) {
                            fprintf(fout, " ");
                        }
                        fprintf(fout, "%s", token);
                    }
                    token = strtok(NULL, " ");
                    first_token = 0;
                }
                fprintf(fout, "\n");
            }
        }
    }

    fclose(fin);
    fclose(fout);
}

/**
 * @brief Makes a new file callen "input file".am (after macro) after macro process.
 * @param name_of_file is the name of input file from the main.
 * @returns 0 if succeded and 1 otherwise.
 */
int preAss(char *name_of_file) {
    char output_file[80];
    char *dot_pos;

    if (!name_of_file)
        return 1;

    remove_blank_lines(name_of_file);

    strcpy(output_file, name_of_file);
    dot_pos = strrchr(output_file, '.');

    if (dot_pos && strcmp(dot_pos, ".as") == 0) {
        strcpy(dot_pos, ".am");  /* Replace ".in" with ".am" */
    } else {
        printf("ERR:no .am file to proceed\n");  /* In case there is no ".in", append ".am" */
    }
    process_file(name_of_file, output_file);
    return 0;
}
