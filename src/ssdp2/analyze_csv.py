import pandas as pd
import numpy as np

# Leer el CSV
df = pd.read_csv('compare_040.csv')

# Procesamos los datos para comparar M vs T
# Primero pivoteamos el dataframe para tener M y T en columnas
pivot = df.pivot(index='test number', columns='who')

# Reemplazar 'TIMEOUT' por NaN en todo el dataframe
for col in pivot.columns:
    if pivot[col].dtype == object:  # Solo para columnas de tipo object (texto)
        pivot[col] = pivot[col].replace('TIMEOUT', np.nan)

# Comparamos tiempo
time_M = pivot[('time (s)', 'M')].astype(float)
time_T = pivot[('time (s)', 'T')].astype(float)

# Creamos máscaras para identificar NaN en cualquiera de las columnas
mask_time_nan_M = time_M.isna()
mask_time_nan_T = time_T.isna()
mask_time_nan_any = mask_time_nan_M | mask_time_nan_T

# Tiempo por iteración
iter_M = pivot[('n iteration', 'M')].astype(float)
iter_T = pivot[('n iteration', 'T')].astype(float)

# Identificar NaN en las iteraciones
mask_iter_nan_M = iter_M.isna()
mask_iter_nan_T = iter_T.isna()
mask_iter_nan_any = mask_iter_nan_M | mask_iter_nan_T

# Lower bound (mayor es mejor)
lb_M = pivot[('max lower bound', 'M')].astype(float)
lb_T = pivot[('max lower bound', 'T')].astype(float)

# Identificar NaN en lower bounds
mask_lb_nan_M = lb_M.isna()
mask_lb_nan_T = lb_T.isna()
mask_lb_nan_any = mask_lb_nan_M | mask_lb_nan_T

# Upper bound (menor es mejor)
ub_M = pivot[('upper bound', 'M')].astype(float)
ub_T = pivot[('upper bound', 'T')].astype(float)

# Identificar NaN en upper bounds
mask_ub_nan_M = ub_M.isna()
mask_ub_nan_T = ub_T.isna()
mask_ub_nan_any = mask_ub_nan_M | mask_ub_nan_T

# Creamos una máscara para identificar filas donde T tiene al menos un NaN en cualquier columna
mask_any_T_param_nan = mask_time_nan_T | mask_iter_nan_T | mask_lb_nan_T | mask_ub_nan_T

# Para filas donde T tiene al menos un NaN, establecemos todos sus valores a 0 (excepto iteraciones = 1)
# Y hacemos lo mismo para M en esos casos
for idx in df['test number'].unique():
    if idx in mask_any_T_param_nan.index and mask_any_T_param_nan.loc[idx]:
        # Para T
        if idx in time_T.index and not pd.isna(time_T.loc[idx]):
            time_T.loc[idx] = 0
        if idx in iter_T.index and not pd.isna(iter_T.loc[idx]):
            iter_T.loc[idx] = 1
        if idx in lb_T.index and not pd.isna(lb_T.loc[idx]):
            lb_T.loc[idx] = 0
        if idx in ub_T.index and not pd.isna(ub_T.loc[idx]):
            ub_T.loc[idx] = 0
        
        # Para M en los mismos casos
        if idx in time_M.index and not pd.isna(time_M.loc[idx]):
            time_M.loc[idx] = 0
        if idx in iter_M.index and not pd.isna(iter_M.loc[idx]):
            iter_M.loc[idx] = 1
        if idx in lb_M.index and not pd.isna(lb_M.loc[idx]):
            lb_M.loc[idx] = 0
        if idx in ub_M.index and not pd.isna(ub_M.loc[idx]):
            ub_M.loc[idx] = 0

# Actualizamos las máscaras después de las modificaciones
mask_time_nan_M = time_M.isna()
mask_time_nan_T = time_T.isna()
mask_time_nan_any = mask_time_nan_M | mask_time_nan_T
mask_iter_nan_M = iter_M.isna()
mask_iter_nan_T = iter_T.isna()
mask_iter_nan_any = mask_iter_nan_M | mask_iter_nan_T
mask_lb_nan_M = lb_M.isna()
mask_lb_nan_T = lb_T.isna()
mask_lb_nan_any = mask_lb_nan_M | mask_lb_nan_T
mask_ub_nan_M = ub_M.isna()
mask_ub_nan_T = ub_T.isna()
mask_ub_nan_any = mask_ub_nan_M | mask_ub_nan_T

