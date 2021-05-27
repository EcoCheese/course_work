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

map<string,vector<myStruct>> dictionary;


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


void serial_ii(string path){
    string line;
    
    myStruct w;
    
    for(auto& p: std::__fs::filesystem::recursive_directory_iterator(path)) {
        
        ifstream myfile (p.path());
        w.path = p.path();
        
        if (myfile.is_open()) {
            
            getline(myfile, line);
 
            line = delete_punctuation(line);
            int itr = 1;
            string word = "";
            stringstream sstr(line);
            
            while (sstr >> word) {
                if (word != "br" && !word.empty() && word != "\0"){
                    w.position = itr;
                    dictionary[word].push_back(w);
                    itr++;
                }
            }
            
            
        } else {
            cout << "cannot open file" << endl;
        }
    }
    
    print_map(dictionary);
}

void find_query (string query) {
    
    if (dictionary.count(query) > 0){
        
        vector <myStruct> vect = dictionary.find(query)->second;
        
        for (unsigned j = 0; j < vect.size(); j++){
            cout << vect[j].path << " -- position " << vect[j].position << endl;
        }
        
    } else {
        cout << "There is no such query" << endl;
    }
}

int main(int argc, const char * argv[]) {
    
    string path = "/Users/i.kangin/Downloads/course_work/datasets/";
    int var = 0;
    bool created = false;
    string query = "";
    
    
    while (true) {
        
        cout << endl << "Choose variant:\n\t1 - serial\n\t2 - find query" << endl;
        
        cin >> var;
        
        switch (var) {
            case 1:
                serial_ii(path);
                created = true;
                break;
                
            case 2:
                
                if (created) {
                    cout << "Enter query: ";
                    cin >> query;
                    
                    find_query(query);
                } else {
                    cout << "Dictionary wasn't formed yet, try again later...";
                }
               
                
                break;
                
            default:
                break;
        }
    }
    
    return 0;
}
