#include"ContainerInfoCollection.h"

ContainerInfoCollection::ContainerInfoCollection(){}

ContainerInfoCollection::~ContainerInfoCollection(){}

// 在主函数main中单独创建一个线程运行runContainerInfoCollection函数
void ContainerInfoCollection::runContainerInfoCollection(){
    PyObject *pModule, *pFunc;
    PyObject *pValue, *pArgs;

    Py_Initialize();
    if(!Py_IsInitialized()){
        std::cerr << "initialize failed!" << std::endl;
        exit(1);
    }else{
        PyRun_SimpleString("import sys");
        PyRun_SimpleString("sys.path.append('../src/python')");

        pModule = PyImport_ImportModule("containerInfo");
        pFunc = PyObject_GetAttrString(pModule, "getContainerInfo");
        if(!pFunc || !PyCallable_Check(pFunc)){
            std::cerr << "can't find function!" << std::endl;
            exit(1);
        }else{
            while(true){
                time_t sinceTime = time(NULL);
                time_t untilTime = sinceTime + FOUND_CYCLE;
                pArgs = PyTuple_New(3);
                PyTuple_SetItem(pArgs, 0, Py_BuildValue("i", sinceTime));
                PyTuple_SetItem(pArgs, 1, Py_BuildValue("i", untilTime));
                PyTuple_SetItem(pArgs, 2, Py_BuildValue("i", FOUND_CYCLE));
                // 调用函数
                pValue = PyEval_CallObject(pFunc, pArgs);
                const char *listStr;
                PyArg_Parse(pValue, "s", &listStr);
                std::stringstream ss;
                ss.str(listStr);
                std::string item;
                while(getline(ss, item, ';')){
                    std::string containerID, status;
                    Json::Value jsonRoot;
                    Json::CharReaderBuilder readerBuilder;
                    unique_ptr<Json::CharReader> const reader(readerBuilder.newCharReader());
                    JSONCPP_STRING errs;
                    bool res = reader->parse(item.c_str(), item.c_str() + item.length(), &jsonRoot, &errs);
                    if (!res || !errs.empty()) 
                    {
                        std::cerr << "parse Json error! " << errs << std::endl;
                        Exit(-1);
                    }else{
                        containerID = jsonRoot["id"].asString();
                        status = jsonRoot["status"].asString();
                        // cout << "id:" << item["id"].asString() << " status:" << item["status"].asString() << endl;
                    }
                    std::unique_ptr<ContainerSelection> temp(new ContainerSelection());
                    temp->adjustContainerList(containerID, status);
                }
            }
        }
    }
}