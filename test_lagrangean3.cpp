#include <iostream>
#include "src/ssdp3/models.h"
#include "src/ssdp3/utils_graph.h"
#include "src/ssdp3/ssdp.h"
#include "src/ssdp3/constants.h"

// Declarar las variables globales externas
info* prob = nullptr;
SublGraph* subl_graph = nullptr;
Tmap* tmap = nullptr;
T2map* t2m = nullptr;
lr2m_mod_t* lr2m_mod = nullptr;

// Declarar las funciones externas de lagrangean3.cpp
extern int lag2_initialize_node(info* prob, double* u);
extern double lag2_get_memory_in_MB(info* prob);
extern void lag2_free(info* prob);

int main() {
    std::cout << "Test de lagrangean3.cpp" << std::endl;
    
    // Crear una estructura info básica para la prueba
    prob = new info();
    prob->n = 5; // 5 jobs
    prob->T = 100; // Horizonte de 100 unidades de tiempo
    prob->p = new int[prob->n + 1];
    prob->w = new int[prob->n + 1];
    prob->d = new int[prob->n + 1];
    prob->s = new int[prob->n + 1];
    prob->e = new int[prob->n + 1];
    prob->mem = 100.0; // 100 MB de memoria permitida
    
    // Inicializar valores para los jobs
    for (int i = 1; i <= prob->n; i++) {
        prob->p[i] = 10; // Cada job toma 10 unidades de tiempo
        prob->w[i] = i;  // Pesos incrementales
        prob->d[i] = 50; // Fecha límite
        prob->s[i] = 0;  // Inicio desde tiempo 0
        prob->e[i] = prob->T; // Puede terminar hasta el horizonte
    }
    
    // Crear multiplicadores Lagrangianos
    double* u = new double[prob->n + 1]();
    
    // Inicializar nodo Lagrangiano
    std::cout << "Inicializando nodo Lagrangiano" << std::endl;
    int ret = lag2_initialize_node(prob, u);
    
    if (ret == SSDP_NORMAL) {
        std::cout << "Inicialización exitosa" << std::endl;
        std::cout << "Memoria utilizada: " << lag2_get_memory_in_MB(prob) << " MB" << std::endl;
    } else {
        std::cout << "Error en la inicialización, código: " << ret << std::endl;
    }
    
    // Liberar memoria
    lag2_free(prob);
    delete[] u;
    delete[] prob->p;
    delete[] prob->w;
    delete[] prob->d;
    delete[] prob->s;
    delete[] prob->e;
    delete prob;
    
    std::cout << "Prueba completada" << std::endl;
    return 0;
} 