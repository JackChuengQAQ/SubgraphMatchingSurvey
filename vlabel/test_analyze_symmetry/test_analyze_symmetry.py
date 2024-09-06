
import networkx as nx

if __name__ == '__main__':
    graph = nx.petersen_graph()

    node_partitions = [set(graph.nodes)]
    edge_colors = {e:0 for e in graph.edges}

    ismags = nx.isomorphism.ISMAGS(graph, graph)
    permutations, cosets = ismags.analyze_symmetry(graph, node_partitions, edge_colors)

    print('Permutations:')
    for permutation in permutations:
        for item in permutation:
            print(list(item), end=' ')
        print()
    
    print('Cosets:')
    for k, v in cosets.items():
        print('{}: '.format(k), end=' ')
        for node in v:
            print(node, end=' ')
        print()
    
    constraints = ismags._make_constraints(cosets)

    print('Constraints:')
    for constraint in constraints:
        print(constraint)

    isomorphisms = list(ismags.isomorphisms_iter(symmetry=False))
    print("symmetry=False:\t",len(isomorphisms))

    isomorphisms = list(ismags.isomorphisms_iter(symmetry=True))
    print("symmetry=True:\t",len(isomorphisms))


    

