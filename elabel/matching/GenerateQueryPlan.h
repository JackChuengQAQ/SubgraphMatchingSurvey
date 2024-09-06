



#ifndef SUBGRAPHMATCHING_GENERATEQUERYPLAN_H
#define SUBGRAPHMATCHING_GENERATEQUERYPLAN_H

#include "graph/graph.h"
#include "utility/relation/catalog.h"
#include "utility/nucleus_decomposition/nd_interface.h"
#include <vector>
#include <unordered_set>
class GenerateQueryPlan {
public:
    static void generateGQLQueryPlan(const Graph *data_graph, const Graph *query_graph, ui *candidates_count,
                                         ui *&order, ui *&pivot);
    static void generateQSIQueryPlan(const Graph *data_graph, const Graph *query_graph, Edges ***edge_matrix,
                                         ui *&order, ui *&pivot);

    static void generateRIQueryPlan(const Graph *data_graph, const Graph* query_graph, ui *&order, ui *&pivot);

    static void generateVF2PPQueryPlan(const Graph* data_graph, const Graph *query_graph, ui *&order, ui *&pivot);

    static void generateVF3QueryPlan(const Graph* data_graph, const Graph *query_graph, ui *&order, ui *&pivot);

    static void generateRMQueryPlan(const Graph *query_graph, ui *&order, Edges ***edge_matrix, ui *&pivot);

    static void computeProbability(const Graph* data_graph, const Graph* query_graph, double *prob);

    static void generateOrderSpectrum(const Graph* query_graph, std::vector<std::vector<ui>>& spectrum, ui num_spectrum_limit);

    static void
    generateTSOQueryPlan(const Graph *query_graph, Edges ***edge_matrix, ui *&order, ui *&pivot,
                             TreeNode *tree, ui *dfs_order);

    static void
    generateCFLQueryPlan(const Graph *data_graph, const Graph *query_graph, Edges ***edge_matrix,
                             ui *&order, ui *&pivot, TreeNode *tree, ui *bfs_order, ui *candidates_count);

    static void
    generateDSPisoQueryPlan(const Graph *query_graph, Edges ***edge_matrix, ui *&order, ui *&pivot,
                                TreeNode *tree, ui *bfs_order, ui *candidates_count, ui **&weight_array);

    static void generateCECIQueryPlan(const Graph* query_graph, TreeNode *tree, ui *bfs_order, ui *&order, ui *&pivot);
    static void checkQueryPlanCorrectness(const Graph* query_graph, ui* order, ui* pivot);

    static void checkQueryPlanCorrectness(const Graph* query_graph, ui* order);

    static void printQueryPlan(const Graph* query_graph, ui* order);

    static void printSimplifiedQueryPlan(const Graph* query_graph, ui* order);
private:
    static VertexID selectGQLStartVertex(const Graph *query_graph, ui *candidates_count);
    static std::pair<VertexID, VertexID> selectQSIStartEdge(const Graph *query_graph, Edges ***edge_matrix);

    static void generateRootToLeafPaths(TreeNode *tree_node, VertexID cur_vertex, std::vector<ui> &cur_path,
                                        std::vector<std::vector<ui>> &paths);
    static void estimatePathEmbeddsingsNum(std::vector<ui> &path, Edges ***edge_matrix,
                                           std::vector<size_t> &estimated_embeddings_num);

    static void generateCorePaths(const Graph* query_graph, TreeNode* tree_node, VertexID cur_vertex, std::vector<ui> &cur_core_path,
                                  std::vector<std::vector<ui>> &core_paths);

    static void generateTreePaths(const Graph* query_graph, TreeNode* tree_node, VertexID cur_vertex,
                                  std::vector<ui> &cur_tree_path, std::vector<std::vector<ui>> &tree_paths);

    static void generateLeaves(const Graph* query_graph, std::vector<ui>& leaves);

    static ui generateNoneTreeEdgesCount(const Graph *query_graph, TreeNode *tree_node, std::vector<ui> &path);
    static void updateValidVertices(const Graph* query_graph, VertexID query_vertex, std::vector<bool>& visited,
                                    std::vector<bool>& adjacent);

    static void construct_density_tree(std::vector<nd_tree_node> &density_tree,
                                       std::vector<nd_tree_node> &k12_tree,
                                       std::vector<nd_tree_node> &k23_tree,
                                       std::vector<nd_tree_node> &k34_tree);

    static void eliminate_node(std::vector<nd_tree_node> &density_tree,
                               std::vector<nd_tree_node> &src_tree,
                               std::vector<nd_tree_node> &target_tree);

    static void merge_tree(std::vector<nd_tree_node> &density_tree);

    static void traversal_density_tree(const Graph *query_graph, Edges***edge_matrix,
                                       std::vector<nd_tree_node> &density_tree,
                                       std::vector<std::vector<uint32_t>> &vertex_orders,
                                       std::vector<std::vector<uint32_t>> &node_orders);

    static void traversal_node(const Graph *query_graph, Edges***edge_matrix, std::vector<nd_tree_node> &density_tree,
                               std::vector<uint32_t> &vertex_order, std::vector<uint32_t> &node_order,
                               vector<bool> &visited_vertex, std::vector<bool> &visited_node, nd_tree_node &cur_node,
                               std::unordered_set<uint32_t> &extendable_vertex);

    static double connectivity_common_neighbors(std::vector<bool> &visited_vertex, nd_tree_node &child_node);

    static void connectivity_shortest_path(const Graph *query_graph, Edges***edge_matrix,
                                           std::vector<uint32_t> &vertex_order,
                                           std::vector<bool> &visited_vertex, nd_tree_node &cur_node,
                                           std::vector<uint32_t> &prev, std::vector<double> &dist);
    static void update_extendable_vertex(const Graph *query_graph, uint32_t u,
                                         std::unordered_set<uint32_t> &extendable_vertex,
                                         vector<bool> &visited_vertex);
    static void greedy_expand(const Graph *query_graph, Edges***edge_matrix, std::vector<uint32_t> &vertex_order,
                              std::vector<bool> &visited_vertex,
                              std::unordered_set<uint32_t> &extendable_vertex,
                              uint32_t bn_cnt_threshold);
    static void select_vertex_order(const Graph *query_graph, std::vector<std::vector<uint32_t>> &vertex_orders,
                                    std::vector<std::vector<uint32_t>> &node_orders);
    static uint32_t query_plan_utility_value(const Graph *query_graph, std::vector<uint32_t> &vertex_order,
                                             std::vector<uint32_t> &bn_cn_list);
};


#endif 
