#!/usr/bin/env zsh
source ~/.zshrc
zmodload zsh/datetime  # para EPOCHREALTIME

if [ -z "$1" ]; then
  echo "Uso: $0 prob.cpp"
  exit 1
fi

SRC="$1"

echo "Compilando $SRC con run++..."
if ! run++ "$SRC"; then
  echo "Error de compilación."
  exit 2
fi

EXEC="./a.out"
if [ ! -x "$EXEC" ]; then
  echo "No se encontró el ejecutable a.out"
  exit 3
fi

typeset -a tiempos
total=0
min_time=999999
max_time=0
ncasos=0

for dir in wt*/; do
  foldername="${dir%/}"
  xxx="${foldername:2}"
  optfile="opt${xxx}.in"

  if [ ! -f "$optfile" ]; then
    echo "Archivo $optfile no encontrado, se omite carpeta $foldername"
    continue
  fi

  echo "Procesando $foldername con respuestas en $optfile"

  for i in $(seq -w 1 125); do
    yyy="$i"
    datfile="${foldername}/${foldername}_${yyy}.dat"

    if [ -f "$datfile" ]; then
      start=$EPOCHREALTIME
      salida=$( "$EXEC" < "$datfile" )
      end=$EPOCHREALTIME

      dur_ms=$(printf "%.0f" "$(echo "($end - $start) * 1000" | bc)")

      tiempos+=($dur_ms)
      total=$(($total + $dur_ms))
      ncasos=$(($ncasos + 1))
      [[ $dur_ms -lt $min_time ]] && min_time=$dur_ms
      [[ $dur_ms -gt $max_time ]] && max_time=$dur_ms

      esperada=$(sed -n "${i}p" "$optfile")

      if [[ "$salida" == "$esperada" ]]; then
        echo "[$foldername/$yyy] ✅ OK (${dur_ms} ms)"
      else
        echo "[$foldername/$yyy] ❌ Mismatch (${dur_ms} ms)"
        echo "  Esperado: $esperada"
        echo "  Obtenido: $salida"
      fi
    fi
  done
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
