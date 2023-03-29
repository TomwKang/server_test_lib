cmake_minimum_required(VERSION 3.10)

project(tom_server_test)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS thread)
include_directories(${Boost_INCLUDE_DIRS})

set(SOURCES
    src/server_test.cpp
)
add_executable(${PROJECT_NAME}_client src/net_test_client.cpp)
add_executable(${PROJECT_NAME}_server src/net_test_server.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
set(PROJECT_LIST
    ${PROJECT_NAME}_client
    ${PROJECT_NAME}_server
    ${PROJECT_NAME}
)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(${PROJECT_NAME}_server PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(${PROJECT_NAME}_client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_NAME} 
    ${Boost_LIBRARIES} 
)
target_link_libraries(${PROJECT_NAME}_client 
${Boost_LIBRARIES} 
)
target_link_libraries(${PROJECT_NAME}_server
${Boost_LIBRARIES} 
)
