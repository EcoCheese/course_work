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
#include <ctime>

using namespace std;


struct myStruct {
    string path;
    int position;
};

map<string,vector<myStruct>> dictionary;
pthread_mutex_t m;


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
    
    std::clock_t start;
    start = std::clock();
    
    for(auto& p: std::__fs::filesystem::recursive_directory_iterator(path)) {
        
        if (p.path().extension() != ".txt")
            continue;
    
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
    
    std::cout << "Time of serial: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
}

//MARK: Multi

struct INPUT {
    int start;
    int end;
    myStruct ms;
    vector<string> paths;
};

void *newFunc(void * arguments){
    
    
    struct INPUT *in = (struct INPUT *)arguments;
    
    string line;
    ifstream myfile ;
    
    
    
    for (int i = in->start; i < in->end; i++){
        myfile.open(in->paths.at(i));
        in->ms.path = in->paths.at(i);
        
        if (myfile.is_open()) {
            
            
            getline(myfile, line);

            line = delete_punctuation(line);
            int itr = 1;
            string word = "";
            stringstream sstr(line);
            
            pthread_mutex_lock(&m);
            
            while (sstr >> word) {
                if (word != "br" && !word.empty() && word != "\0"){
                    in->ms.position = itr;
                    dictionary[word].push_back(in->ms);
                    itr++;
                }
            }
            
            pthread_mutex_unlock(&m);
            
            myfile.close();
            
        } else {
            cout << "cannot open file" << endl;
        }
        
        
    }
    
    return NULL;
}



void multi(myStruct ms, string path){
    
    pthread_mutex_init(&m, NULL);
    
    int t, rc;
    
    cout << "Enter number of threads (>1):";
    cin >> t;
    t--;
    
    if(t == 0){
        cout << "Using serial version instead!" << endl;
        open_serial_files(path, ms);
        return;
    }
    
    pthread_t threads[t];
    
    vector<string>* Paths = new vector<string>;

    for (auto& p : std::__fs::filesystem::recursive_directory_iterator(path)){
        if (p.path().extension() != ".txt")
            continue;
        Paths->push_back(p.path().string());
    }
    
    struct INPUT in[Paths->size()];
    
    std::clock_t start;
    start = std::clock();

    for(int i = 0; i < t; i++){
        in[i].paths = *Paths;
        in[i].start = Paths->size() / (float)t * i;
        in[i].end = Paths->size() / (float)t * (i + 1);
        rc = pthread_create(&threads[i], NULL, &newFunc, (void *)&in[i]);
        if (rc != 0) {
            printf("Thread creation failed");
            exit(-1);
        }
    }
    for (int i = 0; i < t; i++) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_mutex_destroy(&m);
    
    std::cout << "Time of multi: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
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
    
    string path = "/Users/i.kangin/Downloads/course_work/datasets";
    int var = 0;
    bool created = false;
    string query = "";
    myStruct w;
    
    while (true) {
        
        cout << endl << "Choose variant:\n\t1 - serial\n\t2 - multi\n\t3 - find query\n\t4 - erase dictionary\n\t5 - print dictionary" << endl;
        cin >> var;
        
        switch (var) {
            case 1:
                open_serial_files(path, w);
//                print_map(dictionary);
                created = true;
                break;
                
            case 2:
                multi(w, path);
//                print_map(dictionary);
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
                created = false;
                cout << "Dictionary erased.";
                break;
                
            case 5:
                print_map(dictionary);
                break;
                
            default:
                break;
        }
    }
    return 0;
}
