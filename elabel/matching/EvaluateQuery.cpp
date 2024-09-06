#include "EvaluateQuery.h"
#include "utility/computesetintersection.h"
#include "utility/execution_tree/execution_tree_generator.h"
#include <vector>
#include <cstring>

#include "utility/pretty_print.h"

#if ENABLE_QFLITER == 1
BSRGraph ***EvaluateQuery::qfliter_bsr_graph_;
int *EvaluateQuery::temp_bsr_base1_ = nullptr;
int *EvaluateQuery::temp_bsr_state1_ = nullptr;
int *EvaluateQuery::temp_bsr_base2_ = nullptr;
int *EvaluateQuery::temp_bsr_state2_ = nullptr;
#endif

#ifdef SPECTRUM
bool EvaluateQuery::exit_;
#endif

#ifdef DISTRIBUTION
size_t* EvaluateQuery::distribution_count_;
#endif

std::function<bool(std::pair<std::pair<VertexID, ui>, ui>, std::pair<std::pair<VertexID, ui>, ui>)>
    EvaluateQuery::extendable_vertex_compare = [](std::pair<std::pair<VertexID, ui>, ui> l, std::pair<std::pair<VertexID, ui>, ui> r) {
    if (l.first.second == 1 && r.first.second != 1) {
        return true;
    }
    else if (l.first.second != 1 && r.first.second == 1) {
        return false;
    }
    else
    {
        return l.second > r.second;
    }
};

void EvaluateQuery::generateBN(const Graph *query_graph, ui *order, ui *pivot, ui **&bn, ui *&bn_count) {
    ui query_vertices_num = query_graph->getVerticesCount();
    bn_count = new ui[query_vertices_num];
    std::fill(bn_count, bn_count + query_vertices_num, 0);
    bn = new ui *[query_vertices_num];
    for (ui i = 0; i < query_vertices_num; ++i) {
        bn[i] = new ui[query_vertices_num];
    }

    std::vector<bool> visited_vertices(query_vertices_num, false);
    visited_vertices[order[0]] = true;
    for (ui i = 1; i < query_vertices_num; ++i) {
        VertexID vertex = order[i];

        ui nbrs_cnt;
        const ui *nbrs = query_graph->getVertexNeighbors(vertex, nbrs_cnt);
        for (ui j = 0; j < nbrs_cnt; ++j) {
            VertexID nbr = nbrs[j];

            if (visited_vertices[nbr] && nbr != pivot[i]) {
                bn[i][bn_count[i]++] = nbr;
            }
        }

        visited_vertices[vertex] = true;
    }
}


void EvaluateQuery::generateBN(const Graph *query_graph, ui *order, ui **&bn, ui *&bn_count) {
    ui query_vertices_num = query_graph->getVerticesCount();
    bn_count = new ui[query_vertices_num];
    std::fill(bn_count, bn_count + query_vertices_num, 0);
    bn = new ui *[query_vertices_num];
    for (ui i = 0; i < query_vertices_num; ++i) {
        bn[i] = new ui[query_vertices_num];
    }

    std::vector<bool> visited_vertices(query_vertices_num, false);
    visited_vertices[order[0]] = true;
    for (ui i = 1; i < query_vertices_num; ++i) {
        VertexID vertex = order[i];

        ui nbrs_cnt;
        const ui *nbrs = query_graph->getVertexNeighbors(vertex, nbrs_cnt);
        for (ui j = 0; j < nbrs_cnt; ++j) {
            VertexID nbr = nbrs[j];

            if (visited_vertices[nbr]) {
                bn[i][bn_count[i]++] = nbr;
            }
        }

        visited_vertices[vertex] = true;
    }
}

size_t
EvaluateQuery::exploreGraph(const Graph *data_graph, const Graph *query_graph, Edges ***edge_matrix, ui **candidates,
                            ui *candidates_count, ui *order, ui *pivot, size_t output_limit_num, size_t &call_count) {
    
    ui **bn;
    ui *bn_count;
    generateBN(query_graph, order, pivot, bn, bn_count);

    
    ui *idx;
    ui *idx_count;
    ui *embedding;
    ui *idx_embedding;
    ui *temp_buffer;
    ui **valid_candidate_idx;
    bool *visited_vertices;
    allocateBuffer(data_graph, query_graph, candidates_count, idx, idx_count, embedding, idx_embedding,
                   temp_buffer, valid_candidate_idx, visited_vertices);
    
    size_t embedding_cnt = 0;
    int cur_depth = 0;
    ui max_depth = query_graph->getVerticesCount();
    VertexID start_vertex = order[0];

    idx[cur_depth] = 0;
    idx_count[cur_depth] = candidates_count[start_vertex];

    for (ui i = 0; i < idx_count[cur_depth]; ++i) {
        valid_candidate_idx[cur_depth][i] = i;
    }

    while (true) {
        while (idx[cur_depth] < idx_count[cur_depth]) {
            ui valid_idx = valid_candidate_idx[cur_depth][idx[cur_depth]];
            VertexID u = order[cur_depth];
            VertexID v = candidates[u][valid_idx];

            embedding[u] = v;
            idx_embedding[u] = valid_idx;
            visited_vertices[v] = true;
            idx[cur_depth] += 1;

            if (cur_depth == max_depth - 1) {
                embedding_cnt += 1;
                visited_vertices[v] = false;
                if (embedding_cnt >= output_limit_num) {
                    goto EXIT;
                }
            } else {
                call_count += 1;
                cur_depth += 1;
                idx[cur_depth] = 0;
                generateValidCandidateIndex(data_graph, cur_depth, embedding, idx_embedding, idx_count,
                                            valid_candidate_idx, edge_matrix, visited_vertices, bn,
                                            bn_count, order, pivot, candidates, query_graph);
            }
        }

        
        cur_depth -= 1;
        if (cur_depth < 0)
            break;
        else
            visited_vertices[embedding[order[cur_depth]]] = false;
    }


    
    EXIT:
    releaseBuffer(max_depth, idx, idx_count, embedding, idx_embedding, temp_buffer, valid_candidate_idx,
                  visited_vertices,
                  bn, bn_count);

    return embedding_cnt;
}

void
EvaluateQuery::allocateBuffer(const Graph *data_graph, const Graph *query_graph, ui *candidates_count, ui *&idx,
                              ui *&idx_count, ui *&embedding, ui *&idx_embedding, ui *&temp_buffer,
                              ui **&valid_candidate_idx, bool *&visited_vertices) {
    ui query_vertices_num = query_graph->getVerticesCount();
    ui data_vertices_num = data_graph->getVerticesCount();
    ui max_candidates_num = candidates_count[0];

    for (ui i = 1; i < query_vertices_num; ++i) {
        VertexID cur_vertex = i;
        ui cur_candidate_num = candidates_count[cur_vertex];

        if (cur_candidate_num > max_candidates_num) {
            max_candidates_num = cur_candidate_num;
        }
    }

    idx = new ui[query_vertices_num];
    idx_count = new ui[query_vertices_num];
    embedding = new ui[query_vertices_num];
    idx_embedding = new ui[query_vertices_num];
    visited_vertices = new bool[data_vertices_num];
    temp_buffer = new ui[max_candidates_num];
    valid_candidate_idx = new ui *[query_vertices_num];
    for (ui i = 0; i < query_vertices_num; ++i) {
        valid_candidate_idx[i] = new ui[max_candidates_num];
    }

    std::fill(visited_vertices, visited_vertices + data_vertices_num, false);
}


