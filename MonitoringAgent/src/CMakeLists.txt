SET(SRC_LIST ContainerSelection.cpp ContainerDataCollection.cpp main.cpp)

INCLUDE_DIRECTORIES(../include /usr/lib/python3.6)
LINK_DIRECTORIES(/usr/lib/python3.6 ../lib)

ADD_EXECUTABLE(runFlash ${SRC_LIST})

TARGET_LINK_LIBRARIES(funFlash python3.6m libjsoncpp.a)
