#pragma once

#include <vector>
#include <utility>
#include <cassert>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <cstring>
#include <limits>
#include "models.h" 
#include "lagrangean.h"
#include "utils_io.h"
#include "heuristics.h"
#include "dynasearch.h"

// Parámetros alineados con la implementación de libSiPS (usando los valores exactos de default.h)
const double initstep1 = 1.0;      // DEFAULT_INITSTEP1
const double maxstep1 = 1.0;       // DEFAULT_MAXSTEP1
const double shrink1 = 0.9;        // DEFAULT_SHRINK1 (corregido de 0.95 a 0.9)
const double expand1 = 1.25;       // DEFAULT_EXPAND1 (corregido de 1.2 a 1.25)
const double tratio1 = 0.02;       // DEFAULT_TERMRATIO1
const int siter1 = 2;              // DEFAULT_SHRINKITER1
const int minupdate = 10;          // DEFAULT_MINUPDATE
const double inittermiter1 = 3.0;  // DEFAULT_INITTERMITER1
const double termiter1 = 4.0;      // DEFAULT_TERMITER1

// Parámetros adicionales para LR2adj
const double inittermiter2 = 1.0;  // DEFAULT_INITTERMITER2
const double termiter2 = 4.0;      // DEFAULT_TERMITER2 (corregido de 2.0 a 4.0)
const double initstep2 = 1.0;      // DEFAULT_INITSTEP2
const double maxstep2 = 1.0;       // DEFAULT_MAXSTEP2
const double shrink2 = 0.9;        // DEFAULT_SHRINK2
const double expand2 = 1.3;        // DEFAULT_EXPAND2
const double tratio2 = 0.002;      // DEFAULT_TERMRATIO2
const int siter2 = 2;              // DEFAULT_SHRINKITER2
const int ubiter = 10;             // DEFAULT_UBITERATION

// Variables globales utilizadas por las implementaciones
double* dir;                       // Vector de dirección del subgradiente
double dnorm;                      // Norma del vector de dirección
int* x_vals;                       // Vector auxiliar para cálculo del subgradiente

