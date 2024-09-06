
#include "FilterVertices.h"
#include "GenerateFilteringPlan.h"
#include <memory.h>
#include <utility/graphoperations.h>
#include "primitive/scan.h"
#include "primitive/nlf_filter.h"
#include "primitive/semi_join.h"
#include "primitive/projection.h"
#include "bi_matching/bigraphMatching.h"
#include <vector>
#include <algorithm>
#define INVALID_VERTEX_ID 100000000


bool
FilterVertices::LDFFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count) {
    allocateBuffer(data_graph, query_graph, candidates, candidates_count);

    for (ui i = 0; i < query_graph->getVerticesCount(); ++i) {
        LabelID label = query_graph->getVertexLabel(i);
        ui degree = query_graph->getVertexDegree(i);

        ui data_vertex_num;
        const ui* data_vertices = data_graph->getVerticesByLabel(label, data_vertex_num);

        for (ui j = 0; j < data_vertex_num; ++j) {
            ui data_vertex = data_vertices[j];
            if (data_graph->getVertexDegree(data_vertex) >= degree) {
                candidates[i][candidates_count[i]++] = data_vertex;
            }
        }

        if (candidates_count[i] == 0) {
            return false;
        }
    }

    return true;
}


bool
FilterVertices::NLFFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count) {
    allocateBuffer(data_graph, query_graph, candidates, candidates_count);

    for (ui i = 0; i < query_graph->getVerticesCount(); ++i) {
        VertexID query_vertex = i;
        computeCandidateWithNLF(data_graph, query_graph, query_vertex, candidates_count[query_vertex], candidates[query_vertex]);

        if (candidates_count[query_vertex] == 0) {
            return false;
        }
    }

    return true;
}









bool
FilterVertices::GQLFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count) {
    
    if (!NLFFilter(data_graph, query_graph, candidates, candidates_count))
        return false;

    
    ui query_vertices_num = query_graph->getVerticesCount();
    ui data_vertex_num = data_graph->getVerticesCount();

    bool** valid_candidates = new bool*[query_vertices_num];
    for (ui i = 0; i < query_vertices_num; ++i) {
        valid_candidates[i] = new bool[data_vertex_num];
        memset(valid_candidates[i], 0, sizeof(bool) * data_vertex_num);
    }

    ui query_graph_max_degree = query_graph->getGraphMaxDegree();
    ui data_graph_max_degree = data_graph->getGraphMaxDegree();

    int* left_to_right_offset = new int[query_graph_max_degree + 1];
    int* left_to_right_edges = new int[query_graph_max_degree * data_graph_max_degree];
    int* left_to_right_match = new int[query_graph_max_degree];
    int* right_to_left_match = new int[data_graph_max_degree];
    int* match_visited = new int[data_graph_max_degree + 1];
    int* match_queue = new int[query_vertices_num];
    int* match_previous = new int[data_graph_max_degree + 1];

    
    for (ui i = 0; i < query_vertices_num; ++i) {
        VertexID query_vertex = i;
        for (ui j = 0; j < candidates_count[query_vertex]; ++j) {
            VertexID data_vertex = candidates[query_vertex][j];
            valid_candidates[query_vertex][data_vertex] = true;
        }
    }

    
    for (ui l = 0; l < 2; ++l) {
        for (ui i = 0; i < query_vertices_num; ++i) {
            VertexID query_vertex = i;
            for (ui j = 0; j < candidates_count[query_vertex]; ++j) {
                VertexID data_vertex = candidates[query_vertex][j];

                if (data_vertex == INVALID_VERTEX_ID)
                    continue;

                if (!verifyExactTwigIso(data_graph, query_graph, data_vertex, query_vertex, valid_candidates,
                                        left_to_right_offset, left_to_right_edges, left_to_right_match,
                                        right_to_left_match, match_visited, match_queue, match_previous)) {
                    candidates[query_vertex][j] = INVALID_VERTEX_ID;
                    valid_candidates[query_vertex][data_vertex] = false;
                }
            }
        }
    }

    
    compactCandidates(candidates, candidates_count, query_vertices_num);

    
    for (ui i = 0; i < query_vertices_num; ++i) {
        delete[] valid_candidates[i];
    }
    delete[] valid_candidates;
    delete[] left_to_right_offset;
    delete[] left_to_right_edges;
    delete[] left_to_right_match;
    delete[] right_to_left_match;
    delete[] match_visited;
    delete[] match_queue;
    delete[] match_previous;

    return isCandidateSetValid(candidates, candidates_count, query_vertices_num);
}


bool
FilterVertices::TSOFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count,
                          ui *&order, TreeNode *&tree) {
    allocateBuffer(data_graph, query_graph, candidates, candidates_count);
    GenerateFilteringPlan::generateTSOFilterPlan(data_graph, query_graph, tree, order);

    ui query_vertices_num = query_graph->getVerticesCount();

    
    VertexID start_vertex = order[0];
    computeCandidateWithNLF(data_graph, query_graph, start_vertex, candidates_count[start_vertex], candidates[start_vertex]);

    ui* updated_flag = new ui[data_graph->getVerticesCount()];
    ui* flag = new ui[data_graph->getVerticesCount()];
    std::fill(flag, flag + data_graph->getVerticesCount(), 0);

    for (ui i = 1; i < query_vertices_num; ++i) {
        VertexID query_vertex = order[i];
        TreeNode& node = tree[query_vertex];
        generateCandidates(data_graph, query_graph, query_vertex, &node.parent_, 1, candidates, candidates_count, flag, updated_flag);
    }

    for (int i = query_vertices_num - 1; i >= 0; --i) {
        VertexID query_vertex = order[i];
        TreeNode& node = tree[query_vertex];
        if (node.children_count_ > 0) {
            pruneCandidates(data_graph, query_graph, query_vertex, node.children_, node.children_count_, candidates, candidates_count, flag, updated_flag);
        }
    }

    compactCandidates(candidates, candidates_count, query_vertices_num);

    delete[] updated_flag;
    delete[] flag;
    return isCandidateSetValid(candidates, candidates_count, query_vertices_num);
}


bool
FilterVertices::CFLFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count,
                          ui *&order, TreeNode *&tree) {
    allocateBuffer(data_graph, query_graph, candidates, candidates_count);
    int level_count;
    ui* level_offset;
    GenerateFilteringPlan::generateCFLFilterPlan(data_graph, query_graph, tree, order, level_count, level_offset);

    VertexID start_vertex = order[0];
    computeCandidateWithNLF(data_graph, query_graph, start_vertex, candidates_count[start_vertex], candidates[start_vertex]);

    ui* updated_flag = new ui[data_graph->getVerticesCount()];
    ui* flag = new ui[data_graph->getVerticesCount()];
    std::fill(flag, flag + data_graph->getVerticesCount(), 0);

    
    for (int i = 1; i < level_count; ++i) {
        
        for (int j = level_offset[i]; j < level_offset[i + 1]; ++j) {
            VertexID query_vertex = order[j];
            TreeNode& node = tree[query_vertex];
            
            
            generateCandidates(data_graph, query_graph, query_vertex, node.bn_, node.bn_count_, candidates, candidates_count, flag, updated_flag);
        }

        
        for (int j = level_offset[i + 1] - 1; j >= level_offset[i]; --j) {
            VertexID query_vertex = order[j];
            TreeNode& node = tree[query_vertex];

            if (node.fn_count_ > 0) {
                pruneCandidates(data_graph, query_graph, query_vertex, node.fn_, node.fn_count_, candidates, candidates_count, flag, updated_flag);
            }
        }
    }

    
    for (int i = level_count - 2; i >= 0; --i) {
        for (int j = level_offset[i]; j < level_offset[i + 1]; ++j) {
            VertexID query_vertex = order[j];
            TreeNode& node = tree[query_vertex];

            if (node.under_level_count_ > 0) {
                pruneCandidates(data_graph, query_graph, query_vertex, node.under_level_, node.under_level_count_, candidates, candidates_count, flag, updated_flag);
            }
        }
    }


    compactCandidates(candidates, candidates_count, query_graph->getVerticesCount());

    delete[] updated_flag;
    delete[] flag;
    return isCandidateSetValid(candidates, candidates_count, query_graph->getVerticesCount());
}

