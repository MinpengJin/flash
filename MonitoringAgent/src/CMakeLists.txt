SET(SRC_LIST ContainerSelection.cpp DataCollection.cpp jsoncpp.cpp main.cpp)

INCLUDE_DIRECTORIES(../include /usr/lib/python3.6)
LINK_DIRECTORIES(/usr/lib/python3.6)

ADD_EXECUTABLE(runFlash ${SRC_LIST})

TARGET_LINK_LIBRARIES(funFlash python3.6m)
