#!/bin/bash

# Variables configurables
input_dir="wt250"          # Directorio de entrada
pattern="wt250_"           # Patrón del nombre de archivo
opt_file="opt250.in"        # Ruta al archivo opt40.in
start=1                    # Primer archivo (001)
end=125                    # Último archivo (125)

# Validar si el archivo opt40.in existe
if [[ ! -f "$opt_file" ]]; then
  echo "Error: El archivo $opt_file no existe."
  exit 1
fi

# Validar si el directorio de entrada existe
if [[ ! -d "$input_dir" ]]; then
  echo "Error: El directorio $input_dir no existe."
  exit 1
fi

# Contador de línea en opt40.in
line_number=0

# Leer opt40.in línea por línea
while IFS= read -r line; do
  # Incrementar contador de línea
  line_number=$((line_number + 1))
  
  # Verificar si estamos dentro del rango deseado
  if [[ $line_number -lt $start ]]; then
    continue
  fi
  if [[ $line_number -gt $end ]]; then
    break
  fi

  # Formatear el número de archivo a tres dígitos
  formatted_number=$(printf "%03d" $line_number)
  
  # Ruta del archivo destino
  target_file="$input_dir/$pattern$formatted_number.dat"
  
  # Validar si el archivo destino existe
  if [[ -f "$target_file" ]]; then
    # Añadir la línea al final del archivo
    echo "$line" >> "$target_file"
    echo "Línea añadida a $target_file"
  else
    echo "Advertencia: El archivo $target_file no existe."
  fi
done < "$opt_file"