bool
FilterVertices::DPisoFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count,
                            ui *&order, TreeNode *&tree) {
    if (!LDFFilter(data_graph, query_graph, candidates, candidates_count))
        return false;

    GenerateFilteringPlan::generateDPisoFilterPlan(data_graph, query_graph, tree, order);

    ui query_vertices_num = query_graph->getVerticesCount();
    ui* updated_flag = new ui[data_graph->getVerticesCount()];
    ui* flag = new ui[data_graph->getVerticesCount()];
    std::fill(flag, flag + data_graph->getVerticesCount(), 0);

    
    for (ui k = 0; k < 3; ++k) {
        if (k % 2 == 0) {
            for (int i = 1; i < query_vertices_num; ++i) {
                VertexID query_vertex = order[i];
                TreeNode& node = tree[query_vertex];
                pruneCandidates(data_graph, query_graph, query_vertex, node.bn_, node.bn_count_, candidates, candidates_count, flag, updated_flag);
            }
        }
        else {
            for (int i = query_vertices_num - 2; i >= 0; --i) {
                VertexID query_vertex = order[i];
                TreeNode& node = tree[query_vertex];
                pruneCandidates(data_graph, query_graph, query_vertex, node.fn_, node.fn_count_, candidates, candidates_count, flag, updated_flag);
            }
        }
    }

    compactCandidates(candidates, candidates_count, query_graph->getVerticesCount());

    delete[] updated_flag;
    delete[] flag;
    return isCandidateSetValid(candidates, candidates_count, query_graph->getVerticesCount());
}

