set(UTILITY_SRC
        graphoperations.cpp
        commandparser.cpp
        commandparser.h
        computesetintersection.h
        computesetintersection.cpp
        bitsetoperation.cpp
        bitsetoperation.h
        QFilter.h
        han/intersection_algos.cpp han/utils/util.cpp han/intersection_algos.hpp
        hashjoin/hash_helpers.h hashjoin/nop_join.h relation/flat_relation.h
        hashjoin/nop_join.cpp leapfrogtriejoin/leapfrogtriejoin.cpp leapfrogtriejoin/leapfrogtriejoin.h
        relation/trie_relation.h primitive/search.cpp primitive/search.h relation/encoded_trie_relation.h
        relation/hash_relation.h relation/catalog.h relation/catalog.cpp relation/edge_relation.h
        primitive/scan.cpp primitive/scan.h primitive/semi_join.cpp primitive/semi_join.h
        primitive/projection.cpp primitive/projection.h primitive/nlf_filter.cpp primitive/nlf_filter.h
        bi_matching/bigraphMatching.h bi_matching/bigraphMatching.cpp
        analyze_symmetry/analyze_symmetry.cpp analyze_symmetry/analyze_symmetry.h)

add_library(utility SHARED
        ${UTILITY_SRC})

add_subdirectory(nucleus_decomposition)
add_subdirectory(execution_tree)
