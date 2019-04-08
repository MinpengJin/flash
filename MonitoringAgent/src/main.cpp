#include "ContainerInfoCollection.h"
#include "ClientTransmission.h"


int main(){
    std::cout << "*********************************************************" << std::endl;
    std::cout << "**************** [flash] monitoringAgent ****************" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    // 初始化传输模块，并与监控服务器建立连接
    c_transmission->initTransmission();
    std::cout << "> please input the uri of the server you want to connect:";
    std::string uri;
    std::cin >> uri;
    while(c_transmission->connect(uri)){
        std::cout << ">[error] please input the uri again:";
        std::cin >> uri;
    }
    // 运行容器信息收集模块
    ContainerInfoCollection infoCollectionItem;
    // std::thread t(&ContainerInfoCollection::collectionContainerInfo, &infoCollectionItem);
    // t.detach();
    infoCollectionItem.runContainerInfoCollection();
    // std::cout << "------------" << std::endl;

    // 运行监控容器选择模块
    ContainerSelection selectionItem;
    selectionItem.runContainerSelection();
    // todo：运行容器日志收集模块
    
    /*
     * todo：
     * 更改监控周期 ContainerInfoCollection setFoundCycle
     * 查看当前设置监控周期 getFoundCycle
     * 修改CPU核数 ContainerDataCollection setCoreNum
     * 断开与监控服务器连接 c_transmission close
     * 连接监控服务器 c_transmission connect
     */

    while(true){};

    /*
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string input;
    bool done = false;
    while(!done){
        std::cout << "> enter command: ";
        std::getline(std::cin, input);

        if(input == "quit"){
            done = true;
        }else if(input == "help"){
            std::cout 
                << "\nCommand List:\n"
                << "connect <ws uri>\n"
                << "close: Close the current connection to the server\n"
                << "show: Show current ContainerList\n"
                << "help: Display this help text\n"
                << "quit: Exit the program\n"
                << std::endl;
        }else if(input == "show"){
            std::cout << "ContainerList: ";
            selectionItem.showList();
            std::cout << std::endl;
        }else if(input == "close"){
            c_transmission->close();
        }else if(input.substr(0,7) == "connect"){
            c_transmission->close();
            while(c_transmission->connect(input.substr(8))){
                std::cout << "> [error] please input the uri again:";
                std::cin >> uri;
            }
        }else{
            std::cout << "> Unrecognized Command" << std::endl;
        }
    }
    */
    return 0;
}