bool
FilterVertices::CECIFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count,
                           ui *&order, TreeNode *&tree,  std::vector<std::unordered_map<VertexID, std::vector<VertexID >>> &TE_Candidates,
                           std::vector<std::vector<std::unordered_map<VertexID, std::vector<VertexID>>>> &NTE_Candidates) {
    GenerateFilteringPlan::generateCECIFilterPlan(data_graph, query_graph, tree, order);

    allocateBuffer(data_graph, query_graph, candidates, candidates_count);

    ui query_vertices_count = query_graph->getVerticesCount();
    ui data_vertices_count = data_graph->getVerticesCount();
    
    VertexID root = order[0];
    computeCandidateWithNLF(data_graph, query_graph, root, candidates_count[root], candidates[root]);

    if (candidates_count[root] == 0)
        return false;

    
    std::vector<ui> updated_flag(data_vertices_count);
    std::vector<ui> flag(data_vertices_count);
    std::fill(flag.begin(), flag.end(), 0);
    std::vector<bool> visited_query_vertex(query_vertices_count);
    std::fill(visited_query_vertex.begin(), visited_query_vertex.end(), false);

    visited_query_vertex[root] = true;

    TE_Candidates.resize(query_vertices_count);

    for (ui i = 1; i < query_vertices_count; ++i) {
        VertexID u = order[i];
        VertexID u_p = tree[u].parent_;

        ui u_label = query_graph->getVertexLabel(u);
        ui u_degree = query_graph->getVertexDegree(u);
#if OPTIMIZED_VLABELED_GRAPH == 1
        auto u_nlf = query_graph->getVertexNLF(u);
#endif
        candidates_count[u] = 0;

        visited_query_vertex[u] = true;
        VertexID* frontiers = candidates[u_p];
        ui frontiers_count = candidates_count[u_p];

        for (ui j = 0; j < frontiers_count; ++j) {
            VertexID v_f = frontiers[j];

            if (v_f == INVALID_VERTEX_ID)
                continue;

            ui nbrs_cnt;
            const VertexID* nbrs = data_graph->getVertexNeighbors(v_f, nbrs_cnt);

            auto iter_pair = TE_Candidates[u].emplace(v_f, std::vector<VertexID>());
            for (ui k = 0; k < nbrs_cnt; ++k) {
                VertexID v = nbrs[k];

                if (data_graph->getVertexLabel(v) == u_label && data_graph->getVertexDegree(v) >= u_degree) {

                    
#if OPTIMIZED_VLABELED_GRAPH == 1
                    auto v_nlf = data_graph->getVertexNLF(v);
                    if (v_nlf->size() >= u_nlf->size()) {
                        bool is_valid = true;

#ifdef ELABELED_GRAPH
                        for (auto& velement : *u_nlf) {
                            auto viter = v_nlf->find(velement.first);
                            if (viter == v_nlf->end() || viter->second.size() < velement.second.size()) {
                                is_valid = false;
                                break;
                            }
                            for (auto& eelement : velement.second) {
                                auto eiter = viter->second.find(eelement.first);
                                if (eiter == viter->second.end() || eiter->second < eelement.second) {
                                    is_valid = false;
                                    break;
                                }
                            }
                            if (is_valid == false)
                                break;
                        }
#else
                        for (auto element : *u_nlf) {
                            auto iter = v_nlf->find(element.first);
                            if (iter == v_nlf->end() || iter->second < element.second) {
                                is_valid = false;
                                break;
                            }
                        }
#endif

                        if (is_valid) {
                            iter_pair.first->second.push_back(v);
                            if (flag[v] == 0) {
                                flag[v] = 1;
                                candidates[u][candidates_count[u]++] = v;
                            }
                        }
                    }
#else
                    iter_pair.first->second.push_back(v);
                    if (flag[v] == 0) {
                        flag[v] = 1;
                        candidates[u][candidates_count[u]++] = v;
                    }
#endif
                }
            }

            if (iter_pair.first->second.empty()) {
                frontiers[j] = INVALID_VERTEX_ID;
                for (ui k = 0; k < tree[u_p].children_count_; ++k) {
                    VertexID u_c = tree[u_p].children_[k];
                    if (visited_query_vertex[u_c]) {
                        TE_Candidates[u_c].erase(v_f);
                    }
                }
            }
        }

        if (candidates_count[u] == 0)
            return false;

        for (ui j = 0; j < candidates_count[u]; ++j) {
            VertexID v = candidates[u][j];
            flag[v] = 0;
        }
    }

    
    NTE_Candidates.resize(query_vertices_count);
    for (auto& value : NTE_Candidates) {
        value.resize(query_vertices_count);
    }

    for (ui i = 1; i < query_vertices_count; ++i) {
        VertexID u = order[i];
        TreeNode &u_node = tree[u];

        ui u_label = query_graph->getVertexLabel(u);
        ui u_degree = query_graph->getVertexDegree(u);
#if OPTIMIZED_VLABELED_GRAPH == 1
        auto u_nlf = query_graph->getVertexNLF(u);
#endif
        for (ui l = 0; l < u_node.bn_count_; ++l) {
            VertexID u_p = u_node.bn_[l];
            VertexID *frontiers = candidates[u_p];
            ui frontiers_count = candidates_count[u_p];

            for (ui j = 0; j < frontiers_count; ++j) {
                VertexID v_f = frontiers[j];

                if (v_f == INVALID_VERTEX_ID)
                    continue;

                ui nbrs_cnt;
                const VertexID *nbrs = data_graph->getVertexNeighbors(v_f, nbrs_cnt);

                auto iter_pair = NTE_Candidates[u][u_p].emplace(v_f, std::vector<VertexID>());
                for (ui k = 0; k < nbrs_cnt; ++k) {
                    VertexID v = nbrs[k];

                    if (data_graph->getVertexLabel(v) == u_label && data_graph->getVertexDegree(v) >= u_degree) {

                        
#if OPTIMIZED_VLABELED_GRAPH == 1
                        auto v_nlf = data_graph->getVertexNLF(v);
                        if (v_nlf->size() >= u_nlf->size()) {
                            bool is_valid = true;

#ifdef ELABELED_GRAPH
                            for (auto& velement : *u_nlf) {
                                auto viter = v_nlf->find(velement.first);
                                if (viter == v_nlf->end() || viter->second.size() < velement.second.size()) {
                                    is_valid = false;
                                    break;
                                }
                                for (auto& eelement : velement.second) {
                                    auto eiter = viter->second.find(eelement.first);
                                    if (eiter == viter->second.end() || eiter->second < eelement.second) {
                                        is_valid = false;
                                        break;
                                    }
                                }
                                if (is_valid == false)
                                    break;
                            }
#else
                            for (auto element : *u_nlf) {
                                auto iter = v_nlf->find(element.first);
                                if (iter == v_nlf->end() || iter->second < element.second) {
                                    is_valid = false;
                                    break;
                                }
                            }
#endif


                            if (is_valid) {
                                iter_pair.first->second.push_back(v);
                            }
                        }
#else
                        iter_pair.first->second.push_back(v);
#endif
                    }
                }

                if (iter_pair.first->second.empty()) {
                    frontiers[j] = INVALID_VERTEX_ID;
                    for (ui k = 0; k < tree[u_p].children_count_; ++k) {
                        VertexID u_c = tree[u_p].children_[k];
                        TE_Candidates[u_c].erase(v_f);
                    }
                }
            }
        }
    }

    
    std::vector<std::vector<ui>> cardinality(query_vertices_count);
    for (ui i = 0; i < query_vertices_count; ++i) {
        cardinality[i].resize(candidates_count[i], 1);
    }

    std::vector<ui> local_cardinality(data_vertices_count);
    std::fill(local_cardinality.begin(), local_cardinality.end(), 0);

    for (int i = query_vertices_count - 1; i >= 0; --i) {
        VertexID u = order[i];
        TreeNode& u_node = tree[u];

        ui flag_num = 0;
        ui updated_flag_count = 0;

        
        for (ui j = 0; j < candidates_count[u]; ++j) {
            VertexID v = candidates[u][j];

            if (v == INVALID_VERTEX_ID)
                continue;

            if (flag[v] == flag_num) {
                flag[v] += 1;
                updated_flag[updated_flag_count++] = v;
            }
        }

        for (ui j = 0; j < u_node.bn_count_; ++j) {
            VertexID u_bn = u_node.bn_[j];
            flag_num += 1;
            for (auto iter = NTE_Candidates[u][u_bn].begin(); iter != NTE_Candidates[u][u_bn].end(); ++iter) {
                for (auto v : iter->second) {
                    if (flag[v] == flag_num) {
                        flag[v] += 1;
                    }
                }
            }
        }

        flag_num += 1;

        
        for (ui j = 0; j < candidates_count[u]; ++j) {
            VertexID v = candidates[u][j];
            if (v != INVALID_VERTEX_ID && flag[v] == flag_num) {
                local_cardinality[v] = cardinality[u][j];
            }
            else {
                cardinality[u][j] = 0;
            }
        }

        VertexID u_p = u_node.parent_;
        VertexID* frontiers = candidates[u_p];
        ui frontiers_count = candidates_count[u_p];

        
        for (ui j = 0; j < frontiers_count; ++j) {
            VertexID v_f = frontiers[j];

            if (v_f == INVALID_VERTEX_ID) {
                cardinality[u_p][j] = 0;
                continue;
            }

            ui temp_score = 0;
            for (auto iter = TE_Candidates[u][v_f].begin(); iter != TE_Candidates[u][v_f].end();) {
                VertexID v = *iter;
                temp_score += local_cardinality[v];
                if (local_cardinality[v] == 0) {
                    iter = TE_Candidates[u][v_f].erase(iter);
                    for (ui k = 0; k < u_node.children_count_; ++k) {
                        VertexID u_c = u_node.children_[k];
                        TE_Candidates[u_c].erase(v);
                    }

                    for (ui k = 0; k < u_node.fn_count_; ++k) {
                        VertexID u_c = u_node.fn_[k];
                        NTE_Candidates[u_c][u].erase(v);
                    }
                }
                else {
                    ++iter;
                }
            }

            cardinality[u_p][j] *= temp_score;
        }

        
        for (ui j = 0; j < updated_flag_count; ++j) {
            flag[updated_flag[j]] = 0;
            local_cardinality[updated_flag[j]] = 0;
        }
    }

    compactCandidates(candidates, candidates_count, query_vertices_count);
    sortCandidates(candidates, candidates_count, query_vertices_count);


    for (ui i = 0; i < query_vertices_count; ++i) {
        if (candidates_count[i] == 0) {
            return false;
        }
    }

    for (ui i = 1; i < query_vertices_count; ++i) {
        VertexID u = order[i];
        TreeNode& u_node = tree[u];

        
        {
            VertexID u_p = u_node.parent_;
            auto iter = TE_Candidates[u].begin();
            while (iter != TE_Candidates[u].end()) {
                VertexID v_f = iter->first;
                if (!std::binary_search(candidates[u_p], candidates[u_p] + candidates_count[u_p], v_f)) {
                    iter = TE_Candidates[u].erase(iter);
                }
                else {
                    std::sort(iter->second.begin(), iter->second.end());
                    iter++;
                }
            }
        }

        
        {
            for (ui j = 0; j < u_node.bn_count_; ++j) {
                VertexID u_p = u_node.bn_[j];
                auto iter = NTE_Candidates[u][u_p].end();
                while (iter != NTE_Candidates[u][u_p].end()) {
                    VertexID v_f = iter->first;
                    if (!std::binary_search(candidates[u_p], candidates[u_p] + candidates_count[u_p], v_f)) {
                        iter = NTE_Candidates[u][u_p].erase(iter);
                    }
                    else {
                        std::sort(iter->second.begin(), iter->second.end());
                        iter++;
                    }
                }
            }
        }
    }










































    return true;
}