# Calculamos las métricas solicitadas, considerando NaN como empates
valid_comparisons = len(time_M) - sum(mask_time_nan_any)
if valid_comparisons > 0:
    time_M_wins = sum((time_M < time_T) & ~mask_time_nan_any) / len(time_M) * 100
    time_T_wins = sum((time_T < time_M) & ~mask_time_nan_any) / len(time_M) * 100
    time_ties = 100 - time_M_wins - time_T_wins
else:
    time_M_wins = time_T_wins = 0
    time_ties = 100

# Calcular tiempo por iteración solo para casos válidos
time_per_iter_M = time_M.copy()
time_per_iter_T = time_T.copy()

# Solo calcular para valores no NaN
valid_M = ~(mask_time_nan_M | mask_iter_nan_M)
valid_T = ~(mask_time_nan_T | mask_iter_nan_T)
time_per_iter_M[valid_M] = time_M[valid_M] / iter_M[valid_M]
time_per_iter_T[valid_T] = time_T[valid_T] / iter_T[valid_T]

# Considerar NaN en cualquiera como empate
mask_time_per_iter_nan_any = ~valid_M | ~valid_T
valid_time_per_iter = ~mask_time_per_iter_nan_any

if sum(valid_time_per_iter) > 0:
    time_per_iter_M_wins = sum((time_per_iter_M < time_per_iter_T) & valid_time_per_iter) / len(time_per_iter_M) * 100
    time_per_iter_T_wins = sum((time_per_iter_T < time_per_iter_M) & valid_time_per_iter) / len(time_per_iter_T) * 100
    time_per_iter_ties = 100 - time_per_iter_M_wins - time_per_iter_T_wins
else:
    time_per_iter_M_wins = time_per_iter_T_wins = 0
    time_per_iter_ties = 100

# Calcular estadísticas considerando NaN como empates para lower bound
if len(lb_M) - sum(mask_lb_nan_any) > 0:
    lb_M_wins = sum((lb_M > lb_T) & ~mask_lb_nan_any) / len(lb_M) * 100
    lb_T_wins = sum((lb_T > lb_M) & ~mask_lb_nan_any) / len(lb_M) * 100
    lb_ties = 100 - lb_M_wins - lb_T_wins
else:
    lb_M_wins = lb_T_wins = 0
    lb_ties = 100

# Calcular estadísticas considerando NaN como empates para upper bound
if len(ub_M) - sum(mask_ub_nan_any) > 0:
    ub_M_wins = sum((ub_M < ub_T) & ~mask_ub_nan_any) / len(ub_M) * 100
    ub_T_wins = sum((ub_T < ub_M) & ~mask_ub_nan_any) / len(ub_M) * 100
    ub_ties = 100 - ub_M_wins - ub_T_wins
else:
    ub_M_wins = ub_T_wins = 0
    ub_ties = 100

# Calcular el porcentaje promedio en que T supera a M cuando tiene mejor lower bound
T_better_lb_mask = (lb_T > lb_M) & ~mask_lb_nan_any
lb_M_np = lb_M.fillna(0).values
lb_T_np = lb_T.fillna(0).values
T_better_lb_indices = T_better_lb_mask.values

lb_percent_improvements = []
test_numbers_with_improvements = []
best_improvement_index = -1
best_improvement_percent = -1

for i in range(len(lb_M_np)):
    if i < len(T_better_lb_indices) and T_better_lb_indices[i]:
        if not (np.isnan(lb_M_np[i]) or np.isnan(lb_T_np[i])) and lb_M_np[i] > 0:
            percent_improvement = (lb_T_np[i] - lb_M_np[i]) / lb_M_np[i] * 100
            lb_percent_improvements.append(percent_improvement)
            test_number = df['test number'].unique()[i]
            test_numbers_with_improvements.append(test_number)
            
            # Verificar si este es el mejor caso hasta ahora
            if percent_improvement > best_improvement_percent:
                best_improvement_percent = percent_improvement
                best_improvement_index = i

