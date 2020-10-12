#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*
CHYBOVE HLASENIA
 2 nedostal som ziadne argumenty na vstupe
 */

typedef enum {SCAN_DELIM_AND_ARGS, RUN, EXIT} RunMode;

typedef enum {SCAN_DELIM, AWAIT_DELIM, DONE} DelimMode;

typedef enum {SCAN_PARAMS, AWAIT_IROW_PARAM, AWAIT_AROW_PARAM, AWAIT_DROW_PARAM, AWAIT_DROWS_PARAMS_1, AWAIT_DROWS_PARAMS_2, PARAM_ERROR, HAVE_ALL_PARAMS} ParamMode;

typedef enum {NONE, IROW, AROW, DROW, DROWS} ArgsMode;


void find_delim(int argc, char **argv, char *delim, bool *found_delim, char delim_string[]);
void process_args (int argc, char **argv, const bool *found_delim, int *param1, int *param2,ArgsMode *args_mode, ParamMode *param_mode);
int verify_digits_only_in_string(const char string[]);
int verify_unsigned_integer(const char string[]);
void irow(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string);
int is_delim (const char *delim, char *delim_string[], const bool *multi_character_delim, int znak);
void arow(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string);

int main(int argc, char *argv[]){
    // neboli zadane ziadne args ani delim
    if (argc == 1) return 2;

    // pointer na stdin, stdout IO stream
    FILE *file_in = stdin;
    FILE *file_out = stdout;


    char delim;
    char delim_string[101];
    bool found_delim = false;
    bool multi_character_delim = false;

    RunMode run_mode = SCAN_DELIM_AND_ARGS;
    ParamMode param_mode = SCAN_PARAMS;
    ArgsMode args_mode = NONE;
    int param1 = 0;
    int param2 = 0;

    while (run_mode != EXIT) {
        switch (run_mode) {
            case SCAN_DELIM_AND_ARGS:
                find_delim(argc, argv, &delim, &found_delim, delim_string);
                if (found_delim) {
                    if(strlen(delim_string) != 0){
                        delim = delim_string[0];
                        multi_character_delim = true;
                        printf("[NOTICE] Nasiel sa viacznakovy delim: %s\n", delim_string);
                    }else {
                        printf("[NOTICE] Nasiel sa jednoznakovy delim: %c\n",delim);
                    }
                }else{
                    printf("[NOTICE] Nenasiel sa delim, nastavujem default\n");
                    delim = ' ';

                }
                process_args(argc, argv, &found_delim, &param1, &param2, &args_mode, &param_mode);
                printf("param1: %d\n", param1);
                printf("param2: %d\n", param2);

                if (param_mode == HAVE_ALL_PARAMS) {
                    run_mode = RUN;
                }

            case RUN:
                switch (args_mode) {

                    case IROW:
                        irow(file_in, file_out, param1, &delim, delim_string, multi_character_delim);
                        break;

                    case AROW:
                        arow(file_in, file_out, param1, &delim, delim_string, multi_character_delim);
                        break;

                    case NONE:
                        printf("[ERROR] Neboli najdene ziadne argumenty na upravu tabulky.\n");
                        break;
                }
                run_mode = EXIT;


            default:
                break;

        }
    }

}

void process_args (int argc, char **argv, const bool *found_delim,int *param1,int *param2, ArgsMode *args_mode, ParamMode *param_mode){


    // ak uz ma delim tak je arg na 3 pozicii, inak je na prvej
    int init_position =  (*found_delim ? 3:1);

    for (int i=init_position; i<argc; i++) {

        switch (*param_mode) {
            case SCAN_PARAMS:
                if (!strcmp(argv[i], "irow")) {
                    printf("[NOTICE] Nasiel som irow\n");
                    *args_mode = IROW;
                    *param_mode = AWAIT_IROW_PARAM;

                }else if (!strcmp(argv[i], "arow")){
                    printf("nasiel som arow\n");
                    *args_mode = AROW;
                    *param_mode = AWAIT_AROW_PARAM;

                }else if (!strcmp(argv[i], "drow")){
                    printf("nasiel som drow\n");
                    *param_mode = AWAIT_DROW_PARAM;

                }else if (!strcmp(argv[i], "drows")){
                    printf("nasiel som drows\n");
                    *param_mode = AWAIT_DROWS_PARAMS_1;
                }
                break;

            case AWAIT_IROW_PARAM:
                if (verify_unsigned_integer(argv[i]) || verify_digits_only_in_string(argv[i])){
                    break;
                }else if((*param1 = (int)strtol(argv[i], NULL, 10)),*param1 != 0){ // NOLINT(cert-err34-c)

                    // prepnutie modu na IROW
                    *args_mode = IROW;

                    // prepnutie modu, moze sa prepnut main mode na execution
                    *param_mode = HAVE_ALL_PARAMS;
                    break;

                }else{
                    printf("R>0");
                    break;
                }
            case AWAIT_AROW_PARAM:
                if (verify_unsigned_integer(argv[i]) || verify_digits_only_in_string(argv[i])){
                    break;
                }else if((*param1 = (int)strtol(argv[i], NULL, 10)),*param1 != 0){ // NOLINT(cert-err34-c)

                    // prepnutie modu na IROW
                    *args_mode = AROW;

                    // prepnutie modu, moze sa prepnut main mode na execution
                    *param_mode = HAVE_ALL_PARAMS;
                    break;

                }else{
                    printf("R>0");
                    break;
                }


            case AWAIT_DROW_PARAM:
                if (verify_unsigned_integer(argv[i]) || verify_digits_only_in_string(argv[i])){
                    break;
                }else if ((*param1 = (int)*argv[i]), param1 != 0){

                    *args_mode = DROW;
                    *param_mode = HAVE_ALL_PARAMS;
                    break;
                }else{
                    printf("[ERROR] Parameter musi byt vacsi ako nula.\n");
                    break;
                }


            case AWAIT_DROWS_PARAMS_1:

                if (verify_unsigned_integer(argv[i]) || verify_digits_only_in_string(argv[i])){
                    break;
                }else if ((*param1 = (int)strtol(argv[i], NULL, 10)), *param1 != 0){
                        *args_mode = DROWS;
                        *param_mode = AWAIT_DROWS_PARAMS_2;
                        break;
                    }else{
                        printf("[ERROR] Parameter musi byt vacsi ako nula.\n");
                        break;
                    }


            case AWAIT_DROWS_PARAMS_2:

                if (verify_unsigned_integer(argv[i]) || verify_digits_only_in_string(argv[i])){
                    break;
                }else if ((*param2 = (int)strtol(argv[i], NULL, 10)), *param2 != 0){
                        *param_mode = HAVE_ALL_PARAMS;
                        break;
                    }else{
                        printf("[ERROR] Parameter musi byt vacsi ako nula.\n");
                        break;
                    }

            default:
                break;
        }
    }

}