bool FilterVertices::VEQFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count,
                               ui *&order, TreeNode *&tree) {

    
    if (!LDFFilter(data_graph, query_graph, candidates, candidates_count))
        return false;
    GenerateFilteringPlan::generateDPisoFilterPlan(data_graph, query_graph, tree, order);

    ui query_vertices_num = query_graph->getVerticesCount();
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    bool** flag = new bool*[query_graph->getVerticesCount()];
    for (ui i = 0; i < query_graph->getVerticesCount(); i++) {
        flag[i] = new bool[data_graph->getVerticesCount()];
        memset(flag[i], 0, sizeof(bool)*data_graph->getVerticesCount());
        for (ui j = 0; j < candidates_count[i]; j++) {
            flag[i][candidates[i][j]] = true;
        }
    }
    for (ui u = 0; u < query_vertices_num; u++) {
        ui unbrs_count;
        const VertexID* unbrs = query_graph->getVertexNeighbors(u, unbrs_count);
        for (ui i = 0; i < candidates_count[u]; ++i) {
            auto can = candidates[u][i];
            ui cnbrs_count;
            const ui* cnbrs = data_graph->getVertexNeighbors(can, cnbrs_count);
            bool f_del = true;
            for (ui j = 0; j < unbrs_count; j++) {
                auto unbr = unbrs[j];
                f_del = true;
                for (ui k = 0; k < cnbrs_count; k++) {
                    auto cnbr = cnbrs[k];
                    if (flag[unbr][cnbr] == true) {
                        
                        f_del = false;
                        break;
                    }
                }
                if (f_del == true) break;
            }
            if (f_del == true) {
                
                if (candidates_count[u] == 1) {
                    std::cout << "candidates for " << u << " is empty" << std::endl;
                    return false;
                }
                flag[u][can] = false;
                candidates[u][i] = candidates[u][--candidates_count[u]];
                i--;
            }
        }
        
    }

    
    
    
    
    
    
    
    

    
    
    std::vector<std::set<VertexID>> m_candidates;
    m_candidates.resize(query_vertices_num);
    for (ui i = 0; i < query_vertices_num; i++) {
        for (ui j = 0; j < candidates_count[i]; j++) {
            m_candidates[i].insert(candidates[i][j]);
        }
    }
    for (ui k = 0; k < 3; ++k) {
        if (k % 2 == 1) {
            for (ui i = 0; i < query_vertices_num; i++) { 
                VertexID query_vertex = order[i];
                TreeNode& node = tree[query_vertex];
                VEQPruneCandidates(data_graph, query_graph, query_vertex, node.bn_, node.bn_count_,
                                   candidates, candidates_count, m_candidates);
            }
        }
        else {
            for (ui i = query_vertices_num - 1; i != (ui)-1; i--) { 
                VertexID query_vertex = order[i];
                TreeNode& node = tree[query_vertex];
                VEQPruneCandidates(data_graph, query_graph, query_vertex, node.fn_, node.fn_count_,
                                   candidates, candidates_count, m_candidates);
            }
        }
    }

    VEQCompactCandidates(candidates, candidates_count, query_vertices_num, m_candidates);

    
    
    
    
    
    
    
    

    return true;
}

void
FilterVertices::VEQPruneCandidates(const Graph *data_graph, const Graph *query_graph, VertexID u,
                                   VertexID *child_vertices, ui child_vertices_count, VertexID **candidates,
                                   ui *candidates_count, std::vector<std::set<VertexID>>& D) {
#if OPTIMIZED_VLABELED_GRAPH == 1
    auto u_nlf = query_graph->getVertexNLF(u);
#endif
    ui unbrs_num, vnbrs_num;
    const VertexID* unbrs = query_graph->getVertexNeighbors(u, unbrs_num);
    const VertexID* vnbrs;
    ui degree = query_graph->getVertexDegree(u);
    for (ui i = 0; i < candidates_count[u]; i++) {
        
        
        VertexID v = candidates[u][i];
        auto v_iter = D[u].find(v);
        if (v_iter == D[u].end()) continue;
        if (data_graph->getVertexDegree(v) < degree) {
            D[u].erase(v_iter);
            continue;
        }

#if OPTIMIZED_VLABELED_GRAPH == 1
        bool h_value = true;
        vnbrs = data_graph->getVertexNeighbors(v, vnbrs_num);

#ifdef ELABELED_GRAPH
        std::unordered_map<LabelID, std::unordered_map<LabelID, ui>> v_nlf; 
        auto elabels = data_graph->getVertexEdgeLabels(v, vnbrs_num);
        for (ui j = 0; j < vnbrs_num; j++) { 
            VertexID vc = vnbrs[j];
            LabelID vc_label = data_graph->getVertexLabel(vc);
            LabelID elabel = elabels[j];
            for (ui k = 0; k < unbrs_num; k++) {
                VertexID unbr = unbrs[k];
                auto iter = D[unbr].find(vc);
                if (iter != D[unbr].end()) {
                    if (v_nlf.find(vc_label) == v_nlf.end() || v_nlf[vc_label].find(elabel) == v_nlf[vc_label].end()) {
                        v_nlf[vc_label][elabel] = 1;
                    } else {
                        v_nlf[vc_label][elabel]++;
                    }
                    break;
                }
            }
        }
        if (v_nlf.size() < u_nlf->size()) {
            D[u].erase(v_iter);
            continue;
        }
        for (auto& velement : *u_nlf) {
            auto viter = v_nlf.find(velement.first);
            if (viter == v_nlf.end() || viter->second.size() < velement.second.size()) {
                h_value = false;
                break;
            }
            for (auto& eelement : velement.second) {
                auto eiter = viter->second.find(eelement.first);
                if (eiter == viter->second.end() || eiter->second < eelement.second) {
                    h_value = false;
                    break;
                }
            }
            if (h_value == false)
                break;
        }
#else
        std::unordered_map<LabelID, ui> v_nlf; 
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        for (ui j = 0; j < vnbrs_num; j++) { 
            VertexID vc = vnbrs[j];
            for (ui k = 0; k < unbrs_num; k++) {
                VertexID unbr = unbrs[k];
                auto iter = D[unbr].find(vc);
                if (iter != D[unbr].end()) {
                    ui vc_label = data_graph->getVertexLabel(vc);
                    if (v_nlf.find(vc_label) != v_nlf.end()) {
                        v_nlf[vc_label] += 1;
                    } else {
                        v_nlf[vc_label] = 1;
                    }
                    break;
                }
            }
        }
        if (v_nlf.size() < u_nlf->size()) {
            
            
            D[u].erase(v_iter);
            continue;
        }
        
        for (auto u_label : *u_nlf) {
            auto v_label = v_nlf.find(u_label.first);
            if (v_label == v_nlf.end() || v_label->second < u_label.second) {
                
                h_value = false;
                break;
            }
            
        }
#endif
        
        if (h_value == false) {
            D[u].erase(v_iter);
            continue;
        }
        
#endif
        
        for (ui j = 0; j < child_vertices_count; j++) {
            VertexID uc = child_vertices[j];
            bool uc_flag = false; 
            for (ui k = 0; k < vnbrs_num; k++) {
                VertexID vc = vnbrs[k];
                auto iter = D[uc].find(vc);
                if (iter != D[uc].end()) {
                    uc_flag = true;
                    break;
                }
            }
            if (uc_flag == false) {
                
                D[u].erase(v_iter);
                break;
            }
        }
    }
}

void FilterVertices::VEQCompactCandidates(ui** &candidates, ui* &candidates_count, ui query_vertex_num, 
                                          std::vector<std::set<VertexID>>& D) {
    for (ui i = 0; i < query_vertex_num; i++) {
        for (ui j = 0; j < candidates_count[i]; j++) {
            if (D[i].find(candidates[i][j]) == D[i].end()) {
                candidates[i][j] = candidates[i][--candidates_count[i]];
                if(candidates_count[i] == 0) return;
                j--;
            }
        }
    }
}

void FilterVertices::allocateBuffer(const Graph *data_graph, const Graph *query_graph, ui **&candidates,
                                    ui *&candidates_count) {
    ui query_vertices_num = query_graph->getVerticesCount();
    ui candidates_max_num = data_graph->getGraphMaxLabelFrequency();

    candidates_count = new ui[query_vertices_num];
    memset(candidates_count, 0, sizeof(ui) * query_vertices_num);

    candidates = new ui*[query_vertices_num];

    for (ui i = 0; i < query_vertices_num; ++i) {
        candidates[i] = new ui[candidates_max_num];
    }
}

