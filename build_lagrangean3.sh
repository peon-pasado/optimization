#!/bin/bash

g++ -std=c++17 -c src/ssdp3/lagrangean3.cpp -I src/ssdp3 -o src/ssdp3/lagrangean3.o
RESULT=$?

echo "Compilación completada. Verificando errores:"
if [ $RESULT -eq 0 ]; then
    echo "No hay errores de compilación."
else
    echo "Error en la compilación, código: $RESULT"
fi 