#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>

#include "file_reader.cpp"


class solver{
private:
    std::vector<std::string> variables;
    std::vector<std::vector<float>> a;
    std::vector<float> b, c, auxilliary_objective, objective;
    file_reader reader;
    bool solvable = false, auxilliary = false;

    /**
     * @brief prepare for solving the problem by loading it
     * 
     */
    void init_problem(const std::string& input_path){
        std::tuple<std::vector<std::string>, std::tuple<std::vector<std::vector<float>>, 
            std::vector<float>, std::vector<float> >> problem = reader.read_file(input_path);
        
        variables = std::get<0>(problem);

        a = std::get<0>(std::get<1>(problem));
        b = std::get<1>(std::get<1>(problem));
        c = std::get<2>(std::get<1>(problem));

        objective = c;
        
        // negate objective
        for(int i = 0; i < c.size(); ++i) c[i] *= -1;

        solvable = phase1();
    }

    /**
     * @brief find the index of the smallest element in b.
     * 
     */
    int find_min_id_b(){
        // not complicated but this makes the code easier to read
        return std::min_element(b.begin(), b.end()) - b.begin();
    }

    /**
     * @brief Finds a first feasible solution. An auxilliary problem is created if setting the solution all slack variables = 0 was not feasible.
     * 
     */
    bool phase1(){
        // no extra stuff needed, the base solution (set everything to 0) is already feasible
        if (*std::min_element(b.begin(), b.end()) >= 0.) return true;

        auxilliary = true;
        // add the causilliary variable "extra"
        auxilliary_objective = std::vector<float>(variables.size(), 0);
        auxilliary_objective.push_back(1);
        variables.push_back("extra");

        for(int i = 0; i < a.size(); ++i) a[i].push_back(-1);
        c.push_back(0);

        // now extra enters the basis and the lack var of the row with the most megative right side leaves

        exchange_variables(find_min_id_b(), variables.size()-1);
        
        // phase 2

        std::cout << "Auxilliary problem with solution: " << std::endl;
        show_problem();
        auxilliary = false;
        return true;
    }

    /**
     * @brief 
     * 
     */
    float phase2(){

        //std::cout << "optimal solution: " << std::endl;
        //show_problem(variables, a, b, c, auxilliary_objective);
        return 0;
    }

    /**
     * @brief perform a pivot = make one variable enter the basis (only has one 1) and another one leave if the correct values are provided.
     * 
     */
    void exchange_variables(int row, int col){
        // I assume the mehtod is used correctly and dont check if it makes sense(for example entering = leaving...)
        //std::cout << "row: " << row << "\tcol:" << col <<std::endl;
        // change b's
        b[row] /= a[row][col]; 
        for(int i = 0; i < b.size(); ++i){
            if(i != row) b[i] -= a[i][col]*b[row]; 
        }

        // change a
        for(int i = 0; i < variables.size(); ++i){// pivot row
            a[row][i] /= a[row][col]; 
        }

        for (int j = 0; j < a.size(); ++j){// other rows
            if(j == row) continue;
            for (int i = 0; i < variables.size(); ++i){
                if (i == col) continue;
                a[j][i] -=  a[j][col]*a[row][i]; 
            }
        }
        for(int i = 0; i < a.size(); ++i){
            if (i == row) continue;
            a[i][col] = 0;
        }

        // change the auxilliary problem:
        if(auxilliary){
            for (int i = 0; i < variables.size(); ++i){
                auxilliary_objective[i] -=  a[row][i]*auxilliary_objective[col]; 
            }
        }
        
    }
public:
    /**
     * @brief visualize the problem including the auxilliary objective
     * 
     */
    void show_problem(){
        // is very similar to the method with the same name in the file_reader class
        std::cout << "variables: " << std::endl;
        for (auto i: variables) std::cout << i << '\t';
        std::cout << "right_side"<< std::endl;
        std::cout << "maximize: " << std::endl;
        for (auto i: c) std::cout << i << '\t';
        std::cout << std::endl << "auxilliary objective: " << std::endl;
        for (auto i: auxilliary_objective) std::cout << i << '\t';
        std::cout << std::endl << "constraints:"<< std::endl;
        for(int j = 0; j < a.size(); ++j){
            for (auto i: a[j]) std::cout << i << '\t';
            std::cout << "= "<< b[j] << std::endl <<  std::endl;
        }

    }

    /**
     * @brief Construct a new solver and initializes the problem
     * 
     */
    solver(const std::string& input_path){
        
        init_problem(input_path);
    }

    /**
     * @brief updates the solver with a new problem
     * 
     */
    void change_problem(const std::string& input_path){
        
        init_problem(input_path);
    }

    /**
     * @brief Tries finding an optimal solution and returns it. -1 is returned if there is no optimal solution.
     * 
     */
    float solve(){
        if(!solvable) {
            std::cout << "the problem has no solution." << std::endl;
            return -1.;
        }

        float solution = phase2();

        return solution;
    }
};