void find_delim (int argc, char **argv, char *delim, bool *found_delim, char delim_string[]){

    DelimMode arg_mode = SCAN_DELIM;
    unsigned long int size_of_delim;

    if (!*found_delim) {
        for (int i = 1; i < argc; i++) {
            if (!*found_delim) {
                switch (arg_mode) {

                    //hlada delim
                    case SCAN_DELIM:
                        if (!strcmp(argv[i], "-d")) {
                            arg_mode = AWAIT_DELIM;
                        }
                        break;

                        // nasiel sa -d ocakavam char delimu
                    case AWAIT_DELIM:

                        size_of_delim = strlen(argv[i]);
                        if (size_of_delim == 1){
                            *delim = argv[i][0];
                        }else{
                            for (int position_in_delim = 0; position_in_delim < size_of_delim; position_in_delim++){
                                delim_string[position_in_delim] = (char)argv[i][position_in_delim];
                            }
                        }

                        *found_delim = true;
                        arg_mode = DONE;

                        break;

                        // dostal hodnotu, uz nehlada nic
                    default :
                        break;
                }
            }
        }
    }
}

int verify_digits_only_in_string(const char string[]){
    // overenie ci su v argumente v argv[] iba cisla a ziadne ine znaky
    // ak je vsetko v poriadku a je to iba int, vrati 0. inak vrati 1

    for (int position_in_string = 0; string[position_in_string] != '\0'; position_in_string++){
        if(!isdigit((char)string[position_in_string])){
            printf("[ERROR] Nespravne argumenty (objavil sa necislovy znak.)\n");
            return 1;
        }
    }
    return 0;
}

int verify_unsigned_integer(const char string[]){
    // zisti ci je na prvom mieste v parametri -
    // ak ano, tak vrati 1, inak 0

    if (string[0] == '-'){
        printf("[ERROR] Nespravne argumenty (objavilo sa negativne cislo)\n");
        return 1;
    }else{
        return 0;
    }

}

void irow(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string){
    int znak;
    int riadky_counter = 0;
    bool printed_delim = false;

    while (riadky_counter != param-1){
        znak = fgetc(file_in);

        if (is_delim(delim, &delim_string, &multi_character_string, znak)){
            if(!printed_delim) {
                fputc(*delim, stdout);
                printed_delim = true;
            }
        }else {
            fputc(znak, stdout);
            printed_delim = false;
        }

        if (znak == '\n') {
            riadky_counter ++;

        }
    }

    fputc('\n', file_out);

    znak = fgetc(file_in);

    while (znak != EOF){

        if (is_delim(delim, &delim_string, &multi_character_string, znak)){
            if(!printed_delim) {
                fputc(*delim, stdout);
                printed_delim = true;
            }
        }else {
            fputc(znak, stdout);
            printed_delim = false;
        }
        znak = fgetc(file_in);
    }

}

int is_delim (const char *delim, char *delim_string[], const bool *multi_character_delim, int znak) {
    // porovna ci sa znak nachadza v retazci delim
    // ak je znak delim, vrati 1, inak 0

    if (*multi_character_delim) {
        if ( strchr(*delim_string, (char)znak) != NULL )
        {
         return 1;
        }

        return 0;
    }else{
        if(*delim == znak){
            return 1;
        }
        return 0;
    }
}

void arow(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string){

    int znak = 0;

    bool printed_delim = false;
    bool printed_empty_column = false;

    while (znak != EOF) {

        int delim_counter = 0;
        znak = fgetc(file_in);
        while (znak != '\n' && znak != EOF){

            if (delim_counter == param && !printed_empty_column){
                fputc(*delim, file_out);
                printed_empty_column = true;
            }

            if (is_delim(delim, &delim_string, &multi_character_string, znak)){

                if(!printed_delim) {

                    fputc(*delim, stdout);
                    printed_delim = true;
                    delim_counter++;
                }

            }else{
                fputc(znak, stdout);
                printed_delim = false;
            }
            znak = fgetc(file_in);
        }
        if (znak != EOF) {
            fputc(znak, file_out);
        }
        printed_empty_column = false;

    }
}
