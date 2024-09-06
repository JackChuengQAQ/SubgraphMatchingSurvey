set(ET_SRC
        execution_tree_node.h
        execution_tree_node.cpp
        execution_tree.h
        execution_tree.cpp
        execution_tree_generator.h
        execution_tree_generator.cpp)

add_library(et SHARED ${ET_SRC})