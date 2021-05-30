//
//  server.cpp
//  course_work
//
//  Created by Ilya Kangin on 28.05.2021.
//

#include <iostream>
#include <sstream>
#include <filesystem>
#include <fstream>

#include <vector>
#include <map>

#include <string>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define MAX 80
#define PORT 8080
#define SA struct sockaddr

// структура данных для хранения пути к файлу
// и позиции индекса
struct indexes {
    std::string path;
    int position;
};

std::map< std::string,std::vector<indexes> > dictionary;

// метод для удаления знаков препинания в строке
/// сюда же можно добавить и удаление чисел
std::string delete_punctuation(std::string line){
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

void serial_inverted_index(std::string path, indexes ind){
    
    std::string line;
    // начинаем отсчёт времени
    std::clock_t start;
    start = std::clock();
    
    // рекурсивно проходим по даному нам пути к файлам
    for(auto& p: std::__fs::filesystem::recursive_directory_iterator(path)) {
        
        //выбираем только файлы с расширением .txt
        if (p.path().extension() != ".txt")
            continue;
        
        std::ifstream myfile (p.path());
        ind.path = p.path();
        
        if (myfile.is_open()) {
            
            getline(myfile, line);
 
            line = delete_punctuation(line);
            int itr = 1;    // итератор для определения позиции
            std::string word = "";
            std::stringstream sstr(line);
            
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
            std::cout << "Сannot open file" << std::endl;
        }
    }
    std::cout << "Time of serial: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
}

// Функция для общения между сервером и клиентом
void func(int sockfd) {
    char buff[MAX];
    
    while(true) {
        bzero(buff, MAX);
  
        // читает сообщение от клиента и помещает его в буфер
        read(sockfd, buff, sizeof(buff));
        std::cout << "Finding word " << buff;
        
        std::string s;
        std::stringstream ss;
        
        ss << buff;
        ss >> s;
        
        std::vector<indexes> vect = dictionary.find(s)->second;
        std::string str;
        
        // создаём строку ответа
        if (dictionary.count(s) > 0) {
            str = "Word \"" + s + "\" is found in " + std::to_string(vect.size()) + " files\n";
            std::cout << str;
        } else {
            str = "There is no such query\n";
        }
        
        // помещаем строку в буффер char
        strcpy(buff, str.c_str());
        // отправляем обратно на сервер
        write(sockfd, buff, sizeof(buff));
        
    }
}
  
// Driver function
int main(){
    
    std::string path = "/Users/i.kangin/Downloads/course_work/datasets";
    indexes index;
    
    serial_inverted_index(path, index);
    
    
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
  
    // создание и верификация сокета
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    } else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
  
    // присоединение IP/PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
  
    // связывание сокета с данным IP
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    } else
        printf("Socket successfully binded..\n");
  
    // теперь сервер готов к прослушиванию
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else
        printf("Server listening..\n");
    len = sizeof(cli);
  
    // проверка на получение данных с клиента
    connfd = accept(sockfd, (SA*)&cli, (socklen_t*)&len);
    if (connfd < 0) {
        printf("server acccept failed...\n");
        exit(0);
    } else
        printf("server acccept the client...\n");
  
    // функция для общение сервера и клиента
    func(connfd);

    // закрытие сокета
    close(sockfd);
}

