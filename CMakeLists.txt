set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_FLAGS "-DDEBUG ${CMAKE_CXX_FLAGS}")

add_executable(dijkstra dijkstra.cpp)
add_executable(bidirectional_dijkstra bidirectional_dijkstra.cpp)
add_executable(arc_flags arc_flags.cpp)
add_executable(contraction_hierarchy contraction_hierarchy.cpp)