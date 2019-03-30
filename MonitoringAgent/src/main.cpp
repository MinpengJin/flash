#include "ContainerInfoCollection.h"
#include "ClientTransmission.h"


int main(){
    // 初始化传输模块，并与监控服务器建立连接
    std::unique_ptr<ClientTransmission> temp;
    c_transmission.reset(temp.release());
    c_transmission->initTransmission();
    std::cout << "> please input the uri of the server you want to connect:";
    std::string uri;
    std::cin >> uri;
    while(c_transmission->connect(uri)){
        std::cout << "> please input the uri again:";
        std::cin >> uri;
    }
    // 运行容器信息收集模块
    ContainerInfoCollection infoCollectionItem;
    infoCollectionItem.runContainerInfoCollection();
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
    std::string input;
    bool done = false;
    while(!done){
        std::cout << "Enter Command: ";
        std::getline(std::cin, input);

        if(input == "quit"){
            done = true;
        }else if(input == "help"){
            std::cout 
                << "\nCommand List:\n"
                << "connect <ws uri>\n"
                << "send <connection id> <message>\n"
                << "close <connection id> [<close code:default=1000>] [<close reason>]\n"
                << "show <connection id>\n"
                << "help: Display this help text\n"
                << "quit: Exit the program\n"
                << std::endl;
        }
    }
    return 0;
}