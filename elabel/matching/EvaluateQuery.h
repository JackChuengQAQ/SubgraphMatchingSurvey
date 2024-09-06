



#ifndef SUBGRAPHMATCHING_EVALUATEQUERY_H
#define SUBGRAPHMATCHING_EVALUATEQUERY_H

#include "graph/graph.h"
#include "utility/QFilter.h"
#include "utility/primitive/projection.h"
#include "utility/relation/catalog.h"
#include <vector>
#include <queue>
#include <unordered_set>
#include <bitset>



class EvaluateQuery {
public:
    // Min priority queue.
    static std::function<bool(std::pair<std::pair<VertexID, ui>, ui>, std::pair<std::pair<VertexID, ui>, ui>)>
      extendable_vertex_compare;

    typedef std::priority_queue<std::pair<std::pair<VertexID, ui>, ui>, std::vector<std::pair<std::pair<VertexID, ui>, ui>>,
        decltype(EvaluateQuery::extendable_vertex_compare)> dpiso_min_pq;


    static size_t exploreGraph(const Graph *data_graph, const Graph *query_graph, Edges ***edge_matrix, ui **candidates,
                                  ui *candidates_count, ui *order, ui *pivot, size_t output_limit_num, size_t &call_count);

    static size_t LFTJ(const Graph *data_graph, const Graph *query_graph, Edges ***edge_matrix, ui **candidates, ui *candidates_count,
                           ui *order, size_t output_limit_num, size_t &call_count, size_t &valid_vtx_cnt);

    static size_t
    exploreGraphQLStyle(const Graph *data_graph, const Graph *query_graph, ui **candidates, ui *candidates_count, ui *order,
                            size_t output_limit_num, size_t &call_count);

    static size_t
    exploreQuickSIStyle(const Graph *data_graph, const Graph *query_graph, ui **candidates, ui *candidates_count, ui *order,
                            ui *pivot, size_t output_limit_num, size_t &call_count);

    static size_t
    exploreVF3Style(const Graph *data_graph, const Graph *query_graph, ui **candidates, ui *candidates_count, ui *order,
                            ui *pivot, size_t output_limit_num, size_t &call_count);

    static size_t exploreDPisoStyle(const Graph *data_graph, const Graph *query_graph, TreeNode *tree,
                                    Edges ***edge_matrix, ui **candidates, ui *candidates_count,
                                    ui **weight_array, ui *order, size_t output_limit_num,
                                    size_t &call_count);

    static size_t exploreVEQStyle(const Graph *data_graph, const Graph *query_graph, TreeNode *tree,
                                    Edges ***edge_matrix, ui **candidates, ui *candidates_count,
                                    size_t output_limit_num, size_t &call_count);

    static size_t exploreDPisoRecursiveStyle(const Graph *data_graph, const Graph *query_graph, TreeNode *tree,
                                             Edges ***edge_matrix, ui **candidates, ui *candidates_count,
                                             ui **weight_array, ui *order, size_t output_limit_num,
                                             size_t &call_count);

    static size_t exploreCECIStyle(const Graph *data_graph, const Graph *query_graph, TreeNode *tree, ui **candidates,
                                      ui *candidates_count,
                                      std::vector<std::unordered_map<VertexID, std::vector<VertexID>>> &TE_Candidates,
                                      std::vector<std::vector<std::unordered_map<VertexID, std::vector<VertexID>>>> &NTE_Candidates,
                                      ui *order, size_t &output_limit_num, size_t &call_count);

    static size_t exploreRMStyle(const Graph *query_graph, const Graph *data_graph, catalog*&storage, Edges***edge_matrix, ui **candidates,
                                 ui *candidates_count, ui *order, size_t output_limit_num, size_t &call_count);

