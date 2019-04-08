#include <Python.h>
#include <string>
#include <iostream>
#include <sstream>

class CallPython
{
private:
    static PyObject *pModule;
    static PyObject *pFunc1;
    static bool flag1;
    static PyObject *pFunc2;
    static bool flag2;
public:
    CallPython();
    ~CallPython();

    void importModule();
    const char* getConInfo(int foundCycle);
    std::string getConPid(std::string ContainerID);

    void setFunc1();
    void setFunc2();
};

