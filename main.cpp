#include <iostream>
#include <fstream>


int main(){
    std::ios_base::sync_with_stdio(false); 
    
    std::ofstream myfile;
    myfile.open ("examples/ex1.txt");

    return 0;
}