    static size_t exploreKSSStyle(const Graph *query_graph, const Graph *data_graph, Edges***edge_matrix, ui **candidates, ui *candidates_count,
                                 ui *order, size_t output_limit_num, size_t &call_count);


#if ENABLE_QFLITER == 1
    static BSRGraph*** qfliter_bsr_graph_;
    static int* temp_bsr_base1_;
    static int* temp_bsr_state1_;
    static int* temp_bsr_base2_;
    static int* temp_bsr_state2_;
#endif

#ifdef SPECTRUM
    static bool exit_;
#endif

#ifdef DISTRIBUTION
    static size_t* distribution_count_;
#endif
private:
    static void generateBN(const Graph *query_graph, ui *order, ui *pivot, ui **&bn, ui *&bn_count);
    static void generateBN(const Graph *query_graph, ui *order, ui **&bn, ui *&bn_count);
    static void allocateBuffer(const Graph *query_graph, const Graph *data_graph, ui *candidates_count, ui *&idx,
                                   ui *&idx_count, ui *&embedding, ui *&idx_embedding, ui *&temp_buffer,
                                   ui **&valid_candidate_idx, bool *&visited_vertices);
    static void releaseBuffer(ui query_vertices_num, ui *idx, ui *idx_count, ui *embedding, ui *idx_embedding,
                                  ui *temp_buffer, ui **valid_candidate_idx, bool *visited_vertices, ui **bn, ui *bn_count);

    static void generateValidCandidateIndex(const Graph *data_graph, ui depth, ui *embedding, ui *idx_embedding,
                                            ui *idx_count, ui **valid_candidate_index, Edges ***edge_matrix,
                                            bool *visited_vertices, ui **bn, ui *bn_cnt, ui *order, ui *pivot,
                                            ui **candidates, const Graph *query_graph);

    static void generateValidCandidateIndex(ui depth, ui *idx_embedding, ui *idx_count, ui **valid_candidate_index,
                                                Edges ***edge_matrix, ui **bn, ui *bn_cnt, ui *order, ui *&temp_buffer);

    static void generateValidCandidates(const Graph* data_graph, ui depth, ui* embedding, ui* idx_count, ui** valid_candidate,
                                        bool* visited_vertices, ui **bn, ui *bn_cnt, ui* order, ui **candidates, ui* candidates_count,
                                        const Graph *query_graph);

    static void generateValidCandidates(const Graph *query_graph, const Graph *data_graph, ui depth, ui *embedding,
                                            ui *idx_count, ui **valid_candidate, bool *visited_vertices, ui **bn, ui *bn_cnt,
                                            ui *order, ui *pivot);
    static void generateValidCandidates(ui depth, ui *embedding, ui *idx_count, ui **valid_candidates, ui *order,
                                            ui *&temp_buffer, TreeNode *tree,
                                            std::vector<std::unordered_map<VertexID, std::vector<VertexID>>> &TE_Candidates,
                                            std::vector<std::vector<std::unordered_map<VertexID, std::vector<VertexID>>>> &NTE_Candidates);

    static void updateExtendableVertex(ui *idx_embedding, ui *idx_count, ui **valid_candidate_index,
                                          Edges ***edge_matrix, ui *&temp_buffer, ui **weight_array,
                                          TreeNode *tree, VertexID mapped_vertex, ui *extendable,
                                          std::vector<dpiso_min_pq> &vec_rank_queue, const Graph *query_graph);

    static void restoreExtendableVertex(TreeNode* tree, VertexID unmapped_vertex, ui *extendable);
    static void generateValidCandidateIndex(VertexID vertex, ui *idx_embedding, ui *idx_count, ui *&valid_candidate_index,
                                            Edges ***edge_matrix, ui *bn, ui bn_cnt, ui *&temp_buffer);

    static void computeAncestor(const Graph *query_graph, TreeNode *tree, VertexID *order,
                                std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> &ancestors);

    static void computeAncestor(const Graph *query_graph, ui** bn, ui* bn_cnt, VertexID *order,
                                std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> &ancestors);

    static void computeAncestor(const Graph *query_graph, VertexID *order, std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> &ancestors);

    static void generateFeasibility(const Graph *graph, ui v, bool* matched, ui level, ui*** feasibility, ui**feasibility_count);
    static void generateFeasibility(const Graph *query_graph, const ui *order, ui*** feasibility, ui**feasibility_count);
    static bool isFeasibility(const Graph *data_graph, const Graph *query_graph, ui depth, ui cur_u, ui cur_v,
                              ui *embedding, ui* order, ui*** query_feasibility, ui**query_feasibility_count,
                              ui*** data_feasibility, ui**data_feasibility_count);

    static bool exploreVF3Backtrack(const Graph *data_graph, const Graph *query_graph, ui *order, ui *pivot,
                                    size_t output_limit_num, size_t &call_count, ui& embedding_cnt,
                                    ui depth, ui max_depth, ui*idx, ui*idx_count, ui *embedding,
                                    ui **valid_candidates, bool* visited_vertices,
                                    ui*** query_feasibility, ui**query_feasibility_count,
                                    ui*** data_feasibility, ui**data_feasibility_count);

