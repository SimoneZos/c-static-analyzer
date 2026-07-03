#include "myPreCompiler.h"

/* Funzione di supporto: restituisce true se la parola è un tipo base del C */
bool is_standard_type(const char *word) {
    if (strcmp(word, "int") == 0 || strcmp(word, "char") == 0 ||
        strcmp(word, "float") == 0 || strcmp(word, "double") == 0 ||
        strcmp(word, "short") == 0 || strcmp(word, "long") == 0 ||
        strcmp(word, "unsigned") == 0) {
        return true;
    }
    return false;
}

/* Controlla se il nome è una parola riservata del C */
bool is_keyword(const char *name) {
    const char *keywords[] = {"int", "float", "char", "double", "short", "long", 
                              "void", "return", "if", "else", "while", "for", 
                              "do", "switch", "case", "break", "continue", "struct", "typedef"};
    int num_keywords = 19;
    int i;
    for(i = 0; i < num_keywords; i++) {
        if (strcmp(name, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

/* Applica le regole del C per i nomi degli identificatori */
bool is_valid_identifier(const char *name) {
    int i;
    
    if (name[0] == '\0') return false;

    /*Deve iniziare con lettera o underscore */
    if (!((name[0] >= 'a' && name[0] <= 'z') || 
          (name[0] >= 'A' && name[0] <= 'Z') || 
          name[0] == '_')) {
        return false;
    }

    /* Il resto puo' contenere solo lettere, numeri o underscore */
    for (i = 1; name[i] != '\0'; i++) {
        if (!((name[i] >= 'a' && name[i] <= 'z') || 
              (name[i] >= 'A' && name[i] <= 'Z') || 
              (name[i] >= '0' && name[i] <= '9') || 
              name[i] == '_')) {
            return false;
        }
    }

    /* Non deve essere una parola chiave del C */
    if (is_keyword(name)) {
        return false;
    }

    return true;
}

/* Controlla se una parola è presente nella riga (evitando falsi positivi come 'i' in 'printf') */
bool is_word_in_line(const char *line, const char *word) {
    const char *p = line;
    int len = strlen(word);
    

    while ((p = strstr(p, word)) != NULL) {
        /* Controlla il carattere prima e dopo per assicurarsi che sia una parola intera */
        char prima = (p == line) ? ' ' : *(p - 1);
        char dopo = *(p + len);
        
        bool prima_ok = !((prima >= 'a' && prima <= 'z') || (prima >= 'A' && prima <= 'Z') || (prima >= '0' && prima <= '9') || prima == '_');
        bool dopo_ok = !((dopo >= 'a' && dopo <= 'z') || (dopo >= 'A' && dopo <= 'Z') || (dopo >= '0' && dopo <= '9') || dopo == '_');
        
        if (prima_ok && dopo_ok) {
            return true; /* Parola trovata e isolata */
        }
        p += len; /* Vai avanti a cercare saltando parola*/
    }
    return false;
}

int analyze_file(const char *filename, Stats *stats, Variable **vars) {
    FILE *fp;
    char buffer[MAX_LINE_LEN];
    char temp_type[MAX_IDENTIFIER_LEN];
    char temp_name[MAX_IDENTIFIER_LEN];
    
    /* Array dinamico di stringhe per memorizzare i tipi definiti con typedef */
    char **custom_types = NULL;
    int custom_types_count = 0;
    
    int line_counter = 0;
    int j; 

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Errore: impossibile aprire il file di input '%s'\n", filename);
        return -1;
    }

    printf("\nInizio analisi sintattica del file '%s'...\n", filename);

    while (fgets(buffer, MAX_LINE_LEN, fp) != NULL) {
        line_counter++;
        temp_type[0] = '\0';
        temp_name[0] = '\0';
        
        /* Controllo utilizzo variabili già salvate */
        int k;
        for (k = 0; k < stats->total_vars_checked; k++) {
            if (is_word_in_line(buffer, (*vars)[k].name)) {
                (*vars)[k].usage_count++;
            }
        }
        

        /* Controllo se la riga è un typedef (es. typedef int intero;) */
        if (strncmp(buffer, "typedef", 7) == 0) {
            char t1[32], t2[32], t3[32];
            if (sscanf(buffer, "%31s %31s %31s", t1, t2, t3) >= 3) {
                int len = strlen(t3);
                if (len > 0 && t3[len-1] == ';') t3[len-1] = '\0'; /* togliamo il ; */
                
                /* Riallochiamo l'array di puntatori per fare spazio a una nuova stringa */
                custom_types = realloc(custom_types, (custom_types_count + 1) * sizeof(char *));
                
                /* Allochiamo lo spazio effettivo per i caratteri della stringa */
                custom_types[custom_types_count] = malloc(MAX_IDENTIFIER_LEN);
                
                strncpy(custom_types[custom_types_count], t3, MAX_IDENTIFIER_LEN - 1);
                custom_types[custom_types_count][MAX_IDENTIFIER_LEN - 1] = '\0'; 
                
                custom_types_count++;
                printf("Trovato nuovo tipo utente: '%s'\n", t3);
            }
            continue; /* Passiamo alla riga successiva */
        }

        /* Estrazione delle potenziali variabili */
        if (sscanf(buffer, "%31s %31s", temp_type, temp_name) == 2) {
            /* Saltiamo #include, le funzioni (hanno le parentesi) e parole chiave come return */
            if (temp_type[0] == '#' || strstr(buffer, "(") != NULL || strcmp(temp_type, "return") == 0) {
                continue;
            }

            int len = strlen(temp_name);
            /* Se la seconda parola finisce con ; è una dichiarazione di variabile */
            if (len > 0 && (temp_name[len-1] == ';')) {
                temp_name[len-1] = '\0'; /* Togliamo il punto e virgola */

                /* allocazione dinamica */
                int current_count = stats->total_vars_checked;
                *vars = realloc(*vars, (current_count + 1) * sizeof(Variable));
                if (*vars == NULL) {
                    printf("Errore di memoria!\n");
                    for (j = 0; j < custom_types_count; j++) {
                        free(custom_types[j]);
                    }
                    if (custom_types != NULL) {
                      free(custom_types);
                    }
                    fclose(fp);
                    return -1;
                }

                strncpy((*vars)[current_count].type, temp_type, MAX_IDENTIFIER_LEN - 1);
                strncpy((*vars)[current_count].name, temp_name, MAX_IDENTIFIER_LEN - 1);
                (*vars)[current_count].line_declared = line_counter;
                (*vars)[current_count].usage_count = 0;

                /* controllo validità nome */
                (*vars)[current_count].is_name_valid = is_valid_identifier(temp_name);
                if ((*vars)[current_count].is_name_valid == false) {
                    stats->invalid_names++;
                    stats->total_errors++;
                    printf("  -> ERRORE ALLA RIGA %d: Il nome '%s' non e' valido!\n", line_counter, temp_name);
                }

                /* controllo validità tipo */
                bool type_is_valid = is_standard_type(temp_type);
                /* Se non è standard, controlliamo se è stato definito con typedef */
                if (!type_is_valid) {
                    for (j = 0; j < custom_types_count; j++) {
                        if (strcmp(temp_type, custom_types[j]) == 0) {
                            type_is_valid = true;
                            break;
                        }
                    }
                }
                
                (*vars)[current_count].is_type_valid = type_is_valid;
                if (!type_is_valid) {
                    stats->invalid_types++;
                    stats->total_errors++;
                    printf("  -> ERRORE ALLA RIGA %d: Il tipo '%s' non e' riconosciuto!\n", line_counter, temp_type);
                }

                stats->total_vars_checked++;
                printf("Variabile salvata: Tipo = '%s', Nome = '%s'\n", temp_type, temp_name);
            }
        }
    }
    /* libera memoria allocata dinamicamente */
    for (j = 0; j < custom_types_count; j++) {
        free(custom_types[j]);
    }
    if (custom_types != NULL) {
        free(custom_types);
    }
    /* Controllo per il file vuoto */
    if (line_counter == 0) {
        printf("Errore: il file '%s' e' vuoto o non contiene codice leggibile.\n", filename);
        fclose(fp);
        return -1; /* Restituiamo -1 per indicare al main che c'è stato un errore bloccante */
    }
    
    if (!feof(fp)) printf("Errore durante la lettura del file.\n");
    
    /* Controllo errore di chiusura file */
    if (fclose(fp) != 0) {
        printf("Errore: impossibile chiudere correttamente il file di input '%s'.\n", filename);
        return -1;
    }
    
    printf("\nAnalisi completata.\n");
    return 0;
}

void print_statistics(Stats *stats, Variable *vars, const char *out_file, bool verbose, const char *in_file) {
    FILE *dests[2] = {NULL, NULL};
    int num_dests = 0;
    int i, d;

    /* Calcolo delle variabili non utilizzate */
    stats->unused_vars = 0;
    for (i = 0; i < stats->total_vars_checked; i++) {
        
        /* Contiamo come 'non usata' solo se il nome e il tipo erano validi */
        if (vars[i].usage_count == 0 && vars[i].is_name_valid == true && vars[i].is_type_valid == true) {
            stats->unused_vars++;
        }
        
    }
    /*Aggiorniamo il totale degli errori includendo le variabili mai usate */
    stats->total_errors += stats->unused_vars;
    /* Impostiamo le destinazioni di stampa in base ai parametri */
    if (verbose || out_file[0] == '\0') {
        dests[num_dests] = stdout; /* Stampa su terminale */
        num_dests++;
    }
    
    if (out_file[0] != '\0') {
        FILE *f_out = fopen(out_file, "w");
        if (f_out != NULL) {
            dests[num_dests] = f_out; /* Stampa su file */
            num_dests++;
        } else {
            printf("Errore: impossibile creare il file di output '%s'\n", out_file);
        }
    }

    /* Stampiamo le statistiche su tutte le destinazioni richieste */
    for (d = 0; d < num_dests; d++) {
        FILE *f = dests[d];
        fprintf(f, "\n=== STATISTICHE DI ELABORAZIONE ===\n");
        fprintf(f, "Numero totale di variabili controllate: %d\n", stats->total_vars_checked);
        fprintf(f, "Numero totale di errori rilevati: %d\n", stats->total_errors);
        fprintf(f, "Numero di variabili non utilizzate: %d\n", stats->unused_vars);
        fprintf(f, "Numero di nomi di variabili non corretti: %d\n", stats->invalid_names);
        fprintf(f, "Numero di tipi di variabile non corretti: %d\n", stats->invalid_types);
        
        if (stats->total_errors > 0) {
        fprintf(f, "\n--- Dettaglio Errori ---\n");
        for (i = 0; i < stats->total_vars_checked; i++) {
            if (!vars[i].is_name_valid) 
                fprintf(f, "File '%s', Riga %d: Nome variabile '%s' non valido.\n", in_file, vars[i].line_declared, vars[i].name);
            if (!vars[i].is_type_valid) 
                fprintf(f, "File '%s', Riga %d: Tipo di dato '%s' non valido.\n", in_file, vars[i].line_declared, vars[i].type);
        }
    }
        
        if (stats->unused_vars > 0) {
            fprintf(f, "\n--- Variabili Non Utilizzate ---\n");
            for (i = 0; i < stats->total_vars_checked; i++) {
                if (vars[i].usage_count == 0 && vars[i].is_name_valid == true && vars[i].is_type_valid == true) {
                    fprintf(f, "Variabile: %s\n", vars[i].name);
                }
            }
        }
        fprintf(f, "===================================\n");
        
        /* Chiudiamo il file se non e' stdout e controlliamo errori */
        if (f != stdout) {

            if (ferror(f)) {
                printf("Errore: problema durante la scrittura sul file di output.\n");
            }
            if (fclose(f) != 0) {
                printf("Errore: impossibile chiudere correttamente il file di output.\n");
            }
        }
    }
}
