#include "myPreCompiler.h"

int main(int argc, char *argv[]) {
    Stats app_stats = {0, 0, 0, 0, 0};
    Variable *vars_array = NULL;
    char file_input[256] = "";
    char file_output[256] = "";
    bool verbose_mode = false;
    int i; 

    /* Analisi degli argomenti*/
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--in") == 0) {
            if (i + 1 < argc) {
                strncpy(file_input, argv[i + 1], 255);
                i++; 
            }
        }
        else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--out") == 0) {
            if (i + 1 < argc) {
                strncpy(file_output, argv[i + 1], 255);
                i++; 
            }
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose_mode = true;
        }
        else if (strcmp(argv[i], "-vi") == 0) {
            verbose_mode = true;
            if (i + 1 < argc) {
                strncpy(file_input, argv[i + 1], 255);
                i++;
            }
        }
        /* Gestione parametro non riconosciuto */
        else {
            printf("Errore: parametro '%s' non riconosciuto.\n", argv[i]);
            printf("Uso: %s -i <file_in.c> [-o <file_out.txt>] [-v]\n", argv[0]);
            return 1;
        }
    }
    
    /* Verifica presenza parametro obbligatorio */
    if (file_input[0] == '\0') {
        printf("Errore nei parametri: devi specificare un file di input con -i nomefile.c\n");
        return 1;
    }

    /*  Avvio Elaborazione */
    /* Passiamo &vars_array per permettere a analyze_file di allocare memoria */
    if (analyze_file(file_input, &app_stats, &vars_array) != 0) {
        return 1; /* Esce se c'e' stato un errore coi file */
    }

    /* Stampa statistiche*/
    print_statistics(&app_stats, vars_array, file_output, verbose_mode, file_input);

    /* libero memoria allocata dinamicamente */
    if (vars_array != NULL) {
        free(vars_array);
    }

    return 0;
}