    static void RestoreValidCans(const Graph *query_graph, const Graph *data_graph, bool* visited_u,
                                                VertexID last_v, VertexID last_u,
                                                std::vector<std::unordered_map<VertexID, ui>>& valid_cans);
    static void computeNEC(const Graph *query_graph, ui** nec);
    static void computeNEC(const Graph *query_graph, Edges ***edge_matrix, ui *candidates_count, ui** candidates,
                           std::vector<std::vector<ui>>& vec_index,
                           std::vector<std::vector<ui>>& vec_set);
    static ui generateNextU(const Graph *data_graph, const Graph *query_graph, ui **candidates, ui *candidates_count, ui**valid_cans,
                            ui*valid_cans_count, ui* extendable, ui** nec, ui depth, ui* embedding,
                            Edges ***edge_matrix, bool *visited_vertices, bool *visited_u, ui *order, 
                            TreeNode* tree);
    static void ComputeValidCans(const Graph *data_graph, const Graph *query_graph, ui **candidates, ui *candidates_count,
                      ui**valid_cans, ui*valid_cans_count, ui* embedding, VertexID u, bool* visited_u);

    static void ComputeValidCans(const Graph *data_graph, const Graph *query_graph, ui **candidates, ui *candidates_count,
                      ui**valid_cans, ui*valid_cans_count, ui* embedding, VertexID u, bool* visited_u, bool * visited_v);

    static size_t computeKSSEmbeddingNaive(ui shell_num, ui* shell, ui** valid_cans, ui* valid_cans_count, bool * visited_v);
    static size_t computeKSSEmbeddingOpt1(ui shell_num, ui* shell, ui** valid_cans, ui* valid_cans_count, bool * visited_v);
    static size_t computeKSSEmbeddingOpt2(ui shell_num, ui* shell, ui** valid_cans, ui* valid_cans_count);

    static size_t computeKSSEmbeddingNaiveImpl(ui depth, ui shell_num, ui* shell, ui** valid_cans, ui* valid_cans_count, bool * visited_v);
    static size_t computeKSSEmbeddingOpt1Impl(ui depth, ui shell_num, ui* shell, ui** valid_cans, ui* valid_cans_count, 
                                         std::unordered_map<VertexID, ui> & counter,
                                         bool * visited_v);
    static size_t computeKSSEmbeddingOpt2Impl(ui depth, ui shell_num, ui* shell, ui** valid_cans, ui* valid_cans_count,
                                      std::unordered_map<VertexID, ui> & counter,
                                      std::set<VertexID> * used,
                                      std::map<std::set<VertexID>, size_t> * memory_table);

    static std::bitset<MAXIMUM_QUERY_GRAPH_SIZE> exploreDPisoBacktrack(ui max_depth, ui depth, VertexID mapped_vertex, TreeNode *tree, ui *idx_embedding,
                                                     ui *embedding, std::unordered_map<VertexID, VertexID> &reverse_embedding,
                                                     bool *visited_vertices, ui *idx_count, ui **valid_candidate_index,
                                                     Edges ***edge_matrix,
                                                     std::vector<std::bitset<MAXIMUM_QUERY_GRAPH_SIZE>> &ancestors,
                                                     dpiso_min_pq rank_queue, ui **weight_array, ui *&temp_buffer, ui *extendable,
                                                     ui **candidates, size_t &embedding_count, size_t &call_count,
                                                     const Graph *query_graph);

    static void convertCans2Catalog(const Graph *query_graph, ui **candidates, Edges ***edge_matrix, catalog *storage);

    static void convert_to_encoded_relation(catalog *storage, ui *order);

    static void convert_to_encoded_relation(catalog *storage, uint32_t u, uint32_t v);

    static void convert_to_hash_relation(catalog *storage, uint32_t u, uint32_t v);

    static void convert_encoded_relation_to_sparse_bitmap(catalog *storage, ui*order);

    static void updateShell2Kernel(const Graph *query_graph, VertexID u, ui* shell2kernel, bool* kos, std::vector<VertexID> & update);

    static void restoreShell2Kernel(const Graph *query_graph, VertexID u, ui* shell2kernel, bool* kos);
};


#endif 
