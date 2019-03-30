SET(SRC_LIST main.cpp ServerTransmission.cpp DataStorage.cpp)

INCLUDE_DIRECTORIES(../include)
LINK_DIRECTORIES(../lib)

ADD_EXECUTABLE(monitoringServer ${SRC_LIST})

TARGET_LINK_LIBRARIES(monitoringServer libjsoncpp.a boost_system pthread)