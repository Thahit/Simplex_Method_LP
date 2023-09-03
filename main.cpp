#include <iostream>

#include "Solver/solver.cpp"


int main(){
    std::ios_base::sync_with_stdio(false); 
    solver mySolver("examples/ex2.txt");
    float solution = mySolver.solve();
    return 0;
}
