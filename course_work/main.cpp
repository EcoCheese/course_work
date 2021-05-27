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
#include <sstream>

using namespace std;


struct myStruct {
    string path;
    int position;
};


void print_map (map <string, vector <myStruct> > dict) {
    
    for(map<string, vector<myStruct> >::iterator ii=dict.begin(); ii!=dict.end(); ++ii){
        
           cout << (*ii).first << ":\n";
        
           vector <myStruct> vect = (*ii).second;
        
           for (unsigned j = 0; j < vect.size(); j++){
               cout << vect[j].path << " -- position " << vect[j].position << endl;
           }
           cout << endl;
    }
}

string delete_punctuation(string line){

    
    for_each(line.begin(), line.end(), [](char & c){
        if(ispunct(c) && c != '\'' && c != '\n'){
            c = ' ';
        }
        c = ::tolower(c);
    });
    
    return line;
}

void fill_map(){
    
}

int main(int argc, const char * argv[]) {
    
    string path = "/Users/i.kangin/Downloads/course_work/datasets/";
    string line;
    
    myStruct w;
    
    map<string,vector<myStruct>> dictionary;
    
    for(auto& p: std::__fs::filesystem::recursive_directory_iterator(path)) {
        
        ifstream myfile (p.path());
        w.path = p.path();
        
        if (myfile.is_open()) {
            getline(myfile, line);
            
            cout << line << endl;
            
            line = delete_punctuation(line);
            
            cout << line << endl;
            
            
            int itr = 1;
            
            string word = "";

            stringstream sstr(line);
            
            while (sstr >> word) {
                if (word != "br" && !word.empty() && word != "\0"){
                    w.position = itr;
                    dictionary[word].push_back(w);
                    itr++;
                    cout << word << endl;
                }
            }
            
            
        } else {
            cout << "cannot open file" << endl;
        }
    }
    
         
    print_map(dictionary);
    

    return 0;
}
