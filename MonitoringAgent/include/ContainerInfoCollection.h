#include"ContainerSelection.h"

int FOUND_CYCLE = 10;

struct InfoType{
    string id, status;
};

class ContainerInfoCollection{
private:

public:
    ContainerInfoCollection();
    ~ContainerInfoCollection();
    void runContainerInfoCollection();
}