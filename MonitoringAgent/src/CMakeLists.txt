SET(SRC_LIST ContainerSelection.cpp ContainerDataCollection.cpp ContainerInfoCollection.cpp main.cpp ClientTransmission.cpp)

INCLUDE_DIRECTORIES(../include /usr/lib/python3.6)
LINK_DIRECTORIES(/usr/lib/python3.6 ../lib)

ADD_EXECUTABLE(monitoringAgent ${SRC_LIST})

TARGET_LINK_LIBRARIES(monitoringAgent python3.6m libjsoncpp.a boost_system pthread)
