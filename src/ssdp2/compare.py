import subprocess
import re

# Fijamos xxx y el directorio de archivos
xxx = "040"
outfile = f"compare_{xxx}.csv"

with open(outfile, "w") as f_out:
    # Escribimos el encabezado en formato CSV
    f_out.write("who,test number,time (s),max lower bound,upper bound,n iteration\n")
    
    # Recorrer todos los tests de 001 a 125
    for num in range(1, 126):
        yyy = f"{num:03d}"
        filename = f"wt{xxx}/wt{xxx}_{yyy}.dat"
        
        # Procesar salida de la primera versión (T)
        with open(filename, "r") as infile:
            # Ejecuta sipsexec redirigiendo el contenido del archivo
            result_T = subprocess.run(["../libSiPS-1.08p4/sipsexec", "-v", "2"],
                                      stdin=infile,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE,
                                      text=True)
        out_T = result_T.stdout
        # Extraer Iterations, Lower bound, Upper bound y Time del bloque Stage 1
        m = re.search(r"Iterations:\s*(\d+).*?Lower bound:\s*([\d\.]+).*?Upper bound:\s*([\d\.]+).*?Time:\s*([\d\.]+)", out_T, re.DOTALL)
        if m:
            iter_T = m.group(1)
            lb_T   = m.group(2)
            ub_T   = m.group(3)
            time_T = m.group(4)
        else:
            iter_T = lb_T = ub_T = time_T = "N/A"
        # Escribir línea para la salida T (who = T)
        f_out.write(f"T,{yyy},{time_T},{lb_T},{ub_T},{iter_T}\n")
        
        # Procesar salida de la segunda versión (M)
        try:
            with open(filename, "r") as infile:
                result_M = subprocess.run(["./a.out"],
                                        stdin=infile,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.PIPE,
                                        text=True,
                                        timeout=10)  # Añadir timeout de 10 segundos
                out_M = result_M.stdout
                # Dividir la salida en líneas
                lines = out_M.strip().splitlines()
                
                # Buscar la penúltima línea (desde abajo) que contenga "LB=" para extraer lb, ub y n iteration
                lb_line = None
                for line in reversed(lines):
                    if "LB=" in line:
                        lb_line = line
                        break
                if lb_line:
                    # Ejemplo de línea: "[  80] = LB=  19352.380 - UB= 19648 - gamma= 1.000"
                    m2 = re.search(r"\[\s*(\d+)\].*?LB=\s*([\d\.]+)\s*-\s*UB=\s*([\d\.]+)", lb_line)
                    if m2:
                        iter_M = m2.group(1)
                        lb_M   = m2.group(2)
                        ub_M   = m2.group(3)
                    else:
                        iter_M = lb_M = ub_M = "N/A"
                else:
                    iter_M = lb_M = ub_M = "N/A"
                
                # Extraer el time de la última línea que contenga "Time:"
                time_line = None
                for line in reversed(lines):
                    if "Time:" in line:
                        time_line = line
                        break
                if time_line:
                    m3 = re.search(r"Time:\s*([\d\.]+)", time_line)
                    time_M = m3.group(1) if m3 else "N/A"
                else:
                    time_M = "N/A"
        except subprocess.TimeoutExpired:
            # Si ocurre un timeout, registrar N/A
            iter_M = lb_M = ub_M = time_M = "TIMEOUT"
        except Exception as e:
            # Si ocurre cualquier otro error, registrar N/A
            iter_M = lb_M = ub_M = time_M = f"ERROR: {str(e)}"
            
        # Escribir línea para la salida M (who = M)
        f_out.write(f"M,{yyy},{time_M},{lb_M},{ub_M},{iter_M}\n")
        
        # Imprimir progreso
        print(f"Procesado test {yyy} de 125", flush=True)
