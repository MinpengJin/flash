#include"ContainerInfoCollection.h"

ContainerInfoCollection::ContainerInfoCollection(){}
ContainerInfoCollection::~ContainerInfoCollection(){}


void ContainerInfoCollection::collectionContainerInfo(){
    // 初始化python解释器
    Py_Initialize();
    CallPython call;
    call.importModule();
    while(true){
        const char *listStr = call.getConInfo(foundCycle);
        // 解析python返回的数据
        std::stringstream ss;
        ss.str(listStr);
        std::string item;
        while(getline(ss, item, ';')){
            std::string containerID, status;
            Json::Value jsonRoot;
            Json::CharReaderBuilder readerBuilder;
            std::unique_ptr<Json::CharReader> const reader(readerBuilder.newCharReader());
            JSONCPP_STRING errs;
            bool res = reader->parse(item.c_str(), item.c_str() + item.length(), &jsonRoot, &errs);
            if (!res || !errs.empty()) 
            {
                std::cerr << "parse Json error! " << errs << std::endl;
                exit(1);
            }
            containerID = jsonRoot["id"].asString();
            status = jsonRoot["status"].asString();

            if(containerID!="null"){
                // --test--
                std::cout << "[collectionContainerInfo] containerID:" 
                <<  containerID << " status:" << status << std::endl;

                std::unique_ptr<ContainerSelection> selection_ptr(new ContainerSelection());
                selection_ptr->adjustContainerList(containerID, status);
            }
        }
    }
    // 关闭python解释器
    Py_Finalize();
}


void ContainerInfoCollection::setFoundCycle(int cycle){
    foundCycle = cycle;
}


int ContainerInfoCollection::getFoundCycle(){
    return foundCycle;
}


void ContainerInfoCollection::runContainerInfoCollection(){
    std::thread t(&ContainerInfoCollection::collectionContainerInfo, this);
     t.detach();
}