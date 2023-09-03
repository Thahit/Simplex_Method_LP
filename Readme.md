# Simplex Method Linear Programming
The program is not meant to be extremely optimized but was created to obtain a better understanding of the theory behind the method.
## Input
The examples are text files because this allows users to input problems without having to put in the effort of creating some strange matrix, which they might not understand. 
Also, the inputs are intentionally not very consistent with spaces because this is what is bound to happen when used.
Examples can be found in the examples folder.

### Form
The first line is the formula which is to be maximized. Consecutive lines are for constraints. Variables start with letters (e.g., x, y, x2, ...). Each variable should only appear once per line, otherwise the first occurrence is taken. Variables belong to the left side of constraints. Either use <= or >= constraints(= constraints can be built by adding both a <= and a >= constraint). The non-negativity constraints x >= 0 are implied by the program and don't need to be added.

### solutions of the examples:
The true solutions and the results of the solver are the same:
ex1: 10400
ex2: 0.6
ex3: unbounded
ex4: no solution