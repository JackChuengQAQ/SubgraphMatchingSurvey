
#include "analyze_symmetry.h"

std::unordered_map<VertexID, std::set<VertexID>>
ANALYZE_SYMMETRY::analyze_symmetry(Graph* graph,
                                    std::vector<std::set<std::pair<VertexID, VertexID>>>& permutations) {
    
    std::vector<std::set<VertexID>> node_partition;
    init_node_partition(graph, node_partition);
    
    refine_node_partitions(graph, node_partition);
    
    std::vector<std::set<VertexID>> orbits;
    std::unordered_map<VertexID, std::set<VertexID>> coset;
    auto ret = std::move(process_node_partitions(graph, node_partition, node_partition, orbits, coset, permutations));

    return ret;
}


void
ANALYZE_SYMMETRY::make_constraints(std::unordered_map<VertexID, std::set<VertexID>>& cosets, 
                                   std::vector<std::pair<VertexID, VertexID>> & constraints) {
    for (auto& coset: cosets) {
        VertexID u = coset.first;
        for (auto& v: coset.second) {
            if (u != v) {
                
                constraints.push_back(std::make_pair(u, v));
            }
        }
    }
}

void
ANALYZE_SYMMETRY::make_full_constraints(std::vector<std::pair<VertexID, VertexID>>& constraints,
                                        std::unordered_map<VertexID, std::pair<std::set<VertexID>, std::set<VertexID>>>& full_constraints) {
    
    for (auto& constraint: constraints) {
        VertexID u = constraint.first;
        VertexID v = constraint.second;
        if (full_constraints.find(u) == full_constraints.end()) {
            full_constraints[u] = std::make_pair(std::set<VertexID>(), std::set<VertexID>());
        }
        if (full_constraints.find(v) == full_constraints.end()) {
            full_constraints[v] = std::make_pair(std::set<VertexID>(), std::set<VertexID>());
        }
        full_constraints[u].second.insert(v);
        full_constraints[v].first.insert(u);
    }
}

void
ANALYZE_SYMMETRY::make_ordered_constraints(ui* order, ui graph_vertices_num,
                                           std::unordered_map<VertexID, std::pair<std::set<VertexID>, std::set<VertexID>>>& full_constraints,
                                           std::unordered_map<VertexID, std::pair<std::set<VertexID>, std::set<VertexID>>>& ordered_constraints) {
    bool* visited_u = new bool[graph_vertices_num];
    std::fill(visited_u, visited_u + graph_vertices_num, false);

    for (size_t i = 0; i < graph_vertices_num; i++) {
        VertexID u = order[i];

        visited_u[u] = true;

        if (full_constraints.find(u) == full_constraints.end()) {
            continue;
        }

        ordered_constraints[u] = std::make_pair(std::set<VertexID>(), std::set<VertexID>());

        auto& u_cons_prior = full_constraints[u].first;
        auto& u_cons_inferior = full_constraints[u].second;

        for (auto& v: u_cons_prior) {
            if (visited_u[v]) {
                ordered_constraints[u].first.insert(v);
            }
        }

        for (auto& v: u_cons_inferior) {
            if (visited_u[v]) {
                ordered_constraints[u].second.insert(v);
            }
        }
    }
}



void
ANALYZE_SYMMETRY::init_node_partition(Graph* graph, std::vector<std::set<VertexID>>& node_partition) {
    
    std::unordered_map<VertexID, int> label2index;
    
    std::cout << graph->getVerticesCount() << std::endl;
    for (VertexID v_i = 0; v_i < graph->getVerticesCount(); ++v_i) {
        LabelID l_i = graph->getVertexLabel(v_i);
        if (label2index.find(l_i) == label2index.end()) {
            label2index[l_i] = node_partition.size();
            node_partition.push_back(std::set<VertexID>());
        }
        node_partition[label2index[l_i]].insert(v_i);
    }

    print_node_partition(node_partition);
}


