#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HEDER.h"

int main(int argc, char **argv) {
    int i;
    LineInfo lines[MAX_LINES];
    int line_count;
    char name_of_file[MAX_MACRO_NAME];
    char *dot_pos;
    char *preprocessed_filename;
    char new_name[MAX_MACRO_NAME];

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> [<file2> ...]\n", argv[0]);
        return 1;
    }

    for (i = 1; i < argc; ++i) {
        FILE *file = fopen(argv[i], "r");
        if (file == NULL) {
            perror("ERR: File does not exist");
            continue;
        } else {
            fclose(file);
        }

        strcpy(new_name, argv[i]);
        strcat(new_name, ".as");

        if (rename(argv[i], new_name) != 0) {
            perror("Error renaming file");
            continue;
        }

        strcpy(name_of_file, new_name);

        dot_pos = strrchr(name_of_file, '.');
        if (!dot_pos || strcmp(dot_pos, ".as") != 0) {
            printf("ERR: no '.as' file to proceed\n");
            return 1;
        }

        if (preAss(name_of_file) == 1) {
            printf("ERR:Error at macro processing\n");
            return 1;
        }

        preprocessed_filename = (char *)malloc(strlen(name_of_file) + 4);
        if (!preprocessed_filename) {
            perror("ERR: Unable to allocate memory for preprocessed filename");
            return 1;
        }

        strcpy(preprocessed_filename, name_of_file);
        dot_pos = strrchr(preprocessed_filename, '.');

        if (dot_pos && strcmp(dot_pos, ".as") == 0) {
            strcpy(dot_pos, ".am");
        } else {
            printf("ERR:no .am file to proceed\n");
            free(preprocessed_filename);
            return 1;
        }

        line_count = 0;

        if (firstPass(preprocessed_filename, lines, line_count) == 1) {
            printf("ERR:Error at first pass processing\n");
            free(preprocessed_filename);
            return 1;
        }

        free(preprocessed_filename);
    }
    return 0;
}

