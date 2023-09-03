#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>  
#include <regex>
#include <tuple>


class file_reader{
private:
    std::string get_text(const std::string& input_path){
        /**
         * @brief reads the file from the address described in input_path, and returns a char* of the content
         * 
         */
        std::ifstream myfile;
        myfile.open(input_path);
        if (myfile) {
            myfile.seekg (0, myfile.end);
            int length = myfile.tellg();// get len of file
            myfile.seekg (0, myfile.beg);
            //std::cout << "lenght: " << length <<  std::endl;

            // get text
            char * buffer = new char [length];
            myfile.read(buffer, length);

            // cut off trailing \n
            while(buffer[length-1] == '\n') --length;
            
            buffer[length] = '\0';
            
            std::cout << "maximize: " << buffer <<  std::endl <<  std::endl;

            myfile.close();
            std::string res = buffer;

            free(buffer);// cleanup memory (don't know if this gets everything)

            return res;
        }
        else{
            std::cout << "no file" <<  std::endl;
            throw std::invalid_argument("There is no file");
        }

        // kind of pointless
        myfile.close();
        return NULL;
    }

    std::vector<std::string> find_all_variables(std::string &text){
        /**
         * @brief find all variables([a-zA-Z][a-zA-Z0-9]*) in the text
         * 
         */
        const std::regex r("[a-zA-Z][a-zA-Z0-9]*");  
        std::smatch sm;
        std::string text_cp = text;
        std::set<std::string> variables_set;
        while (regex_search(text_cp, sm, r)) {
            variables_set.insert(sm.str(0));
    
            // suffix to find the rest of the string.
            text_cp = sm.suffix().str();
        }
        //std::cout << "variables: ";
        //for(std::string s: variables_set) std::cout << s;
        std::vector<std::string> variables(variables_set.begin(), variables_set.end());// to have an order
        return variables;
    }

