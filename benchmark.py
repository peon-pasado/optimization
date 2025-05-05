import subprocess
import time
import statistics
import re

# Número de repeticiones
REPEATS = 5
TEST_CASE = "wt100/wt100_064.dat"

# Patrón para extraer el tiempo
time_pattern = re.compile(r"Time: (\d+\.\d+) segundos\.")

times = []

print(f"Ejecutando benchmark con {TEST_CASE} ({REPEATS} repeticiones)...")

for i in range(REPEATS):
    start_time = time.time()
    
    # Ejecutar el algoritmo
    result = subprocess.run(
        ["./a.out", f"< {TEST_CASE}"], 
        stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE,
        text=True,
        shell=True
    )
    
    # Tiempo total (incluyendo I/O)
    total_time = time.time() - start_time
    
    # Extraer tiempo reportado por el algoritmo
    match = time_pattern.search(result.stdout)
    if match:
        reported_time = float(match.group(1))
        times.append(reported_time)
        print(f"Ejecución {i+1}: {reported_time:.5f} segundos")
    else:
        print(f"Ejecución {i+1}: No se pudo extraer el tiempo")

if times:
    avg_time = statistics.mean(times)
    median_time = statistics.median(times)
    min_time = min(times)
    max_time = max(times)
    stdev_time = statistics.stdev(times) if len(times) > 1 else 0
    
    print("\nResultados:")
    print(f"Tiempo promedio: {avg_time:.5f} segundos")
    print(f"Tiempo mediano: {median_time:.5f} segundos")
    print(f"Tiempo mínimo: {min_time:.5f} segundos")
    print(f"Tiempo máximo: {max_time:.5f} segundos")
    print(f"Desviación estándar: {stdev_time:.5f} segundos")
else:
    print("No se pudieron recopilar tiempos de ejecución.") 