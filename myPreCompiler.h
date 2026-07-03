#ifndef MYPRECOMPILER_H
#define MYPRECOMPILER_H

/* Librerie standard */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Costanti */
#define MAX_IDENTIFIER_LEN 32 
#define MAX_LINE_LEN 256

/* Strutture Dati */
typedef struct {
    char name[MAX_IDENTIFIER_LEN];
    char type[MAX_IDENTIFIER_LEN];
    int line_declared;
    int usage_count;    
    bool is_name_valid; 
    bool is_type_valid; 
} Variable;

typedef struct {
    int total_vars_checked;
    int total_errors;
    int unused_vars;
    int invalid_names;
    int invalid_types;
} Stats;

/* Prototipi delle funzioni */
/* Passiamo vars tramite doppio puntatore per poter usare realloc */
int analyze_file(const char *filename, Stats *stats, Variable **vars);

/* Funzione di supporto per capire se una parola e' un tipo di dato standard */
bool is_standard_type(const char *word);

/* Funzioni di validazione */
bool is_keyword(const char *name);
bool is_valid_identifier(const char *name);

/* Funzioni per uso e statistiche */
bool is_word_in_line(const char *line, const char *word);
void print_statistics(Stats *stats, Variable *vars, const char *out_file, bool verbose, const char *in_file);

#endif
