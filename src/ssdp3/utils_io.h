#pragma once

#include <iostream>
#include <iomanip>
#include <cmath>
#include "models.h"

inline void print_stage(info* prob) {
    if (prob->verbose > 1) 
        std::cout << "stage #" << prob->stage << "\n"; //std::endl;
}   

inline void print_iteration(info* prob, int iter, double tk=0., int total_nodes=0) {
    if (prob->verbose <= 1) return;
    std::cout.precision(3);
    std::cout<<std::fixed;
    std::cout << "[" << std::setw(4) << iter << "] = ";
    std::cout << "LB= ";
    std::cout<<std::setw(10);
    std::cout<<std::fixed;
    std::cout<< prob->lb << " - UB= " << prob->ub;
    if (std::abs(tk) > 1e-8)
        std::cout << " - gamma= " << tk;
    if (total_nodes > 0)
        std::cout << " - nodes= " << total_nodes;
    
    std::cout << "\n";
}

inline void print_result(info* prob, bool exit = false) {
    std::cout << "obj: " << prob->ub << std::endl;
    if (prob->verbose > 1) {
        for (int i = 0; i<prob->n; ++i) {
            std::cout << prob->sol[i] << ",."[i+1==prob->n];
        }
        //std::cout << "\n";
        std::cout << std::endl;
    }
    if (exit) std::exit(0);
}

