#!/bin/zsh

#         meu          | tanaka  | feedback | x10 + r | x10
# n=40:   0.04         | 0.19    | 0.04     | 0.24    | 0.08
# n=50:   0.10         | 0.39    | 0.09     | 0.51    | 0.17
# n=100:  2.51  (1.9)  | 6.42    | 1.86     |         | 3.13
# n=150:  13.06        | 26.12   | 9.08     |         |
# n=200;  39.3         | 74.2    |          |         |
#
# terminar tabela [ok]
# implementar dominance 5/6 [ok] - jugar con el orden [ok] 
# impementar subgradiente conjugado [ok]
# usar las isntancias dificiles


# Cargar configuraciones de Zsh, incluyendo alias
source ~/.zshrc

# Compilar el programa con run++
run++ solver4.1.cpp -o solver || { echo "Error al compilar con run++"; exit 1; }

# Inicializar variables
input_dir="wt150"
pattern="wt150_"
total_time=0
count=0
line_number=1  # Para saber qué línea de opt40.in estamos procesando

# Leer el archivo opt40.in línea por línea
while IFS= read -r expected_output; do
    input_file="$input_dir/$pattern$(printf "%03d" $line_number).dat"

    if [ -f "$input_file" ]; then
        # Medir tiempo de ejecución
        start=$(date +%s.%N)
        result=$(./solver < "$input_file")  # Guardar la salida del programa
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