    std::tuple<std::vector<std::vector<float>>, std::vector<float>, std::vector<float> >  
                        create_problem(const int &num_lines, std::string &text,
                        std::vector<std::string> &variables){
        /**
         * @brief chreate a format the solver can work with
         * 
         */
        
        // one col per line in the problem - the objective
        // one row per variable + slack variable(= number of lines -1) 
        std::vector<std::vector<float>> a(num_lines - 1, std::vector<float>(variables.size() + num_lines-1, 0.));
        std::vector<float> c(variables.size() + num_lines -1, 0);// to optimize
        std::vector<float> b(num_lines -1, 0);// right side (= a column)

        // separate lines
        std::vector<std::string> lines;
        auto ss = std::stringstream{text};

        for (std::string line; std::getline(ss, line, '\n');) lines.push_back(line);


        // find multipliers of variable
        std::smatch sm;
        std::string number_regex_string = "[+-]?[0-9]*[.]?[0-9]*";

        for(int i = 0; i < variables.size(); ++i){
            std::regex reg_find_multiples_with_var(number_regex_string + variables[i]); // find var with multiplier
            if(std::regex_search (lines[0].cbegin(), lines[0].cend(), sm, reg_find_multiples_with_var)){// find var
                //std::cout << sm[0] << std::endl;
                std::regex reg_rm_var(variables[i]);
                std::string ministr = sm[0];
                ministr = std::regex_replace(ministr, reg_rm_var, "");
                //std::cout << ministr << std::endl;
                if(ministr == "")c[i] = 1;
                else if(ministr == "-")c[i] = -1;
                else{
                    c[i] = std::stof(ministr);// add multiplier to c vector
                }
            }
        }

        // do same for a matrix (vector)
        for(int constraint = 0; constraint<a.size(); ++constraint){
            for(int i = 0; i < variables.size(); ++i){
                std::regex reg_find_multiples_with_var(number_regex_string + variables[i]); // find var with multiplier
                if(std::regex_search (lines[constraint+1].cbegin(), lines[constraint+1].cend(), sm, reg_find_multiples_with_var)){// find var
                    //std::cout << sm[0] << std::endl;
                    std::regex reg_rm_var(variables[i]);
                    std::string ministr = sm[0];
                    ministr = std::regex_replace(ministr, reg_rm_var, "");
                    //std::cout << ministr << std::endl;
                    if(ministr == "") a[constraint][i] = 1;
                    else if(ministr == "-") a[constraint][i] = -1;
                    else a[constraint][i] = std::stof(ministr);// add multiplier to c vector
                }
            }
        }

        // now c
        std::string bigger_smaller_regex_str = "[<>]=";
        for(int constraint = 0; constraint<a.size(); ++constraint){
            std::regex reg_find_left_side(bigger_smaller_regex_str + number_regex_string); // find var with multiplier
            if(std::regex_search (lines[constraint+1].cbegin(), lines[constraint+1].cend(), sm, reg_find_left_side)){// find var
                // only <= is ok, otherwise we need to multiply everythig with -1
                std::string ministr = sm[0];
                if(ministr[0] == '<'){// good
                    std::regex reg_rm_var(bigger_smaller_regex_str);
                    ministr = std::regex_replace(ministr, reg_rm_var, "");
                    //std::cout << ministr << std::endl;
                    if(ministr == "") c[constraint] = 1;
                    else if(ministr == "-") c[constraint] = -1;
                    else c[constraint] = std::stof(ministr);// add multiplier to c vector
                }
                else{
                    std::regex reg_rm_var(bigger_smaller_regex_str);
                    ministr = std::regex_replace(ministr, reg_rm_var, "");
                    //std::cout << ministr << std::endl;
                    if(ministr == "") c[constraint] = -1;
                    else if(ministr == "-") c[constraint] = 1;
                    else c[constraint] = - std::stof(ministr);// add multiplier to c vector
                    
                    // now fix this row:
                    for(int k = 0; k < a[0].size(); ++k){
                        if (a[constraint][k])a[constraint][k] *= -1; 
                        // = only do if not 0, this would not hurt, but -0 looks stupid
                    }
                }
            }
        }        


        // add slack variables to a
        for(int i = 0; i < a.size(); ++i) a[i][variables.size() + i] = 1;

        // append slack variables to variables for readability
        for(int i = 0; i < num_lines-1; ++i) variables.push_back("slack"+std::to_string(i));
        
        show_problem(variables, a, b, c);

        return {a, b, c};
    }

public:
    void show_problem(const std::vector<std::string> &variables,
                    std::vector<std::vector<float>> &a,
                    std::vector<float> &b, std::vector<float> &c
                    ){
        /**
         * @brief print the current problem in a somewhat readable form
         * 
         */
        std::cout << "variables: " << std::endl;
        for (auto i: variables) std::cout << i << '\t';
        std::cout << "right_side"<< std::endl;
        std::cout << "maximize: " << std::endl;
        for (auto i: c) std::cout << i << '\t';
        std::cout << std::endl << "constraints:"<< std::endl;
        for(int j = 0; j < a.size(); ++j){
            for (auto i: a[j]) std::cout << i << '\t';
            std::cout << "= "<< b[j] << std::endl <<  std::endl;
        }

    }
    std::tuple<std::vector<std::string>, std::tuple<std::vector<std::vector<float>>, std::vector<float>, std::vector<float> >>
             read_file(const std::string& input_path){
        /**
         * @brief reads the file from the inpit path and returns the variables, \ 
         * and the vectors a(left side of constraints), b(right side of constraints), c(formula to optimize) in standard form 
         * 
         */
        std::string text = get_text(input_path);

        int num_lines = std::count( text.begin(), text.end(), '\n' ) +1;// undercounts by 1, is probably thechnically an uint
        //std::cout << "number of lines: "<< num_lines << std::endl;

        // remove empty spaces and *
        std::regex reg_rm_spaces("[ *]");
        text = std::regex_replace(text, reg_rm_spaces, "");
        //std::cout << "maximize: " << text <<  std::endl; 

        // find variables in the text
        std::vector<std::string> variables = find_all_variables(text);

        // create output matrix (vector)
        std::tuple<std::vector<std::vector<float>>, std::vector<float>, 
                std::vector<float> > problem = create_problem(num_lines, text, variables);
        
        //std::cout << "maximize: " << text <<  std::endl;
        return {variables, problem};
    }
};
