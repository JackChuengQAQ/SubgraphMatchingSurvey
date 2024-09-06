



#ifndef SUBGRAPHMATCHING_GRAPH_H
#define SUBGRAPHMATCHING_GRAPH_H

#include <unordered_map>
#include <iostream>
#include <vector>
#include "utility/sparsepp/spp.h"
#include "configuration/types.h"
#include "configuration/config.h"


using spp::sparse_hash_map;
class Graph {
private:
    bool enable_vlabel_offset_;

    ui vertices_count_;
    ui edges_count_;
    ui vlabels_count_;
    ui max_degree_;
    ui max_vlabel_frequency_;

    ui* offsets_;
    VertexID * neighbors_;
    LabelID* vlabels_;
    ui* reverse_index_offsets_;
    ui* reverse_index_;

    int* core_table_;
    ui core_length_;

    std::unordered_map<LabelID, ui> vlabels_frequency_;
    sparse_hash_map<uint64_t, std::vector<edge>* >* edge_index_;

#if OPTIMIZED_VLABELED_GRAPH == 1
    ui* vlabels_offsets_;

#ifdef ELABELED_GRAPH
    
    std::unordered_map<LabelID, std::unordered_map<LabelID, ui>>*nlf_;
#else
    
    std::unordered_map<LabelID, ui>* nlf_;
#endif

#endif

#ifdef ELABELED_GRAPH
    ui elabels_count_;
    ui* elabels_;
#endif

private:
    void BuildReverseIndex();

#if OPTIMIZED_VLABELED_GRAPH == 1
    void BuildNLF();
    void BuildVLabelOffset();
#endif

public:
    Graph(const bool enable_label_offset) {
        enable_vlabel_offset_ = enable_label_offset;

        vertices_count_ = 0;
        edges_count_ = 0;
        vlabels_count_ = 0;
        max_degree_ = 0;
        max_vlabel_frequency_ = 0;
        core_length_ = 0;

        offsets_ = NULL;
        neighbors_ = NULL;
        vlabels_ = NULL;
        reverse_index_offsets_ = NULL;
        reverse_index_ = NULL;
        core_table_ = NULL;
        vlabels_frequency_.clear();
        edge_index_ = NULL;
#if OPTIMIZED_VLABELED_GRAPH == 1
        vlabels_offsets_ = NULL;
        nlf_ = NULL;
#endif

#ifdef ELABELED_GRAPH
        elabels_count_ = 0;
        elabels_ = NULL;
#endif
    }

    ~Graph() {
        delete[] offsets_;
        delete[] neighbors_;
        delete[] vlabels_;
        delete[] reverse_index_offsets_;
        delete[] reverse_index_;
        delete[] core_table_;
        delete edge_index_;
#if OPTIMIZED_VLABELED_GRAPH == 1
        delete[] vlabels_offsets_;
        delete[] nlf_;
#endif

#ifdef ELABELED_GRAPH
        
        delete[] elabels_;
#endif
    }

public:
    
    void loadGraphFromFile(const std::string& file_path);
    
    void loadGraphFromFileCompressed(const std::string& degree_path, const std::string& edge_path,
                                     const std::string& label_path);
    
    void storeComparessedGraph(const std::string& degree_path, const std::string& edge_path,
                               const std::string& label_path);
    void printGraphMetaData();
public:
    const ui getLabelsCount() const {
        return vlabels_count_;
    }

    const ui getVerticesCount() const {
        return vertices_count_;
    }

    const ui getEdgesCount() const {
        return edges_count_;
    }

    const ui getGraphMaxDegree() const {
        return max_degree_;
    }

    const ui getGraphMaxLabelFrequency() const {
        return max_vlabel_frequency_;
    }

    const ui getVertexDegree(const VertexID id) const {
        return offsets_[id + 1] - offsets_[id];
    }

    const ui getLabelsFrequency(const LabelID label) const {
        return vlabels_frequency_.find(label) == vlabels_frequency_.end() ? 0 : vlabels_frequency_.at(label);
    }

    const ui getCoreValue(const VertexID id) const {
        return core_table_[id];
    }

    const ui get2CoreSize() const {
        return core_length_;
    }
    const LabelID getVertexLabel(const VertexID id) const {
        return vlabels_[id];
    }

    const ui * getVertexNeighbors(const VertexID id, ui& count) const {
        count = offsets_[id + 1] - offsets_[id];
        return neighbors_ + offsets_[id];
    }

    const sparse_hash_map<uint64_t, std::vector<edge>*>* getEdgeIndex() const {
        return edge_index_;
    }

