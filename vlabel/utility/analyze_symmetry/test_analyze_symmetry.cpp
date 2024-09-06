
#include <iostream>

#include "graph.h"
#include "analyze_symmetry.h"
#include "matching/matchingcommand.h"




int main(int argc, char ** argv) {
    Graph* graph = new Graph(true);
    MatchingCommand command(argc, argv);
    std::string input_data_graph_file = command.getDataGraphFilePath();

    graph->loadGraphFromFile(input_data_graph_file);

    std::vector<std::set<std::pair<VertexID, VertexID>>> permutations;
    std::unordered_map<VertexID, std::set<VertexID>> cosets = ANALYZE_SYMMETRY::analyze_symmetry(graph, permutations);

    
    std::cout << "Permutations: " << std::endl;
    for (auto permutation : permutations) {
        for (auto pair : permutation) {
            std::cout << pair.first << " " << pair.second << " ";
        }
        std::cout << std::endl;
    }

    
    std::cout << "Cosets: " << std::endl;
    for (auto coset : cosets) {
        std::cout << coset.first << " : ";
        for (auto vertex : coset.second) {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
    }

    std::vector<std::pair<VertexID, VertexID>> constraints;
    ANALYZE_SYMMETRY::make_constraints(cosets, constraints);

    
    std::cout << "Constraints: " << std::endl;
    for (auto constraint : constraints) {
        std::cout << constraint.first << " " << constraint.second << std::endl;
    }
    return 0;
}