if len(lb_percent_improvements) > 0:
    avg_lb_percent_improvement = sum(lb_percent_improvements) / len(lb_percent_improvements)
    max_lb_percent_improvement = max(lb_percent_improvements)
    min_lb_percent_improvement = min(lb_percent_improvements)
    
    # Obtener más detalles sobre el mejor caso
    best_test_number = df['test number'].unique()[best_improvement_index]
    best_lb_M = lb_M_np[best_improvement_index]
    best_lb_T = lb_T_np[best_improvement_index]
else:
    avg_lb_percent_improvement = max_lb_percent_improvement = min_lb_percent_improvement = 0
    best_test_number = best_lb_M = best_lb_T = "N/A"

# Promedio de tiempo por el cual T le gana a M (solo sobre los casos donde T gana)
valid_time_diff = ~mask_time_nan_any
T_wins_mask = (time_T < time_M) & valid_time_diff
M_wins_mask = (time_M < time_T) & valid_time_diff

# Convertir a formato numpy para facilitar el manejo
time_M_np = time_M.values
time_T_np = time_T.values
T_wins_indices = T_wins_mask.values
M_wins_indices = M_wins_mask.values

# Para cuando T gana a M
time_diff_when_T_wins = []
percent_faster_when_T_wins = []

# Para cuando M gana a T
time_diff_when_M_wins = []
percent_faster_when_M_wins = []

# Iteramos y calculamos diferencias para ambos casos
for i in range(len(time_M_np)):
    # Caso: T gana a M
    if i < len(T_wins_indices) and T_wins_indices[i]:
        if not (np.isnan(time_M_np[i]) or np.isnan(time_T_np[i])):
            diff = time_M_np[i] - time_T_np[i]  # Positivo cuando T es más rápido
            percent = (diff / time_M_np[i]) * 100 if time_M_np[i] > 0 else 0
            time_diff_when_T_wins.append(diff)
            percent_faster_when_T_wins.append(percent)
    
    # Caso: M gana a T
    if i < len(M_wins_indices) and M_wins_indices[i]:
        if not (np.isnan(time_M_np[i]) or np.isnan(time_T_np[i])):
            diff = time_T_np[i] - time_M_np[i]  # Positivo cuando M es más rápido
            percent = (diff / time_T_np[i]) * 100 if time_T_np[i] > 0 else 0
            time_diff_when_M_wins.append(diff)
            percent_faster_when_M_wins.append(percent)

# Estadísticas para cuando T gana
if len(time_diff_when_T_wins) > 0:
    avg_time_diff_T_wins = sum(time_diff_when_T_wins) / len(time_diff_when_T_wins)
    avg_percent_faster_T = sum(percent_faster_when_T_wins) / len(percent_faster_when_T_wins)
    max_time_diff_T_wins = max(time_diff_when_T_wins)
    min_time_diff_T_wins = min(time_diff_when_T_wins)
    max_percent_faster_T = max(percent_faster_when_T_wins)
    min_percent_faster_T = min(percent_faster_when_T_wins)
else:
    avg_time_diff_T_wins = max_time_diff_T_wins = min_time_diff_T_wins = 0
    avg_percent_faster_T = max_percent_faster_T = min_percent_faster_T = 0

# Estadísticas para cuando M gana
if len(time_diff_when_M_wins) > 0:
    avg_time_diff_M_wins = sum(time_diff_when_M_wins) / len(time_diff_when_M_wins)
    avg_percent_faster_M = sum(percent_faster_when_M_wins) / len(percent_faster_when_M_wins)
    max_time_diff_M_wins = max(time_diff_when_M_wins)
    min_time_diff_M_wins = min(time_diff_when_M_wins)
    max_percent_faster_M = max(percent_faster_when_M_wins)
    min_percent_faster_M = min(percent_faster_when_M_wins)
else:
    avg_time_diff_M_wins = max_time_diff_M_wins = min_time_diff_M_wins = 0
    avg_percent_faster_M = max_percent_faster_M = min_percent_faster_M = 0

