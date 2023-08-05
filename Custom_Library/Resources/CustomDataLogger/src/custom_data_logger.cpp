        #include <iostream>
        #include <iomanip>
        #include <fstream>
        #include <ctime>
        
        #include "custom_data_logger.h"
        
        using namespace std;
        
        string newName = ""; 
        
        void createFile() {
            
            string name = "ExpLog";
            int i = 1;
            newName = name;
            while(true){
            
                ifstream file(newName);
                if(file.good()){
                 
                    newName = name + to_string(i);
                    i++;
                    
                } else {
                
                    break;
                    
                }
                       
                
            }
            
            ofstream file(newName);
            file.close();
           
        }
        
        // This function opens a text file and appends data into it.
        // The function will accept an array of unknown size, and will
        // loop through the array and append the data to the file.
        
        void appendDataToFile(double exp_data[], int num_params)
        {
            
            ofstream myfile;
            myfile.open (newName, ios::app);
            
            for (int i = 0; i < num_params-1; i++) {
                myfile << exp_data[i] << ","; 
            }
            
            myfile << exp_data[num_params-1];
            myfile << endl;
            
            myfile.close();
        }