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
#include <thread>
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

void open_serial_files(string path, myStruct ms){
    
    string line;
    
    for(auto& p: std::__fs::filesystem::recursive_directory_iterator(path)) {
        
        ifstream myfile (p.path());
        ms.path = p.path();
        
        if (myfile.is_open()) {
            
            getline(myfile, line);
 
            line = delete_punctuation(line);
            int itr = 1;
            string word = "";
            stringstream sstr(line);
            
            while (sstr >> word) {
                if (word != "br" && !word.empty() && word != "\0"){
                    ms.position = itr;
                    dictionary[word].push_back(ms);
                    itr++;
                }
            }
            
            myfile.close();
            
        } else {
            cout << "cannot open file" << endl;
        }
    }
}

void something (string path, myStruct ms) {
    
    string line;
    ifstream myfile (path);
    ms.path = path;
    
    if (myfile.is_open()) {
        
        getline(myfile, line);

        line = delete_punctuation(line);
        int itr = 1;
        string word = "";
        stringstream sstr(line);
        
        while (sstr >> word) {
            if (word != "br" && !word.empty() && word != "\0"){
                ms.position = itr;
                dictionary[word].push_back(ms);
                itr++;
            }
        }
        
        myfile.close();
        
    } else {
        cout << "cannot open file" << endl;
    }
    
}

struct INPUT {
    string path;
    myStruct ms;
};

void *newFunc(void * arguments){
    
    struct INPUT *in = (struct INPUT *)arguments;
    
    string line;
    ifstream myfile (in->path);
    in->ms.path = in->path;
    
    if (myfile.is_open()) {
        
        getline(myfile, line);

        line = delete_punctuation(line);
        int itr = 1;
        string word = "";
        stringstream sstr(line);
        
        while (sstr >> word) {
            if (word != "br" && !word.empty() && word != "\0"){
                in->ms.position = itr;
                dictionary[word].push_back(in->ms);
                itr++;
            }
        }
        
        myfile.close();
        
    } else {
        cout << "cannot open file" << endl;
    }
    
    return NULL;
}



void multi(myStruct ms, string path){
    
    pthread_t threads[2];
    
    struct INPUT in;
    
    
    int rc, end, n = 2;
    vector<string>* Paths = new vector<string>;

    for (auto& p : std::__fs::filesystem::directory_iterator(path)){
        Paths->push_back(p.path().string());
    }
    
    
    
    for(int i = 0; i < 2; i++){
        in.path = Paths->at(i);
        rc = pthread_create(&threads[i], NULL, &newFunc, (void *)&in);
        if (rc != 0) {
            printf("Thread creation failed");
            exit(-1);
        }
    }
    
    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }

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
    myStruct w;
    
    
    while (true) {
        
        cout << endl << "Choose variant:\n\t1 - serial\n\t2 - multi\n\t3 - find query" << endl;
        
        cin >> var;
        
        switch (var) {
            case 1:
                
                open_serial_files(path, w);
                print_map(dictionary);
                created = true;
                break;
                
            case 2:
                multi(w, path);
                print_map(dictionary);
                created = true;
                break;
                
            case 3:
                
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