# Análisis para tiempo por iteración
valid_time_per_iter_diff = valid_time_per_iter
T_iter_wins_mask = (time_per_iter_T < time_per_iter_M) & valid_time_per_iter_diff
M_iter_wins_mask = (time_per_iter_M < time_per_iter_T) & valid_time_per_iter_diff

# Convertir a numpy para facilitar manejo
time_per_iter_M_np = time_per_iter_M.values
time_per_iter_T_np = time_per_iter_T.values
T_iter_wins_indices = T_iter_wins_mask.values
M_iter_wins_indices = M_iter_wins_mask.values

# Listas para almacenar diferencias en tiempo por iteración
time_per_iter_diff_when_T_wins = []
percent_faster_per_iter_when_T_wins = []
time_per_iter_diff_when_M_wins = []
percent_faster_per_iter_when_M_wins = []

# Iteramos y consideramos casos donde T gana en tiempo por iteración
for i in range(len(time_per_iter_M_np)):
    # Caso: T gana a M en tiempo por iteración
    if i < len(T_iter_wins_indices) and T_iter_wins_indices[i]:
        if not (np.isnan(time_per_iter_M_np[i]) or np.isnan(time_per_iter_T_np[i]) or 
                np.isinf(time_per_iter_M_np[i]) or np.isinf(time_per_iter_T_np[i])):
            # Solo considerar valores positivos
            if time_per_iter_M_np[i] > 0 and time_per_iter_T_np[i] > 0:
                diff = time_per_iter_M_np[i] - time_per_iter_T_np[i]
                percent = (diff / time_per_iter_M_np[i]) * 100
                time_per_iter_diff_when_T_wins.append(diff)
                percent_faster_per_iter_when_T_wins.append(percent)
    
    # Caso: M gana a T en tiempo por iteración
    if i < len(M_iter_wins_indices) and M_iter_wins_indices[i]:
        if not (np.isnan(time_per_iter_M_np[i]) or np.isnan(time_per_iter_T_np[i]) or 
                np.isinf(time_per_iter_M_np[i]) or np.isinf(time_per_iter_T_np[i])):
            if time_per_iter_M_np[i] > 0 and time_per_iter_T_np[i] > 0:
                diff = time_per_iter_T_np[i] - time_per_iter_M_np[i]  # Positivo cuando M es más rápido
                percent = (diff / time_per_iter_T_np[i]) * 100 if time_per_iter_T_np[i] > 0 else 0
                time_per_iter_diff_when_M_wins.append(diff)
                percent_faster_per_iter_when_M_wins.append(percent)

# Estadísticas para tiempo por iteración cuando T gana
if len(time_per_iter_diff_when_T_wins) > 0:
    avg_time_per_iter_diff_T_wins = sum(time_per_iter_diff_when_T_wins) / len(time_per_iter_diff_when_T_wins)
    avg_percent_faster_per_iter_T = sum(percent_faster_per_iter_when_T_wins) / len(percent_faster_per_iter_when_T_wins)
    max_time_per_iter_diff_T_wins = max(time_per_iter_diff_when_T_wins)
    min_time_per_iter_diff_T_wins = min(time_per_iter_diff_when_T_wins)
    max_percent_faster_per_iter_T = max(percent_faster_per_iter_when_T_wins)
    min_percent_faster_per_iter_T = min(percent_faster_per_iter_when_T_wins)
else:
    avg_time_per_iter_diff_T_wins = max_time_per_iter_diff_T_wins = min_time_per_iter_diff_T_wins = 0
    avg_percent_faster_per_iter_T = max_percent_faster_per_iter_T = min_percent_faster_per_iter_T = 0

# Estadísticas para tiempo por iteración cuando M gana
if len(time_per_iter_diff_when_M_wins) > 0:
    avg_time_per_iter_diff_M_wins = sum(time_per_iter_diff_when_M_wins) / len(time_per_iter_diff_when_M_wins)
    avg_percent_faster_per_iter_M = sum(percent_faster_per_iter_when_M_wins) / len(percent_faster_per_iter_when_M_wins)
    max_time_per_iter_diff_M_wins = max(time_per_iter_diff_when_M_wins)
    min_time_per_iter_diff_M_wins = min(time_per_iter_diff_when_M_wins)
    max_percent_faster_per_iter_M = max(percent_faster_per_iter_when_M_wins)
    min_percent_faster_per_iter_M = min(percent_faster_per_iter_when_M_wins)