// Implementación de subgradient_LR1 siguiendo exactamente el código de Tanaka
int subgradient_LR1() {
    // Definir constantes y límites
    const int max_iter = 10000;     // Número máximo de iteraciones
    
    // En la implementación de Tanaka, el tamaño del historial es igual a n
    int d_T = prob->n;
    if (d_T < 1) d_T = 1;
    
    // Calcular ititer (límite de iteraciones iniciales) usando DEFAULT_INITTERMITER1
    int ititer;
    if (inittermiter1 > prob->eps) {
        if (prob->n < inittermiter1) {
            ititer = 1;
        } else {
            double g = prob->n / inittermiter1;
            ititer = (int)g;
            if ((ititer + 1) - g < prob->eps) {
                ititer++;
            }
        }
    } else if (inittermiter1 < -1.0 + prob->eps) {
        ititer = (int)(-inittermiter1);
        if (inittermiter1 + (double)(ititer + 1) < prob->eps) {
            ititer++;
        }
    } else {
        ititer = 0;
    }
    
    // Calcular titer (criterio de terminación) usando DEFAULT_TERMITER1
    int titer;
    if (termiter1 > prob->eps) {
        if (prob->n < termiter1) {
            titer = 1;
        } else {
            double g = prob->n / termiter1;
            titer = (int)g;
            if ((titer + 1) - g < prob->eps) {
                titer++;
            }
        }
    } else if (termiter1 < -1.0 + prob->eps) {
        titer = (int)(-termiter1);
        if (termiter1 + (double)(titer + 1) < prob->eps) {
            titer++;
        }
    } else {
        titer = 0;
    }
    
    // Garantizar que ititer >= titer como en la implementación original
    if (ititer < titer) {
        ititer = titer;
    }
    
    // Parámetros - tamaño de paso inicial
    double g = initstep1;
    if (g < prob->eps) g = 1.0;
    
    // Inicializamos contadores y variables
    int iter = 0;                    // Contador de iteraciones
    int gap_pos = 0;                 // Posición en el buffer circular
    int nupdated = 0;                // Contador de mejoras al LB
    int updated = 0;                 // Contador de iteraciones sin mejora
    int nsiter = 0;                  // Total de iteraciones sin mejora
    double glb = prob->lb;           // Mejor LB global
    double lmax2 = -1e9;             // Segundo mejor LB para detección de estancamiento
    
    // Inicializamos el vector de dirección y vector auxiliar si es necesario
    if (dir == nullptr) {
        dir = new double[prob->n + 2];
    }
    if (x_vals == nullptr) {
        x_vals = new int[prob->n + 1];
    }
    
    // Inicializamos todos los elementos a 0
    for (int i = 1; i <= prob->n; i++) {
        dir[i] = 0.0;
    }
    
    // Guardamos los mejores multiplicadores encontrados
    double* gu = new double[prob->n + 2];
    for (int i = 0; i < prob->n + 2; i++) {
        gu[i] = prob->u[i];
    }
    
    // Buffer circular para el historial de gaps
    double* gap_table = nullptr;
    if (titer > 0) {  // Solo alocamos memoria si titer > 0, exactamente como Tanaka
        gap_table = new double[titer];  // Tanaka usa titer, no d_T para alocar
        for (int i = 0; i < titer; i++) {
            gap_table[i] = -1e9;  // Inicialización a un valor muy negativo
        }
    }
    
    // Inicializamos la norma
    dnorm = 1.0;
    
    // Iniciar medición de tiempo
    auto start = std::chrono::high_resolution_clock::now();
    
    // Ciclo principal del método del subgradiente
    for (; iter < max_iter; iter++) {
        // Resolver problema LR1 (relajación Lagrangiana)
        auto [jobs, cost] = solve_LR1_fordward(iter == 0);
        prob->lb = cost;
        
        // Si mejoramos el lower bound
        if (prob->lb > glb + prob->eps) {
            // Reiniciar contadores de iteraciones sin mejora
            updated = nsiter = 0;
            nupdated++;
            
            // Actualizar el mejor lower bound
            glb = prob->lb;
            
            // Guardar los multiplicadores actuales como mejores
            for (int i = 0; i < prob->n + 2; i++) {
                gu[i] = prob->u[i];
            }
            
            // Aumentar el paso después de una mejora (excepto en primera iteración)
            if (iter > 0) {
                g *= expand1;
                if (g > maxstep1 && g > initstep1) {
                    g = std::max(maxstep1, initstep1);
                }
            }
            
            // Imprimir información de la iteración
            print_iteration(prob, iter, g);
            
            // Si el gap es lo suficientemente pequeño, terminamos
            if (prob->ub - glb < 1.0 - prob->eps) {
                break;
            }
        } else if (lmax2 >= prob->lb - prob->eps) {
            // Si no mejoramos respecto al segundo mejor, incrementar contador
            updated++;
        }
        
        // Actualizar el segundo mejor lower bound
        if (iter > 0) {
            lmax2 = std::max(lmax2, prob->lb);
        }
        
        // Ejecutar backward DP en la primera iteración para inicializar valores
        if (iter == 0) {
            solve_LR1_backward();
        }
        
        // Criterios de terminación basados en el historial
        if (gap_table != nullptr) {  // Tanaka verifica si gap_table es NULL
            // Actualizamos el historial circular (primero actualizar, luego verificar terminación)
            gap_table[gap_pos] = glb;  // Tanaka asigna directamente a pos, luego incrementa
            gap_pos++;
            gap_pos %= titer;  // Esto es exactamente el código de Tanaka para el módulo
            
            if (nupdated <= 1) {
                if (iter > ititer) {
                    break;
                }
            } else if (tratio1 * (prob->ub - gap_table[gap_pos]) > glb - gap_table[gap_pos]
                && tratio1 * glb + prob->eps > glb - gap_table[gap_pos]
                && (nupdated * minupdate >= prob->n
                || nsiter >= prob->n)) {
                break;
            }
        }
        
        // Reducir paso después de varias iteraciones sin mejora
        if (updated >= siter1) {
            nsiter += updated;
            updated = 0;
            g *= shrink1;
            if (g < 1e-4) {
                break;  // Paso demasiado pequeño
            }
        }
        
        // Actualizar los multiplicadores para la siguiente iteración
        // Calculamos el vector subgradiente en x_vals
        memset(x_vals, 0, (prob->n + 1) * sizeof(int));
        for (auto j : jobs) {
            x_vals[j] += 1;
        }
        
        // Calcular la norma del subgradiente
        int osq = 0;  // Tanaka usa osq, no norm para mantener coherencia
        for (int i = 1; i <= prob->n; i++) {
            osq += (1 - x_vals[i]) * (1 - x_vals[i]);
        }
        
        // Si todas las restricciones se satisfacen, hemos encontrado una solución óptima
        if (osq == 0) {
            break;
        }
        
        // Actualizar el vector de dirección usando la fórmula de Camerini et al.
        double ksi = std::sqrt((double)osq / dnorm);
        for (int i = 1; i <= prob->n; i++) {
            dir[i] = (1 - x_vals[i]) + ksi * dir[i];
        }
        
        // Recalcular la nueva norma
        dnorm = 0.0;
        for (int i = 1; i <= prob->n; i++) {
            dnorm += dir[i] * dir[i];
        }
        
        // Si la norma es demasiado pequeña, reinicializar
        if (dnorm < prob->eps) {
            dnorm = osq;
            for (int i = 1; i <= prob->n; i++) {
                dir[i] = 1 - x_vals[i];
            }
        }
        
        // Calcular el tamaño de paso y actualizar multiplicadores
        double tk = g * (prob->ub - prob->lb) / dnorm;
        prob->su = 0.0;
        for (int i = 1; i <= prob->n; i++) {
            prob->u[i] += tk * dir[i];
            prob->su += prob->u[i];
        }
    }
    
    // Finalizar medición de tiempo
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Restaurar los mejores multiplicadores encontrados
    prob->lb = glb;
    prob->su = 0.0;
    for (int i = 0; i < prob->n + 2; i++) {
        prob->u[i] = gu[i];
        prob->su += prob->u[i];
    }
    
    // Volver a resolver con los mejores multiplicadores para obtener la solución final
    auto [l1_job, l1_cst] = solve_LR1_fordward();
    auto [l2_job, l2_cst] = solve_LR1_backward();
    
    // Mostrar resultados
    print_iteration(prob, iter);
    std::cout.precision(5);
    std::cout << std::fixed;
    std::cout << "LR1 Time: " << duration.count() / 1000000. << " segundos." << std::endl;
    
    // Liberar memoria
    delete[] gap_table;
    delete[] gu;
    
    // Retornar éxito
    return 1;
}