    const ui * getVerticesByLabel(const LabelID id, ui& count) const {
        count = reverse_index_offsets_[id + 1] - reverse_index_offsets_[id];
        return reverse_index_ + reverse_index_offsets_[id];
    }

#if OPTIMIZED_VLABELED_GRAPH == 1
    const ui * getNeighborsByLabel(const VertexID id, const LabelID label, ui& count) const {
        ui offset = id * vlabels_count_ + label;
        count = vlabels_offsets_[offset + 1] - vlabels_offsets_[offset];
        return neighbors_ + vlabels_offsets_[offset];
    }

#ifdef ELABELED_GRAPH
    const std::unordered_map<LabelID, std::unordered_map<LabelID, ui>>* getVertexNLF(const VertexID id) const {
        return nlf_ + id;
    }
    bool checkEdgeExistence(const VertexID u, const VertexID v, const LabelID u_label, const LabelID e_label) const {
        ui count = 0;
        const VertexID* neighbors = getNeighborsByLabel(v, u_label, count);
        int begin = 0;
        int end = count - 1;
        while (begin <= end) {
            int mid = begin + ((end - begin) >> 1);
            if (neighbors[mid] == u) {
                if (e_label == (elabels_+offsets_[v])[mid])
                    return true;
                else
                    return false;
            }
            else if (neighbors[mid] > u)
                end = mid - 1;
            else
                begin = mid + 1;
        }

        return false;
    }
#else
    const std::unordered_map<LabelID, ui>* getVertexNLF(const VertexID id) const {
        return nlf_ + id;
    }
    bool checkEdgeExistence(const VertexID u, const VertexID v, const LabelID u_label) const {
        ui count = 0;
        const VertexID* neighbors = getNeighborsByLabel(v, u_label, count);
        int begin = 0;
        int end = count - 1;
        while (begin <= end) {
            int mid = begin + ((end - begin) >> 1);
            if (neighbors[mid] == u) {
                return true;
            }
            else if (neighbors[mid] > u)
                end = mid - 1;
            else
                begin = mid + 1;
        }

        return false;
    }
#endif

#endif

#ifdef ELABELED_GRAPH
    
    LabelID getEdgeLabel(VertexID s, VertexID e, bool non_sense) const {
        ui nbrs_num = 0;
        const ui* nbrs = getVertexNeighbors(s, nbrs_num);
        for (ui i = 0; i < nbrs_num; i++) {
            if (nbrs[i] == e) {
                return (elabels_+offsets_[s])[i];
            }
        }
        return (LabelID)-1;
    }
    LabelID getEdgeLabelCheck(VertexID s, VertexID e, ui offset) const {
        if ((neighbors_+offsets_[s])[offset] == e) {
            return (elabels_+offsets_[s])[offset];
        } else {
            std::cout << "error in edge label check" << std::endl;
        }
    }
    
    inline LabelID getEdgeLabel(VertexID s, ui offset) const {
        return (elabels_+offsets_[s])[offset];
    }
    
    const ui * getVertexEdgeLabels(const VertexID id, ui& count) const {
        count = offsets_[id + 1] - offsets_[id];
        return elabels_ + offsets_[id];
    }

    bool checkEdgeExistence(VertexID u, VertexID v, const LabelID e_label) const {
        if (getVertexDegree(u) < getVertexDegree(v)) {
            std::swap(u, v);
        }
        ui count = 0;
        const VertexID* neighbors =  getVertexNeighbors(v, count);

        int begin = 0;
        int end = count - 1;
        while (begin <= end) {
            int mid = begin + ((end - begin) >> 1);
            if (neighbors[mid] == u) {
                if (e_label == (elabels_+offsets_[v])[mid])
                    return true;
                else
                    return false;
            }
            else if (neighbors[mid] > u)
                end = mid - 1;
            else
                begin = mid + 1;
        }

        return false;
    }

#endif

    bool checkEdgeExistence(VertexID u, VertexID v) const {
        if (getVertexDegree(u) < getVertexDegree(v)) {
            std::swap(u, v);
        }
        ui count = 0;
        const VertexID* neighbors =  getVertexNeighbors(v, count);

        int begin = 0;
        int end = count - 1;
        while (begin <= end) {
            int mid = begin + ((end - begin) >> 1);
            if (neighbors[mid] == u) {
                return true;
            }
            else if (neighbors[mid] > u)
                end = mid - 1;
            else
                begin = mid + 1;
        }

        return false;
    }

    void buildCoreTable();

    void buildEdgeIndex();
};


#endif 