bool
FilterVertices::verifyExactTwigIso(const Graph *data_graph, const Graph *query_graph, ui data_vertex, ui query_vertex,
                                   bool **valid_candidates, int *left_to_right_offset, int *left_to_right_edges,
                                   int *left_to_right_match, int *right_to_left_match, int* match_visited,
                                   int* match_queue, int* match_previous) {
    
    ui left_partition_size;
    ui right_partition_size;
    const VertexID* query_vertex_neighbors = query_graph->getVertexNeighbors(query_vertex, left_partition_size);
    const VertexID* data_vertex_neighbors = data_graph->getVertexNeighbors(data_vertex, right_partition_size);

    ui edge_count = 0;
    for (int i = 0; i < left_partition_size; ++i) {
        VertexID query_vertex_neighbor = query_vertex_neighbors[i];
        left_to_right_offset[i] = edge_count;

        for (int j = 0; j < right_partition_size; ++j) {
            VertexID data_vertex_neighbor = data_vertex_neighbors[j];

            
            if (valid_candidates[query_vertex_neighbor][data_vertex_neighbor]) {
                left_to_right_edges[edge_count++] = j;
            }
        }
    }
    left_to_right_offset[left_partition_size] = edge_count;
    

    memset(left_to_right_match, -1, left_partition_size * sizeof(int));
    memset(right_to_left_match, -1, right_partition_size * sizeof(int));

    
    
    
    
    
    
    GraphOperations::match_bfs(left_to_right_offset, left_to_right_edges, left_to_right_match, right_to_left_match,
                               match_visited, match_queue, match_previous, left_partition_size, right_partition_size);
    for (int i = 0; i < left_partition_size; ++i) {
        if (left_to_right_match[i] == -1)
            return false;
    }

    return true;
}

void FilterVertices::compactCandidates(ui **&candidates, ui *&candidates_count, ui query_vertices_num) {
    for (ui i = 0; i < query_vertices_num; ++i) {
        VertexID query_vertex = i;
        ui next_position = 0;
        for (ui j = 0; j < candidates_count[query_vertex]; ++j) {
            VertexID data_vertex = candidates[query_vertex][j];

            if (data_vertex != INVALID_VERTEX_ID) {
                candidates[query_vertex][next_position++] = data_vertex;
            }
        }

        candidates_count[query_vertex] = next_position;
    }
}

bool FilterVertices::isCandidateSetValid(ui **&candidates, ui *&candidates_count, ui query_vertices_num) {
    for (ui i = 0; i < query_vertices_num; ++i) {
        if (candidates_count[i] == 0)
            return false;
    }
    return true;
}

void
FilterVertices::computeCandidateWithNLF(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                               ui &count, ui *buffer) {
    LabelID label = query_graph->getVertexLabel(query_vertex);
    ui degree = query_graph->getVertexDegree(query_vertex);
#if OPTIMIZED_VLABELED_GRAPH == 1
    auto u_nlf = query_graph->getVertexNLF(query_vertex);
#endif
    ui data_vertex_num;
    const ui* data_vertices = data_graph->getVerticesByLabel(label, data_vertex_num);
    count = 0;
    for (ui j = 0; j < data_vertex_num; ++j) {
        ui data_vertex = data_vertices[j];
        if (data_graph->getVertexDegree(data_vertex) >= degree) {

            
#if OPTIMIZED_VLABELED_GRAPH == 1
            auto v_nlf = data_graph->getVertexNLF(data_vertex);
            if (v_nlf->size() >= u_nlf->size()) {
                bool is_valid = true;

#ifdef ELABELED_GRAPH
                for (auto& velement : *u_nlf) {
                    auto viter = v_nlf->find(velement.first);
                    if (viter == v_nlf->end() || viter->second.size() < velement.second.size()) {
                        is_valid = false;
                        break;
                    }
                    for (auto& eelement : velement.second) {
                        auto eiter = viter->second.find(eelement.first);
                        if (eiter == viter->second.end() || eiter->second < eelement.second) {
                            is_valid = false;
                            break;
                        }
                    }
                    if (is_valid == false)
                        break;
                }
#else
                for (auto element : *u_nlf) {
                    auto iter = v_nlf->find(element.first);
                    if (iter == v_nlf->end() || iter->second < element.second) {
                        is_valid = false;
                        break;
                    }
                }
#endif

                if (is_valid) {
                    if (buffer != NULL) {
                        buffer[count] = data_vertex;
                    }
                    count += 1;
                }
            }
#else
            if (buffer != NULL) {
                buffer[count] = data_vertex;
            }
            count += 1;
#endif
        }
    }

}

void FilterVertices::computeCandidateWithLDF(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                             ui &count, ui *buffer) {
    LabelID label = query_graph->getVertexLabel(query_vertex);
    ui degree = query_graph->getVertexDegree(query_vertex);
    count = 0;
    ui data_vertex_num;
    const ui* data_vertices = data_graph->getVerticesByLabel(label, data_vertex_num);

    if (buffer == NULL) {
        for (ui i = 0; i < data_vertex_num; ++i) {
            VertexID v = data_vertices[i];
            if (data_graph->getVertexDegree(v) >= degree) {
                count += 1;
            }
        }
    }
    else {
        for (ui i = 0; i < data_vertex_num; ++i) {
            VertexID v = data_vertices[i];
            if (data_graph->getVertexDegree(v) >= degree) {
                buffer[count++] = v;
            }
        }
    }
}

void FilterVertices::generateCandidates(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                       VertexID *pivot_vertices, ui pivot_vertices_count, VertexID **candidates,
                                       ui *candidates_count, ui *flag, ui *updated_flag) {
    LabelID query_vertex_label = query_graph->getVertexLabel(query_vertex);
    ui query_vertex_degree = query_graph->getVertexDegree(query_vertex);
#if OPTIMIZED_VLABELED_GRAPH == 1
    auto u_nlf = query_graph->getVertexNLF(query_vertex);
#endif
    ui count = 0;
    ui updated_flag_count = 0;
    for (ui i = 0; i < pivot_vertices_count; ++i) {
        VertexID pivot_vertex = pivot_vertices[i];

        for (ui j = 0; j < candidates_count[pivot_vertex]; ++j) {
            VertexID v = candidates[pivot_vertex][j];

            if (v == INVALID_VERTEX_ID)
                continue;
            ui v_nbrs_count;
            const VertexID* v_nbrs = data_graph->getVertexNeighbors(v, v_nbrs_count);

            for (ui k = 0; k < v_nbrs_count; ++k) {
                VertexID v_nbr = v_nbrs[k];
                LabelID v_nbr_label = data_graph->getVertexLabel(v_nbr);
                ui v_nbr_degree = data_graph->getVertexDegree(v_nbr);

                if (flag[v_nbr] == count && v_nbr_label == query_vertex_label && v_nbr_degree >= query_vertex_degree) {
                    flag[v_nbr] += 1;

                    if (count == 0) {
                        updated_flag[updated_flag_count++] = v_nbr;
                    }
                }
            }
        }

        count += 1;
    }

    for (ui i = 0; i < updated_flag_count; ++i) {
        VertexID v = updated_flag[i];
        if (flag[v] == count) {
            
#if OPTIMIZED_VLABELED_GRAPH == 1
            auto v_nlf = data_graph->getVertexNLF(v);
            if (v_nlf->size() >= u_nlf->size()) {
                bool is_valid = true;

#ifdef ELABELED_GRAPH
                for (auto& velement : *u_nlf) {
                    auto viter = v_nlf->find(velement.first);
                    if (viter == v_nlf->end() || viter->second.size() < velement.second.size()) {
                        is_valid = false;
                        break;
                    }
                    for (auto& eelement : velement.second) {
                        auto eiter = viter->second.find(eelement.first);
                        if (eiter == viter->second.end() || eiter->second < eelement.second) {
                            is_valid = false;
                            break;
                        }
                    }
                    if (is_valid == false)
                        break;
                }
#else
                for (auto element : *u_nlf) {
                    auto iter = v_nlf->find(element.first);
                    if (iter == v_nlf->end() || iter->second < element.second) {
                        is_valid = false;
                        break;
                    }
                }
#endif

                if (is_valid) {
                    candidates[query_vertex][candidates_count[query_vertex]++] = v;
                }
            }
#else
            candidates[query_vertex][candidates_count[query_vertex]++] = v;
#endif
        }
    }

    for (ui i = 0; i < updated_flag_count; ++i) {
        ui v = updated_flag[i];
        flag[v] = 0;
    }
}


