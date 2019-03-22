#include<Python.h>
#include<sstream>
#include<memory>
#include"json/json.h"
#include"ContainerSelection.h"

int FOUND_CYCLE = 10;

struct InfoType{
    std::string id, status;
};

class ContainerInfoCollection{
private:

public:
    ContainerInfoCollection();
    ~ContainerInfoCollection();
    void runContainerInfoCollection();
}