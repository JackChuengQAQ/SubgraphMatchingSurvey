



#include "graph.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <utility/graphoperations.h>

void Graph::BuildReverseIndex() {
    reverse_index_ = new ui[vertices_count_];
    reverse_index_offsets_= new ui[vlabels_count_ + 1];
    reverse_index_offsets_[0] = 0;

    ui total = 0;
    for (ui i = 0; i < vlabels_count_; ++i) {
        reverse_index_offsets_[i + 1] = total;
        total += vlabels_frequency_[i];
    }

    for (ui i = 0; i < vertices_count_; ++i) {
        LabelID vlabel = vlabels_[i];
        reverse_index_[reverse_index_offsets_[vlabel + 1]++] = i;
    }
}

#if OPTIMIZED_VLABELED_GRAPH == 1

#ifdef ELABELED_GRAPH
void Graph::BuildNLF() {
    nlf_ = new std::unordered_map<LabelID, std::unordered_map<LabelID, ui>>[vertices_count_];
    for (ui i = 0; i < vertices_count_; ++i) {
        ui count;
        const VertexID* neighbors = getVertexNeighbors(i, count);
        const LabelID* elabels = getVertexEdgeLabels(i, count);

        for (ui j = 0; j < count; ++j) {
            VertexID u = neighbors[j];
            LabelID vlabel = getVertexLabel(u);
            LabelID elabel = elabels[j];
            auto nlf = nlf_[i].find(vlabel);
            if (nlf == nlf_[i].end() || nlf->second.find(elabel) == nlf->second.end()) {
                nlf_[i][vlabel][elabel] = 1;
            } else {
                nlf_[i][vlabel][elabel]++;
            }
        }
    }
}
#else
void Graph::BuildNLF() {
    nlf_ = new std::unordered_map<LabelID, ui>[vertices_count_];
    for (ui i = 0; i < vertices_count_; ++i) {
        ui count;
        const VertexID * neighbors = getVertexNeighbors(i, count);

        for (ui j = 0; j < count; ++j) {
            VertexID u = neighbors[j];
            LabelID vlabel = getVertexLabel(u);
            nlf_[i][vlabel] += 1;
        }
    }
}
#endif

void Graph::BuildVLabelOffset() {
    size_t vlabels_offset_size = (size_t)vertices_count_ * vlabels_count_ + 1;
    vlabels_offsets_ = new ui[vlabels_offset_size];
    std::fill(vlabels_offsets_, vlabels_offsets_ + vlabels_offset_size, 0);

    for (ui i = 0; i < vertices_count_; ++i) {
        std::sort(neighbors_ + offsets_[i], neighbors_ + offsets_[i + 1],
            [this](const VertexID u, const VertexID v) -> bool {
                return vlabels_[u] == vlabels_[v] ? u < v : vlabels_[u] < vlabels_[v];
            });
    }

    for (ui i = 0; i < vertices_count_; ++i) {
        LabelID previous_vlabel = 0;
        LabelID current_vlabel = 0;

        vlabels_offset_size = i * vlabels_count_;
        vlabels_offsets_[vlabels_offset_size] = offsets_[i];

        for (ui j = offsets_[i]; j < offsets_[i + 1]; ++j) {
            current_vlabel = vlabels_[neighbors_[j]];

            if (current_vlabel != previous_vlabel) {
                for (ui k = previous_vlabel + 1; k <= current_vlabel; ++k) {
                    vlabels_offsets_[vlabels_offset_size + k] = j;
                }
                previous_vlabel = current_vlabel;
            }
        }

        for (ui l = current_vlabel + 1; l <= vlabels_count_; ++l) {
            vlabels_offsets_[vlabels_offset_size + l] = offsets_[i + 1];
        }
    }
}

#endif

