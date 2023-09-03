#include <iostream>
#include "Solver/file_reader.cpp"


int main(){
    std::ios_base::sync_with_stdio(false); 
    file_reader reader;
    reader.read_file("examples/ex1.txt");

    return 0;
}