void
ANALYZE_SYMMETRY::refine_node_partitions(Graph* graph, std::vector<std::set<VertexID>>& node_partition) {
    std::vector<int> colors;
    colors.resize(graph->getVerticesCount());

    std::unordered_map<VertexID, std::unordered_map<int, int>> node_edge_colors;

    std::vector<std::set<VertexID>> temp_node_partition;
    std::vector<std::set<VertexID>> temp_new_partition;

    while (true) {
        partition_to_color(node_partition, colors);
        update_node_edge_color(graph, colors, node_edge_colors);

        bool is_all_equal_flag = true;
        for (int i = 0; i < node_partition.size(); ++i) {
            if (node_partition[i].size() > 1 && !is_partition_equal(i, node_partition, colors, node_edge_colors)) {
                is_all_equal_flag = false;
                
                make_partitions(node_partition[i], temp_new_partition, node_partition.size(), colors, node_edge_colors);
                for (auto& new_part: temp_new_partition) {
                    temp_node_partition.push_back(new_part);
                }
            } else {
                temp_node_partition.push_back(node_partition[i]);
            }
        }

        std::swap(node_partition, temp_node_partition);
        temp_node_partition.clear();

        if (is_all_equal_flag) {
            break;
        }
    }
    std::sort(node_partition.begin(), node_partition.end(), [](const std::set<VertexID>& a, const std::set<VertexID>& b) {
            return a.size() < b.size();
        });
}


std::unordered_map<VertexID, std::set<VertexID>>
ANALYZE_SYMMETRY::process_node_partitions(Graph* graph,
                                           std::vector<std::set<VertexID>>& top_partition,
                                           std::vector<std::set<VertexID>>& bottom_partition,
                                           std::vector<std::set<VertexID>>& orbits,
                                           std::unordered_map<VertexID, std::set<VertexID>> coset,
                                           std::vector<std::set<std::pair<VertexID, VertexID>>>& permutations) {
    
    if (orbits.empty()) {
        orbits.resize(graph->getVerticesCount());
        for (VertexID v = 0; v < graph->getVerticesCount(); ++v) {
            orbits[v].insert(v);
        }
    }

    
    
    
    

    assert(equal_length(top_partition, bottom_partition));

    
    if (all_single_length(top_partition)) {
        std::set<std::pair<VertexID, VertexID>> permutation = std::move(find_permutation(top_partition, bottom_partition));
        update_orbits(orbits, permutation);

        if (!permutation.empty()) {
            permutations.push_back(permutation);
            
            
            
            
            
        }
        
        
        return coset;
    }

    
    
    VertexID min_unmapped_node = -1;
    int min_idx = -1;
    for (int i = 0; i < top_partition.size(); ++i) {
        if (top_partition[i].size() > 1) {
            for (auto& u: top_partition[i]) {
                if (min_unmapped_node == -1 || u < min_unmapped_node) {
                    min_unmapped_node = u;
                    min_idx = i;
                }
            }
        }
    }
    auto bottom_part = bottom_partition[min_idx];

    
    
    

    VertexID u = min_unmapped_node; 

    assert(bottom_part.size() > 1);

    for (auto& v: bottom_part) {
        if (u != v && is_in_orbits(orbits, u, v)) {
            continue;
        }

        
        

        
        auto partitions = std::move(couple_nodes(graph, top_partition, bottom_partition, min_idx, u, v));
        auto & new_top_partition = partitions.first;
        auto & new_bottom_partition = partitions.second;

        auto new_coset = std::move(process_node_partitions(graph, new_top_partition, new_bottom_partition, orbits, coset, permutations));

        
        for (const auto& p: new_coset) {
            coset[p.first] = std::move(p.second);
        }
    }

    
    int node_cnt = 0;
    for (int i = 0; i < top_partition.size(); ++i) {
        if (top_partition[i].size() == 1 && bottom_partition[i].size() == 1 && *top_partition[i].begin() == *bottom_partition[i].begin()) {
            VertexID k = *top_partition[i].begin();
            if (k < u) {
                node_cnt += 1;
            }
        }
    }
    
    if (node_cnt == u && coset.find(u) == coset.end()) {
        for (auto& orbit: orbits) {
            if (orbit.find(u) != orbit.end()) {
                coset[u] = orbit;
                break;
            }
        }
    }

    return coset;
}


void 
ANALYZE_SYMMETRY::partition_to_color(std::vector<std::set<VertexID>>& node_partition,
                                     std::vector<int>& colors) {
    for (int i = 0; i < node_partition.size(); ++i) {
        for (auto v: node_partition[i]) {
            colors[v] = i;
        }
    }
}


