#ifndef CONTAINERINFOCOLLECTION_H
#define CONTAINERINFOCOLLECTION_H

#include <Python.h>
#include <sstream>
#include <memory>
#include <algorithm>
#include "json/json.h"
#include "ContainerSelection.h"

int FOUND_CYCLE = 10;

struct InfoType{
    std::string id, status;
};

class ContainerInfoCollection{
private:
    std::unique_ptr<ContainerSelection> selection_ptr;
    std::unique_ptr<std::thread> runThread;

public:
    ContainerInfoCollection();
    ~ContainerInfoCollection();
    void runContainerInfoCollection();
};

#endif