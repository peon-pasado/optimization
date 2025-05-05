#!/usr/bin/env zsh
source ~/.zshrc
zmodload zsh/datetime  # habilita EPOCHREALTIME

# Verifica argumentos
if [[ $# -ne 1 ]]; then
  echo "Uso: $0 xxx"
  exit 1
fi

XXX="$1"
FOLDER="wt${XXX}"
EXEC="../libSiPS-1.08p4/sipsexec"

if [ ! -x "$EXEC" ]; then
  echo "No se encontró el ejecutable: $EXEC"
  exit 2
fi

if [ ! -d "$FOLDER" ]; then
  echo "No se encontró la carpeta: $FOLDER"
  exit 3
fi

typeset -a tiempos
total=0
min_time=999999
max_time=0
ncasos=0

echo "Procesando $FOLDER..."

for i in $(seq -w 1 125); do
  yyy="$i"
  datfile="${FOLDER}/${FOLDER}_${yyy}.dat"

  if [ -f "$datfile" ]; then
    start=$EPOCHREALTIME
    # Ejecuta el programa con el archivo .dat
    salida=$( "$EXEC" -v 0 < "$datfile" )  
    end=$EPOCHREALTIME

    dur_ms=$(printf "%.0f" "$(echo "($end - $start) * 1000" | bc)")

    tiempos+=($dur_ms)
    total=$(($total + $dur_ms))
    ncasos=$(($ncasos + 1))
    [[ $dur_ms -lt $min_time ]] && min_time=$dur_ms
    [[ $dur_ms -gt $max_time ]] && max_time=$dur_ms

    echo "[$FOLDER/$yyy] Ejecutado en ${dur_ms} ms"
  fi
done

if [[ $ncasos -gt 0 ]]; then
  prom=$(($total / $ncasos))
  echo ""
  echo "📊 Estadísticas de tiempo (ms):"
  echo "  Casos ejecutados: $ncasos"
  echo "  Total: $total"
  echo "  Promedio: $prom"
  echo "  Mínimo: $min_time"
  echo "  Máximo: $max_time"
else
  echo "No se ejecutó ningún caso."
fi