void
FilterVertices::pruneCandidates(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                    VertexID *pivot_vertices, ui pivot_vertices_count, VertexID **candidates,
                                    ui *candidates_count, ui *flag, ui *updated_flag) {
    LabelID query_vertex_label = query_graph->getVertexLabel(query_vertex);
    ui query_vertex_degree = query_graph->getVertexDegree(query_vertex);

    ui count = 0;
    ui updated_flag_count = 0;
#ifdef ELABELED_GRAPH
    for (ui i = 0; i < pivot_vertices_count; ++i) {
        VertexID pivot_vertex = pivot_vertices[i];
        LabelID p_elabel = query_graph->getEdgeLabel(query_vertex, pivot_vertex, true);

        for (ui j = 0; j < candidates_count[pivot_vertex]; ++j) {
            VertexID v = candidates[pivot_vertex][j];

            if (v == INVALID_VERTEX_ID)
                continue;
            ui v_nbrs_count;
            auto v_nbrs = data_graph->getVertexNeighbors(v, v_nbrs_count);
            auto v_elabels = data_graph->getVertexEdgeLabels(v, v_nbrs_count);

            for (ui k = 0; k < v_nbrs_count; ++k) {
                VertexID v_nbr = v_nbrs[k];
                LabelID v_nbr_elabel = v_elabels[k];
                LabelID v_nbr_label = data_graph->getVertexLabel(v_nbr);
                ui v_nbr_degree = data_graph->getVertexDegree(v_nbr);

                if (flag[v_nbr] == count
                    && v_nbr_label == query_vertex_label
                    && v_nbr_degree >= query_vertex_degree
                    && p_elabel == v_nbr_elabel) {
                    flag[v_nbr] += 1;

                    if (count == 0) {
                        updated_flag[updated_flag_count++] = v_nbr;
                    }
                }
            }
        }

        count += 1;
    }
#else
    for (ui i = 0; i < pivot_vertices_count; ++i) {
        VertexID pivot_vertex = pivot_vertices[i];

        for (ui j = 0; j < candidates_count[pivot_vertex]; ++j) {
            VertexID v = candidates[pivot_vertex][j];

            if (v == INVALID_VERTEX_ID)
                continue;
            ui v_nbrs_count;
            auto v_nbrs = data_graph->getVertexNeighbors(v, v_nbrs_count);

            for (ui k = 0; k < v_nbrs_count; ++k) {
                VertexID v_nbr = v_nbrs[k];
                LabelID v_nbr_label = data_graph->getVertexLabel(v_nbr);
                ui v_nbr_degree = data_graph->getVertexDegree(v_nbr);

                if (flag[v_nbr] == count && v_nbr_label == query_vertex_label && v_nbr_degree >= query_vertex_degree) {
                    flag[v_nbr] += 1;

                    if (count == 0) {
                        updated_flag[updated_flag_count++] = v_nbr;
                    }
                }
            }
        }

        count += 1;
    }
#endif

    for (ui i = 0; i < candidates_count[query_vertex]; ++i) {
        ui v = candidates[query_vertex][i];
        if (v == INVALID_VERTEX_ID)
            continue;

        if (flag[v] != count) {
            candidates[query_vertex][i] = INVALID_VERTEX_ID;
        }
    }

    for (ui i = 0; i < updated_flag_count; ++i) {
        ui v = updated_flag[i];
        flag[v] = 0;
    }
}

void FilterVertices::printCandidatesInfo(const Graph *query_graph, ui *candidates_count, std::vector<ui> &optimal_candidates_count) {
    std::vector<std::pair<VertexID, ui>> core_vertices;
    std::vector<std::pair<VertexID, ui>> tree_vertices;
    std::vector<std::pair<VertexID, ui>> leaf_vertices;

    ui query_vertices_num = query_graph->getVerticesCount();
    double sum = 0;
    double optimal_sum = 0;
    for (ui i = 0; i < query_vertices_num; ++i) {
        VertexID cur_vertex = i;
        ui count = candidates_count[cur_vertex];
        sum += count;
        optimal_sum += optimal_candidates_count[cur_vertex];

        if (query_graph->getCoreValue(cur_vertex) > 1) {
            core_vertices.emplace_back(std::make_pair(cur_vertex, count));
        }
        else {
            if (query_graph->getVertexDegree(cur_vertex) > 1) {
                tree_vertices.emplace_back(std::make_pair(cur_vertex, count));
            }
            else {
                leaf_vertices.emplace_back(std::make_pair(cur_vertex, count));
            }
        }
    }

    printf("#Candidate Information: CoreVertex(%zu), TreeVertex(%zu), LeafVertex(%zu)\n", core_vertices.size(), tree_vertices.size(), leaf_vertices.size());

    for (auto candidate_info : core_vertices) {
        printf("CoreVertex %u: %u, %u \n", candidate_info.first, candidate_info.second, optimal_candidates_count[candidate_info.first]);
    }

    for (auto candidate_info : tree_vertices) {
        printf("TreeVertex %u: %u, %u\n", candidate_info.first, candidate_info.second, optimal_candidates_count[candidate_info.first]);
    }

    for (auto candidate_info : leaf_vertices) {
        printf("LeafVertex %u: %u, %u\n", candidate_info.first, candidate_info.second, optimal_candidates_count[candidate_info.first]);
    }

    printf("Total #Candidates: %.1lf, %.1lf\n", sum, optimal_sum);
}

void FilterVertices::sortCandidates(ui **candidates, ui *candidates_count, ui num) {
    for (ui i = 0; i < num; ++i) {
        std::sort(candidates[i], candidates[i] + candidates_count[i]);
    }
}

double
FilterVertices::computeCandidatesFalsePositiveRatio(const Graph *data_graph, const Graph *query_graph, ui **candidates,
                                                    ui *candidates_count, std::vector<ui> &optimal_candidates_count) {
    ui query_vertices_count = query_graph->getVerticesCount();
    ui data_vertices_count = data_graph->getVerticesCount();

    std::vector<std::vector<ui>> candidates_copy(query_vertices_count);
    for (ui i = 0; i < query_vertices_count; ++i) {
        candidates_copy[i].resize(candidates_count[i]);
        std::copy(candidates[i], candidates[i] + candidates_count[i], candidates_copy[i].begin());
    }

    std::vector<int> flag(data_vertices_count, 0);
    std::vector<ui> updated_flag;
    std::vector<double> per_query_vertex_false_positive_ratio(query_vertices_count);
    optimal_candidates_count.resize(query_vertices_count);

    bool is_steady = false;
    while (!is_steady) {
        is_steady = true;
        for (ui i = 0; i < query_vertices_count; ++i) {
            ui u = i;

            ui u_nbr_cnt;
            const ui *u_nbrs = query_graph->getVertexNeighbors(u, u_nbr_cnt);

            ui valid_flag = 0;
            for (ui j = 0; j < u_nbr_cnt; ++j) {
                ui u_nbr = u_nbrs[j];

                for (ui k = 0; k < candidates_count[u_nbr]; ++k) {
                    ui v = candidates_copy[u_nbr][k];

                    if (v == INVALID_VERTEX_ID)
                        continue;

                    ui v_nbr_cnt;
                    const ui *v_nbrs = data_graph->getVertexNeighbors(v, v_nbr_cnt);

                    for (ui l = 0; l < v_nbr_cnt; ++l) {
                        ui v_nbr = v_nbrs[l];

                        if (flag[v_nbr] == valid_flag) {
                            flag[v_nbr] += 1;

                            if (valid_flag == 0) {
                                updated_flag.push_back(v_nbr);
                            }
                        }
                    }
                }
                valid_flag += 1;
            }

            for (ui j = 0; j < candidates_count[u]; ++j) {
                ui v = candidates_copy[u][j];

                if (v == INVALID_VERTEX_ID)
                    continue;

                if (flag[v] != valid_flag) {
                    candidates_copy[u][j] = INVALID_VERTEX_ID;
                    is_steady = false;
                }
            }

            for (auto v : updated_flag) {
                flag[v] = 0;
            }
            updated_flag.clear();
        }
    }

    double sum = 0;
    for (ui i = 0; i < query_vertices_count; ++i) {
        ui u = i;
        ui negative_count = 0;
        for (ui j = 0; j < candidates_count[u]; ++j) {
            ui v = candidates_copy[u][j];

            if (v == INVALID_VERTEX_ID)
                negative_count += 1;
        }

        per_query_vertex_false_positive_ratio[u] =
                (negative_count) / (double) candidates_count[u];
        sum += per_query_vertex_false_positive_ratio[u];
        optimal_candidates_count[u] = candidates_count[u] - negative_count;
    }

    return sum / query_vertices_count;
}