else:
    avg_time_per_iter_diff_M_wins = max_time_per_iter_diff_M_wins = min_time_per_iter_diff_M_wins = 0
    avg_percent_faster_per_iter_M = max_percent_faster_per_iter_M = min_percent_faster_per_iter_M = 0

# Imprimimos resultados
print(f'STATISTICAL COMPARISON M vs T (treating NaN as ties):')
print(f'-----------------------------------------------------')
print(f'Execution time:')
print(f'  % M wins over T: {time_M_wins:.2f}%')
print(f'  % T wins over M: {time_T_wins:.2f}%')
print(f'  % Ties: {time_ties:.2f}%')
print(f'Time per iteration:')
print(f'  % M wins over T: {time_per_iter_M_wins:.2f}%')
print(f'  % T wins over M: {time_per_iter_T_wins:.2f}%')
print(f'  % Ties: {time_per_iter_ties:.2f}%')
print(f'Lower bound (higher is better):')
print(f'  % M wins over T: {lb_M_wins:.2f}%')
print(f'  % T wins over M: {lb_T_wins:.2f}%')
print(f'  % Ties: {lb_ties:.2f}%')
print(f'Upper bound (lower is better):')
print(f'  % M wins over T: {ub_M_wins:.2f}%')
print(f'  % T wins over M: {ub_T_wins:.2f}%')
print(f'  % Ties: {ub_ties:.2f}%')
print(f'Average time that T wins over M (only in cases where T wins): {avg_time_diff_T_wins:.6f} s')
print(f'On average, T is {avg_percent_faster_T:.2f}% faster than M when it wins')
print(f'Average time that M wins over T (only in cases where M wins): {avg_time_diff_M_wins:.6f} s')
print(f'On average, M is {avg_percent_faster_M:.2f}% faster than T when it wins')
print(f'Average time per iteration that T wins over M (only in cases where T wins): {avg_time_per_iter_diff_T_wins:.6f} s')
print(f'On average, T is {avg_percent_faster_per_iter_T:.2f}% faster than M per iteration when it wins')
print(f'Average time per iteration that M wins over T (only in cases where M wins): {avg_time_per_iter_diff_M_wins:.6f} s')
print(f'On average, M is {avg_percent_faster_per_iter_M:.2f}% faster than T per iteration when it wins')

# Imprimir estadísticas adicionales
print(f'\nTIME DIFFERENCE STATISTICS:')
print(f'---------------------------------')
print(f'When T wins in total time:')
print(f'  Average difference: {avg_time_diff_T_wins:.6f} s ({avg_percent_faster_T:.2f}%)')
print(f'  Maximum difference: {max_time_diff_T_wins:.6f} s ({max_percent_faster_T:.2f}%)')
print(f'  Minimum difference: {min_time_diff_T_wins:.6f} s ({min_percent_faster_T:.2f}%)')
print(f'When M wins in total time:')
print(f'  Average difference: {avg_time_diff_M_wins:.6f} s ({avg_percent_faster_M:.2f}%)')
print(f'  Maximum difference: {max_time_diff_M_wins:.6f} s ({max_percent_faster_M:.2f}%)')
print(f'  Minimum difference: {min_time_diff_M_wins:.6f} s ({min_percent_faster_M:.2f}%)')
print(f'\nWhen T wins in time per iteration:')
print(f'  Average difference: {avg_time_per_iter_diff_T_wins:.6f} s ({avg_percent_faster_per_iter_T:.2f}%)')
print(f'  Maximum difference: {max_time_per_iter_diff_T_wins:.6f} s ({max_percent_faster_per_iter_T:.2f}%)')
print(f'  Minimum difference: {min_time_per_iter_diff_T_wins:.6f} s ({min_percent_faster_per_iter_T:.2f}%)')
print(f'When M wins in time per iteration:')
print(f'  Average difference: {avg_time_per_iter_diff_M_wins:.6f} s ({avg_percent_faster_per_iter_M:.2f}%)')
print(f'  Maximum difference: {max_time_per_iter_diff_M_wins:.6f} s ({max_percent_faster_per_iter_M:.2f}%)')
print(f'  Minimum difference: {min_time_per_iter_diff_M_wins:.6f} s ({min_percent_faster_per_iter_M:.2f}%)')