void
ANALYZE_SYMMETRY::update_node_edge_color(Graph* graph,
                                         std::vector<int>& node_colors,
                                         std::unordered_map<VertexID, std::unordered_map<int, int>>& node_edge_colors) {
    node_edge_colors.clear();
    for (VertexID v = 0; v < graph->getVerticesCount(); ++v) {
        ui v_nbrs_cnt;
        const ui * v_nbs = graph->getVertexNeighbors(v, v_nbrs_cnt);
        for (ui i = 0; i < v_nbrs_cnt; ++i) {
            VertexID u = v_nbs[i];
            if (node_edge_colors.find(v) == node_edge_colors.end()) {
                node_edge_colors[v] = std::unordered_map<int, int>();
            }
            if (node_edge_colors[v].find(node_colors[u]) == node_edge_colors[v].end()) {
                node_edge_colors[v][node_colors[u]] = 0;
            }
            node_edge_colors[v][node_colors[u]] += 1;
        }
    }
}


bool 
ANALYZE_SYMMETRY::is_all_equal(std::vector<std::set<VertexID>>& node_partition,
             std::vector<int>& node_colors,
             std::unordered_map<VertexID, std::unordered_map<int, int>>& node_edge_colors) {
    for (auto & part: node_partition) { 
        for (auto i = part.begin(); i != part.end(); ++i) {
            for (auto j = std::next(i); j != part.end(); ++j) {
                if (node_colors[*i] != node_colors[*j]) {
                    return false;
                }
                for (int p = 0; p < node_partition.size(); ++p) {
                    if (node_edge_colors[*i][p] != node_edge_colors[*j][p]) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}


bool 
ANALYZE_SYMMETRY::is_partition_equal(int part_id,
                  std::vector<std::set<VertexID>>& node_partition,
                  std::vector<int>& node_colors,
                  std::unordered_map<VertexID, std::unordered_map<int, int>>& node_edge_colors) {
    std::set<VertexID> & part = node_partition[part_id];
    for (auto i = part.begin(); i != part.end(); ++i) {
        for (auto j = std::next(i); j != part.end(); ++j) {
            if (node_colors[*i] != node_colors[*j]) {
                return false;
            }
            for (int p = 0; p < node_partition.size(); ++p) {
                if (node_edge_colors[*i][p] != node_edge_colors[*j][p]) {
                    return false;
                }
            }
        }
    }
    return true;
}


void
ANALYZE_SYMMETRY::make_partitions(std::set<VertexID> & part,
                                  std::vector<std::set<VertexID>>& new_partitions,
                                  int node_partition_size,
                                  std::vector<int>& node_colors,
                                  std::unordered_map<VertexID, std::unordered_map<int, int>>& node_edge_colors) {
    new_partitions.clear();
    for (auto& u: part) {
        bool new_item_flag = true;
        for (auto& new_part: new_partitions) {
            bool is_equal_flag = true;
            if (node_colors[u] == node_colors[*new_part.begin()]) {   
                for (int p = 0; p < node_partition_size; ++p) {
                    if (node_edge_colors[u][p] != node_edge_colors[*new_part.begin()][p]) {
                        is_equal_flag = false;
                        break;
                    }
                }
            } else {
                is_equal_flag = false;
            }
            if (is_equal_flag) {
                new_part.insert(u);
                new_item_flag = false;
                break;
            }
        }
        if (new_item_flag) {
            std::set<VertexID> new_part;
            new_part.insert(u);
            new_partitions.push_back(new_part);
        }
    }
}

bool
ANALYZE_SYMMETRY::equal_length(std::vector<std::set<VertexID>>& top_partition,
                               std::vector<std::set<VertexID>>& bottom_partition) {
    if (top_partition.size() != bottom_partition.size()) {
        return false;
    }
    for (int i = 0; i < top_partition.size(); ++i) {
        if (top_partition[i].size() != bottom_partition[i].size()) {
            return false;
        }
    }
    return true;
}

bool
ANALYZE_SYMMETRY::all_single_length(std::vector<std::set<VertexID>>& top_partition) {
    for (auto& part: top_partition) {
        if (part.size() != 1) {
            return false;
        }
    }
    return true;
}


std::set<std::pair<VertexID, VertexID>>
ANALYZE_SYMMETRY::find_permutation(std::vector<std::set<VertexID>>& top_partition,
                                    std::vector<std::set<VertexID>>& bottom_partition) {
    std::set<std::pair<VertexID, VertexID>> permutation; 
    for (int i = 0; i < top_partition.size(); ++i) {
        assert(top_partition[i].size() == 1 && bottom_partition[i].size() == 1);

        VertexID u = *top_partition[i].begin();
        VertexID v = *bottom_partition[i].begin();

        if (u != v) {
            
            if (u > v) {
                std::swap(u, v);
            }
            permutation.insert(std::make_pair(u, v));
        }
    }
    return permutation;
}


void
ANALYZE_SYMMETRY::update_orbits(std::vector<std::set<VertexID>>& orbits,
                                std::set<std::pair<VertexID, VertexID>>& permutation) {
    for (auto& p: permutation) {
        VertexID u = p.first;
        VertexID v = p.second;
        int first_idx = -1;
        int second_idx = -1;
        for (int i = 0; i < orbits.size(); ++i) {
            if (first_idx != -1 && second_idx != -1) {
                break;
            }
            if (orbits[i].find(u) != orbits[i].end()) {
                first_idx = i;
            }
            if (orbits[i].find(v) != orbits[i].end()) {
                second_idx = i;
            }
        }
        assert(first_idx != -1 && second_idx != -1);
        if (first_idx != second_idx) {
            orbits[first_idx].insert(orbits[second_idx].begin(), orbits[second_idx].end());
            orbits.erase(orbits.begin() + second_idx);
            
            
            
            
            
            
            
            
            
        }
    }
}


bool
ANALYZE_SYMMETRY::is_in_orbits(std::vector<std::set<VertexID>>& orbits,
                               VertexID u,
                               VertexID v) {
    for (const auto& orbit: orbits) {
        if (orbit.find(u) != orbit.end() && orbit.find(v) != orbit.end()) {
            return true;
        }
    }
    return false;
}

std::pair<std::vector<std::set<VertexID>>, std::vector<std::set<VertexID>>> 
ANALYZE_SYMMETRY::couple_nodes(Graph* graph,
                               std::vector<std::set<VertexID>>& top_partitions,
                               std::vector<std::set<VertexID>>& bottom_partitions,
                               int pair_idx,
                               VertexID t_node,
                               VertexID b_node) {
    
    std::vector<std::set<VertexID>> new_top_partitions = top_partitions;
    std::vector<std::set<VertexID>> new_bottom_partitions = bottom_partitions;

    auto & t_partition = new_top_partitions[pair_idx];
    auto & b_partition = new_bottom_partitions[pair_idx];

    assert(t_partition.find(t_node) != t_partition.end() && b_partition.find(b_node) != b_partition.end());

    t_partition.erase(t_node);
    b_partition.erase(b_node);

    std::set<VertexID> new_t_part = {t_node};
    std::set<VertexID> new_b_part = {b_node};

    new_top_partitions.insert(new_top_partitions.begin() + pair_idx, new_t_part);
    new_bottom_partitions.insert(new_bottom_partitions.begin() + pair_idx, new_b_part);

    
    refine_node_partitions(graph, new_top_partitions);
    refine_node_partitions(graph, new_bottom_partitions);

    return std::make_pair(new_top_partitions, new_bottom_partitions);
}

    
void 
ANALYZE_SYMMETRY::print_permutations(std::vector<std::set<std::pair<VertexID, VertexID>>>& permutations) {
    std::cout << "Permutations: " << std::endl;
    for (auto& permutation : permutations) {
        for (auto& pair : permutation) {
            std::cout << "[" << pair.first << ", " << pair.second << "] ";
        }
        std::cout << std::endl;
    }
}

void 
ANALYZE_SYMMETRY::print_cosets(std::unordered_map<VertexID, std::set<VertexID>>& cosets) {
    std::cout << "Cosets: " << std::endl;
    for (auto& coset : cosets) {
        std::cout << coset.first << ": ";
        for (auto& vertex : coset.second) {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
    }
}

void 
ANALYZE_SYMMETRY::print_constraints(std::vector<std::pair<VertexID, VertexID>> & constraints) {
    std::cout << "Constraints: " << std::endl;
    for (auto& constraint : constraints) {
        std::cout << "(" << constraint.first << " " << constraint.second << ")" << std::endl;
    }
}

void 
ANALYZE_SYMMETRY::print_constraints(std::unordered_map<VertexID, std::pair<std::set<VertexID>, std::set<VertexID>>>& full_constraints) {
    std::cout << "Used Constraints: " << std::endl;
    for (auto& used_constraint : full_constraints) {
        std::cout << used_constraint.first << ": ";
        for (auto& vertex : used_constraint.second.first) {
            std::cout << vertex << " ";
        }
        std::cout << "| ";
        for (auto& vertex : used_constraint.second.second) {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
    }
}

void 
ANALYZE_SYMMETRY::print_node_partition(std::vector<std::set<VertexID>>& node_partition) {
    std::cout << "node partition: \t";
    for (auto& part : node_partition) {
        for (auto& vertex : part) {
            std::cout << vertex << " ";
        }
        std::cout << "|";
    }
    std::cout << "\b" << std::endl;
}