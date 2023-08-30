#include <fstream>
#include <iostream>


class file_reader{
private:
public:
    void read_file(const std::string& input_path){
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

            std::cout << "maximize: " << buffer <<  std::endl;
        }
        else{
            std::cout << "no file" <<  std::endl;
        }

        myfile.close();
    }
};