bool
FilterVertices::RMFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates,
                         ui *&candidates_count, catalog *&storage) {
    storage = new catalog(query_graph, data_graph);
    auto vertices_count = query_graph->getVerticesCount();
    auto data_vertices_count = data_graph->getVerticesCount();
    auto non_core_vertices_count = vertices_count - query_graph->get2CoreSize();
    auto degeneracy_ordering = new uint32_t[vertices_count];
    auto vertices_index = new uint32_t[vertices_count];
    uint32_t *non_core_vertices_parent = NULL;
    uint32_t *non_core_vertices_children = NULL;
    uint32_t *non_core_vertices_children_offset = NULL;
    if (non_core_vertices_count != 0) {
        non_core_vertices_parent = new uint32_t[non_core_vertices_count];
        non_core_vertices_children = new uint32_t[non_core_vertices_count];
        non_core_vertices_children_offset = new uint32_t[non_core_vertices_count + 1];
    }
    scan_relation(data_graph, query_graph, storage);

    auto filter = new nlf_filter(query_graph, data_graph);
    filter->execute(storage);

    generate_preprocess_plan(query_graph, degeneracy_ordering, vertices_index, non_core_vertices_count,
                             non_core_vertices_parent, non_core_vertices_children,
                             non_core_vertices_children_offset);
    eliminate_dangling_tuples(data_graph, query_graph, storage, degeneracy_ordering, vertices_index,
                              non_core_vertices_count, non_core_vertices_parent, non_core_vertices_children,
                              non_core_vertices_children_offset);
    
    candidates = new ui*[vertices_count];
    candidates_count = new ui[vertices_count];
    ui candidates_max_num = data_graph->getGraphMaxLabelFrequency();
    
    bool* index = new bool[data_vertices_count];
    for (ui i = 0; i < vertices_count; i++) {
        candidates[i] = new ui[candidates_max_num];
        memset(index, 0, sizeof(bool)*data_vertices_count);
        ui can_cnt = 0;
        ui nbr_cnt;
        const ui* nbrs = query_graph->getVertexNeighbors(i, nbr_cnt);
        assert(nbr_cnt != 0);
        ui v = nbrs[0];
        ui pos = i < v ? 0 : 1;  
        ui u = i < v ? i : v;
        v = i > v ? i : v;
        auto& edges = storage->edge_relations_[u][v];
        for (ui j = 0; j < edges.size_; j++) {
            auto& edge = edges.edges_[j];
            if (!index[edge[pos]]) {
                if (can_cnt >= candidates_max_num) {
                    std::cout << "can_cnt:" << can_cnt << std::endl;
                }
                assert(can_cnt < candidates_max_num);
                index[edge[pos]] = true;
                candidates[i][can_cnt++] = edge[pos];
            }
        }
        candidates_count[i] = can_cnt;
    }
    
    
    
    
    
    delete[] index;
    delete[] degeneracy_ordering;
    delete[] vertices_index;
    delete[] non_core_vertices_parent;
    delete[] non_core_vertices_children;
    delete[] non_core_vertices_children_offset;
    delete filter;
    return true;
}

void FilterVertices::scan_relation(const Graph *data_graph, const Graph *query_graph, catalog *storage) {
    auto scan_operator = new scan(data_graph);
    auto vertices_count = query_graph->getVerticesCount();

    for (uint32_t u = 0; u < vertices_count; ++u) {
        uint32_t u_nbrs_cnt;
        const uint32_t* u_nbrs = query_graph->getVertexNeighbors(u, u_nbrs_cnt);

        for (uint32_t i = 0; i < u_nbrs_cnt; ++i) {
            uint32_t v = u_nbrs[i];

            if (u > v)
                continue;

            uint32_t u_label = query_graph->getVertexLabel(u);
            uint32_t v_label = query_graph->getVertexLabel(v);
            scan_operator->execute(u_label, v_label, &(storage->edge_relations_[u][v]), true);
        }
    }

    delete scan_operator;
}

void FilterVertices::generate_preprocess_plan(const Graph *query_graph, uint32_t *degeneracy_ordering, uint32_t *vertices_index,
                              uint non_core_vertices_count, uint32_t *non_core_vertices_parent,
                              uint32_t *non_core_vertices_children, uint32_t *non_core_vertices_children_offset) {
    GraphOperations::compute_degeneracy_order(query_graph, degeneracy_ordering);
    auto vertices_count = query_graph->getVerticesCount();
    for (uint32_t i = 0; i < vertices_count; ++i) {
        vertices_index[degeneracy_ordering[i]] = i;
    }

    if (non_core_vertices_count == 0)
        return;

    uint32_t children_offset = 0;
    for (uint32_t i = 0; i < non_core_vertices_count; ++i) {
        uint32_t u = degeneracy_ordering[i];
        uint32_t u_nbrs_cnt;
        const uint32_t* u_nbrs = query_graph->getVertexNeighbors(u, u_nbrs_cnt);
        non_core_vertices_children_offset[i] = children_offset;

        for (uint32_t j = 0; j < u_nbrs_cnt; ++j) {
            uint32_t v = u_nbrs[j];

            if (vertices_index[v] < i) {
                non_core_vertices_children[children_offset++] = v;
            }
            else {
                non_core_vertices_parent[i] = v;
            }
        }
    }

    non_core_vertices_children_offset[non_core_vertices_count] = children_offset;
}

