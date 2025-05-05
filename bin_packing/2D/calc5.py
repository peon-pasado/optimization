from pulp import LpProblem, LpVariable, LpMinimize, PULP_CBC_CMD, LpInteger, value

def solve_minimization_problem(n):
    try:
        # Iterar sobre todos los posibles valores de L y R
        nil = 0.
        for L in range(2, n + 1):
            for R in range(2, n + 1):
                if R < L:
                    #print(f"{nil:10.10f}", end=" ")
                    continue
                # Crear el problema de minimización
                problem = LpProblem(f"Minimize_Sum_xi_L{L}_R{R}", LpMinimize)

                # Crear variables: x_i >= 0 y enteras para i en [L, R]
                x = {i: LpVariable(f"x_{i}", lowBound=0, cat=LpInteger) for i in range(L, R + 1)}

                # Definir la función objetivo: minimizar \sum_{L <= i <= R} x_i / i**2
                problem += sum(x[i] * (1 / (i**2)) for i in range(L, R + 1))

                # Restricción 1: 1 - 1/L - 1/R < \sum_{L <= i <= R} x_i / i
                problem += sum(x[i] * (1 / i) for i in range(L, R + 1)) >= 1 - 1 / L - 1 / R + 1/100000, f"Constraint_1_L{L}_R{R}"

                # Restricción 2: \sum_{L <= i <= R} x_i / i <= 1 - 1/L
                problem += sum(x[i] * (1 / i) for i in range(L, R + 1)) <= 1 - 1 / L + 1/100000., f"Constraint_2_L{L}_R{R}"

                # Resolver el problema
                problem.solve(PULP_CBC_CMD(msg=False, gapRel=0.0001, timeLimit=60))

                # Verificar si se encontró una solución óptima
                if problem.status == 1:  # status == 1 significa "Óptimo"
                    
                    sm = 0
                    for i in range(L, R):
                        sm += x[i].varValue > 0 and x[i+1].varValue == 0
                    if sm > 0:
                        print(f"Problem Model for L={L}, R={R}:")
                        problem.writeLP("p.lp")  # Esto imprime la formulación del problema en formato .lp
                        print("\n")

                        for i in range(L, R + 1):
                            if x[i].varValue > 0:
                                print(i, x[i].varValue, end=" ")
                        print("")
                        print(sm, L, R)
                        print(f"{1. / L / L + value(problem.objective):10.10f}")

                    #if L == 2 and R == 8:
                    #    print(f"Optimal solution for L={L}, R={R}:")
                    #    for i in range(L, R + 1):
                    #        print(f"x[{i}] = {x[i].varValue}")
                    #print(f"Objective Value: {value(problem.objective)}\n")
                    #print(f"{1. / L / L + value(problem.objective):10.10f}", end=" ")
                #else:
                    #print(f"No optimal solution for L={L}, R={R}.\n")
            #print("\n")

    except Exception as e:
        print(f"Error: {e}")

# Ejemplo de uso
#n = int(input("Enter the value of n (n >= 2): "))
n = int(input())
if n < 2:
    print("Invalid value for n. Please ensure n >= 2.")
else:
    solve_minimization_problem(n)
