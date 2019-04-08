#include "CallPython.h"

PyObject* CallPython::pModule;
PyObject* CallPython::pFunc1;
bool CallPython::flag1 = false;
PyObject* CallPython::pFunc2;
bool CallPython::flag2 = false;

CallPython::CallPython(){};
CallPython::~CallPython(){};

void CallPython::importModule(){
    if(!Py_IsInitialized()){
        std::cout << "initialize failed!" << std::endl;
        exit(1);
    }else{
        PyRun_SimpleString("import sys");
        PyRun_SimpleString("sys.path.append('../src/python')");
        // 导入python模块
        pModule = PyImport_ImportModule("containerInfo");
    }
}


void CallPython::setFunc1(){
    pFunc1 = PyObject_GetAttrString(pModule, "getContainerInfo");
    flag1 = true;
    if(!pFunc1 || !PyCallable_Check(pFunc1)){
        std::cout << "can't find function!" << std::endl;
        exit(1);
    }
}

const char* CallPython::getConInfo(int foundCycle){
    const char *listStr;
    PyObject *pValue, *pArgs;
    if(!flag1){
        setFunc1();
    }
    // 获取GIL
    PyGILState_STATE state = PyGILState_Ensure();
    time_t sinceTime = time(NULL);
    time_t untilTime = sinceTime + foundCycle;
    pArgs = PyTuple_New(3);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("i", sinceTime));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("i", untilTime));
    PyTuple_SetItem(pArgs, 2, Py_BuildValue("i", foundCycle));
    // 调用函数
    pValue = PyEval_CallObject(pFunc1, pArgs);
    Py_DECREF(pArgs);
    PyArg_Parse(pValue, "s", &listStr);
    Py_DECREF(pValue);
    // 判断异常
    if (PyErr_Occurred()) {
        PyErr_Print();
    }
    // 释放GIL
    PyGILState_Release(state);
    return listStr;
}


void CallPython::setFunc2(){
    pFunc2 = PyObject_GetAttrString(pModule, "getContainerPID");
    flag2 = true;
    if(!pFunc2 || !PyCallable_Check(pFunc2)){
        std::cout << "can not find function!" << std::endl;
        exit(1);
    }
}

std::string CallPython::getConPid(std::string ContainerID){
    std::string pid;
    PyObject *pArgs, *pValue;
    if(!flag2){
        setFunc2();
    }
    // 获取GIL
    PyGILState_STATE state = PyGILState_Ensure();
    // 设置参数 
    pArgs = PyTuple_New(1);
    const char *id = ContainerID.c_str();
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", id));
    // 调用函数 
    pValue = PyEval_CallObject(pFunc2, pArgs);
    Py_DECREF(pArgs);
    int temp;
    PyArg_Parse(pValue, "i", &temp);
    Py_DECREF(pValue);
    std::stringstream ss;
    ss << temp;
    pid = ss.str();
    // 释放GIL
    PyGILState_Release(state);
    return pid;
}