// Implementación de subgradient_LR2adj siguiendo exactamente el código de Tanaka
int subgradient_LR2adj() {
    // Definir constantes y límites
    const int max_iter = 10000;     // Número máximo de iteraciones
    
    // En la implementación de Tanaka, tamaño del historial es 1.5*n para LR2adj
    int d_T;
    if (termiter2 > prob->eps) {
        if (prob->n < termiter2) {
            d_T = 1;
        } else {
            double g = prob->n / termiter2;
            d_T = (int)g;
            if ((d_T + 1) - g < prob->eps) {
                d_T++;
            }
        }
    } else if (termiter2 < -1.0 + prob->eps) {
        d_T = (int)(-termiter2);
        if (termiter2 + (d_T + 1) < prob->eps) {
            d_T++;
        }
    } else {
        d_T = 0;
    }
    
    if (d_T < 1) d_T = 1;
    
    // Calcular ititer (límite de iteraciones iniciales) usando DEFAULT_INITTERMITER2
    int ititer;
    if (inittermiter2 > prob->eps) {
        if (prob->n < inittermiter2) {
            ititer = 1;
        } else {
            double g = prob->n / inittermiter2;
            ititer = (int)g;
            if ((ititer + 1) - g < prob->eps) {
                ititer++;
            }
        }
    } else if (inittermiter2 < -1.0 + prob->eps) {
        ititer = (int)(-inittermiter2);
        if (inittermiter2 + (double)(ititer + 1) < prob->eps) {
            ititer++;
        }
    } else {
        ititer = 0;
    }
    
    // Cálculo de titer (criterio de terminación) usando DEFAULT_TERMITER2
    int titer;
    if (termiter2 > prob->eps) {
        if (prob->n < termiter2) {
            titer = 1;
        } else {
            double g = prob->n / termiter2;
            titer = (int)g;
            if ((titer + 1) - g < prob->eps) {
                titer++;
            }
        }
    } else if (termiter2 < -1.0 + prob->eps) {
        titer = (int)(-termiter2);
        if (termiter2 + (double)(titer + 1) < prob->eps) {
            titer++;
        }
    } else {
        titer = 0;
    }
    
    // Garantizar que ititer >= titer
    if (ititer < titer) {
        ititer = titer;
    }
    
    // Parámetros - tamaño de paso inicial
    double g = initstep2;
    if (g < prob->eps) g = 1.0;
    
    // Inicializamos contadores y variables
    int iter = 0;                    // Contador de iteraciones
    int gap_pos = 0;                 // Posición en el buffer circular
    int nupdated = 0;                // Contador de mejoras al LB
    int updated = 0;                 // Contador de iteraciones sin mejora
    int nsiter = 0;                  // Total de iteraciones sin mejora
    double glb = prob->lb;           // Mejor LB global
    double lmax2 = -1e9;             // Segundo mejor LB para detección de estancamiento
    
    // Inicializamos el vector de dirección y vector auxiliar si es necesario
    if (dir == nullptr) {
        dir = new double[prob->n + 2];
    }
    if (x_vals == nullptr) {
        x_vals = new int[prob->n + 1];
    }
    
    // Inicializamos todos los elementos a 0
    for (int i = 1; i <= prob->n; i++) {
        dir[i] = 0.0;
    }
    
    // Guardamos los mejores multiplicadores encontrados
    double* gu = new double[prob->n + 2];
    for (int i = 0; i < prob->n + 2; i++) {
        gu[i] = prob->u[i];
    }
    
    // Buffer circular para el historial de gaps
    double* gap_table = nullptr;
    if (titer > 0) {  // Solo alocamos memoria si titer > 0, exactamente como Tanaka
        gap_table = new double[titer];  // Tanaka usa titer, no d_T para alocar
        for (int i = 0; i < titer; i++) {
            gap_table[i] = -1e9;  // Inicialización a un valor muy negativo
        }
    }
    
    // Variables para detección de oscilaciones y almacenamiento de soluciones
    int64_t bosq = std::numeric_limits<int64_t>::max();
    std::vector<int> csol;         // Mejor solución encontrada (para UB)
    double blg = 0;                // Costo asociado a la mejor solución
    
    // Inicializamos la norma
    dnorm = 1.0;
    
    // Iniciar medición de tiempo
    auto start = std::chrono::high_resolution_clock::now();
    bool infeasible = false;
    // Ciclo principal del método del subgradiente
    for (; iter < max_iter; iter++) {
        // Resolver problema LR2 (relajación Lagrangiana)
        auto [jobs, cost] = solve_LR2_fordward(prob, t2m, iter == 0);
        if (jobs.empty()) {
            infeasible = true;
            break;  // No hay solución factible
        }
        
        prob->lb = cost;
        
        // Si mejoramos el lower bound
        if (prob->lb > glb + prob->eps) {
            // Reiniciar contadores de iteraciones sin mejora
            updated = nsiter = 0;
            nupdated++;
            
            // Actualizar el mejor lower bound
            glb = prob->lb;
            
            // Guardar los multiplicadores actuales como mejores
            for (int i = 0; i < prob->n + 2; i++) {
                gu[i] = prob->u[i];
            }
            
            // Aumentar el paso después de una mejora (excepto en primera iteración)
            if (iter > 0) {
                g *= expand2;
                if (g > maxstep2 && g > initstep2) {
                    g = std::max(maxstep2, initstep2);
                }
            }
            

            
            // Si el gap es lo suficientemente pequeño, terminamos
            if (prob->ub - glb < 1.0 - prob->eps) {
                break;
            }
        } else if (lmax2 >= prob->lb - prob->eps) {
            // Si no mejoramos respecto al segundo mejor, incrementar contador
            updated++;
        }
        
        // Actualizar el segundo mejor lower bound
        if (iter > 0) {
            lmax2 = std::max(lmax2, prob->lb);
        }
        
        // Calculamos el subgradiente en x_vals
        memset(x_vals, 0, (prob->n + 1) * sizeof(int));
        for (auto j : jobs) {
            x_vals[j] += 1;
        }
        
        // Calcular la norma del subgradiente para detección de oscilaciones
        int osq = 0;
        for (int i = 1; i <= prob->n; i++) {
            osq += (1 - x_vals[i]) * (1 - x_vals[i]);
        }
        
        // Guardar la solución con menor oscilación
        if (bosq > osq) {
            bosq = osq;
            csol = jobs;
            blg = cost;
        }
        
        // Actualizaciones periódicas del upper bound (corregido a iter % ubiter == 1 como en Tanaka)
        if (ubiter != 0 && iter % ubiter == 1) {  // Tanaka usa exactamente iter % ubiter == 1
            // Refinar la solución usando upper bound heurístico y dynasearch
            auto refined = upper_bound_refine(prob, t2m->tm, csol, blg);
            auto [improved_sol, improved_cost] = dynasearch(prob, t2m->tm, refined);
            
            // Si mejoramos el upper bound, actualizar
            if (improved_cost < prob->ub) {
                update(improved_sol, improved_cost, prob);
                
                // Ajustar el tamaño del paso según la mejora
                if (iter > 1 && prob->ub > glb) {
                    g *= (prob->ub - glb) / (improved_cost - glb);
                    if (g > maxstep2) g = maxstep2;
                }
                
                // Si el gap es lo suficientemente pequeño, terminamos
                if (prob->ub - glb < prob->eps) {
                    break;
                }
            }
            
            // Reiniciar para la siguiente búsqueda
            bosq = std::numeric_limits<int64_t>::max();
        }
        
        // Resolver backward para completar la iteración
        // Tanaka solo llama a backward cuando hay una mejora de LB o se realiza una actualización de UB

        
        if (updated == 0) {
            auto [bj, bc] = solve_LR2_backward(prob, t2m);
            // Imprimir información de la iteración
            print_iteration(prob, iter, g, total_nodes);
             if (bj.empty()) {
                infeasible = true;
                break;
            }
        }

        // Criterios de terminación basados en el historial
        if (gap_table != nullptr) {  // Tanaka verifica si gap_table es NULL
            // Actualizamos el historial circular (primero actualizar, luego verificar terminación)
            gap_table[gap_pos] = glb;  // Tanaka asigna directamente a pos, luego incrementa
            gap_pos++;
            gap_pos %= titer;  // Esto es exactamente el código de Tanaka para el módulo
            
            if (nupdated <= 1) {
                if (iter > ititer) {
                    break;
                }
            } else if (tratio2 * (prob->ub - gap_table[gap_pos]) > glb - gap_table[gap_pos]
                && tratio2 * glb + prob->eps > glb - gap_table[gap_pos]
                && (nupdated * minupdate >= prob->n
                || nsiter >= prob->n)) {
                break;
            }
        }
        
        // Reducir paso después de varias iteraciones sin mejora
        if (updated >= siter2) {
            nsiter += updated;
            updated = 0;
            g *= shrink2;
            if (g < 1e-4) {
                break;  // Paso demasiado pequeño
            }
        }
        
        // Actualizar el vector de dirección usando la fórmula de Camerini et al.
        double ksi = std::sqrt((double)osq / dnorm);
        for (int i = 1; i <= prob->n; i++) {
            dir[i] = (1 - x_vals[i]) + ksi * dir[i];
        }
        
        // Recalcular la nueva norma
        dnorm = 0.0;
        for (int i = 1; i <= prob->n; i++) {
            dnorm += dir[i] * dir[i];
        }
        
        // Si la norma es demasiado pequeña, reinicializar
        if (dnorm < prob->eps) {
            dnorm = osq;
            for (int i = 1; i <= prob->n; i++) {
                dir[i] = 1 - x_vals[i];
            }
        }
        
        // Calcular el tamaño de paso y actualizar multiplicadores
        double tk = g * (prob->ub - prob->lb) / dnorm;
        prob->su = 0.0;
        for (int i = 1; i <= prob->n; i++) {
            prob->u[i] += tk * dir[i];
            prob->su += prob->u[i];
        }
    }
    
    // Finalizar medición de tiempo
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    if (!infeasible) {
        // Restaurar los mejores multiplicadores encontrados
        prob->lb = glb;
        prob->su = 0.0;
        for (int i = 0; i < prob->n + 2; i++) {
            prob->u[i] = gu[i];
            prob->su += prob->u[i];
        }
        
        // Volver a resolver con los mejores multiplicadores para obtener la solución final
        auto [l2_job, l2_cst] = solve_LR2_fordward(prob, t2m);
        auto [b2_job, b2_cst] = solve_LR2_backward(prob, t2m);
        
        // Aplicar heurísticas para mejorar la solución final
        auto sol = upper_bound_refine(prob, t2m->tm, l2_job, l2_cst);
        auto [improved_sol, improved_cost] = dynasearch(prob, t2m->tm, sol);
        update(improved_sol, improved_cost, prob);
    }
    // Mostrar resultados
    print_iteration(prob, iter);
    std::cout.precision(5);
    std::cout << std::fixed;
    std::cout << "LR2adj Time: " << duration.count() / 1000000. << " segundos." << std::endl;
    
    // Liberar memoria
    delete[] gap_table;
    delete[] gu;
    
    // Retornar éxito
    return 1;
}
