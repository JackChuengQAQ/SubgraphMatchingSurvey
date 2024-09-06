#ifndef ANALYZE_SYMMETRY_H
#define ANALYZE_SYMMETRY_H

#include "graph/graph.h"
#include "../../configuration/types.h"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>




class ANALYZE_SYMMETRY {

public:
    static std::unordered_map<VertexID, std::set<VertexID>>
                                  analyze_symmetry(Graph* graph,
                                  std::vector<std::set<std::pair<VertexID, VertexID>>>& permutations);
    
    static void make_constraints(std::unordered_map<VertexID, std::set<VertexID>>& cosets, 
                                 std::vector<std::pair<VertexID, VertexID>> & constraints);
    
    static void make_full_constraints(std::vector<std::pair<VertexID, VertexID>>& constraints,
                                      std::unordered_map<VertexID, std::pair<std::set<VertexID>, std::set<VertexID>>>& full_constraints);
    
    static void make_ordered_constraints(ui * order,  ui graph_vertices_num,
                                         std::unordered_map<VertexID, std::pair<std::set<VertexID>, std::set<VertexID>>>& full_constraints,
                                         std::unordered_map<VertexID, std::pair<std::set<VertexID>, std::set<VertexID>>>& ordered_constraints);

    
    static void print_permutations(std::vector<std::set<std::pair<VertexID, VertexID>>>& permutations);

    static void print_cosets(std::unordered_map<VertexID, std::set<VertexID>>& cosets);

    static void print_constraints(std::vector<std::pair<VertexID, VertexID>> & constraints);

    static void print_constraints(std::unordered_map<VertexID, std::pair<std::set<VertexID>, std::set<VertexID>>>& constraints);

    static void print_node_partition(std::vector<std::set<VertexID>>& node_partition);



private:
    
    static void init_node_partition(Graph* graph, std::vector<std::set<VertexID>>& node_partition);

    
    static void refine_node_partitions(Graph* graph, std::vector<std::set<VertexID>>& node_partition);

    
    static std::unordered_map<VertexID, std::set<VertexID>>     
                process_node_partitions(Graph* graph,
                                        std::vector<std::set<VertexID>>& top_partition,
                                        std::vector<std::set<VertexID>>& bottom_partition,
                                        std::vector<std::set<VertexID>>& orbits,
                                        std::unordered_map<VertexID, std::set<VertexID>> coset,
                                        std::vector<std::set<std::pair<VertexID, VertexID>>>& permutations);

    static void partition_to_color(std::vector<std::set<VertexID>>& node_partition,
                                   std::vector<int>& node_colors);
    
    static void update_node_edge_color(Graph* graph,
                                  std::vector<int>& node_colors,
                                  std::unordered_map<VertexID, std::unordered_map<int, int>>& node_edge_colors);
    
    static bool is_all_equal(std::vector<std::set<VertexID>>& node_partition,
                             std::vector<int>& node_colors,
                             std::unordered_map<VertexID, std::unordered_map<int, int>>& node_edge_colors);
    
    static bool is_partition_equal(int part_id,
                                   std::vector<std::set<VertexID>>& node_partition,
                                   std::vector<int>& node_colors,
                                   std::unordered_map<VertexID, std::unordered_map<int, int>>& node_edge_colors);
    
    static void make_partitions(std::set<VertexID> & part,
                                std::vector<std::set<VertexID>>& new_partitions,
                                int node_partition_size,
                                std::vector<int>& node_colors,
                                std::unordered_map<VertexID, std::unordered_map<int, int>>& node_edge_colors);
    
    static bool equal_length(std::vector<std::set<VertexID>>& top_partition,
                             std::vector<std::set<VertexID>>& bottom_partition);

    static bool all_single_length(std::vector<std::set<VertexID>>& top_partition);

    static std::set<std::pair<VertexID, VertexID>> 
    find_permutation(std::vector<std::set<VertexID>>& top_partition,
                     std::vector<std::set<VertexID>>& bottom_partition);
    
    static void update_orbits(std::vector<std::set<VertexID>>& orbits,
                              std::set<std::pair<VertexID, VertexID>>& permutation);

    static bool is_in_orbits(std::vector<std::set<VertexID>>& orbits,
                          VertexID v1, VertexID v2);
    
    static std::pair<std::vector<std::set<VertexID>>, std::vector<std::set<VertexID>>> 
    couple_nodes(Graph* graph,
                 std::vector<std::set<VertexID>>& top_partitions,
                 std::vector<std::set<VertexID>>& bottom_partitions,
                 int pair_idx,
                 VertexID t_node,
                 VertexID b_node);

};

#endif 