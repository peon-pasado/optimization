#!/usr/bin/env python3
import os
import random

# Lista de carpetas a procesar
carpetas = ["wt200"]#["wt040", "wt050", "wt100", "wt150", "wt200", "wt250", "wt300"]

for carpeta in carpetas:
    if not os.path.isdir(carpeta):
        print(f"La carpeta {carpeta} no existe.")
        continue
    # Recorre todos los archivos en la carpeta
    for nombre in os.listdir(carpeta):
        # Procesa solo archivos que terminen en .dat pero que no sean los _ex.dat
        if nombre.endswith(".dat") and not nombre.endswith("_ex.dat"):
            ruta_archivo = os.path.join(carpeta, nombre)
            # Construye el nombre del archivo de salida
            base, ext = os.path.splitext(nombre)
            nombre_salida = base + "_ex" + ext
            ruta_salida = os.path.join(carpeta, nombre_salida)
            
            with open(ruta_archivo, "r") as entrada, open(ruta_salida, "w") as salida:
                lineas = entrada.readlines()
                if not lineas:
                    continue  # Salta archivos vacíos
                
                # La primera línea se escribe sin cambios
                salida.write(lineas[0])
                
                # Procesa las demás líneas
                for linea in lineas[1:]:
                    partes = linea.split()
                    if len(partes) < 3:
                        continue  # Si la línea no tiene el formato esperado, se salta
                    try:
                        p = float(partes[0])
                        d_val = float(partes[1])
                        w = partes[2]
                    except ValueError:
                        continue  # Si la conversión falla, salta la línea
                    
                    # Genera números aleatorios entre 0 y 9 para r y s
                    r = random.randint(0, 9)
                    s = random.randint(0, 9)
                    nuevo_p = int(10 * p + r)
                    nuevo_d = int(10 * d_val + s)
                    # Escribe la nueva línea en el archivo de salida
                    salida.write(f"{nuevo_p} {nuevo_d} {w}\n")
            
            print(f"Procesado: {ruta_archivo}  -->  {ruta_salida}")

