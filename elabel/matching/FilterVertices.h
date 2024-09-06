



#ifndef SUBGRAPHMATCHING_FILTERVERTICES_H
#define SUBGRAPHMATCHING_FILTERVERTICES_H

#include "graph/graph.h"
#include "utility/relation/catalog.h"
#include <map>
#include <set>
#include <vector>

class FilterVertices {
public:
    static bool LDFFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count);
    static bool NLFFilter(const Graph* data_graph, const Graph* query_graph, ui** &candidates, ui* &candidates_count);
    static bool GQLFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count);
    static bool TSOFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count,
                          ui *&order, TreeNode *&tree);
    static bool CFLFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count,
                              ui *&order, TreeNode *&tree);
    static bool DPisoFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count,
                            ui *&order, TreeNode *&tree);

    static bool CECIFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count,
                           ui *&order, TreeNode *&tree,   std::vector<std::unordered_map<VertexID, std::vector<VertexID >>> &TE_Candidates,
                           std::vector<std::vector<std::unordered_map<VertexID, std::vector<VertexID>>>> &NTE_Candidates);

    static bool VEQFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count,
                          ui *&order, TreeNode *&tree);
    static bool RMFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count,
                         catalog*&storage);
    static bool CaLiGFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count);

    

    static void computeCandidateWithNLF(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                        ui &count, ui *buffer = NULL);

    static void computeCandidateWithLDF(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                        ui &count, ui *buffer = NULL);

    static void generateCandidates(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                      VertexID *pivot_vertices, ui pivot_vertices_count, VertexID **candidates,
                                      ui *candidates_count, ui *flag, ui *updated_flag);

    static void pruneCandidates(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                   VertexID *pivot_vertices, ui pivot_vertices_count, VertexID **candidates,
                                   ui *candidates_count, ui *flag, ui *updated_flag);

    static void VEQPruneCandidates(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                   VertexID *child_vertices, ui child_vertices_count, VertexID **candidates,
                                   ui *candidates_count, std::vector<std::set<VertexID>>& D);

    static void VEQCompactCandidates(ui** &candidates, ui* &candidates_count, ui query_vertex_num,
                                     std::vector<std::set<VertexID>>& D);

    static void
    printCandidatesInfo(const Graph *query_graph, ui *candidates_count, std::vector<ui> &optimal_candidates_count);

    static void sortCandidates(ui** candidates, ui* candidates_count, ui num);

    static double computeCandidatesFalsePositiveRatio(const Graph *data_graph, const Graph *query_graph, ui **candidates,
                                                          ui *candidates_count, std::vector<ui> &optimal_candidates_count);
    
    static void scan_relation(const Graph *data_graph, const Graph *query_graph, catalog *storage);

    static void generate_preprocess_plan(const Graph *query_graph, uint32_t *degeneracy_ordering, uint32_t *vertices_index,
                              uint non_core_vertices_count, uint32_t *non_core_vertices_parent,
                              uint32_t *non_core_vertices_children, uint32_t *non_core_vertices_children_offset);

    static void eliminate_dangling_tuples(const Graph *data_graph, const Graph *query_graph, catalog *storage,
                               uint32_t *degeneracy_ordering, uint32_t *vertices_index,
                               uint non_core_vertices_count, uint32_t *non_core_vertices_parent,
                               uint32_t *non_core_vertices_children, uint32_t *non_core_vertices_children_offset);
    static edge_relation *get_key_position_in_relation(uint32_t u, uint32_t v, catalog *storage, uint32_t &kp);
    static void CaLiGPruneCandidate(const Graph *data_graph, const Graph *query_graph, VertexID u,
                                    ui** &candidates, ui* &candidates_count);
private:
    static void allocateBuffer(const Graph* data_graph, const Graph* query_graph, ui** &candidates, ui* &candidates_count);
    static bool verifyExactTwigIso(const Graph *data_graph, const Graph *query_graph, ui data_vertex, ui query_vertex,
                                   bool **valid_candidates, int *left_to_right_offset, int *left_to_right_edges,
                                   int *left_to_right_match, int *right_to_left_match, int* match_visited,
                                   int* match_queue, int* match_previous);
    static void compactCandidates(ui** &candidates, ui* &candidates_count, ui query_vertex_num);
    static bool isCandidateSetValid(ui** &candidates, ui* &candidates_count, ui query_vertex_num);
};


#endif 
