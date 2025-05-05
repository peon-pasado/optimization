#!/usr/bin/env python3
import sys

# Leer todas las líneas del archivo
lines = [line.strip() for line in sys.stdin if line.strip()]

# Determinar el número de trabajos (asumiendo 3 valores por trabajo)
n = len(lines) // 3

# Crear el archivo en formato wt150
print(n)
for i in range(n):
    p = lines[i*3]
    d = lines[i*3 + 1]
    w = lines[i*3 + 2]
    print(f'{p} {d} {w}') 