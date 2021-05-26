//
//  main.cpp
//  course_work
//
//  Created by Ilya Kangin on 26.05.2021.
//

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <map>

using namespace std;
//namespace fs = std::filesystem;

struct myStruct {
    string path;
    int position;
};


int main(int argc, const char * argv[]) {
    
    string path = "/Users/i.kangin/Downloads/course_work/datasets";
    string filepath;
    string line;
    string temp;
    
    myStruct w;
    
    map<string,vector<myStruct>> dictionary;
    
    for(auto& p: std::__fs::filesystem::recursive_directory_iterator(path)) {
        
        std::cout << p.path() << endl;
        
        ifstream myfile (p.path());
                
        w.path = p.path();
        
        if (myfile.is_open()) {
            getline(myfile, line);
            
            std::for_each(line.begin(), line.end(), [](char & c){
                if(ispunct(c)){
                    c = '\0';
                }
                c = ::tolower(c);
            });
            
            
            int itr = 1;
            
            string word = "";
            for (auto x : line){
                if (x == ' ') {
                    w.position = itr;
                    dictionary[word].push_back(w);
                    
                    cout << word << endl;
                    word = "";
                    itr++;
                } else {
                    word = word + x;
                }
            }
            
        } else {
            cout << "cannot open file" << endl;
        }
    }
    
    cout << endl << endl;
     
    for(map<string, vector<myStruct> >::iterator ii=dictionary.begin(); ii!=dictionary.end(); ++ii){
           cout << (*ii).first << ":\n";
           vector <myStruct> inVect = (*ii).second;
           for (unsigned j=0; j<inVect.size(); j++){
               cout << inVect[j].path << " -- position" << inVect[j].position << endl;
           }
           cout << endl;
    }

    return 0;
}
