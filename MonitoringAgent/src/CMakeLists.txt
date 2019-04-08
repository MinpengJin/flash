SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

SET(SRC_LIST ContainerSelection.cpp ContainerDataCollection.cpp ContainerInfoCollection.cpp main.cpp ClientTransmission.cpp CallPython.cpp)

INCLUDE_DIRECTORIES(../include /usr/include/python3.6m)
LINK_DIRECTORIES(/usr/lib/python3.6 ../lib)

ADD_EXECUTABLE(monitoringAgent ${SRC_LIST})

TARGET_LINK_LIBRARIES(monitoringAgent python3.6m libjsoncpp.a boost_system pthread)
