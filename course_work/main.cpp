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
//pthread_mutex_t m;


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

//MARK: Serial

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

//MARK: Multi

struct INPUT {
    int start;
    int end;
//    string path;
    myStruct ms;
    vector<string> ppp; //paths
};

void *newFunc(void * arguments){
    
//    pthread_mutex_lock(&m);
    
    struct INPUT *in = (struct INPUT *)arguments;
    
    string line;
    ifstream myfile (in->ppp.at(in->start));
    in->ms.path = in->ppp.at(in->start);
    
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
    
//    pthread_mutex_unlock(&m);
    
    return NULL;
}



void multi(myStruct ms, string path){
    
    pthread_t threads[2];
    
    
    
//    pthread_mutex_init(&m, NULL);
    
    int rc, n = 2;
    vector<string>* Paths = new vector<string>;

    for (auto& p : std::__fs::filesystem::directory_iterator(path)){
        Paths->push_back(p.path().string());
    }
    
    int size = Paths->size();
    
    struct INPUT in[size];
    

    
    
    for(int i = 0; i < 2; i++){
//        in.path = Paths->at(i);
//        in = malloc(sizeof(INPUT));
        in[i].ppp = *Paths;
        in[i].start = Paths->size() / (float)n * i;
        in[i].end = Paths->size() / (float)n * (i + 1);
        rc = pthread_create(&threads[i], NULL, &newFunc, (void *)&in[i]);
        if (rc != 0) {
            printf("Thread creation failed");
            exit(-1);
        }
    }
    
    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }

//    pthread_mutex_destroy(&m);
}

//MARK: Lookup query

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
                
            case 4:
                dictionary.clear();
                break;
                
            default:
                break;
        }
    }
    
    return 0;
}