void Graph::loadGraphFromFile(const std::string &file_path) {
    std::ifstream infile(file_path);

    if (!infile.is_open()) {
        std::cout << "Can not open the graph file " << file_path << " ." << std::endl;
        exit(-1);
    }

    char type;
    infile >> type >> vertices_count_ >> edges_count_;
    offsets_ = new ui[vertices_count_ +  1];
    offsets_[0] = 0;

    neighbors_ = new VertexID[edges_count_ * 2];
    vlabels_ = new LabelID[vertices_count_];
    vlabels_count_ = 0;
    max_degree_ = 0;

#ifdef ELABELED_GRAPH
    elabels_ = new VertexID[edges_count_ * 2];
    elabels_count_ = 0;
    LabelID max_edge_id = 0;
#endif

    LabelID max_vlabel_id = 0;
    std::vector<ui> neighbors_offset(vertices_count_, 0);

    while (infile >> type) {
        if (type == 'v') { 
            VertexID id;
            LabelID  vlabel;
            ui degree;
            infile >> id >> vlabel >> degree;

            vlabels_[id] = vlabel;
            offsets_[id + 1] = offsets_[id] + degree;

            if (degree > max_degree_) {
                max_degree_ = degree;
            }

            if (vlabels_frequency_.find(vlabel) == vlabels_frequency_.end()) {
                vlabels_frequency_[vlabel] = 0;
                if (vlabel > max_vlabel_id)
                    max_vlabel_id = vlabel;
            }

            vlabels_frequency_[vlabel] += 1;
        }
        else if (type == 'e') { 
            VertexID begin;
            VertexID end;
            infile >> begin >> end;

            ui begin_offset = offsets_[begin] + neighbors_offset[begin];
            neighbors_[begin_offset] = end;

            ui end_offset = offsets_[end] + neighbors_offset[end];
            neighbors_[end_offset] = begin;

            neighbors_offset[begin] += 1;
            neighbors_offset[end] += 1;
#ifdef ELABELED_GRAPH
            LabelID elabel;
            infile >> elabel;
            elabels_[begin_offset] = elabel;
            elabels_[end_offset] = elabel;
            if (elabel > max_edge_id) {
                max_edge_id = elabel;
            }
#endif
        }
    }

    infile.close();
    vlabels_count_ = (ui)vlabels_frequency_.size() > (max_vlabel_id + 1) ? (ui)vlabels_frequency_.size() : max_vlabel_id + 1;
#ifdef ELABELED_GRAPH
    elabels_count_ = max_edge_id+1;
#endif
    for (auto element : vlabels_frequency_) {
        if (element.second > max_vlabel_frequency_) {
            max_vlabel_frequency_ = element.second;
        }
    }

    for (ui i = 0; i < vertices_count_; ++i) {
        std::sort(neighbors_ + offsets_[i], neighbors_ + offsets_[i + 1]);
    }

    BuildReverseIndex();
    buildEdgeIndex();

#if OPTIMIZED_VLABELED_GRAPH == 1
    if (enable_vlabel_offset_) {
        BuildNLF();
        
    }
#endif
}

void Graph::printGraphMetaData() {
    std::cout << "|V|: " << vertices_count_ << ", |E|: " << edges_count_ << ", |\u03A3|: " << vlabels_count_ << std::endl;
    std::cout << "Max Degree: " << max_degree_ << ", Max Label Frequency: " << max_vlabel_frequency_ << std::endl;
#ifdef ELABELED_GRAPH
    std::cout << "#Edge Label: " << elabels_count_ << std::endl;
#endif
}

void Graph::buildCoreTable() {
    core_table_ = new int[vertices_count_];
    GraphOperations::getKCore(this, core_table_);

    for (ui i = 0; i < vertices_count_; ++i) {
        if (core_table_[i] > 1) {
            core_length_ += 1;
        }
    }
}

void Graph::loadGraphFromFileCompressed(const std::string &degree_path, const std::string &edge_path,
                                        const std::string &vlabel_path) {
    std::ifstream deg_file(degree_path, std::ios::binary);

    if (deg_file.is_open()) {
        std::cout << "Open degree file " << degree_path << " successfully." << std::endl;
    }
    else {
        std::cerr << "Cannot open degree file " << degree_path << " ." << std::endl;
        exit(-1);
    }

    auto start = std::chrono::high_resolution_clock::now();
    int int_size;
    deg_file.read(reinterpret_cast<char *>(&int_size), 4);
    deg_file.read(reinterpret_cast<char *>(&vertices_count_), 4);
    deg_file.read(reinterpret_cast<char *>(&edges_count_), 4);

    offsets_ = new ui[vertices_count_ + 1];
    ui* degrees = new unsigned int[vertices_count_];

    deg_file.read(reinterpret_cast<char *>(degrees), sizeof(int) * vertices_count_);


    deg_file.close();
    deg_file.clear();

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Load degree file time: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds" << std::endl;

    std::ifstream adj_file(edge_path, std::ios::binary);

    if (adj_file.is_open()) {
        std::cout << "Open edge file " << edge_path << " successfully." << std::endl;
    }
    else {
        std::cerr << "Cannot open edge file " << edge_path << " ." << std::endl;
        exit(-1);
    }

    start = std::chrono::high_resolution_clock::now();
    size_t neighbors_count = (size_t)edges_count_ * 2;
    neighbors_ = new ui[neighbors_count];

    offsets_[0] = 0;
    for (ui i = 1; i <= vertices_count_; ++i) {
        offsets_[i] = offsets_[i - 1] + degrees[i - 1];
    }

    max_degree_ = 0;

    for (ui i = 0; i < vertices_count_; ++i) {
        if (degrees[i] > 0) {
            if (degrees[i] > max_degree_)
                max_degree_ = degrees[i];
            adj_file.read(reinterpret_cast<char *>(neighbors_ + offsets_[i]), degrees[i] * sizeof(int));
            std::sort(neighbors_ + offsets_[i], neighbors_ + offsets_[i + 1]);
        }
    }

    adj_file.close();
    adj_file.clear();

    delete[] degrees;

    end = std::chrono::high_resolution_clock::now();
    std::cout << "Load adj file time: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds" << std::endl;


    std::ifstream vlabel_file(vlabel_path, std::ios::binary);
    if (vlabel_file.is_open())  {
        std::cout << "Open vlabel file " << vlabel_path << " successfully." << std::endl;
    }
    else {
        std::cerr << "Cannot open vlabel file " << vlabel_path << " ." << std::endl;
        exit(-1);
    }

    start = std::chrono::high_resolution_clock::now();

    vlabels_ = new ui[vertices_count_];
    vlabel_file.read(reinterpret_cast<char *>(vlabels_), sizeof(int) * vertices_count_);

    vlabel_file.close();
    vlabel_file.clear();

    ui max_vlabel_id = 0;
    for (ui i = 0; i < vertices_count_; ++i) {
        ui vlabel = vlabels_[i];

        if (vlabels_frequency_.find(vlabel) == vlabels_frequency_.end()) {
            vlabels_frequency_[vlabel] = 0;
            if (vlabel > max_vlabel_id)
                max_vlabel_id = vlabel;
        }

        vlabels_frequency_[vlabel] += 1;
    }

    vlabels_count_ = (ui)vlabels_frequency_.size() > (max_vlabel_id + 1) ? (ui)vlabels_frequency_.size() : max_vlabel_id + 1;

    for (auto element : vlabels_frequency_) {
        if (element.second > max_vlabel_frequency_) {
            max_vlabel_frequency_ = element.second;
        }
    }

    end = std::chrono::high_resolution_clock::now();
    std::cout << "Load vlabel file time: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    BuildReverseIndex();
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Build reverse index file time: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    buildEdgeIndex();
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Build edge index time: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds" << std::endl;

#if OPTIMIZED_VLABELED_GRAPH == 1
    if (enable_vlabel_offset_) {
        BuildNLF();
        
    }
#endif
}

