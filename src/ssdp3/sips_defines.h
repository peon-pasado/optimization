#pragma once

/*
 * Códigos de retorno y constantes de la implementación original de Tanaka
 */

// Códigos de retorno generales
#define SIPS_NORMAL     0     // Procesamiento normal
#define SIPS_OPTIMAL    1     // Solución óptima encontrada 
#define SIPS_SOLVED     2     // Problema resuelto
#define SIPS_INFEASIBLE 3     // Problema infactible
#define SIPS_MEMLIMIT   4     // Límite de memoria alcanzado
#define SIPS_TIMELIMIT  5     // Límite de tiempo alcanzado
#define SIPS_UNSOLVED   6     // Problema no resuelto

// Códigos para operaciones
#define SIPS_OK         0     // Operación exitosa 
#define SIPS_FAIL       1     // Fallo en la operación

// Tipos de problema
#define SIPS_PROB_NONE     0
#define SIPS_PROB_REGULAR  1
#define SIPS_PROB_IDLETIME 2

// Tipos de costo
#define SIPS_COST_INTEGER  0
#define SIPS_COST_REAL     1

// Tipos de local search
#define SIPS_LS_NONE       0
#define SIPS_LS_DYNA       1  
#define SIPS_LS_EDYNA      2
#define SIPS_LS_COMBINED_A 3
#define SIPS_LS_COMBINED_B 4

// Tipos de tiebreak 
#define SIPS_TIEBREAK_HEURISTIC  0
#define SIPS_TIEBREAK_RHEURISTIC 1
#define SIPS_TIEBREAK_EDD        2
#define SIPS_TIEBREAK_LDD        3
#define SIPS_TIEBREAK_SPT        4
#define SIPS_TIEBREAK_LPT        5
#define SIPS_TIEBREAK_MDD        6
#define SIPS_TIEBREAK_MWD        7
#define SIPS_TIEBREAK_MWDD       8

#define SIPS_FORWARD 0
#define SIPS_BACKWARD 1

// Macros para verificación
#define is_true(x)  ((x) != 0)
#define not_true(x) ((x) == 0) 
#define is_false(x) ((x) == 0)
#define not_false(x) ((x) != 0) 