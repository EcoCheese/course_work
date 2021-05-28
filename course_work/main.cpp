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

// структура данных для хранения пути к файлу
// и позиции индекса
struct indexes {
    string path;
    int position;
};

// ассоциативный массив, который служит словарём
map< string,vector<indexes> > dictionary;
//объявление мютекса
pthread_mutex_t m;

// метод для вывода значений динамического массива
void print_vector (vector<indexes> vect){
    for (unsigned j = 0; j < vect.size(); j++){
        cout << vect[j].path << " -- position " << vect[j].position << endl;
    }
}

// метод для вывода данных ассоциативного массива
void print_map (map <string, vector <indexes> > dict) {
    for(map<string, vector<indexes> >::iterator ii=dict.begin(); ii!=dict.end(); ++ii){
        cout << (*ii).first << ":\n";
        print_vector((*ii).second);
        cout << endl;
    }
}

// метод для удаления знаков препинания в строке
/// сюда же можно добавить и удаление чисел
string delete_punctuation(string line){
    for_each(line.begin(), line.end(), [](char & c){
        if(ispunct(c) && c != '\''){
            c = ' ';
        }
        c = ::tolower(c);
    });
    return line;
}

// MARK: Serial
// Реализация прямого метода инвертированного индекса.
// Принимает значения пути к файлу и струкртуры
// которая объявлена в main()

void serial_inverted_index(string path, indexes ind){
    
    string line;
    // начинаем отсчёт времени
    std::clock_t start;
    start = std::clock();
    
    // рекурсивно проходим по даному нам пути к файлам
    for(auto& p: std::__fs::filesystem::recursive_directory_iterator(path)) {
        
        //выбираем только файлы с расширением .txt
        if (p.path().extension() != ".txt")
            continue;
        
        ifstream myfile (p.path());
        ind.path = p.path();
        
        if (myfile.is_open()) {
            
            getline(myfile, line);
 
            line = delete_punctuation(line);
            int itr = 1;    // итератор для определения позиции
            string word = "";
            stringstream sstr(line);
            
            while (sstr >> word) {
                // в данном случае делаем проверку на тег <br></br>,
                // которые не хотим видеть в нашем словаре
                if (word != "br" && !word.empty() && word != "\0"){
                    ind.position = itr;
                    // заносит данные в словарь
                    dictionary[word].push_back(ind);
                    itr++;
                }
            }
            myfile.close();
        } else {
            cout << "Сannot open file" << endl;
        }
    }
    std::cout << "Time of serial: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
}

//MARK: Multi
// Реализация параллельного метода инвертированного индекса

// структура которая передаётся как вводные значения
// для созданых pthread, так как нам нужно передать несколько данных
// для каждого потока
struct INPUT {
    int start;
    int end;
    indexes ms;
    vector<string> paths;
};

// метод, который обрабатывает каждый созданный кодом поток
// принимает указатель на аргументы, определённые в структуре INPUT
void *thread_func(void * arguments){
    
    struct INPUT *in = (struct INPUT *)arguments;
    string line;
    ifstream file;
    
    // цикл ограничен рамками start и end, которые являются
    // порядковыми номерами тех путей, которые мы получаем
    // при определении потоков
    // имеем 500 файлов и 10 потоков - для первой итерации
    // start = 0, end = 50
    for (int i = in->start; i < in->end; i++){
        file.open(in->paths.at(i));
        in->ms.path = in->paths.at(i);
        
        if (file.is_open()) {
            getline(file, line);

            line = delete_punctuation(line);
            int itr = 1;
            string word = "";
            stringstream sstr(line);
            
            // блокировка к памяти мютексом
            pthread_mutex_lock(&m);
            while (sstr >> word) {
                // в данном случае делаем проверку на тег <br></br>,
                // которые не хотим видеть в нашем словаре
                if (word != "br" && !word.empty() && word != "\0"){
                    in->ms.position = itr;
                    // заносит данные в словарь
                    dictionary[word].push_back(in->ms);
                    itr++;
                }
            }
            // разблокировка мютекса
            pthread_mutex_unlock(&m);
            file.close();
        } else {
            cout << "cannot open file" << endl;
        }
    }
    return NULL;
}

// Принимает значения пути к файлу и струкртуры
// которая объявлена в main()
/// В данном случае она нужна для запуска прямого метода
void multi_inverted_index(string path, indexes ind){
    
    pthread_mutex_init(&m, NULL);
    int t, tc;
    
    cout << "Enter number of threads (>1):";
    cin >> t;
    t--;
    
    if(t == 0){
        cout << "Using serial version instead!" << endl;
        serial_inverted_index(path, ind);
        return;
    }
    
    pthread_t threads[t];
    
    vector<string>* Paths = new vector<string>;

    // рекурсивно проходим по даному нам пути к файлам
    for (auto& p : std::__fs::filesystem::recursive_directory_iterator(path)){
        // выбираем только файлы с расширением .txt
        if (p.path().extension() != ".txt")
            continue;
        // данные пути мы заносим в массив, который
        // хранит в себе все значение путей к файлам
        Paths->push_back(p.path().string());
    }
    
    // объявляем массив структуру размерности нашего
    // массива со всеми значениями путей
    struct INPUT in[Paths->size()];
    
    std::clock_t start;
    start = std::clock();

    for(int i = 0; i < t; i++){
        // заносим необходимые данные для каждого элемента
        in[i].paths = *Paths;
        in[i].start = Paths->size() / (float)t * i;
        in[i].end = Paths->size() / (float)t * (i + 1);
        // создаём поток, передавая в него все аргументы
        tc = pthread_create(&threads[i], NULL, &thread_func, (void *)&in[i]);
        if (tc != 0) {
            printf("Thread creation failed");
            exit(-1);
        }
    }
    // ожидаем завершения работы всех потоков
    for (int i = 0; i < t; i++) {
        pthread_join(threads[i], NULL);
    }
    // уничтожаем наш мютекс
    pthread_mutex_destroy(&m);
    
    std::cout << "Time of multi: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
}

// MARK: Lookup query
// Выполняем поиск лексемы в словаре
void lookup_query (string query) {
    if (dictionary.count(query) > 0)
        print_vector(dictionary.find(query)->second);
    else
        cout << "There is no such query" << endl;
}

// MARK: Driver function
int main(int argc, const char * argv[]) {
    
    string path = "/Users/i.kangin/Downloads/course_work/datasets";
    int var = 0;
    bool created = false;
    string query = "";
    indexes index;
    
    while (true) {
        cout << endl << "Choose variant: " <<
                endl << "\t1 - serial" <<
                endl << "\t2 - multi" <<
                endl << "\t3 - lookup query" <<
                endl << "\t4 - erase dictionary" <<
                endl << "\t5 - print dictionary" << endl;
        cin >> var;
        switch (var) {
            case 1:
                serial_inverted_index(path, index);
                created = true;
                break;
            case 2:
                multi_inverted_index (path, index);
                created = true;
                break;
            case 3:
                if (created) {
                    cout << "Enter query: ";
                    cin >> query;
                    lookup_query(query);
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
