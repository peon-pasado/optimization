#include <iostream>
#include <algorithm>
#include <numeric>
#include "models.h"
#include "utils_graph.h"
#include "ssdp.h"

void read_params() {
    prob = new info();
    prob->verbose = 3;
    int n;
    std::cin>>n;
    N = n + 2;
    N2 = (n + 2) * (n + 2);
    prob->n = n;
    prob->p = new int[n + 2];
    prob->d = new int[n + 2];
    prob->w = new int[n + 2];
    for (int i=1; i<=n; ++i) {
        std::cin>>prob->p[i];
        std::cin>>prob->d[i];
        std::cin>>prob->w[i];
    }
    prob->p[0] = 0;
    prob->p[n + 1] = 1;
    prob->T = std::accumulate(prob->p+1, prob->p+n+1, 0);
    std::cout << "T: " << prob->T << std::endl;
    prob->ord = new int[n + 2];
    int* id = new int[n + 2];
    std::iota(id, id+n+2, 0);
    std::stable_sort(id+1, id+n+1, 
        [](int i, int j) {
            return prob->d[i] < prob->d[j] || 
                (prob->d[i] == prob->d[j] && prob->p[i] < prob->p[j]);
        }
    );    
    prob->s = new int[n + 2];
    prob->e = new int[n + 2];
    prob->s[0] = 0;
    prob->e[0] = 0;
    for (int i=1; i<=n; ++i) {
        prob->s[i] = prob->p[i];
        prob->e[i] = prob->T; 
    }
    prob->s[n + 1] = prob->T + 1;
    prob->e[n + 1] = prob->T + 1;
    prob->eps = 1e-5;
    prob->sol = new int[n];
    prob->fobj = 1e9;
    prob->ub = 1e9;
    for (int i=1; i<=n; ++i) {
        prob->ord[id[i]] = i;
    }
    prob->ord[0] = 0;
    prob->ord[n + 1] = n + 1;
    id[n + 1] = n + 1;
}
    
int func(int i, int T) {
    return (i >= 1 && i <= prob->n && T > prob->d[i]) ? prob->w[i] * (T - prob->d[i]) : 0;
}

void setup() {
    std::cin.tie(nullptr)->sync_with_stdio(false);
    std::cout << "Setting up..." << std::endl;
    tmap = new Tmap[prob->T + 2];
    prob->u = new double[prob->n + 2];
    for (int i=0; i<=prob->n + 1; ++i) {
        prob->u[i] = 0;
    }
    prob->su = 0;
    t2m = new T2map();
    t2m->tm = tmap;
    t2m->tb = new bit_set((prob->T + 2) * N2);
    prob->Tmin = 0;
    prob->Tmax = prob->T;
    for (int t=0; t<=prob->T + 1; ++t) {
        tmap[t].b = new bit_set(prob->n + 2);
        tmap[t].f = new int[prob->n + 2]{0};
        tmap[t].n_nos = prob->n + 2;
        tmap[t].at = nullptr;
        for (int i=0; i<=prob->n + 1; ++i) {
            tmap[t].f[i] = func(i, t);
            if (t < prob->s[i] || t > prob->e[i]) {
                delete_node(t, i);
                continue;
            }
            if (prob->p[i] > t || (t > 0 && !check_time(t - prob->p[i]))) {
                delete_node(t, i);
                continue;
            }
            tmap[t].n_nos++;
        } 
    }
}

int main(int argc, char* argv[]) {
    std::cin.tie(nullptr)->sync_with_stdio(false);
    
    // Inicializar parámetros y configuración
    read_params();
    setup();
    
    // Procesar argumentos de línea de comandos
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-v") 
            prob->verbose = 3;
    }
    
    // Ejecutar el algoritmo SSDP
    ssdp(prob, tmap, t2m);
    return 0;
}