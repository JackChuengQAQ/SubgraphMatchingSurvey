

import networkx as nx

def read_graph(graph, file):
    
    f = open(file)
    
    meta_data = f.readline().split()
    v = int(meta_data[1])
    e = int(meta_data[2])
    
    for i in range(v):
        vertex = f.readline().split()
        if vertex[0] != 'v':
            continue
        n = int(vertex[1])
        graph.add_node(n)
        graph.nodes[n]['label'] = int(vertex[2])
    
    if vertex[0] == 'e' and len(vertex) == 3:
        graph.add_edge(int(vertex[1]), int(vertex[2]))
    for i in range(e):
        edge = f.readline().split()
        if len(edge) != 3 or edge[0] != 'e':
            break
        fro = int(edge[1])
        to = int(edge[2])
        graph.add_edge(fro, to)


def assign_label(graph):
    
    for n in graph.nodes:
        graph.nodes[n]['label'] = 0


def write_graph(graph, filename):
    f = open(filename, 'x')  

    
    graph.graph['E'] = nx.number_of_edges(graph)  
    graph.graph['V'] = nx.number_of_nodes(graph)
    f.write("t " + str(graph.graph['V']) + " " + str(graph.graph['E']) + "\n")

    

    
    for n in graph.nodes:
        output = "v " + str(n) + " " + str(graph.nodes[n]['label']) + " "
        k = len(graph[n])
        output = output + str(k) + "\n"
        f.write(output)

    
    for n in graph.nodes:
        n_nbr = sorted(graph[n])
        for m in n_nbr:
            if n < m:
                output = "e " + str(n) + " " + str(m) + "\n"
                f.write(output)

    f.close()

if __name__ == '__main__':
    
    
    graph = nx.cycle_graph(4)
    
    assign_label(graph)
    write_graph(graph, 'toy/4_ring.txt')
    