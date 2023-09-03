#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <limits>
#include <stdexcept>

#include "file_reader.cpp"


class solver{
private:
    std::vector<std::string> variables;
    std::vector<std::vector<float>> a;
    std::vector<float> b, c, auxilliary_objective, objective;
    file_reader reader;
    bool auxilliary = false;
    float epsilon = 1e-7;

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

        phase1();
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
    void phase1(){
        // no extra stuff needed, the base solution (set everything to 0) is already feasible
        if (*std::min_element(b.begin(), b.end()) >= 0.) return ;

        auxilliary = true;
        // add the causilliary variable "extra"
        auxilliary_objective = std::vector<float>(variables.size(), 0);
        auxilliary_objective.push_back(1);
        variables.push_back("extra");

        for(int i = 0; i < a.size(); ++i) a[i].push_back(-1);
        c.push_back(0);// not needed

        // now extra enters the basis and the lack var of the row with the most megative right side leaves
        exchange_variables(find_min_id_b(), variables.size()-1);

        // phase 2
        phase2(auxilliary_objective);

        std::cout << "Auxilliary problem with solution: " << std::endl;
        show_problem();
        auxilliary = false;
        //now the extra variable can be removed again
        for(int i = 0; i < a.size(); ++i) a[i].pop_back();
        variables.pop_back();
        c.pop_back();
    }

    /**
     * @brief Finds out if a variable is basic (contains only one 1 in the column(the rest are 0's)) and returns the result and the row index of the 1
     * 
     */
    std::tuple<bool, int> is_basic_with_id(int col){
        int index = -1;
        for(int i = 0; i < a.size(); ++i){
            if(std::abs(a[i][col]) <= epsilon) continue;
            if(a[i][col] == 1){
                if (index == -1) index = i; // first 1
                else return {false, -1};
            }
            else return {false, -1};
        }
        return {true, index};
    }

    /**
     * @brief compute the value of the current assignment of variables
     * 
     */
    float compute_value(){
        // this vextor checks if this row has already been used, 
        //there might accidentally be another row with only one 1 and the rest 0's
        std::vector<bool> row_used(a.size(), false);

        if (auxilliary){
            std::tuple<bool, int> p = is_basic_with_id(objective.size()-1);
            if(! std::get<0>(p)){// this is the optimum 
                return 0;
            }
            else{
                return -1;// not the actual value, but we only care if the value is 0
            }
        }
        float solution = 0;
        for(int i = 0; i < objective.size(); ++i){
            float var = objective[i];
            if(std::abs(var) > epsilon){
                std::tuple<bool, int> p = is_basic_with_id(i);
                int row = std::get<1>(p);
                if(std::get<0>(p) && !row_used[row]){
                    solution += b[row] * var;
                    row_used[row] = true;
                }
            }
        }
        
        return solution;
    }
    
    /**
     * @brief Find a variable which improves the objective if made basic
     * 
     */
    int find_improveable_var(std::vector<float> &obj){
        // there are several ways to do this
        // selecting the most negative one would make the program converge faster
        // but that version is not guaranteed to converge(might have cycles)

        std::vector<bool> row_used(obj.size(), false);

        for(int i = 0; i < obj.size(); ++i){
            if(obj[i] < -epsilon){//can improve
                std::tuple<bool, int> p = is_basic_with_id(i);
                if(!std::get<0>(p) || (row_used[std::get<1>(p)])) return i;
                row_used[std::get<1>(p)] = true;//mark as basic
            }
        } 
        return -1;// cannot improve
    }

    /**
     * @brief find the row of the pivot element (and thereby te variable which leaves the basis)
     * 
     */
    int find_row(int col){
        int row = -1;
        float min_ratio = std::numeric_limits<float>::max();
        int smaller_equal_0 = 0;
        for(int i = 0; i < b.size(); ++i){
            if(a[i][col] <= epsilon){
                ++smaller_equal_0;
                continue;
            }
            float ratio = b[i]/a[i][col];
            if(ratio < min_ratio){
                min_ratio = ratio;
                row = i;
            }
        }
        if(smaller_equal_0 == b.size()){
            std::cout << "The problem is unbounded" << std::endl;
            throw std::invalid_argument("The problem is unbounded");
            // theoretically you could also return a point + improving direction
            return -1;
        }
        return row;
    }


    /**
     * @brief optimize the problem iteratively
     * 
     */
    float phase2(std::vector<float> &obj){
        // check optimality
        while(true){
            bool optimal = find_improveable_var(obj) == -1;
            
            if(optimal && auxilliary){
                // extra might be basic with val 0, (need to fix) (problem is degenerate)
                // non basic (perfect)
                // basic with non 0 -> no solution
                std::tuple<bool, int> p =is_basic_with_id(obj.size()-1);

                if(std::get<0>(p)){// is basic
                    if(std::abs(b[std::get<1>(p)]) > epsilon){// not 0->unsolvable
                        std::cout << "problem has no solution" << std::endl;
                        throw std::invalid_argument("Problem is not solvable");
                    }
                    else{// fix form
                        exchange_variables(std::get<1>(p), obj.size()-1);
                        // now extra should be basic with same solution
                    }
                }
            }

            if (optimal){
                return compute_value();
            }
            // find the variable to change:
            int col = find_improveable_var(obj);
            int row = find_row(col);

            // pivot
            exchange_variables(row, col);

            //show_problem();
            //reader.show_problem(variables, a, b, c);
        }
        
        return -1;
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

        
        for(int i = 0; i < variables.size(); ++i){// pivot row
            if(i != col) a[row][i] /= a[row][col]; 
        }
        

        // change the auxilliary problem:
        if(auxilliary){
            for (int i = 0; i < variables.size(); ++i){
                if(i != col) auxilliary_objective[i] -=  a[row][i]*auxilliary_objective[col]; 
            }
            auxilliary_objective[col] = 0;
        }

        // change the  problem:
        for (int i = 0; i < variables.size(); ++i){
            if(i != col) c[i] -=  a[row][i]*c[col]; 
        }
        c[col] = 0;
        

        // change a
        for (int j = 0; j < a.size(); ++j){// other rows
            if(j == row) continue;
            for (int i = 0; i < variables.size(); ++i){
                if (i == col) continue;
                a[j][i] -=  a[j][col]*a[row][i]; 
            }
        }

        for(int i = 0; i < a.size(); ++i){// pivot col
            if (i == row) continue;
            a[i][col] = 0;
        }
        a[row][col] = 1;
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
        std::cout << "minimize: " << std::endl;
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
        float solution = phase2(c);
        std::cout << std::endl <<"optimized problem: " << std::endl; 
        reader.show_problem(variables, a, b, c);
        std::cout << std::endl << "maximum value: " << solution << std::endl;
        // I could also have returned the assignments

        return solution;
    }
};