void Graph::storeComparessedGraph(const std::string& degree_path, const std::string& edge_path,
                                  const std::string& vlabel_path) {
    ui* degrees = new ui[vertices_count_];
    for (ui i = 0; i < vertices_count_; ++i) {
        degrees[i] = offsets_[i + 1] - offsets_[i];
    }

    std::ofstream deg_outputfile(degree_path, std::ios::binary);

    if (deg_outputfile.is_open()) {
        std::cout << "Open degree file " << degree_path << " successfully." << std::endl;
    }
    else {
        std::cerr << "Cannot degree edge file " << degree_path << " ." << std::endl;
        exit(-1);
    }

    int int_size = sizeof(int);
    size_t vertex_array_bytes = ((size_t)vertices_count_) * 4;
    deg_outputfile.write(reinterpret_cast<const char *>(&int_size), 4);
    deg_outputfile.write(reinterpret_cast<const char *>(&vertices_count_), 4);
    deg_outputfile.write(reinterpret_cast<const char *>(&edges_count_), 4);
    deg_outputfile.write(reinterpret_cast<const char *>(degrees), vertex_array_bytes);

    deg_outputfile.close();
    deg_outputfile.clear();

    delete[] degrees;

    std::ofstream edge_outputfile(edge_path, std::ios::binary);

    if (edge_outputfile.is_open()) {
        std::cout << "Open edge file " << edge_path << " successfully." << std::endl;
    }
    else {
        std::cerr << "Cannot edge file " << edge_path << " ." << std::endl;
        exit(-1);
    }

    size_t edge_array_bytes = ((size_t)edges_count_ * 2) * 4;
    edge_outputfile.write(reinterpret_cast<const char *>(neighbors_), edge_array_bytes);

    edge_outputfile.close();
    edge_outputfile.clear();

    std::ofstream vlabel_outputfile(vlabel_path, std::ios::binary);

    if (vlabel_outputfile.is_open()) {
        std::cout << "Open vlabel file " << vlabel_path << " successfully." << std::endl;
    }
    else {
        std::cerr << "Cannot vlabel file " << vlabel_path << " ." << std::endl;
        exit(-1);
    }

    size_t vlabel_array_bytes = ((size_t)vertices_count_) * 4;
    vlabel_outputfile.write(reinterpret_cast<const char *>(vlabels_), vlabel_array_bytes);

    vlabel_outputfile.close();
    vlabel_outputfile.clear();
}

void Graph::buildEdgeIndex() {
    edge_index_ = new sparse_hash_map<uint64_t, std::vector<edge>*>();

    edge cur_edge;
    for (uint32_t u = 0; u < vertices_count_; ++u) {
        uint32_t u_l = getVertexLabel(u);

        uint32_t u_nbrs_cnt;
        const uint32_t* u_nbrs = getVertexNeighbors(u, u_nbrs_cnt);

        cur_edge.vertices_[0] = u;

        for (uint32_t i = 0; i < u_nbrs_cnt; ++i) {
            uint32_t v = u_nbrs[i];
            uint32_t v_l = getVertexLabel(v);

            uint64_t key = (uint64_t) u_l << 32 | v_l;
            cur_edge.vertices_[1] = v;

            if (!edge_index_->contains(key)) {
                (*edge_index_)[key] = new std::vector<edge>();
            }
            (*edge_index_)[key]->emplace_back(cur_edge);
        }
    }

}