void FilterVertices::eliminate_dangling_tuples(const Graph *data_graph, const Graph *query_graph, catalog *storage,
                               uint32_t *degeneracy_ordering, uint32_t *vertices_index,
                               uint non_core_vertices_count, uint32_t *non_core_vertices_parent,
                               uint32_t *non_core_vertices_children, uint32_t *non_core_vertices_children_offset) {
    auto semi_join_operator = new semi_join(data_graph->getVerticesCount());
    auto vertices_count = query_graph->getVerticesCount();
    
    for (uint32_t i = 0; i < non_core_vertices_count; ++i) {
        uint32_t u = degeneracy_ordering[i];

        if (i == vertices_count - 1)
            break;

        uint32_t v = non_core_vertices_parent[i];

        if (vertices_index[v] < non_core_vertices_count && vertices_index[v] < (vertices_count - 1)) {
            uint32_t w = non_core_vertices_parent[vertices_index[v]];

            
            uint32_t lkp;
            edge_relation* l_relation = get_key_position_in_relation(v, w, storage, lkp);

            uint32_t rkp;
            edge_relation* r_relation = get_key_position_in_relation(v, u, storage, rkp);

            storage->num_candidates_[v] = semi_join_operator->execute(l_relation, lkp, r_relation, rkp);
        }
    }

    
    for (uint32_t i = non_core_vertices_count; i < vertices_count; ++i) {
        uint32_t u = degeneracy_ordering[i];
        uint32_t u_nbrs_cnt;
        const uint32_t* u_nbrs = query_graph->getVertexNeighbors(u, u_nbrs_cnt);

        
        for (int j = 0; j < static_cast<int>(u_nbrs_cnt) - 1; ++j) {

            uint32_t v = u_nbrs[j];
            uint32_t w = u_nbrs[j + 1];

            
            uint32_t lkp;
            edge_relation* l_relation = get_key_position_in_relation(u, w, storage, lkp);
            uint32_t rkp;
            edge_relation* r_relation = get_key_position_in_relation(u, v, storage, rkp);

            storage->num_candidates_[u] = semi_join_operator->execute(l_relation, lkp, r_relation, rkp);
        }

        
        for (int j = static_cast<int>(u_nbrs_cnt) - 1; j > 0; --j) {
            uint32_t v = u_nbrs[j - 1];
            uint32_t w = u_nbrs[j];

            
            uint32_t lkp;
            edge_relation* l_relation = get_key_position_in_relation(u, v, storage, lkp);
            uint32_t rkp;
            edge_relation* r_relation = get_key_position_in_relation(u, w, storage, rkp);

            storage->num_candidates_[u] = semi_join_operator->execute(l_relation, lkp, r_relation, rkp);
        }
    }

    
    for (int i = static_cast<int>(vertices_count) - 1; i >= static_cast<int>(non_core_vertices_count); --i) {
        uint32_t u = degeneracy_ordering[i];
        uint32_t u_nbrs_cnt;
        const uint32_t* u_nbrs = query_graph->getVertexNeighbors(u, u_nbrs_cnt);

        
        for (int j = 0; j < static_cast<int>(u_nbrs_cnt) - 1; ++j) {

            uint32_t v = u_nbrs[j];
            uint32_t w = u_nbrs[j + 1];

            
            uint32_t lkp;
            edge_relation* l_relation = get_key_position_in_relation(u, w, storage, lkp);
            uint32_t rkp;
            edge_relation* r_relation = get_key_position_in_relation(u, v, storage, rkp);

            storage->num_candidates_[u] = semi_join_operator->execute(l_relation, lkp, r_relation, rkp);
        }

        
        uint32_t cur_v = u_nbrs[u_nbrs_cnt - 1];
        uint32_t min_size = 0;
        if (u < cur_v) {
            min_size = storage->edge_relations_[u][cur_v].size_;
        }
        else {
            min_size = storage->edge_relations_[cur_v][u].size_;
        }

        for (int j = static_cast<int>(u_nbrs_cnt) - 1; j > 0; --j) {
            uint32_t v = u_nbrs[j - 1];
            uint32_t w = u_nbrs[j];

            
            uint32_t lkp;
            edge_relation* l_relation = get_key_position_in_relation(u, v, storage, lkp);
            uint32_t rkp;
            edge_relation* r_relation = get_key_position_in_relation(u, w, storage, rkp);

            storage->num_candidates_[u] = semi_join_operator->execute(l_relation, lkp, r_relation, rkp);

            if (l_relation->size_ < min_size) {
                min_size = l_relation->size_;
                cur_v = v;
            }
        }
    }

    
    for (int i = static_cast<int>(non_core_vertices_count) - 1; i >= 0; --i) {
        
        if (i == vertices_count - 1)
            continue;

        uint32_t u = degeneracy_ordering[i];
        uint32_t v = non_core_vertices_parent[i];

        uint32_t rkp;
        edge_relation* r_relation = get_key_position_in_relation(u, v, storage, rkp);

        uint32_t u_children_cnt = non_core_vertices_children_offset[i + 1] - non_core_vertices_children_offset[i];
        uint32_t* u_children = non_core_vertices_children + non_core_vertices_children_offset[i];

        for (uint32_t j = 0; j < u_children_cnt; ++j) {
            uint32_t w = u_children[j];

            
            uint32_t lkp;
            edge_relation* l_relation = get_key_position_in_relation(u, w, storage, lkp);
            storage->num_candidates_[u] = semi_join_operator->execute(l_relation, lkp, r_relation, rkp);
        }
    }

    delete semi_join_operator;
}

edge_relation *FilterVertices::get_key_position_in_relation(uint32_t u, uint32_t v, catalog *storage, uint32_t &kp) {
    kp = 0;

    if (u > v) {
        kp = 1;
        std::swap(u, v);
    }
    return &(storage->edge_relations_[u][v]);
}

bool
FilterVertices::CaLiGFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count) {
    if (!NLFFilter(data_graph, query_graph, candidates, candidates_count))
        return false;

    auto query_vertices_num = query_graph->getVerticesCount();

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    for (ui k = 0; k < 3; k++) {
        if (k%2 == 0) {
            for (VertexID u = 0; u < query_vertices_num; u++) {
                CaLiGPruneCandidate(data_graph,query_graph, u, candidates, candidates_count);
            }
        } else {
            for (VertexID u = query_vertices_num - 1; u != (ui)-1; u--) {
                CaLiGPruneCandidate(data_graph,query_graph, u, candidates, candidates_count);
            }
        }
    }
    
    
    
    
    
    
    
    
    return true;
}

void
FilterVertices::CaLiGPruneCandidate(const Graph *data_graph, const Graph *query_graph, VertexID u,
                                    ui** &candidates, ui* &candidates_count) {
    
    ui unbrs_num = 0, vnbrs_num = 0;
    const VertexID* unbrs = query_graph->getVertexNeighbors(u, unbrs_num);
    const VertexID* vnbrs;
    ui left_cnt = unbrs_num, right_cnt = 0;
    VertexID** adj = new VertexID*[unbrs_num];
    ui* adj_cnt = new ui[unbrs_num];
    for (ui i = 0; i < candidates_count[u]; i++) {
        
        VertexID v = candidates[u][i];
        bool f_del = false;
        vnbrs = data_graph->getVertexNeighbors(v, vnbrs_num);
        right_cnt = vnbrs_num;
        
        std::vector<std::vector<VertexID>> edge_info;
        edge_info.resize(unbrs_num);
        for (ui j = 0; j < unbrs_num; j++) {
            VertexID unbr = unbrs[j];
            for (ui k = 0; k < vnbrs_num; k++) {
                VertexID vnbr = vnbrs[k];
                if (std::find(candidates[unbr], candidates[unbr]+candidates_count[unbr], vnbr)
                        != candidates[unbr]+candidates_count[unbr]) {
                    edge_info[j].emplace_back(k+unbrs_num);
                }
            }
            
            if (edge_info[j].size() == 0) {
                
                
                
                
                
                
                
                
                
                
                
                f_del = true;
                break;
            }
        }

        if (f_del == false) {  
            
            
            for (ui j = 0; j < left_cnt; j++) {
                adj[j] = edge_info[j].data();
                adj_cnt[j] = edge_info[j].size();
                
            }
            
            
            
            
            
            
            
            
            
            HKmatch sample = HKmatch(left_cnt, right_cnt, adj, adj_cnt);
            ui max_match = sample.MaxMatch();
            if (max_match != left_cnt) {
                
                f_del = true;
            }
        }

        if (f_del == true) {
            candidates[u][i] = candidates[u][--candidates_count[u]];
            i--;
        }
    }
    delete[] adj;
    delete[] adj_cnt;
    
    
    
    return;
}