# Calculamos estadísticas adicionales - con NaN tratados como 0
print(f'\nADDITIONAL STATISTICS:')
print(f'-------------------------')
print(f'Average time M: {time_M.fillna(0).mean():.6f} s')
print(f'Average time T: {time_T.fillna(0).mean():.6f} s')
print(f'Average lower bound M: {lb_M.fillna(0).mean():.2f}')
print(f'Average lower bound T: {lb_T.fillna(0).mean():.2f}')
print(f'Average upper bound M: {ub_M.fillna(0).mean():.2f}')
print(f'Average upper bound T: {ub_T.fillna(0).mean():.2f}')

# Análisis por cuartiles
print(f'\nQUARTILE ANALYSIS WHEN T WINS:')
print(f'--------------------------------')

# Cuartiles para tiempo de ejecución cuando T gana
if len(percent_faster_when_T_wins) > 0:
    q1_time = np.percentile(percent_faster_when_T_wins, 25)
    q2_time = np.percentile(percent_faster_when_T_wins, 50)  # mediana
    q3_time = np.percentile(percent_faster_when_T_wins, 75)
    print(f'Execution time improvement quartiles:')
    print(f'  Q1 (25%): {q1_time:.2f}%')
    print(f'  Q2 (50%, median): {q2_time:.2f}%')
    print(f'  Q3 (75%): {q3_time:.2f}%')
else:
    print(f'No data available for execution time quartiles')

# Cuartiles para tiempo por iteración cuando T gana
if len(percent_faster_per_iter_when_T_wins) > 0:
    q1_time_iter = np.percentile(percent_faster_per_iter_when_T_wins, 25)
    q2_time_iter = np.percentile(percent_faster_per_iter_when_T_wins, 50)  # mediana
    q3_time_iter = np.percentile(percent_faster_per_iter_when_T_wins, 75)
    print(f'Time per iteration improvement quartiles:')
    print(f'  Q1 (25%): {q1_time_iter:.2f}%')
    print(f'  Q2 (50%, median): {q2_time_iter:.2f}%')
    print(f'  Q3 (75%): {q3_time_iter:.2f}%')
else:
    print(f'No data available for time per iteration quartiles')

# Cuartiles para mejora en lower bound cuando T gana
if len(lb_percent_improvements) > 0:
    # Para lower bound, ordenamos de mayor a menor (ya que mayor es mejor)
    q1_lb = np.percentile(lb_percent_improvements, 75)  # 25% superior
    q2_lb = np.percentile(lb_percent_improvements, 50)  # mediana
    q3_lb = np.percentile(lb_percent_improvements, 25)  # 75% superior
    print(f'Lower bound improvement quartiles (higher is better, reversed order):')
    print(f'  Q1 (top 25%): {q1_lb:.2f}%')
    print(f'  Q2 (50%, median): {q2_lb:.2f}%')
    print(f'  Q3 (top 75%): {q3_lb:.2f}%')
else:
    print(f'No data available for lower bound quartiles')

# Estadísticas sobre porcentaje de mejora
print(f'Average percentage improvement when T has better lower bound: {avg_lb_percent_improvement:.2f}%')
print(f'Maximum percentage improvement when T has better lower bound: {max_lb_percent_improvement:.2f}%')
print(f'Minimum percentage improvement when T has better lower bound: {min_lb_percent_improvement:.2f}%')
print(f'Test number with best improvement: {best_test_number}')
print(f'Best improvement: {best_improvement_percent:.2f}%')
print(f'Lower bound M: {best_lb_M}')
print(f'Lower bound T: {best_lb_T}')

ub_M_np = ub_M.fillna(0).values
ub_T_np = ub_T.fillna(0).values 