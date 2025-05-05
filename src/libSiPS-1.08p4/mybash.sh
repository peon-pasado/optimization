#!/bin/zsh

#          meu | now | feedback | tanaka (2009) | tanaka (2012)  |(meu 2009) feedback | x10 + r (meu 2009) | x10   (meu 2009) | tanaka 2012 (my cpu)| tanaka blog (2012)  | x10 (tanaka 2012) | x10 + r (tanaka 2012) 
# n=40:     0.04 ( 0.04) [ 0.04]|   0.19        |  0.02          | 0.04                | 0.24               | 0.08             | 0.02                | 0.09                | 0.05              | 0.20
# n=50:     0.10 ( 0.09) [ 0.08]|   0.39        |  0.05          | 0.09                | 0.51               | 0.17             | 0.05                | 0.18                | 0.12              | 0.34
# n=100:    2.51 ( 0.94) [ 0.63]|   6.42        |  0.34          | 1.86                | 10.01              | 3.13             | 0.34                | 2.25                | 0.96              | 3.72
# n=150:   13.06 ( 4.14) [ 2.21]|  26.12        |  1.27          | 9.08                | 43.12              | 16.91            | 1.27                | 11.26               | 3.38              | 12.43
# n=200:   39.30 (13.12) [ 6.13]|  74.20        |  3.40          | 40.0                | 112.60             | 45.50            | 3.40                | 31.82               | 10.67             | 39.45
# n=250:  135.20 (40.50)        | 170.36        | 25.72          | 116.30              | 168.32             | 137.64           | 25.72               | 72.08               | 18.43             | 66.52
# n=300:  211.45                | 353.60        | 15.75          | 176.58              | 358.02             | 210.80           | 15.752              | 142.80              | 16.27             | 59.40
 


# Cargar configuraciones de Zsh, incluyendo alias
source ~/.zshrc

# Compilar el programa con run++
# run++ solver3.1.cpp -o solver || { echo "Error al compilar con run++"; exit 1; }


# Inicializar variables
input_dir="wt150"
pattern="wt150_"
total_time=0
count=0
line_number=1  # Para saber qué línea de opt40.in estamos procesando

# Leer el archivo opt40.in línea por línea
while IFS= read -r expected_output; do
    input_file="$input_dir/$pattern$(printf "%03d" $line_number)_ex.dat"

    echo "$input_file"

    if [ -f "$input_file" ]; then
        # Medir tiempo de ejecución
        start=$(date +%s.%N)
        result=$(./sipsexec -v 0 < "$input_file")  # Guardar la salida del programa
        end=$(date +%s.%N)

        # Calcular tiempo transcurrido
        elapsed=$(echo "$end - $start" | bc)
        total_time=$(echo "$total_time + $elapsed" | bc)
        count=$((count + 1))

        # Imprimir mensaje
        echo "Archivo procesado: $input_file en $elapsed segundos."

        # Comparar el resultado con el valor esperado
        if [ "$result" != "$expected_output" ]; then
            echo "¡Error! El resultado para $input_file es $result, pero se esperaba $expected_output."
        else
            echo "El resultado para $input_file es correcto."
        fi
    else
        echo "Archivo no encontrado: $input_file"
    fi

    line_number=$((line_number + 1))  # Aumentar el número de línea
done < opt150.in

# Calcular el promedio de tiempo de ejecución
if [ $count -gt 0 ]; then
    avg_time=$(echo "$total_time / $count" | bc -l)
    echo "Promedio de tiempo de ejecución: $avg_time segundos"
else
    echo "No se encontraron archivos de entrada válidos."
fi
