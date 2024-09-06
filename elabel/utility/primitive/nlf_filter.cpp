#include "nlf_filter.h"

void nlf_filter::execute(std::vector<std::vector<uint32_t>> &candidate_sets) {
    uint32_t n = query_graph_->getVerticesCount();
    candidate_sets.resize(n);

    for (uint32_t u = 0; u < n; ++u) {
        uint32_t label = query_graph_->getVertexLabel(u);
        uint32_t degree = query_graph_->getVertexDegree(u);
#if OPTIMIZED_VLABELED_GRAPH == 1
        auto u_nlf = query_graph_->getVertexNLF(u);
#endif

        uint32_t data_vertex_num;
        const uint32_t* data_vertices = data_graph_->getVerticesByLabel(label, data_vertex_num);
        auto& candidate_set = candidate_sets[u];

        for (uint32_t j = 0; j < data_vertex_num; ++j) {
            uint32_t v = data_vertices[j];
            if (data_graph_->getVertexDegree(v) >= degree) {

                
#if OPTIMIZED_VLABELED_GRAPH == 1
                auto v_nlf = data_graph_->getVertexNLF(v);

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
                        candidate_set.push_back(v);
                    }
                }
#endif
            }
        }
    }
}

void nlf_filter::execute(catalog *storage) {
    for (uint32_t u = 0; u < query_graph_->getVerticesCount(); ++u) {
        uint32_t u_nbrs_cnt;
        const uint32_t* u_nbrs = query_graph_->getVertexNeighbors(u, u_nbrs_cnt);
        uint32_t uu = u_nbrs[u_nbrs_cnt - 1];

        if (u < uu) {
            filter_ordered_relation(u, &storage->edge_relations_[u][uu]);
        }
        else {
            filter_unordered_relation(u, &storage->edge_relations_[uu][u]);
        }
    }
}

void nlf_filter::filter_ordered_relation(uint32_t u, edge_relation *relation) {
    uint32_t u_deg = query_graph_->getVertexDegree(u);

#if OPTIMIZED_VLABELED_GRAPH == 1
    auto u_nlf = query_graph_->getVertexNLF(u);
#endif

    uint32_t valid_edge_count = 0;
    uint32_t prev_v = std::numeric_limits<uint32_t>::max();
    bool add = false;

    for (uint32_t i = 0; i < relation->size_; ++i) {
        uint32_t v = relation->edges_[i].vertices_[0];

        if (v != prev_v) {
            prev_v = v;
            add = false;
            uint32_t v_deg = data_graph_->getVertexDegree(v);

            if (v_deg >= u_deg) {
                add = true;
#if OPTIMIZED_VLABELED_GRAPH == 1
                auto v_nlf = data_graph_->getVertexNLF(v);
                if (v_nlf->size() >= u_nlf->size()) {
#ifdef ELABELED_GRAPH
                    for (auto& velement : *u_nlf) {
                        auto viter = v_nlf->find(velement.first);
                        if (viter == v_nlf->end() || viter->second.size() < velement.second.size()) {
                            add = false;
                            break;
                        }
                        for (auto& eelement : velement.second) {
                            auto eiter = viter->second.find(eelement.first);
                            if (eiter == viter->second.end() || eiter->second < eelement.second) {
                                add = false;
                                break;
                            }
                        }
                        if (add == false)
                            break;
                    }
                }
#else
                    for (auto element : *u_nlf) {
                        auto iter = v_nlf->find(element.first);
                        if (iter == v_nlf->end() || iter->second < element.second) {
                            add = false;
                            break;
                        }
                    }
                }
#endif

#endif
            }
        }

        if (add) {
            if(valid_edge_count != i)
                relation->edges_[valid_edge_count] = relation->edges_[i];
            valid_edge_count += 1;
        }
    }

    relation->size_ = valid_edge_count;
}

void nlf_filter::filter_unordered_relation(uint32_t u, edge_relation *relation) {
    uint32_t u_deg = query_graph_->getVertexDegree(u);

#if OPTIMIZED_VLABELED_GRAPH == 1
    auto u_nlf = query_graph_->getVertexNLF(u);
#endif

    uint32_t valid_edge_count = 0;
    for (uint32_t i = 0; i < relation->size_; ++i) {
        uint32_t v = relation->edges_[i].vertices_[1];

        
        if (status_[v] == 'u') {
            status_[v] = 'r';
            updated_.push_back(v);
            uint32_t v_deg = data_graph_->getVertexDegree(v);
            if (v_deg >= u_deg) {
                status_[v] = 'a';
#if OPTIMIZED_VLABELED_GRAPH == 1
                auto v_nlf = data_graph_->getVertexNLF(v);
                if (v_nlf->size() >= u_nlf->size()) {
#ifdef ELABELED_GRAPH
                    for (auto& velement : *u_nlf) {
                        auto viter = v_nlf->find(velement.first);
                        if (viter == v_nlf->end() || viter->second.size() < velement.second.size()) {
                            status_[v] = 'r';
                            break;
                        }
                        for (auto& eelement : velement.second) {
                            auto eiter = viter->second.find(eelement.first);
                            if (eiter == viter->second.end() || eiter->second < eelement.second) {
                                status_[v] = 'r';
                                break;
                            }
                        }
                        if (status_[v] == 'r')
                            break;
                    }
                }
#else
                    for (auto element : *u_nlf) {
                        auto iter = v_nlf->find(element.first);
                        if (iter == v_nlf->end() || iter->second < element.second) {
                            status_[v] = 'r';
                            break;
                        }
                    }
                }
#endif

#endif
            }
        }

        if (status_[v] == 'a') {
            if (valid_edge_count != i)
                relation->edges_[valid_edge_count] = relation->edges_[i];
            valid_edge_count += 1;
        }
    }

    for (auto v : updated_)
        status_[v] = 'u';
    updated_.clear();

    relation->size_ = valid_edge_count;
}
