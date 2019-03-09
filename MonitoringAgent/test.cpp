#include<thread>
#include"ContainerSelection.h"

using namespace std;

ContainerSelection temp;

// 随机产生ContainerID
void createContainer(){
    int time = 8;
    while(time > 0){
        string ContainerID = "";
        for(int i = 0; i < 12; i++){
            switch (rand() % 3)
            {
                case 0: ContainerID += 'A' + rand() % 26;break;
                case 1: ContainerID += 'a' + rand() % 26;break;
                case 2: ContainerID += '0' + rand() % 10;break;
            }
        }
        temp.adjustContainerList(ContainerID, 1);
        time--;
        sleep(3);
    }
}

int main(){
    ifstream fs;
    fs.open("text1.txt");
    if(!fs.is_open()){
        cout << "error opening file" << endl;
        exit(1);
    }
    char buffer[256];
    while(!fs.eof()){
        fs.getline(buffer, 100);
        unsigned long long number = atoll(buffer);
        cout << buffer << " " << number << endl;
    }
    fs.close();
    // thread t1(createContainer);
    // t1.detach();
    // thread t2(&ContainerSelection::runContainerSelection, &temp);
    // t2.detach();
    // sleep(20);
    return 0;
}