void EvaluateQuery::generateValidCandidateIndex(const Graph *data_graph, ui depth, ui *embedding, ui *idx_embedding,
                                                ui *idx_count, ui **valid_candidate_index, Edges ***edge_matrix,
                                                bool *visited_vertices, ui **bn, ui *bn_cnt, ui *order, ui *pivot,
                                                ui **candidates, const Graph *query_graph) {
    VertexID u = order[depth];
    VertexID pivot_vertex = pivot[depth];
    ui idx_id = idx_embedding[pivot_vertex];
    Edges &edge = *edge_matrix[pivot_vertex][u];
    ui count = edge.offset_[idx_id + 1] - edge.offset_[idx_id];
    ui *candidate_idx = edge.edge_ + edge.offset_[idx_id];

    ui valid_candidate_index_count = 0;

    if (bn_cnt[depth] == 0) {
        for (ui i = 0; i < count; ++i) {
            ui temp_idx = candidate_idx[i];
            VertexID temp_v = candidates[u][temp_idx];

            if (!visited_vertices[temp_v])
                valid_candidate_index[depth][valid_candidate_index_count++] = temp_idx;
        }
    } else {
        for (ui i = 0; i < count; ++i) {
            ui temp_idx = candidate_idx[i];
            VertexID temp_v = candidates[u][temp_idx];

            if (!visited_vertices[temp_v]) {
                bool valid = true;

                for (ui j = 0; j < bn_cnt[depth]; ++j) {
                    VertexID u_bn = bn[depth][j];
                    VertexID u_bn_v = embedding[u_bn];
#ifdef ELABELED_GRAPH
                    LabelID elabel = query_graph->getEdgeLabel(u, u_bn, true);
                    if (!data_graph->checkEdgeExistence(temp_v, u_bn_v, elabel)) {
#else
                    if (!data_graph->checkEdgeExistence(temp_v, u_bn_v)) {
#endif
                        valid = false;
                        break;
                    }
                }

                if (valid)
                    valid_candidate_index[depth][valid_candidate_index_count++] = temp_idx;
            }
        }
    }

    idx_count[depth] = valid_candidate_index_count;
}

void EvaluateQuery::releaseBuffer(ui query_vertices_num, ui *idx, ui *idx_count, ui *embedding, ui *idx_embedding,
                                  ui *temp_buffer, ui **valid_candidate_idx, bool *visited_vertices, ui **bn,
                                  ui *bn_count) {
    delete[] idx;
    delete[] idx_count;
    delete[] embedding;
    delete[] idx_embedding;
    delete[] visited_vertices;
    delete[] bn_count;
    delete[] temp_buffer;
    for (ui i = 0; i < query_vertices_num; ++i) {
        delete[] valid_candidate_idx[i];
        delete[] bn[i];
    }

    delete[] valid_candidate_idx;
    delete[] bn;
}

size_t
EvaluateQuery::LFTJ(const Graph *data_graph, const Graph *query_graph, Edges ***edge_matrix, ui **candidates,
                    ui *candidates_count,
                    ui *order, size_t output_limit_num, size_t &call_count, size_t &valid_vtx_cnt) {

#ifdef DISTRIBUTION
    distribution_count_ = new size_t[data_graph->getVerticesCount()];
    memset(distribution_count_, 0, data_graph->getVerticesCount() * sizeof(size_t));
    size_t* begin_count = new size_t[query_graph->getVerticesCount()];
    memset(begin_count, 0, query_graph->getVerticesCount() * sizeof(size_t));
#endif

    
    ui **bn;
    ui *bn_count;
    generateBN(query_graph, order, bn, bn_count);

    
    ui *idx;
    ui *idx_count;
    ui *embedding;
    ui *idx_embedding;
    ui *temp_buffer;
    ui **valid_candidate_idx;
    bool *visited_vertices;
    allocateBuffer(data_graph, query_graph, candidates_count, idx, idx_count, embedding, idx_embedding,
                   temp_buffer, valid_candidate_idx, visited_vertices);

    size_t embedding_cnt = 0;
    int cur_depth = 0;
    ui max_depth = query_graph->getVerticesCount();
    VertexID start_vertex = order[0];

    idx[cur_depth] = 0;
    idx_count[cur_depth] = candidates_count[start_vertex];

    
    
    
    
    
    

    for (ui i = 0; i < idx_count[cur_depth]; ++i) {
        valid_candidate_idx[cur_depth][i] = i;
    }

#ifdef ENABLE_FAILING_SET
    std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> ancestors;
    computeAncestor(query_graph, bn, bn_count, order, ancestors);
    std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> vec_failing_set(max_depth);
    std::unordered_map<VertexID, VertexID> reverse_embedding;
    reverse_embedding.reserve(MAXIMUM_QUERY_GRAPH_SIZE * 2);
#endif

#ifdef SPECTRUM
    exit_ = false;
#endif

    while (true) {
        while (idx[cur_depth] < idx_count[cur_depth]) {
            ui valid_idx = valid_candidate_idx[cur_depth][idx[cur_depth]];
            VertexID u = order[cur_depth];
            VertexID v = candidates[u][valid_idx];

            if (visited_vertices[v]) {
                idx[cur_depth] += 1;
#ifdef ENABLE_FAILING_SET
                vec_failing_set[cur_depth] = ancestors[u];
                vec_failing_set[cur_depth] |= ancestors[reverse_embedding[v]];
                vec_failing_set[cur_depth - 1] |= vec_failing_set[cur_depth];
#endif
                continue;
            }

            embedding[u] = v;
            idx_embedding[u] = valid_idx;
            visited_vertices[v] = true;
            idx[cur_depth] += 1;

#ifdef DISTRIBUTION
            begin_count[cur_depth] = embedding_cnt;
            
#endif

#ifdef ENABLE_FAILING_SET
            reverse_embedding[v] = u;
#endif

            if (cur_depth == max_depth - 1) {
                
                
                
                embedding_cnt += 1;
                visited_vertices[v] = false;

#ifdef DISTRIBUTION
                distribution_count_[v] += 1;
#endif

#ifdef ENABLE_FAILING_SET
                reverse_embedding.erase(embedding[u]);
                vec_failing_set[cur_depth].set();
                vec_failing_set[cur_depth - 1] |= vec_failing_set[cur_depth];
#endif
                if (embedding_cnt >= output_limit_num) {
                    goto EXIT;
                }
            } else {
                call_count += 1;
                cur_depth += 1;

                idx[cur_depth] = 0;
                generateValidCandidateIndex(cur_depth, idx_embedding, idx_count, valid_candidate_idx, edge_matrix, bn,
                                            bn_count, order, temp_buffer);

#ifdef ENABLE_FAILING_SET
                if (idx_count[cur_depth] == 0) {
                    vec_failing_set[cur_depth - 1] = ancestors[order[cur_depth]];
                } else {
                    vec_failing_set[cur_depth - 1].reset();
                }
#endif
            }
        }

#ifdef SPECTRUM
        if (exit_) {
            goto EXIT;
        }
#endif

        cur_depth -= 1;
        if (cur_depth < 0)
            break;
        else {
            VertexID u = order[cur_depth];
#ifdef ENABLE_FAILING_SET
            reverse_embedding.erase(embedding[u]);
            if (cur_depth != 0) {
                if (!vec_failing_set[cur_depth].test(u)) {
                    vec_failing_set[cur_depth - 1] = vec_failing_set[cur_depth];
                    idx[cur_depth] = idx_count[cur_depth];
                } else {
                    vec_failing_set[cur_depth - 1] |= vec_failing_set[cur_depth];
                }
            }
#endif
            visited_vertices[embedding[u]] = false;

#ifdef DISTRIBUTION
            distribution_count_[embedding[u]] += embedding_cnt - begin_count[cur_depth];
            
#endif
        }
    }

    
    
    
    
    
    
    
    
    

    

#ifdef DISTRIBUTION
    if (embedding_cnt >= output_limit_num) {
        for (int i = 0; i < max_depth - 1; ++i) {
            ui v = embedding[order[i]];
            distribution_count_[v] += embedding_cnt - begin_count[i];
        }
    }
    delete[] begin_count;
#endif

    EXIT:
    releaseBuffer(max_depth, idx, idx_count, embedding, idx_embedding, temp_buffer, valid_candidate_idx,
                  visited_vertices,
                  bn, bn_count);

    
    
    
    
    

#if ENABLE_QFLITER == 1
    delete[] temp_bsr_base1_;
    delete[] temp_bsr_base2_;
    delete[] temp_bsr_state1_;
    delete[] temp_bsr_state2_;

    for (ui i = 0; i < max_depth; ++i) {
        for (ui j = 0; j < max_depth; ++j) {

        }
        delete[] qfliter_bsr_graph_[i];
    }
    delete[] qfliter_bsr_graph_;
#endif

    return embedding_cnt;
}

void EvaluateQuery::generateValidCandidateIndex(ui depth, ui *idx_embedding, ui *idx_count, ui **valid_candidate_index,
                                                Edges ***edge_matrix, ui **bn, ui *bn_cnt, ui *order,
                                                ui *&temp_buffer) {
    VertexID u = order[depth];
    VertexID previous_bn = bn[depth][0];
    ui previous_index_id = idx_embedding[previous_bn];
    ui valid_candidates_count = 0;

#if ENABLE_QFLITER == 1
    BSRGraph &bsr_graph = *qfliter_bsr_graph_[previous_bn][u];
    BSRSet &bsr_set = bsr_graph.bsrs[previous_index_id];

    if (bsr_set.size_ != 0){
        offline_bsr_trans_uint(bsr_set.base_, bsr_set.states_, bsr_set.size_,
                               (int *) valid_candidate_index[depth]);
        
        valid_candidates_count = bsr_set.size_;
    }

    if (bn_cnt[depth] > 0) {
        if (temp_bsr_base1_ == nullptr) { temp_bsr_base1_ = new int[1024 * 1024]; }
        if (temp_bsr_state1_ == nullptr) { temp_bsr_state1_ = new int[1024 * 1024]; }
        if (temp_bsr_base2_ == nullptr) { temp_bsr_base2_ = new int[1024 * 1024]; }
        if (temp_bsr_state2_ == nullptr) { temp_bsr_state2_ = new int[1024 * 1024]; }
        int *res_base_ = temp_bsr_base1_;
        int *res_state_ = temp_bsr_state1_;
        int *input_base_ = temp_bsr_base2_;
        int *input_state_ = temp_bsr_state2_;

        memcpy(input_base_, bsr_set.base_, sizeof(int) * bsr_set.size_);
        memcpy(input_state_, bsr_set.states_, sizeof(int) * bsr_set.size_);

        for (ui i = 1; i < bn_cnt[depth]; ++i) {
            VertexID current_bn = bn[depth][i];
            ui current_index_id = idx_embedding[current_bn];
            BSRGraph &cur_bsr_graph = *qfliter_bsr_graph_[current_bn][u];
            BSRSet &cur_bsr_set = cur_bsr_graph.bsrs[current_index_id];

            if (valid_candidates_count == 0 || cur_bsr_set.size_ == 0) {
                valid_candidates_count = 0;
                break;
            }

            valid_candidates_count = intersect_qfilter_bsr_b4_v2(cur_bsr_set.base_, cur_bsr_set.states_,
                                                                 cur_bsr_set.size_,
                                                                 input_base_, input_state_, valid_candidates_count,
                                                                 res_base_, res_state_);

            swap(res_base_, input_base_);
            swap(res_state_, input_state_);
        }

        if (valid_candidates_count != 0) {
            valid_candidates_count = offline_bsr_trans_uint(input_base_, input_state_, valid_candidates_count,
                                                            (int *) valid_candidate_index[depth]);
        }
    }

    idx_count[depth] = valid_candidates_count;

    
#ifdef YCHE_DEBUG
    Edges &previous_edge = *edge_matrix[previous_bn][u];

    auto gt_valid_candidates_count =
            previous_edge.offset_[previous_index_id + 1] - previous_edge.offset_[previous_index_id];
    ui *previous_candidates = previous_edge.edge_ + previous_edge.offset_[previous_index_id];
    ui *gt_valid_candidate_index = new ui[1024 * 1024];
    memcpy(gt_valid_candidate_index, previous_candidates, gt_valid_candidates_count * sizeof(ui));

    ui temp_count;
    for (ui i = 1; i < bn_cnt[depth]; ++i) {
        VertexID current_bn = bn[depth][i];
        Edges &current_edge = *edge_matrix[current_bn][u];
        ui current_index_id = idx_embedding[current_bn];

        ui current_candidates_count =
                current_edge.offset_[current_index_id + 1] - current_edge.offset_[current_index_id];
        ui *current_candidates = current_edge.edge_ + current_edge.offset_[current_index_id];

        ComputeSetIntersection::ComputeCandidates(current_candidates, current_candidates_count,
                                                  gt_valid_candidate_index, gt_valid_candidates_count, temp_buffer,
                                                  temp_count);

        std::swap(temp_buffer, gt_valid_candidate_index);
        gt_valid_candidates_count = temp_count;
    }
    assert(valid_candidates_count == gt_valid_candidates_count);

    cout << "Ret, Level:" << bn_cnt[depth] << ", BSR:"
         << pretty_print_array(valid_candidate_index[depth], valid_candidates_count)
         << "; GT: " << pretty_print_array(gt_valid_candidate_index, gt_valid_candidates_count) << "\n";

    for (auto i = 0; i < valid_candidates_count; i++) {
        assert(gt_valid_candidate_index[i] == valid_candidate_index[depth][i]);
    }
    delete[] gt_valid_candidate_index;
#endif
#else
    Edges& previous_edge = *edge_matrix[previous_bn][u];

    valid_candidates_count = previous_edge.offset_[previous_index_id + 1] - previous_edge.offset_[previous_index_id];
    ui* previous_candidates = previous_edge.edge_ + previous_edge.offset_[previous_index_id];

    memcpy(valid_candidate_index[depth], previous_candidates, valid_candidates_count * sizeof(ui));

    ui temp_count;
    for (ui i = 1; i < bn_cnt[depth]; ++i) {
        VertexID current_bn = bn[depth][i];
        Edges& current_edge = *edge_matrix[current_bn][u];
        ui current_index_id = idx_embedding[current_bn];

        ui current_candidates_count = current_edge.offset_[current_index_id + 1] - current_edge.offset_[current_index_id];
        ui* current_candidates = current_edge.edge_ + current_edge.offset_[current_index_id];

        ComputeSetIntersection::ComputeCandidates(current_candidates, current_candidates_count, valid_candidate_index[depth], valid_candidates_count,
                        temp_buffer, temp_count);

        std::swap(temp_buffer, valid_candidate_index[depth]);
        valid_candidates_count = temp_count;
    }

    idx_count[depth] = valid_candidates_count;
#endif
}

size_t EvaluateQuery::exploreGraphQLStyle(const Graph *data_graph, const Graph *query_graph, ui **candidates,
                                          ui *candidates_count, ui *order,
                                          size_t output_limit_num, size_t &call_count) {
    size_t embedding_cnt = 0;
    int cur_depth = 0;
    ui max_depth = query_graph->getVerticesCount();
    VertexID start_vertex = order[0];

    
    ui **bn;
    ui *bn_count;

    bn = new ui *[max_depth];
    for (ui i = 0; i < max_depth; ++i) {
        bn[i] = new ui[max_depth];
    }

    bn_count = new ui[max_depth];
    std::fill(bn_count, bn_count + max_depth, 0);

    std::vector<bool> visited_query_vertices(max_depth, false);
    visited_query_vertices[start_vertex] = true;
    for (ui i = 1; i < max_depth; ++i) {
        VertexID cur_vertex = order[i];
        ui nbr_cnt;
        const VertexID *nbrs = query_graph->getVertexNeighbors(cur_vertex, nbr_cnt);

        for (ui j = 0; j < nbr_cnt; ++j) {
            VertexID nbr = nbrs[j];

            if (visited_query_vertices[nbr]) {
                bn[i][bn_count[i]++] = nbr;
            }
        }

        visited_query_vertices[cur_vertex] = true;
    }

    
    ui *idx;
    ui *idx_count;
    ui *embedding;
    VertexID **valid_candidate;
    bool *visited_vertices;

    idx = new ui[max_depth];
    idx_count = new ui[max_depth];
    embedding = new ui[max_depth];
    visited_vertices = new bool[data_graph->getVerticesCount()];
    std::fill(visited_vertices, visited_vertices + data_graph->getVerticesCount(), false);
    valid_candidate = new ui *[max_depth];

    for (ui i = 0; i < max_depth; ++i) {
        VertexID cur_vertex = order[i];
        ui max_candidate_count = candidates_count[cur_vertex];
        valid_candidate[i] = new VertexID[max_candidate_count];
    }

    idx[cur_depth] = 0;
    idx_count[cur_depth] = candidates_count[start_vertex];
    std::copy(candidates[start_vertex], candidates[start_vertex] + candidates_count[start_vertex],
              valid_candidate[cur_depth]);

    while (true) {
        while (idx[cur_depth] < idx_count[cur_depth]) {
            VertexID u = order[cur_depth];
            VertexID v = valid_candidate[cur_depth][idx[cur_depth]];
            embedding[u] = v;
            visited_vertices[v] = true;
            idx[cur_depth] += 1;

            if (cur_depth == max_depth - 1) {
                embedding_cnt += 1;
                visited_vertices[v] = false;
                if (embedding_cnt >= output_limit_num) {
                    goto EXIT;
                }
            } else {
                call_count += 1;
                cur_depth += 1;
                idx[cur_depth] = 0;
                generateValidCandidates(data_graph, cur_depth, embedding, idx_count, valid_candidate,
                                        visited_vertices, bn, bn_count, order, candidates, candidates_count,
                                        query_graph);
            }
        }

        cur_depth -= 1;
        if (cur_depth < 0)
            break;
        else
            visited_vertices[embedding[order[cur_depth]]] = false;
    }

    
    EXIT:
    delete[] bn_count;
    delete[] idx;
    delete[] idx_count;
    delete[] embedding;
    delete[] visited_vertices;
    for (ui i = 0; i < max_depth; ++i) {
        delete[] bn[i];
        delete[] valid_candidate[i];
    }

    delete[] bn;
    delete[] valid_candidate;

    return embedding_cnt;
}

void EvaluateQuery::generateValidCandidates(const Graph *data_graph, ui depth, ui *embedding, ui *idx_count,
                                            ui **valid_candidate, bool *visited_vertices, ui **bn, ui *bn_cnt,
                                            ui *order, ui **candidates, ui *candidates_count,
                                            const Graph *query_graph) {
    VertexID u = order[depth];

    idx_count[depth] = 0;

    for (ui i = 0; i < candidates_count[u]; ++i) {
        VertexID v = candidates[u][i];

        if (!visited_vertices[v]) {
            bool valid = true;

            for (ui j = 0; j < bn_cnt[depth]; ++j) {
                VertexID u_bn = bn[depth][j];
                VertexID u_bn_v = embedding[u_bn];
#ifdef ELABELED_GRAPH
                LabelID elabel = query_graph->getEdgeLabel(u, u_bn, true);
                if (!data_graph->checkEdgeExistence(v, u_bn_v, elabel)) {
#else
                if (!data_graph->checkEdgeExistence(v, u_bn_v)) {
#endif
                    valid = false;
                    break;
                }
            }

            if (valid) {
                valid_candidate[depth][idx_count[depth]++] = v;
            }
        }
    }
}

size_t EvaluateQuery::exploreQuickSIStyle(const Graph *data_graph, const Graph *query_graph, ui **candidates,
                                          ui *candidates_count, ui *order,
                                          ui *pivot, size_t output_limit_num, size_t &call_count) {
    size_t embedding_cnt = 0;
    int cur_depth = 0;
    ui max_depth = query_graph->getVerticesCount();
    VertexID start_vertex = order[0];

    
    ui **bn;
    ui *bn_count;
    generateBN(query_graph, order, pivot, bn, bn_count);

    
    ui *idx;
    ui *idx_count;
    ui *embedding;
    VertexID **valid_candidate;
    bool *visited_vertices;

    idx = new ui[max_depth];
    idx_count = new ui[max_depth];
    embedding = new ui[max_depth];
    visited_vertices = new bool[data_graph->getVerticesCount()];
    std::fill(visited_vertices, visited_vertices + data_graph->getVerticesCount(), false);
    valid_candidate = new ui *[max_depth];

    ui max_candidate_count = data_graph->getGraphMaxLabelFrequency();
    for (ui i = 0; i < max_depth; ++i) {
        valid_candidate[i] = new VertexID[max_candidate_count];
    }

    idx[cur_depth] = 0;
    idx_count[cur_depth] = candidates_count[start_vertex];
    std::copy(candidates[start_vertex], candidates[start_vertex] + candidates_count[start_vertex],
              valid_candidate[cur_depth]);

    while (true) {
        while (idx[cur_depth] < idx_count[cur_depth]) {
            VertexID u = order[cur_depth];
            VertexID v = valid_candidate[cur_depth][idx[cur_depth]];
            embedding[u] = v;
            visited_vertices[v] = true;
            idx[cur_depth] += 1;

            if (cur_depth == max_depth - 1) {
                embedding_cnt += 1;
                visited_vertices[v] = false;
                if (embedding_cnt >= output_limit_num) {
                    goto EXIT;
                }
            } else {
                call_count += 1;
                cur_depth += 1;
                idx[cur_depth] = 0;
                generateValidCandidates(query_graph, data_graph, cur_depth, embedding, idx_count, valid_candidate,
                                        visited_vertices, bn, bn_count, order, pivot);
            }
        }

        cur_depth -= 1;
        if (cur_depth < 0)
            break;
        else
            visited_vertices[embedding[order[cur_depth]]] = false;
    }

    
    EXIT:
    delete[] bn_count;
    delete[] idx;
    delete[] idx_count;
    delete[] embedding;
    delete[] visited_vertices;
    for (ui i = 0; i < max_depth; ++i) {
        delete[] bn[i];
        delete[] valid_candidate[i];
    }

    delete[] bn;
    delete[] valid_candidate;

    return embedding_cnt;
}


void EvaluateQuery::generateValidCandidates(const Graph *query_graph, const Graph *data_graph, ui depth, ui *embedding,
                                            ui *idx_count, ui **valid_candidate, bool *visited_vertices, ui **bn,
                                            ui *bn_cnt, ui *order, ui *pivot) {
    VertexID u = order[depth];
    LabelID u_label = query_graph->getVertexLabel(u);
    ui u_degree = query_graph->getVertexDegree(u);

    idx_count[depth] = 0;

    VertexID p = embedding[pivot[depth]];
    ui nbr_cnt;
    auto nbrs = data_graph->getVertexNeighbors(p, nbr_cnt);
#ifdef ELABELED_GRAPH
    auto uqlabel = query_graph->getEdgeLabel(u, pivot[depth], true);
#endif

    for (ui i = 0; i < nbr_cnt; ++i) {
        VertexID v = nbrs[i];
#ifdef ELABELED_GRAPH
        if (data_graph->getEdgeLabel(p, nbrs[i], true) != uqlabel) continue;
#endif
        if (!visited_vertices[v] && u_label == data_graph->getVertexLabel(v) &&
            u_degree <= data_graph->getVertexDegree(v)) {
            bool valid = true;

            for (ui j = 0; j < bn_cnt[depth]; ++j) {
                VertexID u_bn = bn[depth][j];
                VertexID u_bn_v = embedding[u_bn];
#ifdef ELABELED_GRAPH
                LabelID elabel = query_graph->getEdgeLabel(u, u_bn, true);
                if (!data_graph->checkEdgeExistence(v, u_bn_v, elabel)) {
#else
                if (!data_graph->checkEdgeExistence(v, u_bn_v)) {
#endif
                    valid = false;
                    break;
                }
            }

            if (valid) {
                valid_candidate[depth][idx_count[depth]++] = v;
            }
        }
    }
}

size_t EvaluateQuery::exploreDPisoStyle(const Graph *data_graph, const Graph *query_graph, TreeNode *tree,
                                        Edges ***edge_matrix, ui **candidates, ui *candidates_count,
                                        ui **weight_array, ui *order, size_t output_limit_num,
                                        size_t &call_count) {
    ui max_depth = query_graph->getVerticesCount();

    ui *extendable = new ui[max_depth];
    for (ui i = 0; i < max_depth; ++i) {
        extendable[i] = tree[i].bn_count_;
    }

    
    ui **bn;
    ui *bn_count;
    generateBN(query_graph, order, bn, bn_count);

    
    ui *idx;
    ui *idx_count;
    ui *embedding;
    ui *idx_embedding;
    ui *temp_buffer;
    ui **valid_candidate_idx;
    bool *visited_vertices;
    allocateBuffer(data_graph, query_graph, candidates_count, idx, idx_count, embedding, idx_embedding,
                   temp_buffer, valid_candidate_idx, visited_vertices);

    
    size_t embedding_cnt = 0;
    int cur_depth = 0;

#ifdef ENABLE_FAILING_SET
    std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> ancestors;
    computeAncestor(query_graph, tree, order, ancestors);
    std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> vec_failing_set(max_depth);
    std::unordered_map<VertexID, VertexID> reverse_embedding;  
    reverse_embedding.reserve(MAXIMUM_QUERY_GRAPH_SIZE * 2);
#endif

    VertexID start_vertex = order[0];
    std::vector<dpiso_min_pq> vec_rank_queue;

    for (ui i = 0; i < candidates_count[start_vertex]; ++i) {
        VertexID v = candidates[start_vertex][i];
        embedding[start_vertex] = v;
        idx_embedding[start_vertex] = i;
        visited_vertices[v] = true;

#ifdef ENABLE_FAILING_SET
        reverse_embedding[v] = start_vertex;
#endif
        vec_rank_queue.emplace_back(dpiso_min_pq(extendable_vertex_compare));
        updateExtendableVertex(idx_embedding, idx_count, valid_candidate_idx, edge_matrix, temp_buffer, weight_array,
                               tree, start_vertex, extendable,
                               vec_rank_queue, query_graph);

        VertexID u = vec_rank_queue.back().top().first.first;
        vec_rank_queue.back().pop();

#ifdef ENABLE_FAILING_SET
        if (idx_count[u] == 0) {
            vec_failing_set[cur_depth] = ancestors[u];
        } else {
            vec_failing_set[cur_depth].reset();
        }
#endif

        call_count += 1;
        cur_depth += 1;
        order[cur_depth] = u;
        idx[u] = 0;
        while (cur_depth > 0) { 
            while (idx[u] < idx_count[u]) { 
                ui valid_idx = valid_candidate_idx[u][idx[u]];
                v = candidates[u][valid_idx];

                if (visited_vertices[v]) {
                    idx[u] += 1;
#ifdef ENABLE_FAILING_SET 
                    vec_failing_set[cur_depth] = ancestors[u];
                    vec_failing_set[cur_depth] |= ancestors[reverse_embedding[v]];
                    
                    vec_failing_set[cur_depth - 1] |= vec_failing_set[cur_depth];
#endif
                    continue;
                }
                embedding[u] = v;
                idx_embedding[u] = valid_idx;
                visited_vertices[v] = true;
                idx[u] += 1;

#ifdef ENABLE_FAILING_SET
                reverse_embedding[v] = u;
#endif

                if (cur_depth == max_depth - 1) {
                    embedding_cnt += 1;
                    visited_vertices[v] = false;
#ifdef ENABLE_FAILING_SET 
                    reverse_embedding.erase(embedding[u]);
                    vec_failing_set[cur_depth].set();
                    vec_failing_set[cur_depth - 1] |= vec_failing_set[cur_depth];

#endif
                    if (embedding_cnt >= output_limit_num) {
                        goto EXIT;
                    }
                } else { 
                    call_count += 1;
                    cur_depth += 1;
                    vec_rank_queue.emplace_back(vec_rank_queue.back());
                    updateExtendableVertex(idx_embedding, idx_count, valid_candidate_idx, edge_matrix, temp_buffer,
                                           weight_array, tree, u, extendable,
                                           vec_rank_queue, query_graph);

                    u = vec_rank_queue.back().top().first.first;
                    vec_rank_queue.back().pop();
                    idx[u] = 0;
                    order[cur_depth] = u;

#ifdef ENABLE_FAILING_SET
                    if (idx_count[u] == 0) { 
                        
                        vec_failing_set[cur_depth - 1] = ancestors[u];
                    } else { 
                        vec_failing_set[cur_depth - 1].reset();
                    }
#endif
                }
            }

            
            cur_depth -= 1;
            vec_rank_queue.pop_back();
            u = order[cur_depth];
            visited_vertices[embedding[u]] = false;
            restoreExtendableVertex(tree, u, extendable);
#ifdef ENABLE_FAILING_SET
            reverse_embedding.erase(embedding[u]);
            if (cur_depth != 0) {
                if (!vec_failing_set[cur_depth].test(u)) {
                    vec_failing_set[cur_depth - 1] = vec_failing_set[cur_depth];
                    
                    
                    
                    idx[u] = idx_count[u];
                } else {
                    vec_failing_set[cur_depth - 1] |= vec_failing_set[cur_depth];
                }
            }
#endif
        }
    }

    
    EXIT:
    releaseBuffer(max_depth, idx, idx_count, embedding, idx_embedding, temp_buffer, valid_candidate_idx,
                  visited_vertices,
                  bn, bn_count);

    return embedding_cnt;
}

void EvaluateQuery::updateExtendableVertex(ui *idx_embedding, ui *idx_count, ui **valid_candidate_index,
                                           Edges ***edge_matrix, ui *&temp_buffer, ui **weight_array,
                                           TreeNode *tree, VertexID mapped_vertex, ui *extendable,
                                           std::vector<dpiso_min_pq> &vec_rank_queue, const Graph *query_graph) {
    TreeNode &node = tree[mapped_vertex];
    for (ui i = 0; i < node.fn_count_; ++i) {
        VertexID u = node.fn_[i];
        extendable[u] -= 1;
        if (extendable[u] == 0) {
            generateValidCandidateIndex(u, idx_embedding, idx_count, valid_candidate_index[u], edge_matrix, tree[u].bn_,
                                        tree[u].bn_count_, temp_buffer);

            ui weight = 0;
            for (ui j = 0; j < idx_count[u]; ++j) {
                ui idx = valid_candidate_index[u][j];
                weight += weight_array[u][idx];
            }
            vec_rank_queue.back().emplace(std::make_pair(std::make_pair(u, query_graph->getVertexDegree(u)), weight));
        }
    }
}

void EvaluateQuery::restoreExtendableVertex(TreeNode *tree, VertexID unmapped_vertex, ui *extendable) {
    TreeNode &node = tree[unmapped_vertex];
    for (ui i = 0; i < node.fn_count_; ++i) {
        VertexID u = node.fn_[i];
        extendable[u] += 1;
    }
}

void
EvaluateQuery::generateValidCandidateIndex(VertexID u, ui *idx_embedding, ui *idx_count, ui *&valid_candidate_index,
                                           Edges ***edge_matrix, ui *bn, ui bn_cnt, ui *&temp_buffer) {
    VertexID previous_bn = bn[0];
    Edges &previous_edge = *edge_matrix[previous_bn][u];
    ui previous_index_id = idx_embedding[previous_bn];

    ui previous_candidates_count =
            previous_edge.offset_[previous_index_id + 1] - previous_edge.offset_[previous_index_id];
    ui *previous_candidates = previous_edge.edge_ + previous_edge.offset_[previous_index_id];

    ui valid_candidates_count = 0;
    for (ui i = 0; i < previous_candidates_count; ++i) {
        valid_candidate_index[valid_candidates_count++] = previous_candidates[i];
    }

    ui temp_count;
    for (ui i = 1; i < bn_cnt; ++i) {
        VertexID current_bn = bn[i];
        Edges &current_edge = *edge_matrix[current_bn][u];
        ui current_index_id = idx_embedding[current_bn];

        ui current_candidates_count =
                current_edge.offset_[current_index_id + 1] - current_edge.offset_[current_index_id];
        ui *current_candidates = current_edge.edge_ + current_edge.offset_[current_index_id];

        ComputeSetIntersection::ComputeCandidates(current_candidates, current_candidates_count, valid_candidate_index,
                                                  valid_candidates_count,
                                                  temp_buffer, temp_count);

        std::swap(temp_buffer, valid_candidate_index);
        valid_candidates_count = temp_count;
    }

    idx_count[u] = valid_candidates_count;
}

void EvaluateQuery::computeAncestor(const Graph *query_graph, TreeNode *tree, VertexID *order,
                                    std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> &ancestors) {
    ui query_vertices_num = query_graph->getVerticesCount();
    ancestors.resize(query_vertices_num);

    
    for (ui i = 0; i < query_vertices_num; ++i) {
        VertexID u = order[i];
        TreeNode &u_node = tree[u];
        ancestors[u].set(u);
        for (ui j = 0; j < u_node.bn_count_; ++j) {
            VertexID u_bn = u_node.bn_[j];
            ancestors[u] |= ancestors[u_bn];
        }
    }
}

size_t EvaluateQuery::exploreDPisoRecursiveStyle(const Graph *data_graph, const Graph *query_graph, TreeNode *tree,
                                                 Edges ***edge_matrix, ui **candidates, ui *candidates_count,
                                                 ui **weight_array, ui *order, size_t output_limit_num,
                                                 size_t &call_count) {
    ui max_depth = query_graph->getVerticesCount();

    ui *extendable = new ui[max_depth];
    for (ui i = 0; i < max_depth; ++i) {
        extendable[i] = tree[i].bn_count_;
    }

    
    ui **bn;
    ui *bn_count;
    generateBN(query_graph, order, bn, bn_count);

    
    ui *idx;
    ui *idx_count;
    ui *embedding;
    ui *idx_embedding;
    ui *temp_buffer;
    ui **valid_candidate_idx;
    bool *visited_vertices;
    allocateBuffer(data_graph, query_graph, candidates_count, idx, idx_count, embedding, idx_embedding,
                   temp_buffer, valid_candidate_idx, visited_vertices);

    
    size_t embedding_cnt = 0;

    std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> ancestors;
    computeAncestor(query_graph, tree, order, ancestors);

    std::unordered_map<VertexID, VertexID> reverse_embedding;
    reverse_embedding.reserve(MAXIMUM_QUERY_GRAPH_SIZE * 2);
    VertexID start_vertex = order[0];

    for (ui i = 0; i < candidates_count[start_vertex]; ++i) {
        VertexID v = candidates[start_vertex][i];
        embedding[start_vertex] = v;
        idx_embedding[start_vertex] = i;
        visited_vertices[v] = true;
        reverse_embedding[v] = start_vertex;
        call_count += 1;

        exploreDPisoBacktrack(max_depth, 1, start_vertex, tree, idx_embedding, embedding, reverse_embedding,
                              visited_vertices, idx_count, valid_candidate_idx, edge_matrix,
                              ancestors, dpiso_min_pq(extendable_vertex_compare), weight_array, temp_buffer, extendable,
                              candidates, embedding_cnt,
                              call_count, nullptr);

        visited_vertices[v] = false;
        reverse_embedding.erase(v);
    }

    
    releaseBuffer(max_depth, idx, idx_count, embedding, idx_embedding, temp_buffer, valid_candidate_idx,
                  visited_vertices,
                  bn, bn_count);

    return embedding_cnt;
}

std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>
EvaluateQuery::exploreDPisoBacktrack(ui max_depth, ui depth, VertexID mapped_vertex, TreeNode *tree, ui *idx_embedding,
                                     ui *embedding, std::unordered_map<VertexID, VertexID> &reverse_embedding,
                                     bool *visited_vertices, ui *idx_count, ui **valid_candidate_index,
                                     Edges ***edge_matrix,
                                     std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> &ancestors,
                                     dpiso_min_pq rank_queue, ui **weight_array, ui *&temp_buffer, ui *extendable,
                                     ui **candidates, size_t &embedding_count, size_t &call_count,
                                     const Graph *query_graph) {
    
    TreeNode &node = tree[mapped_vertex];
    for (ui i = 0; i < node.fn_count_; ++i) {
        VertexID u = node.fn_[i];
        extendable[u] -= 1;
        if (extendable[u] == 0) {
            generateValidCandidateIndex(u, idx_embedding, idx_count, valid_candidate_index[u], edge_matrix, tree[u].bn_,
                                        tree[u].bn_count_, temp_buffer);

            ui weight = 0;
            for (ui j = 0; j < idx_count[u]; ++j) {
                ui idx = valid_candidate_index[u][j];
                weight += weight_array[u][idx];
            }
            rank_queue.emplace(std::make_pair(std::make_pair(u, query_graph->getVertexDegree(u)), weight));
        }
    }

    VertexID u = rank_queue.top().first.first;
    rank_queue.pop();

    if (idx_count[u] == 0) {
        restoreExtendableVertex(tree, mapped_vertex, extendable);
        return ancestors[u];
    } else {
        std::bitset<MAXIMUM_QUERY_GRAPH_SIZE> current_fs;
        std::bitset<MAXIMUM_QUERY_GRAPH_SIZE> child_fs;

        for (ui i = 0; i < idx_count[u]; ++i) {
            ui valid_index = valid_candidate_index[u][i];
            VertexID v = candidates[u][valid_index];

            if (!visited_vertices[v]) {
                embedding[u] = v;
                idx_embedding[u] = valid_index;
                visited_vertices[v] = true;
                reverse_embedding[v] = u;
                if (depth != max_depth - 1) {
                    call_count += 1;
                    child_fs = exploreDPisoBacktrack(max_depth, depth + 1, u, tree, idx_embedding, embedding,
                                                     reverse_embedding, visited_vertices, idx_count,
                                                     valid_candidate_index, edge_matrix,
                                                     ancestors, rank_queue, weight_array, temp_buffer, extendable,
                                                     candidates, embedding_count,
                                                     call_count, query_graph);
                } else {
                    embedding_count += 1;
                    child_fs.set();
                }
                visited_vertices[v] = false;
                reverse_embedding.erase(v);

                if (!child_fs.test(u)) {
                    current_fs = child_fs;
                    break;
                }
            } else {
                child_fs.reset();
                child_fs |= ancestors[u];
                child_fs |= ancestors[reverse_embedding[v]];
            }

            current_fs |= child_fs;
        }

        restoreExtendableVertex(tree, mapped_vertex, extendable);
        return current_fs;
    }
}

size_t
EvaluateQuery::exploreCECIStyle(const Graph *data_graph, const Graph *query_graph, TreeNode *tree, ui **candidates,
                                ui *candidates_count,
                                std::vector<std::unordered_map<VertexID, std::vector<VertexID>>> &TE_Candidates,
                                std::vector<std::vector<std::unordered_map<VertexID, std::vector<VertexID>>>> &NTE_Candidates,
                                ui *order, size_t &output_limit_num, size_t &call_count) {

    ui max_depth = query_graph->getVerticesCount();
    ui data_vertices_count = data_graph->getVerticesCount();
    ui max_valid_candidates_count = 0;
    for (ui i = 0; i < max_depth; ++i) {
        if (candidates_count[i] > max_valid_candidates_count) {
            max_valid_candidates_count = candidates_count[i];
        }
    }
    
    ui *idx = new ui[max_depth];
    ui *idx_count = new ui[max_depth];
    ui *embedding = new ui[max_depth];
    ui *temp_buffer = new ui[max_valid_candidates_count];
    ui **valid_candidates = new ui *[max_depth];
    for (ui i = 0; i < max_depth; ++i) {
        valid_candidates[i] = new ui[max_valid_candidates_count];
    }
    bool *visited_vertices = new bool[data_vertices_count];
    std::fill(visited_vertices, visited_vertices + data_vertices_count, false);

    
    size_t embedding_cnt = 0;
    int cur_depth = 0;
    VertexID start_vertex = order[0];

    idx[cur_depth] = 0;
    idx_count[cur_depth] = candidates_count[start_vertex];

    for (ui i = 0; i < idx_count[cur_depth]; ++i) {
        valid_candidates[cur_depth][i] = candidates[start_vertex][i];
    }

#ifdef ENABLE_FAILING_SET
    std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> ancestors;
    computeAncestor(query_graph, order, ancestors);
    std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> vec_failing_set(max_depth);
    std::unordered_map<VertexID, VertexID> reverse_embedding;
    reverse_embedding.reserve(MAXIMUM_QUERY_GRAPH_SIZE * 2);
#endif

    while (true) {
        while (idx[cur_depth] < idx_count[cur_depth]) {
            VertexID u = order[cur_depth];
            VertexID v = valid_candidates[cur_depth][idx[cur_depth]];
            idx[cur_depth] += 1;

            if (visited_vertices[v]) {
#ifdef ENABLE_FAILING_SET
                vec_failing_set[cur_depth] = ancestors[u];
                vec_failing_set[cur_depth] |= ancestors[reverse_embedding[v]];
                vec_failing_set[cur_depth - 1] |= vec_failing_set[cur_depth];
#endif
                continue;
            }

            embedding[u] = v;
            visited_vertices[v] = true;

#ifdef ENABLE_FAILING_SET
            reverse_embedding[v] = u;
#endif
            if (cur_depth == max_depth - 1) {
                embedding_cnt += 1;
                visited_vertices[v] = false;
#ifdef ENABLE_FAILING_SET
                reverse_embedding.erase(embedding[u]);
                vec_failing_set[cur_depth].set();
                vec_failing_set[cur_depth - 1] |= vec_failing_set[cur_depth];
#endif
                if (embedding_cnt >= output_limit_num) {
                    goto EXIT;
                }
            } else {
                call_count += 1;
                cur_depth += 1;
                idx[cur_depth] = 0;
                generateValidCandidates(cur_depth, embedding, idx_count, valid_candidates, order, temp_buffer, tree,
                                        TE_Candidates,
                                        NTE_Candidates);
#ifdef ENABLE_FAILING_SET
                if (idx_count[cur_depth] == 0) {
                    vec_failing_set[cur_depth - 1] = ancestors[order[cur_depth]];
                } else {
                    vec_failing_set[cur_depth - 1].reset();
                }
#endif
            }
        }

        cur_depth -= 1;
        if (cur_depth < 0)
            break;
        else {
            VertexID u = order[cur_depth];
#ifdef ENABLE_FAILING_SET
            reverse_embedding.erase(embedding[u]);
            if (cur_depth != 0) {
                if (!vec_failing_set[cur_depth].test(u)) {
                    vec_failing_set[cur_depth - 1] = vec_failing_set[cur_depth];
                    idx[cur_depth] = idx_count[cur_depth];
                } else {
                    vec_failing_set[cur_depth - 1] |= vec_failing_set[cur_depth];
                }
            }
#endif
            visited_vertices[embedding[u]] = false;
        }
    }

    
    EXIT:
    delete[] idx;
    delete[] idx_count;
    delete[] embedding;
    delete[] temp_buffer;
    delete[] visited_vertices;
    for (ui i = 0; i < max_depth; ++i) {
        delete[] valid_candidates[i];
    }
    delete[] valid_candidates;

    return embedding_cnt;
}

void EvaluateQuery::generateValidCandidates(ui depth, ui *embedding, ui *idx_count, ui **valid_candidates, ui *order,
                                            ui *&temp_buffer, TreeNode *tree,
                                            std::vector<std::unordered_map<VertexID, std::vector<VertexID>>> &TE_Candidates,
                                            std::vector<std::vector<std::unordered_map<VertexID, std::vector<VertexID>>>> &NTE_Candidates) {

    VertexID u = order[depth];
    idx_count[depth] = 0;
    ui valid_candidates_count = 0;
    {
        VertexID u_p = tree[u].parent_;
        VertexID v_p = embedding[u_p];

        auto iter = TE_Candidates[u].find(v_p);
        if (iter == TE_Candidates[u].end() || iter->second.empty()) {
            return;
        }

        valid_candidates_count = iter->second.size();
        VertexID *v_p_nbrs = iter->second.data();

        for (ui i = 0; i < valid_candidates_count; ++i) {
            valid_candidates[depth][i] = v_p_nbrs[i];
        }
    }
    ui temp_count;
    for (ui i = 0; i < tree[u].bn_count_; ++i) {
        VertexID u_p = tree[u].bn_[i];
        VertexID v_p = embedding[u_p];

        auto iter = NTE_Candidates[u][u_p].find(v_p);
        if (iter == NTE_Candidates[u][u_p].end() || iter->second.empty()) {
            return;
        }

        ui current_candidates_count = iter->second.size();
        ui *current_candidates = iter->second.data();

        ComputeSetIntersection::ComputeCandidates(current_candidates, current_candidates_count,
                                                  valid_candidates[depth], valid_candidates_count,
                                                  temp_buffer, temp_count);

        std::swap(temp_buffer, valid_candidates[depth]);
        valid_candidates_count = temp_count;
    }

    idx_count[depth] = valid_candidates_count;
}

void EvaluateQuery::computeAncestor(const Graph *query_graph, ui **bn, ui *bn_cnt, VertexID *order,
                                    std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> &ancestors) {
    ui query_vertices_num = query_graph->getVerticesCount();
    ancestors.resize(query_vertices_num);

    
    for (ui i = 0; i < query_vertices_num; ++i) {
        VertexID u = order[i];
        ancestors[u].set(u);
        for (ui j = 0; j < bn_cnt[i]; ++j) {
            VertexID u_bn = bn[i][j];
            ancestors[u] |= ancestors[u_bn];
        }
    }
}

void EvaluateQuery::computeAncestor(const Graph *query_graph, VertexID *order,
                                    std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> &ancestors) {
    ui query_vertices_num = query_graph->getVerticesCount();
    ancestors.resize(query_vertices_num);

    
    for (ui i = 0; i < query_vertices_num; ++i) {
        VertexID u = order[i];
        ancestors[u].set(u);
        for (ui j = 0; j < i; ++j) {
            VertexID u_bn = order[j];
            if (query_graph->checkEdgeExistence(u, u_bn)) {
                ancestors[u] |= ancestors[u_bn];
            }
        }
    }
}

size_t EvaluateQuery::exploreVF3Style(const Graph *data_graph, const Graph *query_graph, ui **candidates,
                                          ui *candidates_count, ui *order,
                                          ui *pivot, size_t output_limit_num, size_t &call_count) {
    ui query_vertices_num = query_graph->getVerticesCount();
    ui query_labels_num = query_graph->getLabelsCount();
    ui query_max_label_fre = query_graph->getGraphMaxLabelFrequency();
    ui data_vertices_num = data_graph->getVerticesCount();
    ui data_labels_num = data_graph->getLabelsCount();
    ui data_max_label_fre = data_graph->getGraphMaxLabelFrequency();

    
    
    ui ***query_feasibility = NULL;
    ui **query_feasibility_count = NULL;
    query_feasibility = new ui**[query_vertices_num + 1]; 
    query_feasibility_count = new ui*[query_vertices_num + 1];
    for (ui i = 0; i <= query_vertices_num; i++){
        query_feasibility[i] = new ui*[query_labels_num];
        query_feasibility_count[i] = new ui[query_labels_num];
        memset(query_feasibility_count[i], 0, query_labels_num*sizeof(ui));
        for (ui j = 0; j < query_labels_num; j++){
            query_feasibility[i][j] = new ui[query_max_label_fre];
        }
    }
    
    
    generateFeasibility(query_graph, order, query_feasibility, query_feasibility_count);
    
    
    
    
    
    
    
    
    
    
    

    
    ui ***data_feasibility = NULL;
    ui **data_feasibility_count = NULL;
    data_feasibility = new ui**[query_vertices_num + 1];
    data_feasibility_count = new ui*[query_vertices_num + 1];
    for (ui i = 0; i <= query_vertices_num; i++){
        data_feasibility[i] = new ui*[data_labels_num];
        data_feasibility_count[i] = new ui[data_labels_num];
        memset(data_feasibility_count[i], 0, data_labels_num*sizeof(ui));
        for (ui j = 0; j < data_labels_num; j++){
            data_feasibility[i][j] = new ui[data_max_label_fre];
        }
    }

    
    ui max_depth = query_graph->getVerticesCount();
    ui *idx = new ui[max_depth];
    ui *idx_count = new ui[max_depth];
    ui *embedding = new ui[max_depth];
    ui embedding_cnt = 0;
    ui depth = 0;
    ui **valid_candidates = new ui *[max_depth];
    
    
    for (ui i = 0; i < max_depth; ++i) {
        valid_candidates[i] = new ui[data_max_label_fre];
    }
    bool *visited_vertices = new bool[data_vertices_num];
    memset(visited_vertices, 0, data_vertices_num*sizeof(bool));
    exploreVF3Backtrack(data_graph, query_graph, order, pivot, output_limit_num, call_count, embedding_cnt,
                        depth, max_depth, idx, idx_count, embedding, valid_candidates, visited_vertices,
                        query_feasibility, query_feasibility_count, data_feasibility, data_feasibility_count);

    for (ui i = 0; i <= query_vertices_num; i++){
        for (ui j = 0; j < query_labels_num; j++){
            delete[] query_feasibility[i][j];
        }
        delete[] query_feasibility[i];
        delete[] query_feasibility_count[i];
    }
    delete[] query_feasibility;
    delete[] query_feasibility_count;
    for (ui i = 0; i <= query_vertices_num; i++){
        for (ui j = 0; j < data_labels_num; j++){
            delete[] data_feasibility[i][j];
        }
        delete[] data_feasibility[i];
        delete[] data_feasibility_count[i];
    }
    delete[] data_feasibility;
    delete[] data_feasibility_count;
    delete[] idx;
    delete[] idx_count;
    delete[] embedding;
    for (ui i = 0; i < max_depth; i++) {
        delete[] valid_candidates[i];
    }
    delete[] valid_candidates;
    delete[] visited_vertices;
    return embedding_cnt;
}


void EvaluateQuery::generateFeasibility(const Graph *graph, ui v, bool* matched, ui level, ui*** feasibility, ui**feasibility_count) {
    if (level < (ui)1) {
        
        exit(1);
    }
    
    
    ui labels_num = graph->getLabelsCount();
    for (ui j = 0; j < labels_num; j++) {
        feasibility_count[level][j] = 0;
        
        for (ui k = 0; k < feasibility_count[level - 1][j]; k++) {
            if (feasibility[level - 1][j][k] != v) {
                feasibility[level][j][feasibility_count[level][j]++] = feasibility[level - 1][j][k];
            }
        }
        
    }
    
    
    
    
    
    
    
    ui nbrs_cnt;
    const ui *nbrs = graph->getVertexNeighbors(v, nbrs_cnt);
    for (ui j = 0; j < nbrs_cnt; j++) {
        if (!matched[nbrs[j]]) {
            ui label = graph->getVertexLabel(nbrs[j]);
            bool f_add = true;
            for (ui k  = 0; k < feasibility_count[level][label]; k++) {
                if (nbrs[j] == feasibility[level][label][k]) {
                    f_add = false;
                    break;
                }
            }
            if (f_add == true) {
                ui count = feasibility_count[level][label]++;
                feasibility[level][label][count] = nbrs[j];
            }
        }
    }
}


void EvaluateQuery::generateFeasibility(const Graph *query_graph, const ui *order, ui*** feasibility, ui**feasibility_count) {
    ui vertices_num = query_graph->getVerticesCount();

    bool* query_matched = new bool[vertices_num];
    memset(query_matched, 0, vertices_num*sizeof(bool));
    
    
    
    for (ui i = 1; i < vertices_num; i++) {
        
        ui u = order[i - 1];
        query_matched[u] = true;
        generateFeasibility(query_graph, u, query_matched, i, feasibility, feasibility_count);
    }
    delete[] query_matched;
}

bool EvaluateQuery::exploreVF3Backtrack(const Graph *data_graph, const Graph *query_graph,
                            ui *order, ui *pivot, size_t output_limit_num, size_t &call_count,
                            ui& embedding_cnt, ui depth, ui max_depth, ui*idx, ui*idx_count,
                            ui *embedding, ui **valid_candidates, bool* visited_vertices,
                            ui*** query_feasibility, ui**query_feasibility_count,
                            ui*** data_feasibility, ui**data_feasibility_count) {
    if (depth == max_depth){
        
        embedding_cnt++;
        if (embedding_cnt >= output_limit_num) {
            for (ui i = 0; i < depth; i++){
                idx[i] = idx_count[i];
            }
            return true;
        }
        return true;
    }
    ui u = order[depth];
    
    
    const ui*candidates;
    ui candidate_count;
    idx[depth] = 0;
    idx_count[depth] = 0;
    
    if (pivot[depth] == (ui)-1) {
        
        ui query_label = query_graph->getVertexLabel(u);
        candidates = data_graph->getVerticesByLabel(query_label, candidate_count);
        for (ui i = 0; i < candidate_count; i++) {
            if (!visited_vertices[candidates[i]]) {
                valid_candidates[depth][idx_count[depth]++] = candidates[i];
                
            }
        }
    } else {
        
        if (!visited_vertices[embedding[pivot[depth]]]){
            
            exit(1);
        }
        
        ui v = embedding[pivot[depth]];
        candidates = data_graph->getVertexNeighbors(v, candidate_count);
        ui query_label = query_graph->getVertexLabel(u);
        for (ui i = 0; i < candidate_count; i++) {
            ui data_label = data_graph->getVertexLabel(candidates[i]);
            if (!visited_vertices[candidates[i]] && data_label == query_label) {
                valid_candidates[depth][idx_count[depth]++] = candidates[i];
                
            }
        }
    }
    
    
    bool result = false;
    while (idx[depth] < idx_count[depth]) {
        
        if (isFeasibility(data_graph, query_graph, depth, order[depth], valid_candidates[depth][idx[depth]],
                            embedding, order, query_feasibility, query_feasibility_count,
                            data_feasibility, data_feasibility_count) == true) {
            embedding[order[depth]] = valid_candidates[depth][idx[depth]];
            visited_vertices[embedding[order[depth]]] = true;
            call_count++;
            if (exploreVF3Backtrack(data_graph, query_graph, order, pivot, output_limit_num, call_count, embedding_cnt,
                        depth + 1, max_depth, idx, idx_count, embedding, valid_candidates, visited_vertices,
                        query_feasibility, query_feasibility_count, data_feasibility, data_feasibility_count) == true) 
                result = true;
            visited_vertices[embedding[order[depth]]] = false;
        }
        idx[depth]++;
    }
    return result;
}

bool EvaluateQuery::isFeasibility(const Graph *data_graph, const Graph *query_graph, ui depth, ui cur_u, ui cur_v,
                    ui *embedding, ui* order, ui*** query_feasibility, ui**query_feasibility_count,
                    ui*** data_feasibility, ui**data_feasibility_count) {
    ui data_vertices_num = data_graph->getVerticesCount();
    ui query_vertices_num = query_graph->getVerticesCount();
    ui data_labels_num = data_graph->getLabelsCount();
    ui query_labels_num = query_graph->getLabelsCount();
    ui labels_num = query_labels_num;
    if (data_labels_num < query_labels_num) {
        
        exit(1);
    }
    
    bool *data_matched = new bool[data_vertices_num];
    bool *query_matched = new bool[query_vertices_num];
    memset(data_matched, false, data_vertices_num*sizeof(bool));
    memset(query_matched, false, query_vertices_num*sizeof(bool));
    for (ui i = 0; i < depth; i++) {
        data_matched[embedding[order[i]]] = true;
        query_matched[order[i]] = true;
    }
    
    generateFeasibility(data_graph, cur_v, data_matched, depth + 1, data_feasibility, data_feasibility_count);

    
    bool *data_fmatched = new bool[data_vertices_num];
    bool *query_fmatched = new bool[query_vertices_num];
    memset(data_fmatched, false, data_vertices_num*sizeof(bool));
    memset(query_fmatched, false, query_vertices_num*sizeof(bool));
    for (ui i = 0; i < query_labels_num; i++) {
        for (ui j = 0; j < query_feasibility_count[depth+1][i]; j++) {
            query_fmatched[query_feasibility[depth+1][i][j]] = true;
        }
    }
    for (ui i = 0; i < data_labels_num; i++) {
        for (ui j = 0; j < data_feasibility_count[depth+1][i]; j++) {
            data_fmatched[data_feasibility[depth+1][i][j]] = true;
        }
    }

    
    ui unbrs_count = 0 ;
    auto unbrs = query_graph->getVertexNeighbors(cur_u, unbrs_count);
#ifdef ELABELED_GRAPH
    auto elabels = query_graph->getVertexEdgeLabels(cur_u, unbrs_count);
#endif
    ui vnbrs_count = 0;
    const ui *vnbrs = data_graph->getVertexNeighbors(cur_v, vnbrs_count);
    if (vnbrs_count < unbrs_count) return false;
    

    
    ui *unbrs_flabeled_num = new ui[query_labels_num];
    ui *vnbrs_flabeled_num = new ui[data_labels_num];
    memset(unbrs_flabeled_num, 0, query_labels_num*sizeof(ui));
    memset(vnbrs_flabeled_num, 0, data_labels_num*sizeof(ui));
    
    ui *unbrs_labeled_num = new ui[query_labels_num];
    ui *vnbrs_labeled_num = new ui[data_labels_num];
    memset(unbrs_labeled_num, 0, query_labels_num*sizeof(ui));
    memset(vnbrs_labeled_num, 0, data_labels_num*sizeof(ui));

    for (ui i = 0; i < unbrs_count; i++) {
        ui unbr = unbrs[i];
        ui unbr_label = query_graph->getVertexLabel(unbr);
        
        if (query_matched[unbr] == true) {
#ifdef ELABELED_GRAPH
            if (!data_graph->checkEdgeExistence(cur_v, embedding[unbr], elabels[i]))
#else
            if (!data_graph->checkEdgeExistence(cur_v, embedding[unbr]))
#endif
                return false;
        } else if (query_fmatched[unbr] == true) {
            unbrs_flabeled_num[unbr_label]++;
        } else {
            unbrs_labeled_num[unbr_label]++;
        }
    }
    

    for (ui i = 0; i < vnbrs_count; i++) {
        ui vnbr = vnbrs[i];
        ui vnbr_label = data_graph->getVertexLabel(vnbr);
        
        if (data_matched[vnbr] == true) {
            true; 
        } else if (data_fmatched[vnbr] == true) {
            vnbrs_flabeled_num[vnbr_label]++;
        } else {
            vnbrs_labeled_num[vnbr_label]++;
        }
    }

    for (ui i = 0; i < labels_num; i++) {
        if (vnbrs_labeled_num[i] < unbrs_labeled_num[i]) {
            
            return false;
        }
        if (vnbrs_flabeled_num[i] < unbrs_flabeled_num[i]){
            
            
            return false;
        }
    }

    return true;
}

size_t EvaluateQuery::exploreVEQStyle(const Graph *data_graph, const Graph *query_graph, TreeNode *tree,
                                    Edges ***edge_matrix, ui **candidates, ui *candidates_count,
                                    size_t output_limit_num, size_t &call_count) {
    
    ui query_vertices_num = query_graph->getVerticesCount();
    ui data_vertices_num = data_graph->getVerticesCount();
    ui** nec = new ui*[query_vertices_num];
    memset(nec, 0, sizeof(ui*)*query_vertices_num);
    computeNEC(query_graph, nec);

    
    
    
    
    
    
    
    

    
    int depth = 0;
    ui max_depth = query_graph->getVerticesCount();
    ui *embedding = new ui[query_vertices_num]; 
    ui* order = new ui[max_depth];      
    ui embedding_cnt = 0;               
    ui *extendable = new ui[query_vertices_num]; 
    for (ui i = 0; i < query_vertices_num; ++i) {
        extendable[i] = tree[i].bn_count_;
    }
    ui *idx = new ui[max_depth];                   
    ui *idx_count = new ui[max_depth];             
    ui** valid_cans = new ui*[query_vertices_num]; 
    for (ui i = 0; i < query_vertices_num; i++) {
        valid_cans[i] = new ui[candidates_count[i]];
    }
    ui* valid_cans_count = new ui[query_vertices_num]; 
    bool* visited_u = new bool[query_vertices_num]; 
    memset(visited_u, 0, query_vertices_num*sizeof(bool));
    bool *visited_vertices = new bool[data_vertices_num]; 
    memset(visited_vertices, 0, data_vertices_num*sizeof(bool));

#ifdef ENABLE_EQUIVALENT_SET
    ui** TM = new ui*[query_vertices_num];   
    for (ui i = 0; i < query_vertices_num; i++) {
        TM[i] = new ui[candidates_count[i] + 1]; 
    }
    memset(TM[0], 0, sizeof(ui)*(candidates_count[0] + 1));
     
    std::vector<std::vector<ui>> vec_index(query_vertices_num);
    for (ui i = 0; i < query_vertices_num; i++) {
        vec_index[i].resize(candidates_count[i]);
        std::fill(vec_index[i].begin(), vec_index[i].end(), (ui)-1);
    }
    std::vector<std::vector<ui>> vec_set;  
    computeNEC(query_graph, edge_matrix, candidates_count, candidates, vec_index, vec_set);
    
    std::vector<std::vector<ui>> pi_m_index(query_vertices_num);
    for (ui i = 0; i < query_vertices_num; i++) {
        pi_m_index[i].resize(candidates_count[i], (ui)-1);
    }
    std::vector<std::vector<ui>> pi_m;  
    ui* pi_m_count = new ui[max_depth]; 
    pi_m_count[0] = 0;
    
    std::vector<std::vector<ui>> dm(query_vertices_num);  
    std::unordered_map<VertexID, VertexID> reverse_embedding;  
    reverse_embedding.reserve(query_vertices_num); 
#endif

    
    ui start_vertex = (ui)-1;
    for (ui i = 0; i < query_vertices_num; i++) {
        if (extendable[i] == 0) {
            start_vertex = i;
            break;
        }
    }
    assert(start_vertex != (ui)-1);
    
    order[depth] = start_vertex;
    visited_u[start_vertex] = true;
    
    idx[depth] = 0;
    valid_cans_count[start_vertex] = candidates_count[start_vertex];
    idx_count[depth] = valid_cans_count[start_vertex];
    for (ui i = 0; i < candidates_count[start_vertex]; i++) {
        valid_cans[start_vertex][i] = candidates[start_vertex][i];
    }
#ifdef ENABLE_EQUIVALENT_SET
    std::fill(pi_m_index[start_vertex].begin(), pi_m_index[start_vertex].end(), (ui)-1);
    
    
    
    
    
#endif
    
    while (true) {
        while (idx[depth] < idx_count[depth]) {
            
#ifdef ENABLE_DYNAMIC_CANS
            
            if (idx[order[depth]] != valid_cans[order[depth]].begin()) {
                RestoreValidCans(query_graph, data_graph, visited_u, order[depth], embedding[order[depth]], valid_cans);
            }
#endif
            VertexID u = order[depth];
            VertexID v = valid_cans[u][idx[depth]];
            

            if (visited_vertices[v]) {
#ifdef ENABLE_EQUIVALENT_SET 
                TM[u][idx[depth]] = 0;
                if (reverse_embedding.find(v) == reverse_embedding.end())
                    std::cout << "error on conflict" << std::endl;
                VertexID con_u = reverse_embedding[v];
                ui con_v_index = 0, v_index = 0;
                
                for (; con_v_index < valid_cans_count[con_u]; con_v_index++) {
                    if (valid_cans[con_u][con_v_index] == v) break;
                }
                for (; v_index < candidates_count[u]; v_index++) {
                    if (candidates[u][v_index] == v) break;
                }
                
                
                assert(con_v_index < valid_cans_count[con_u]);
                assert(v_index < candidates_count[u]);
                
                auto& con_uv_idx = pi_m_index[con_u][con_v_index];
                
                
                
                
                
                for (ui i = 0; i < pi_m[con_uv_idx].size(); i++) {
                    bool f_in = false;
                    for (ui j = 0; j < vec_set[vec_index[u][v_index]].size(); j++) {
                        if (vec_set[vec_index[u][v_index]][j] == pi_m[con_uv_idx][i]) {
                            f_in = true;
                            break;
                        }
                    }
                    if (f_in == false) {
                        pi_m[con_uv_idx][i] = pi_m[con_uv_idx][pi_m[con_uv_idx].size() - 1];
                        pi_m[con_uv_idx].pop_back();
                        i--;
                    }
                }
                
                
                
                
                
#endif
                idx[depth]++;
                continue;
            }
#ifdef ENABLE_EQUIVALENT_SET 
            if (pi_m_index[u][idx[depth]] != (ui)-1) {
                
                
                
                
                
                
                
                VertexID equ_v = pi_m[pi_m_index[u][idx[depth]]][0];
                ui equ_v_index = 0;
                for (; equ_v_index < idx_count[depth]; equ_v_index++) {
                    if (equ_v == valid_cans[u][equ_v_index]) break;
                }
                assert(equ_v_index < idx_count[depth]);
                
                embedding_cnt += TM[u][equ_v_index];
                TM[u][candidates_count[u]] += TM[u][equ_v_index];
                idx[depth]++;
                continue;
            }
            reverse_embedding[v] = u;
#endif
            embedding[u] = v;
            visited_vertices[v] = true;
            ui cur_idx = idx[depth]++;
#ifdef ENABLE_EQUIVALENT_SET 
            pi_m_index[u][cur_idx] = pi_m_count[depth]++;
            ui v_index = 0;
            for (; v_index < candidates_count[u]; v_index++) {
                if (candidates[u][v_index] == v) break;
            }
            
            auto& pi = vec_set[vec_index[u][v_index]];
            pi_m.push_back(pi); 
            assert(pi_m.size() == pi_m_count[depth]);
            dm[u].clear();                                  
            for (ui i = 0; i < depth; i++) {                
                bool va_in_pi = false, sec_empty = true;
                ui va_index = 0;
                VertexID ua = order[i];
                VertexID va = embedding[ua];
                for (; va_index < candidates_count[ua]; va_index++) {
                    if (candidates[ua][va_index] == va) break;
                }
                assert(va_index < candidates_count[ua]);
                auto& pi_a = vec_set[vec_index[ua][va_index]];
                for (ui j = 0; j < pi.size(); j++) { 
                    if (pi[j] == embedding[order[i]]) { 
                        va_in_pi = true;
                        break;
                    }
                    for (ui k = 0; k < pi_a.size(); k++) { 
                        if (pi_a[k] == pi[j]) {
                            sec_empty = false;
                            break;
                        }
                    }
                    if (sec_empty == false) break;
                }
                if (va_in_pi == false && sec_empty == false) { 
                    ui dm_size = dm[ua].size();
                    for (ui j = 0; j < pi.size(); j++) {
                        bool f_add = true;
                        for (ui k = 0; k < dm_size; k++) {
                            if (dm[ua][k] == pi[j]) {
                                f_add = false;
                                break;
                            }
                        }
                        if (f_add == true) {
                            dm[ua].push_back(pi[j]);
                        }
                    }
                }
            }
#endif
            if (depth == max_depth - 1) { 
                embedding_cnt += 1;
                visited_vertices[v] = false;
                
                
                
                
                
                if (embedding_cnt >= output_limit_num) {
                    goto EXIT;
                }
#ifdef ENABLE_EQUIVALENT_SET 
                reverse_embedding.erase(v);
                TM[u][cur_idx] = 1;
                TM[u][candidates_count[u]]++;
                auto& uv_idx = pi_m_index[u][cur_idx];
                for (ui i = 0; i < pi_m[uv_idx].size(); i++) { 
                    bool f_del = false;
                    for (ui j = 0; j < dm[u].size(); j++) {
                        if (pi_m[uv_idx][i] == dm[u][j]) {
                            f_del = true;
                            break;
                        }
                    }
                    if (f_del == true) {
                        pi_m[uv_idx][i] = pi_m[uv_idx][pi_m[uv_idx].size() - 1];
                        pi_m[uv_idx].pop_back();
                        i--;
                    }
                }
                for (ui i = 1; i < pi_m[uv_idx].size(); i++) { 
                    ui v_equ_index = 0;
                    for (; v_equ_index < valid_cans_count[u]; v_equ_index++) {
                        if (valid_cans[u][v_equ_index] == pi_m[uv_idx][i])
                            break;
                    }
                    
                    
                    
                    assert(v_equ_index < valid_cans_count[u]);
                    pi_m_index[u][v_equ_index] = uv_idx;
                }
                
                if (pi_m[uv_idx].size() > 1) {
                    auto pi_v_idx = std::find(pi_m[uv_idx].begin(), pi_m[uv_idx].end(), v);
                    assert(pi_v_idx != pi_m[uv_idx].end());
                    ui tmp = *pi_v_idx;
                    *pi_v_idx = pi_m[uv_idx][0];
                    pi_m[uv_idx][0] = tmp;
                }
#endif
            } else { 
                call_count += 1;
                depth += 1;
                order[depth] = generateNextU(data_graph, query_graph, candidates, candidates_count, valid_cans,
                                             valid_cans_count, extendable, nec, depth, embedding,
                                             edge_matrix, visited_vertices, visited_u, order, tree);
                
                
                if (order[depth] == (ui)-1) {
                    break; 
                } else {
                    visited_u[order[depth]] = true;
                    idx[depth] = 0;
                    idx_count[depth] = valid_cans_count[order[depth]];
                }
#ifdef ENABLE_EQUIVALENT_SET 
                memset(TM[order[depth]], 0, sizeof(ui)*(candidates_count[order[depth]] + 1));
                std::fill(pi_m_index[order[depth]].begin(), pi_m_index[order[depth]].end(), (ui)-1);
                pi_m_count[depth] = pi_m_count[depth - 1];
#endif
            }
        }
        
        depth -= 1;
        
        if (depth < 0)
            break;
        VertexID u = order[depth];
        ui cur_idx = idx[depth] - 1;
        visited_vertices[embedding[u]] = false;
        restoreExtendableVertex(tree, u, extendable);
        if (order[depth + 1] != (ui)-1) { 
            VertexID last_u = order[depth + 1];
            
            
            visited_u[last_u] = false;
            if (nec[last_u] != NULL) (*(nec[last_u]))++;
#ifdef ENABLE_DYNAMIC_CANS
            RestoreValidCans(query_graph, data_graph, visited_u, last_u, last_v, valid_cans);
#endif
            
            
            
            
            
            
            
            
            
            
            
        }
#ifdef ENABLE_EQUIVALENT_SET 
        if (order[depth + 1] != (ui)-1) {
            TM[u][cur_idx] = TM[order[depth + 1]][candidates_count[order[depth + 1]]];
        } else {
            TM[u][cur_idx] = 0;
        }
        TM[u][candidates_count[u]] += TM[u][cur_idx];
        auto& uv_idx = pi_m_index[u][cur_idx];
        if (TM[u][cur_idx] != 0) {
            
            for (ui i = 0; i < pi_m[uv_idx].size(); i++) {
                
            }
            
            
            
            
            
            
            for (ui i = 0; i < pi_m[uv_idx].size(); i++) {
                bool f_del = false;
                for (ui j = 0; j < dm[u].size(); j++) {
                    if (pi_m[uv_idx][i] == dm[u][j]) {
                        f_del = true;
                        break;
                    }
                }
                if (f_del == true) {
                    pi_m[uv_idx][i] = pi_m[uv_idx][pi_m[uv_idx].size() - 1];
                    pi_m[uv_idx].pop_back();
                    i--;
                }
            }
            
            for (ui i = 0; i < pi_m[uv_idx].size(); i++) {
                
            }
            
            
            if (pi_m[uv_idx].size() > 1) {
                auto pi_v_idx = std::find(pi_m[uv_idx].begin(), pi_m[uv_idx].end(), embedding[u]);
                assert(pi_v_idx != pi_m[uv_idx].end());
                ui tmp = *pi_v_idx;
                *pi_v_idx = pi_m[uv_idx][0];
                pi_m[uv_idx][0] = tmp;
            }
            
            
            
            
            
        }
        for (ui i = 1; i < pi_m[uv_idx].size(); i++) {
            ui v_equ_index = 0;
            for (; v_equ_index < valid_cans_count[u]; v_equ_index++) {
                if (valid_cans[u][v_equ_index] == pi_m[uv_idx][i])
                    break;
            }
            assert(v_equ_index < valid_cans_count[u]);
            
            pi_m_index[u][v_equ_index] = uv_idx;
        }
        pi_m.resize(pi_m_count[depth]);
#endif
    }

    EXIT:
    
    for (ui i = 0; i < query_vertices_num; i++) {
        
    }
    delete []nec;
    delete []embedding;
    delete []visited_u;
    delete []visited_vertices;
    delete []order;
    delete []extendable;
    delete []idx;
    delete []idx_count;
    for (ui i = 0; i < query_vertices_num; i++) {
        delete []valid_cans[i];
    }
    delete []valid_cans;
    delete []valid_cans_count;
#ifdef ENABLE_EQUIVALENT_SET
    delete[] TM;
#endif
    return embedding_cnt;
}

void EvaluateQuery::RestoreValidCans(const Graph *query_graph, const Graph *data_graph, bool* visited_u,
                                     VertexID last_u, VertexID last_v,
                                     std::vector<std::unordered_map<VertexID, ui>>& valid_cans) {
    ui last_unbrs_count;
    const ui* last_unbrs = query_graph->getVertexNeighbors(last_u, last_unbrs_count);
    ui last_vnbrs_count;
    const ui* last_vnbrs = data_graph->getVertexNeighbors(last_v, last_vnbrs_count);
    
    
    
    
    
    for (ui i = 0; i < last_unbrs_count; i++) {
        ui last_unbr = last_unbrs[i];
        
        if (visited_u[last_unbr] == true) continue;
        for (ui j = 0; j < last_vnbrs_count; j++) {
            auto vertex = valid_cans[last_unbr].find(last_vnbrs[j]);
            if (vertex != valid_cans[last_unbr].end()) {
                if (vertex->second == 1) {
                    valid_cans[last_unbr].erase(vertex);
                } else {
                    vertex->second--;
                }
            }
        }
    }
    
    
    
    
    
}

ui EvaluateQuery::generateNextU(const Graph *data_graph, const Graph *query_graph, ui **candidates, ui *candidates_count,
                                ui**valid_cans, ui*valid_cans_count, ui* extendable,  ui** nec,
                                ui depth, ui* embedding, Edges ***edge_matrix, bool *visited_vertices,
                                bool *visited_u, ui *order, TreeNode* tree) {
    
    ui query_vertices_num = query_graph->getVerticesCount();
    ui cur_vertex = -1;
    
    
    
    
    
    TreeNode &node = tree[order[depth - 1]];
    for (ui i = 0; i < node.fn_count_; ++i) {
        VertexID u = node.fn_[i];
        extendable[u] -= 1;
        if (extendable[u] == 0) { 
            ComputeValidCans(data_graph, query_graph, candidates, candidates_count, valid_cans,
                             valid_cans_count, embedding, u, visited_u);
        }
    }
    
    
    
    
    

    
    
    
    
    
    
    

    
    
    
    
    
    
    
    
    

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    bool f_only_1 = true;
    
    for (ui i = 0; i < query_vertices_num; i++) {
        if (visited_u[i] == true || extendable[i] != 0) continue;
        ui u_count = 0;
        for (ui j = 0; j < valid_cans_count[i]; j++) {
            if (visited_vertices[valid_cans[i][j]] == false) u_count++;
        }
        if (nec[i] != NULL && *(nec[i]) >= u_count) {
            if (*(nec[i]) > u_count) {
                
                return (ui)-1;
            } else {
                (*(nec[i]))--;
                return i;
            }
        }
        if (nec[i] != NULL) {
            if(cur_vertex == (ui)-1) cur_vertex = i;
        } else if (f_only_1 == true) {
            f_only_1 = false;
        }
    }

    if (f_only_1 == false) { 
        cur_vertex = (ui)-1;
        ui min_Um_u = (ui)-1;
        for (ui i = 0; i < query_vertices_num; i++) {
            if (visited_u[i] == true || nec[i] != 0 || extendable[i] != 0) continue;
            if (valid_cans_count[i] != 0 && min_Um_u > valid_cans_count[i]) {
                cur_vertex = i;
                min_Um_u = valid_cans_count[i];
            }
            
        }
    }

  
    if (cur_vertex != (ui)-1 && nec[cur_vertex] != NULL) {
        (*nec[cur_vertex])--;
    }

    return cur_vertex;
}

void EvaluateQuery::ComputeValidCans(const Graph *data_graph, const Graph *query_graph, ui **candidates, ui *candidates_count,
                      ui**valid_cans, ui*valid_cans_count, ui* embedding, VertexID u, bool* visited_u) {
    ui unbrs_count;
    auto unbrs = query_graph->getVertexNeighbors(u, unbrs_count);
#ifdef ELABELED_GRAPH
    auto elabels = query_graph->getVertexEdgeLabels(u, unbrs_count);
#endif
    valid_cans_count[u] = 0;
    
    
    
    
    
    for (ui i = 0; i < candidates_count[u]; i++) {
        VertexID v = candidates[u][i];
        bool flag = true;
        for (ui j = 0; j < unbrs_count; j++) {
            if (visited_u[unbrs[j]] == true
#ifdef ELABELED_GRAPH
                && !data_graph->checkEdgeExistence(v, embedding[unbrs[j]], elabels[j])) {
                std::cout << "v-v:" << data_graph->getEdgeLabel(v, embedding[unbrs[j]], true) << ", "
                          << "u-u:" << query_graph->getEdgeLabel(u, unbrs[j], true) << ", "
                          << "elabels[j]:" << elabels[j] << std::endl;
#else
                && !data_graph->checkEdgeExistence(v, embedding[unbrs[j]])) {
#endif
                flag = false;
                break;
            }
        }
        if (flag == true) {
            valid_cans[u][valid_cans_count[u]++] = v;
        }
    }
    
    
    
    
    
}


void EvaluateQuery::computeNEC(const Graph *query_graph, Edges ***edge_matrix, ui *candidates_count,
                               ui**candidates, std::vector<std::vector<ui>>& vec_index,
                               std::vector<std::vector<ui>>& vec_set) {
    std::vector<ui> tmp_vec;
    ui vec_count = 0;
    for (ui i = 0; i < vec_index.size(); i++) {
        tmp_vec.reserve(candidates_count[i]);
        ui unbrs_count;
        const ui *unbrs = query_graph->getVertexNeighbors(i, unbrs_count);
        for (ui j = 0; j < candidates_count[i]; j++) {
            if (vec_index[i][j] != (ui)-1)
                continue;
            vec_index[i][j] = vec_count++;
            tmp_vec.push_back(candidates[i][j]);
            for (ui k = j + 1; k < candidates_count[i]; k++) {
                if (vec_index[i][k] != (ui)-1)
                    continue;
                
                bool equ = true;
                for (ui u1 = 0; u1 < unbrs_count; u1++) {
                    ui unbr = unbrs[u1];
                    const Edges* edges = edge_matrix[i][unbr];
                    if (edges->offset_[j+1]-edges->offset_[j] == 0) {
                        
                        
                        goto SKIP;
                    }
                    if (edges->offset_[j+1]-edges->offset_[j] != edges->offset_[k+1] - edges->offset_[k]) {
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        equ = false;
                        break;
                    }
                    
                    
                    for (ui u2 = 0; u2 < edges->offset_[j+1] - edges->offset_[j]; u2++) {
                        if (edges->edge_[u2+edges->offset_[j]] != edges->edge_[u2+edges->offset_[k]]) {
                            
                            
                            
                            equ = false;
                            break;
                        }
                    }
                    if (equ == false) break;
                }
                if (equ == true) {
                    tmp_vec.push_back(candidates[i][k]);
                    vec_index[i][k] = vec_index[i][j];
                }
            }
            SKIP:
            
            vec_set.push_back(tmp_vec);
            tmp_vec.clear();
        }
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
}


void EvaluateQuery::computeNEC(const Graph *query_graph, ui** nec) {
    ui query_vertices_num = query_graph->getVerticesCount();
    ui* nec_tmp = new ui[query_vertices_num];
    ui flag = true; 
    for (ui i = 0; i < query_vertices_num; i++) {
        
        if (query_graph->getVertexDegree(i) != 1 || nec[i] != NULL) {
            continue;
        }
        ui unbrs_num;
        auto unbrs = query_graph->getVertexNeighbors(i, unbrs_num);
        auto u1elabels = query_graph->getVertexEdgeLabels(i, unbrs_num);
        ui u_label = query_graph->getVertexLabel(i);
        ui* nec_count = new ui(0);
        nec_tmp[(*nec_count)++] = i;
        for (ui j = i + 1; j < query_vertices_num; j++) {
            if (query_graph->getVertexDegree(i) != 1 || nec[i] != NULL) {
                continue;
            }
            
            if (u_label != query_graph->getVertexLabel(j)) {
                continue;
            }
            ui u2_nbrs_num;
            auto u2_nbrs = query_graph->getVertexNeighbors(j, u2_nbrs_num);
            auto u2elabels = query_graph->getVertexEdgeLabels(j, u2_nbrs_num);
            if (u2_nbrs_num != unbrs_num) {
                continue;
            }
            ui k1 = 0;
            for (k1 = 0; k1 < unbrs_num; k1++) {
                flag = false;
                for (ui k2 = 0; k2 < u2_nbrs_num; k2++) {
                    if (unbrs[k1] == u2_nbrs[k2]) {
                        if (u1elabels[k1] == u2elabels[k2])
                            flag = true;
                        break;
                    }
                }
                if (flag == false) break; 
            }
            if (k1 != unbrs_num) continue;
            nec_tmp[(*nec_count)++] = j;
        }
        
        
        for (ui j = 0; j < *nec_count; j++) {
            nec[nec_tmp[j]] = nec_count;
            
        }
        
    }
    delete []nec_tmp;
}

size_t
EvaluateQuery::exploreRMStyle(const Graph *query_graph, const Graph *data_graph, catalog*&storage, Edges ***edge_matrix,
                              ui **candidates, ui *candidates_count, ui *order, size_t output_limit_num, size_t &call_count) {
    
    std::cout << "num_cans before encoded:";
    for (ui i = 0; i < query_graph->getVerticesCount(); i++) {
        std::cout << candidates_count[i] << ",";
    }
    std::cout << std::endl;
#ifdef ELABELED_GRAPH
    if (storage != NULL) {
        delete storage;
        storage = NULL;
    }
#endif
    if (storage == NULL) {
        std::cout << "storage init" << std::endl;
        storage = new catalog(query_graph, data_graph);
        convertCans2Catalog(query_graph, candidates, edge_matrix, storage);
        storage->query_graph_->get2CoreSize();
        storage->data_graph_->getVerticesCount();
    }
    std::cout << "order in exploreRM: ";
    for (ui i = 0; i < query_graph->getVerticesCount(); i++) {
        std::cout << order[i] << ',';
    }
    std::cout << std::endl;
    convert_to_encoded_relation(storage, order);
#ifdef SPARSE_BITMAP
    convert_encoded_relation_to_sparse_bitmap(storage, order);
#endif
    std::vector<ui> vorder;
    vorder.reserve(query_graph->getVerticesCount());
    vorder.insert(vorder.end(), order, order + query_graph->getVerticesCount());
    auto tree = execution_tree_generator::generate_single_node_execution_tree(vorder);
    return tree->execute(*storage, output_limit_num, call_count);
}

void EvaluateQuery::convertCans2Catalog(const Graph *query_graph, ui **candidates, Edges ***edge_matrix, catalog *storage) {
    
    for (ui u = 0; u < storage->num_sets_; u++) {
        ui unbrs_cnt;
        const ui* unbrs = query_graph->getVertexNeighbors(u, unbrs_cnt);
        for (ui i = 0; i < unbrs_cnt; i++) {
            ui v = unbrs[i];
            
            if (u > v) continue;
            std::vector<edge> tmp_edges;
            auto& edges = *edge_matrix[u][v];
            auto& relation = storage->edge_relations_[u][v];
            for (ui j = 0; j < edges.vertex_count_; j++) {
                ui src = candidates[u][j];
                for (ui k = edges.offset_[j]; k < edges.offset_[j+1]; k++) {
                    ui dst = candidates[v][edges.edge_[k]];
                    tmp_edges.push_back(std::move(edge(src, dst)));
                }
            }
            relation.size_ = tmp_edges.size();
            relation.edges_ = new edge[relation.size_];
            memcpy(relation.edges_, tmp_edges.data(), sizeof(edge) * relation.size_);
            std::cout << u << '|' << v << "(edge):" << tmp_edges.size() << std::endl;
        }
    }
}

void EvaluateQuery::convert_to_encoded_relation(catalog *storage, ui*order) {
    auto& query_graph = storage->query_graph_;
    uint32_t core_vertices_cnt = query_graph->get2CoreSize();
    auto max_vertex_id = storage->data_graph_->getVerticesCount();

    auto projection_operator = new projection(max_vertex_id);
    for (uint32_t i = 0; i < core_vertices_cnt || i == 0; ++i) {
        uint32_t u = order[i];
        uint32_t nbr_cnt;
        const uint32_t* nbrs = query_graph->getVertexNeighbors(u, nbr_cnt);
        for (uint32_t j = 0; j < nbr_cnt; ++j) {
            uint32_t v = nbrs[j];
            uint32_t src = u;
            uint32_t dst = v;
            uint32_t kp = 0;
            if (src > dst) {
                std::swap(src, dst);
                kp = 1;
            }

            projection_operator->execute(&storage->edge_relations_[src][dst], kp, storage->candidate_sets_[u], storage->num_candidates_[u]);
            
            break;
        }
    }
    std::cout << "num_cans after encoded:";
    for (ui i = 0; i < query_graph->getVerticesCount(); i++) {
        std::cout << storage->num_candidates_[i] << ",";
    }
    std::cout << std::endl;

    delete projection_operator;

    uint32_t n = query_graph->getVerticesCount();
    for (uint32_t i = 1; i < n; ++i) {
        uint32_t u = order[i];
        for (uint32_t j = 0; j < i; ++j) {
            uint32_t bn = order[j];
            if (query_graph->checkEdgeExistence(bn, u)) {
                if (i < core_vertices_cnt) {
                    convert_to_encoded_relation(storage, bn, u);
                }
                else {
                    convert_to_hash_relation(storage, bn, u);
                }
            }
        }
    }
}

void EvaluateQuery::convert_to_encoded_relation(catalog *storage, uint32_t u, uint32_t v) {
    uint32_t src = std::min(u, v);
    uint32_t dst = std::max(u, v);
    edge_relation& target_edge_relation = storage->edge_relations_[src][dst];
    edge* edges = target_edge_relation.edges_;
    uint32_t edge_size = target_edge_relation.size_;
    auto max_vertex_id = storage->data_graph_->getVerticesCount();
    assert(edge_size > 0);

    auto buffer = new uint32_t[max_vertex_id];
    memset(buffer, 0, sizeof(uint32_t)*max_vertex_id);

    uint32_t v_candidates_cnt = storage->get_num_candidates(v);
    uint32_t* v_candidates = storage->get_candidates(v);

    for (uint32_t i = 0; i < v_candidates_cnt; ++i) {
        uint32_t v_candidate = v_candidates[i];
        buffer[v_candidate] = i + 1;
    }

    uint32_t u_p = 0;
    uint32_t v_p = 1;
    if (u > v) {
        
        std::sort(edges, edges + edge_size, [](edge& l, edge& r) -> bool {
            if (l.vertices_[1] == r.vertices_[1])
                return l.vertices_[0] < r.vertices_[0];
            return l.vertices_[1] < r.vertices_[1];
        });
        u_p = 1;
        v_p = 0;
    }

    encoded_trie_relation& target_encoded_trie_relation = storage->encoded_trie_relations_[u][v];
    uint32_t edge_cnt = edge_size;
    uint32_t u_candidates_cnt = storage->get_num_candidates(u);
    uint32_t* u_candidates = storage->get_candidates(u);
    target_encoded_trie_relation.size_ = u_candidates_cnt;
    target_encoded_trie_relation.offset_ = new uint32_t[u_candidates_cnt + 1];
    target_encoded_trie_relation.children_ = new uint32_t[edge_size];

    uint32_t offset = 0;
    uint32_t edge_index = 0;

    for (uint32_t i = 0; i < u_candidates_cnt; ++i) {
        uint32_t u_candidate = u_candidates[i];
        target_encoded_trie_relation.offset_[i] = offset;
        uint32_t local_degree = 0;
        while (edge_index < edge_cnt) {
            uint32_t u0 = edges[edge_index].vertices_[u_p];
            uint32_t v0 = edges[edge_index].vertices_[v_p];
            if (u0 == u_candidate) {
                if (buffer[v0] > 0) {
                    target_encoded_trie_relation.children_[offset + local_degree] = buffer[v0] - 1;
                    local_degree += 1;
                }
            }
            else if (u0 > u_candidate) {
                break;
            }

            edge_index += 1;
        }

        offset += local_degree;

        if (local_degree > target_encoded_trie_relation.max_degree_) {
            target_encoded_trie_relation.max_degree_ = local_degree;
        }
    }

    target_encoded_trie_relation.offset_[u_candidates_cnt] = offset;

    for (uint32_t i = 0; i < v_candidates_cnt; ++i) {
        uint32_t v_candidate = v_candidates[i];
        buffer[v_candidate] = 0;
    }
}

void EvaluateQuery::convert_to_hash_relation(catalog *storage, uint32_t u, uint32_t v) {
    
    uint32_t src = std::min(u, v);
    uint32_t dst = std::max(u, v);
    edge_relation& target_edge_relation = storage->edge_relations_[src][dst];
    hash_relation& target_hash_relation1 = storage->hash_relations_[u][v];
    auto max_vertex_id = storage->data_graph_->getVerticesCount();

    edge* edges = target_edge_relation.edges_;
    uint32_t edge_size = target_edge_relation.size_;

    assert(edge_size > 0);

    uint32_t u_key = 0;
    uint32_t v_key = 1;

    if (src != u) {
        std::swap(u_key, v_key);
        
        std::sort(edges, edges + edge_size, [](edge& l, edge& r)-> bool {
            if (l.vertices_[1] == r.vertices_[1])
                return l.vertices_[0] < r.vertices_[0];
            return l.vertices_[1] < r.vertices_[1];
        });
    }

    target_hash_relation1.children_ = new uint32_t[edge_size];

    uint32_t offset = 0;
    uint32_t local_degree = 0;
    uint32_t prev_u0 = max_vertex_id + 1;

    for (uint32_t i = 0; i < edge_size; ++i) {
        uint32_t u0 = edges[i].vertices_[u_key];
        uint32_t u1 = edges[i].vertices_[v_key];
        if (u0 != prev_u0 ) {
            if (prev_u0 != max_vertex_id + 1)
                target_hash_relation1.trie_->emplace(prev_u0, std::make_pair(local_degree, offset));

            offset += local_degree;

            if (local_degree > target_hash_relation1.max_degree_) {
                target_hash_relation1.max_degree_ = local_degree;
            }

            local_degree = 0;
            prev_u0 = u0;
        }

        target_hash_relation1.children_[offset + local_degree] = u1;
        local_degree += 1;
    }

    target_hash_relation1.cardinality_ = edge_size;
    target_hash_relation1.trie_->emplace(prev_u0, std::make_pair(local_degree, offset));
    if (local_degree > target_hash_relation1.max_degree_) {
        target_hash_relation1.max_degree_ = local_degree;
    }
}

void EvaluateQuery::convert_encoded_relation_to_sparse_bitmap(catalog *storage, ui*order) {
    uint32_t core_vertices_cnt = storage->query_graph_->get2CoreSize();

    for (uint32_t i = 1; i < core_vertices_cnt; ++i) {
        uint32_t u = order[i];

        for (uint32_t j = 0; j < i; ++j) {
            uint32_t bn = order[j];
            if (storage->query_graph_->checkEdgeExistence(u, bn)) {
                storage->bsr_relations_[bn][u].load(storage->encoded_trie_relations_[bn][u].get_size(),
                                                   storage->encoded_trie_relations_[bn][u].offset_,
                                                   storage->encoded_trie_relations_[bn][u].offset_,
                                                   storage->encoded_trie_relations_[bn][u].children_,
                                                   storage->max_num_candidates_per_vertex_, true);
            }
        }
    }
}


void EvaluateQuery::ComputeValidCans(const Graph *data_graph, const Graph *query_graph, ui **candidates, ui *candidates_count,
                      ui**valid_cans, ui*valid_cans_count, ui* embedding, VertexID u, bool* visited_u, bool * visited_v) {
    ui unbrs_count;
    auto unbrs = query_graph->getVertexNeighbors(u, unbrs_count);
#ifdef ELABELED_GRAPH
    auto elabels = query_graph->getVertexEdgeLabels(u, unbrs_count);
#endif
    valid_cans_count[u] = 0;
    
    
    
    
    
    for (ui i = 0; i < candidates_count[u]; i++) {
        VertexID v = candidates[u][i];
        if (visited_v[v]) continue;
        bool flag = true;
        for (ui j = 0; j < unbrs_count; j++) {
            if (visited_u[unbrs[j]] == true
#ifdef ELABELED_GRAPH
                && !data_graph->checkEdgeExistence(v, embedding[unbrs[j]], elabels[j])) {
#else
                && !data_graph->checkEdgeExistence(v, embedding[unbrs[j]])) {
#endif
                flag = false;
                break;
            }
        }
        if (flag == true) {
            valid_cans[u][valid_cans_count[u]++] = v;
        }
    }
    
    
    
    
    
}

size_t
EvaluateQuery::exploreKSSStyle(const Graph *query_graph, const Graph *data_graph, Edges***edge_matrix,
                               ui **candidates, ui *candidates_count, ui *order, size_t output_limit_num,
                               size_t &call_count) {
    size_t embedding_cnt = 0;
    int cur_depth = 0;
    ui query_vertices_num = query_graph->getVerticesCount();
    ui data_vertices_num = data_graph->getVerticesCount();
    
    ui kernel_num = 0, shell_num = 0;
    ui* kernel = new ui[query_vertices_num];
    ui* shell = new ui[query_vertices_num];
    
    bool* kos = new bool[query_vertices_num];
    memset(kos, 0, sizeof(bool)*query_vertices_num);
    
    ui * degree = new ui[query_vertices_num];
    for (ui i = 0; i < query_vertices_num; i++) {
        degree[i] = query_graph->getVertexDegree(i);
    }
    for (ui i = 0; i < query_vertices_num; i++) {
        VertexID u = order[i];
        if (degree[u] == 0) {
            shell[shell_num++] = u;
            continue;
        }
        kernel[kernel_num++] = u;
        kos[u] = true;
        ui nbr_num = 0;
        const ui* nbrs = query_graph->getVertexNeighbors(u, nbr_num);
        for (ui j = 0; j < nbr_num; j++) {
            degree[nbrs[j]]--;  
        }
    }

    
    
    
    ui* shell2kernel = new ui[query_vertices_num];
    memset(shell2kernel, 0, sizeof(ui)*query_vertices_num);
    for (ui i = 0; i < shell_num; i++) {
        VertexID v = shell[i];
        ui nbr_num = 0;
        const ui* nbrs = query_graph->getVertexNeighbors(v, nbr_num);
        for (ui j = 0; j < nbr_num; j++) {
            if (kos[nbrs[j]] == true) {
                shell2kernel[v]++;
            }
        }
    }

    
    ui *idx = new ui [kernel_num];                   
    ui *idx_count = new ui [kernel_num];               
    ui *embedding = new ui [query_vertices_num];     
    ui **valid_cans = new ui* [query_vertices_num];  
    for (ui i = 0; i < query_vertices_num; i++) {
        valid_cans[i] = new ui [candidates_count[i]];
    }
    ui *valid_cans_cnt = new ui [query_vertices_num];
    bool *visited_v = new bool [data_vertices_num];  
    memset(visited_v, 0, sizeof(bool)*data_vertices_num);
    bool *visited_u = new bool[query_vertices_num];  
    memset(visited_u, 0, sizeof(bool)*query_vertices_num);

    VertexID start_vertex = kernel[0];
    visited_u[start_vertex] = true;
    idx[cur_depth] = 0;
    idx_count[cur_depth] = candidates_count[start_vertex];

    for (ui i = 0; i < idx_count[cur_depth]; ++i) {
        valid_cans[start_vertex][i] = candidates[start_vertex][i];
    }

    std::vector<VertexID> update;

    
    std::cout << "query_vertices_num:" << query_vertices_num << std::endl;
    std::cout << "kernel_num:" << kernel_num << std::endl;
    std::cout << "shell_num:"  << shell_num << std::endl;

    while (true) {
        while (idx[cur_depth] < idx_count[cur_depth]) {
            VertexID u = kernel[cur_depth];
            VertexID v = valid_cans[u][idx[cur_depth]];

            embedding[u] = v;
            visited_v[v] = true;
            idx[cur_depth] += 1;

            
            bool f_ok = true;
            updateShell2Kernel(query_graph, u, shell2kernel, kos, update);
            for (auto ushell: update) {
                ComputeValidCans(data_graph, query_graph, candidates, candidates_count, valid_cans,
                                 valid_cans_cnt, embedding, ushell, visited_u, visited_v);
                if (valid_cans_cnt[ushell] == 0) {
                    f_ok = false;
                    std::cout << "break" << std::endl;
                    break;
                }
            }
            if (!f_ok) {
                visited_v[v] = false;
                restoreShell2Kernel(query_graph, u, shell2kernel, kos);
                continue; 
            }

            if (cur_depth == kernel_num - 1) {
                for(int i = 0; i < shell_num; i++) {
                    VertexID u_shell = shell[i];
                    
                    
                    std::cout << "candidate of u_shell " << u_shell << ": ";
                    for(int j = 0; j < valid_cans_cnt[u_shell]; j++) {
                        std::cout << valid_cans[u_shell][j] << ",";
                    }
                    std::cout << std::endl;
                }
                
                embedding_cnt += computeKSSEmbeddingNaive(shell_num, shell, valid_cans, valid_cans_cnt, visited_v);
                
                std::cout << embedding_cnt << std::endl;
                visited_v[v] = false;
                if (embedding_cnt >= output_limit_num) {
                    goto EXIT;
                }
                
                restoreShell2Kernel(query_graph, u, shell2kernel, kos);
            } else {
                cur_depth += 1;
                
                VertexID next_u = kernel[cur_depth];
                idx[cur_depth] = 0;
                call_count += 1;
                ComputeValidCans(data_graph, query_graph, candidates, candidates_count, valid_cans,
                                 valid_cans_cnt, embedding, next_u, visited_u, visited_v);
                
                visited_u[next_u] = true;
                idx_count[cur_depth] = valid_cans_cnt[next_u];
            }
        }

        
        VertexID last_u = kernel[cur_depth];

        cur_depth -= 1;
        if (cur_depth < 0)
            break;
        visited_v[embedding[kernel[cur_depth]]] = false;
        visited_u[last_u] = false;
        
        restoreShell2Kernel(query_graph, kernel[cur_depth], shell2kernel, kos);
        
    }

    
    EXIT:
    delete[] kernel;
    delete[] shell;
    delete[] kos;
    delete[] degree;
    delete[] shell2kernel;
    delete[] idx;
    delete[] idx_count;
    delete[] embedding;
    for (ui i = 0; i < query_vertices_num; i++) {
        delete[] valid_cans[i];
    }
    delete[] valid_cans;
    delete[] valid_cans_cnt;
    delete[] visited_u;
    delete[] visited_v;
    return embedding_cnt;
}

void 
EvaluateQuery::updateShell2Kernel(const Graph *query_graph, VertexID u, ui* shell2kernel, bool* kos, std::vector<VertexID> & update) {
    ui nbr_num = 0;
    const ui* nbrs = query_graph->getVertexNeighbors(u, nbr_num);
    
    

    update.clear();
    for (ui i = 0; i < nbr_num; i++) {
        VertexID nbr = nbrs[i];
        if (kos[nbr] == false) {
            shell2kernel[nbr]--;
            
            
            
            
            
            
            
            if(shell2kernel[nbr] == 0)
                update.emplace_back(nbr);
        }
    }
}

void  
EvaluateQuery::restoreShell2Kernel(const Graph *query_graph, VertexID u, ui* shell2kernel, bool* kos) {
    ui nbr_num = 0;
    
    
    const ui*nbrs = query_graph->getVertexNeighbors(u, nbr_num);
    for (ui i = 0; i < nbr_num; i++) {
        VertexID nbr = nbrs[i];
        if (kos[nbr] == false) {
            shell2kernel[nbr]++;
            
            
        }
    }
}


size_t 
EvaluateQuery::computeKSSEmbeddingNaive(ui shell_num, ui* shell, ui** valid_cans, ui* valid_cans_count, bool * visited_v) {
    
    return computeKSSEmbeddingNaiveImpl(0, shell_num, shell, valid_cans, valid_cans_count, visited_v);
}

size_t 
EvaluateQuery::computeKSSEmbeddingNaiveImpl(ui depth, ui shell_num, ui* shell, ui** valid_cans, ui* valid_cans_count, bool * visited_v) {
    VertexID u_shell = shell[depth];
    size_t embedding_cnt = 0;

    
    if (depth == shell_num - 1) {
        
        for(int i = 0; i < valid_cans_count[u_shell]; i++) {
            VertexID v_id = valid_cans[u_shell][i];
            if (!visited_v[v_id]) embedding_cnt += 1;
        }
        return embedding_cnt;
    }
    
    else {
        for(int i = 0; i < valid_cans_count[u_shell]; i++) {
            VertexID v_id = valid_cans[u_shell][i];
            if(!visited_v[v_id]){
                visited_v[v_id] = true;
                embedding_cnt += computeKSSEmbeddingNaiveImpl(depth+1, shell_num, shell, valid_cans, valid_cans_count, visited_v);
                visited_v[v_id] = false;
            }
        }
        return embedding_cnt;
    }
}




size_t 
EvaluateQuery::computeKSSEmbeddingOpt1(ui shell_num, ui* shell, ui** valid_cans, ui* valid_cans_count, bool * visited_v) {
    
    std::unordered_map<VertexID, ui> counter;

    
    for (int i = 0; i < shell_num; i++) {
        VertexID u_shell = shell[i];
        for (int j = 0; j < valid_cans_count[u_shell]; j++) {
            VertexID v_id = valid_cans[u_shell][j];
            if (counter.count(v_id) == 0) {  
                counter[v_id] = 0;  
            } else {    
                counter[v_id] = i;    
            }
        }
    }

    return computeKSSEmbeddingOpt1Impl(0, shell_num, shell, valid_cans, valid_cans_count, counter, visited_v);
}


size_t 
EvaluateQuery::computeKSSEmbeddingOpt1Impl(ui depth, ui shell_num, ui* shell, ui** valid_cans, ui* valid_cans_count,
                                      std::unordered_map<VertexID, ui> & counter,
                                      bool * visited_v) {
    size_t embedding_cnt = 0;
    VertexID u_shell = shell[depth];

    
    if (depth == shell_num - 1) {
        
        for(int i = 0; i < valid_cans_count[u_shell]; i++) {
            VertexID v_id = valid_cans[u_shell][i];
            if (!visited_v[v_id]) embedding_cnt += 1;
        }
        return embedding_cnt;
    }
    
    else {

        ui no_conflict_cnt = 0;  
        for(int i = 0; i < valid_cans_count[u_shell]; i++) {
            VertexID v_id = valid_cans[u_shell][i];

            if(!visited_v[v_id]) {
                
                if (counter[v_id] == 0 || counter[v_id] < depth) {
                    no_conflict_cnt += 1;
                
                } else {
                    if (counter[v_id] < depth + 1) {    
                        no_conflict_cnt += 1;
                    } else {
                        visited_v[v_id] = true;
                        embedding_cnt += computeKSSEmbeddingOpt1Impl(depth+1, shell_num, shell, valid_cans, valid_cans_count, counter, visited_v);
                        visited_v[v_id] = false;
                    }
                }
            }
        }

        
        embedding_cnt += no_conflict_cnt * computeKSSEmbeddingOpt1Impl(depth+1, shell_num, shell, valid_cans, valid_cans_count, counter, visited_v);

        return embedding_cnt;
    }
}



size_t 
EvaluateQuery::computeKSSEmbeddingOpt2(ui shell_num, ui* shell, ui** valid_cans, ui* valid_cans_count) {
    
    std::unordered_map<VertexID, ui> counter;
    std::set<VertexID> * used = new std::set<VertexID> [shell_num];
    std::map<std::set<VertexID>, size_t> * memorized_table = new std::map<std::set<VertexID>, size_t> [shell_num];

    
    for (int i = 0; i < shell_num; i++) {
        VertexID u_shell = shell[i];
        for (int j = 0; j < valid_cans_count[u_shell]; j++) {
            VertexID v_id = valid_cans[u_shell][j];
            if (counter.count(v_id) == 0) {  
                counter[v_id] = 0;  
            } else {    
                counter[v_id] = i;    
            }
        }
    }

    return computeKSSEmbeddingOpt2Impl(0, shell_num, shell, valid_cans, valid_cans_count, counter, used, memorized_table);
}

size_t 
EvaluateQuery::computeKSSEmbeddingOpt2Impl(ui depth, ui shell_num, ui* shell, ui** valid_cans, ui* valid_cans_count,
                                      std::unordered_map<VertexID, ui> & counter,
                                      std::set<VertexID> * used,    
                                      std::map<std::set<VertexID>, size_t> * memorized_table) {
    
    std::map<std::set<VertexID>, size_t> cur_table = memorized_table[depth];

    std::set<VertexID> & cur_used = used[depth];
    std::set<VertexID> & nxt_used = used[depth+1];

    if (cur_table.count(cur_used)) {
        return cur_table[cur_used];
    }

    
    size_t embedding_cnt = 0;
    VertexID u_shell = shell[depth];

    
    if (depth == shell_num - 1) {
        size_t dup = 0;
        return valid_cans_count[u_shell] - cur_used.size();
    }
    
    else {

        
        nxt_used.clear();
        for(auto & v_used: cur_used){
            if (counter[v_used] > depth) {
                nxt_used.insert(v_used);
            }
        }

        ui no_conflict_cnt = 0;  
        for(int i = 0; i < valid_cans_count[u_shell]; i++) {
            VertexID v_id = valid_cans[u_shell][i];
            
            if (counter[v_id] == 0 || counter[v_id] < depth) {
                no_conflict_cnt += 1;
            
            } else {
                if(cur_used.find(v_id) == cur_used.end()){
                    if (counter[v_id] == depth) {
                        no_conflict_cnt += 1;
                    } else {
                        nxt_used.insert(v_id);
                        embedding_cnt += computeKSSEmbeddingOpt2Impl(depth+1, shell_num, shell, valid_cans, valid_cans_count, counter, used, memorized_table);
                        nxt_used.erase(v_id);
                    }
                }
            }
        }

        
        embedding_cnt += computeKSSEmbeddingOpt2Impl(depth+1, shell_num, shell, valid_cans, valid_cans_count, counter, used, memorized_table);

        cur_table[cur_used] = embedding_cnt;
        return embedding_cnt;
    }
}
