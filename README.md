# SubgraphMatchingSurvey

The source code of "A Comprehensive Survey and Experimental Study of Subgraph Matching: Trends, Unbiasedness, and Interaction"[1], accepted by SIGMOD 2024.

# A Comprehensive Survey and Experimental Study of Subgraph Matching: Trend, Unbiasedness, and Interaction

The source code for "A Comprehensive Survey and Experimental Study of Subgraph Matching: Trend, Unbiasedness, and Interaction".

## Introduction

Subgraph matching is a fundamental problem in graph analysis. In recent years, many subgraph matching algorithms have been proposed, making it pressing and challenging to compare their performance and identify their strengths and weaknesses. We observe that (1) The embedding enumeration in the classic filtering-ordering-enumerating framework dominates the overall performance, and thus enhancing the backtracking paradigm is becoming a current research trend; (2) Simply changing the limitation of output size results in a substantial variation in the ranking of different methods, leading to biased performance evaluation; (3) The techniques employed at different stages of subgraph matching interact with each other, making it less feasible to replace and evaluate a single technique in isolation. Therefore, a comprehensive survey and experimental study of subgraph matching is necessary to identify the current trend, ensure the unbiasedness, and investigate the potential interactions. In this paper, we comprehensively review the methods in the trend and experimentally confirm their advantage over prior approaches. We unbiasedly evaluate the performance of these algorithms by using an effective metric, namely embeddings per second. To fully investigate the interactions between various techniques, 534 algorithm combinations are thoroughly evaluated and explored through both real-world and synthetic graphs.

## file structure

Within the directory, there are four folders here, including the two code branches of `vlabel` and `elabel`.

```
├─ automorphism       // Subgraph matching with automorphic graph detection
│  ├─ configuration   // Basic data structure & macro definitions
│  ├─ graph           // Graph structure
│  ├─ matching        // Core code: methods implementation (filter, order, engine)
│  ├─ test*           // Tests for source code correctness
│  └─ utility         // Supporting utilities for code execution
├─ elabel             // Support for edge-labeled graphs in subgraph matching
│  ├─ configuration   // Basic data structure & macro definitions
│  ├─ graph           // Graph structure
│  ├─ matching        // Core code: methods implementation (filter, order, engine)
│  ├─ elabel_test*    // Tests for source code correctness
│  └─ utility         // Supporting utilities for code execution
```

### `vlabel` Directory

The `vlabel` directory contains the foundational implementation for all evaluated algorithms, focusing on vertex-labeled undirected graphs, and provides the feature for  automorphic graphs (generating at most one result for one automorphism group), which can be activated using the `-symmetry 1` parameter.

### `elabel` Directory

The `elabel` directory supports edge-labeled graphs with the same algorithms. To enable this feature, we incorporate the `ELABELED_GRAPH` macro definition as a switch within the configuration.

## Upcoming Merge

In the future, the `vlabel` and `elabel` branches will be merged to unify their functionalities. 

## build

Within the `vlabel` or `elabel` directory, create a build directory and compile the source code.

```zsh
mkdir build & cd build
cmake ..
make
```

## Execute

Take `vlabel` directory as an example.
After compiling the source code, you can find the binary file `SubgraphMatching.out` under the `build/matching` directory.
Execute the binary with the following command.

```zsh
./SubgraphMatching.out -d data_graphs -q query_graphs
-filter method_of_filtering_candidate_vertices -order method_of_ordering_query_vertices -engine method_of_enumerating_partial_results -num number_of_embeddings,
```

For detailed parameter settings, see `matching/matchingcommand.h`.

**Example 1**: This approach uses the filtering and ordering methods of GraphQL to generate candidate vertex sets and determine the matching order. Results are enumerated using the set-intersection-based local candidate computation method.


```zsh
./SubgraphMatching.out -d ../../test/sample_dataset/test_case_1.graph -q ../../test/sample_dataset/query1_positive.graph -filter GQL -order GQL -engine LFTJ -num MAX
```

***Example2*** (Use the filtering method CaLiG to generate the candidate vertex sets, ordering method of RI to generate the matching order,  KSS engine to enumerate the results with automorphic graphs detection):

```zsh
./SubgraphMatching.out -d ../../test/sample_dataset/test_case_1.graph -q ../../test/sample_dataset/query1_positive.graph -filter CaLiG -order RI -engine KSS -num MAX -symmetry 1
```

## Input

The input format of data graph and query graph is fixed which is the same as the previous study [2]. Each graph file starts with 't N M' where N is the number of vertices and M is the number of edges. A vertex and an edge are formatted as 'v VertexID LabelId Degree' and 'e VertexId VertexId (EdgeLabel)?' respectively. The vertex id must start from 0 and the range is [0,N - 1]. The following is an input sample. You can also find sample data sets and query sets under the test folder. For single graph the format of edges must be the same.

```
t <vertex_number> <edge number>
v <vertex_id> <vertex_label> <vertex_degree>
e <source> <target> (<edge_label>)?
```

For example, the input graph without edge label could be:

```zsh
t 5 6
v 0 0 2
v 1 1 3
v 2 2 3
v 3 1 2
v 4 2 2
e 0 1
e 0 2
e 1 2
e 1 3
e 2 4
e 3 4
```

A sample input graph with edge labels is as follows, where the third column of the edge line is the edge label (e.g. the edge label of the first edge is 0).

```zsh
t 5 6
v 0 0 2
v 1 1 3
v 2 2 3
v 3 1 2
v 4 2 2
e 0 1 0
e 0 2 1
e 1 2 2
e 1 3 3
e 2 4 4
e 3 4 5
```


## Techniques Supported

For each technique, we support 10 methods, as shown in the following table.

| -filter | -order | -engine |
| :-----: | :----: | :-----: |
|   LDF   |  QSI   |   QSI   |
|   NLF   |  VF3   |   VF3   |
|   GQL   |  GQL   |   GQL   |
|   TSO   |  TSO   | EXPLORE |
|   CFL   |  CFL   |  LFTJ   |
|  DPiso  | DPiso  |  DPiso  |
|  CECI   |  CECI  |  CECI   |
|   VEQ   |   RI   |   VEQ   |
|   RM    |   RM   |   RM    |
|  CaLiG  | VF2PP  |   KSS   |

## Configuration

In addition to setting the filtering, ordering and enumeration methods, you can also configure the set intersection algorithms and optimization techniques by defining macros in 'configuration/config.h'. Please see the comments in 'configuration/config.h' for more details. We briefly introduce these methods in the following.


|         Macro         |                         Description                          |
| :-------------------: | :----------------------------------------------------------: |
|       HYBRID 0        | a hybrid method handling the cardinality skew by integrating the merge-based method with the galloping-based method |
|       HYBRID 1        |               the merge-based set intersection               |
|         SI 0          |          Accelerate the set intersection with AVX2           |
|         SI 1          |         Accelerate the set intersection with AVX512          |
|         SI 2          |                   Scalar set intersection                    |
|    ENABLE_QFLITER     | the [QFilter](https://dl.acm.org/doi/10.1145/3183713.3196924) set intersection algorithm |
|  ENABLE_FAILING_SET   |         the failing set pruning technique in DP-iso          |
| ENABLE_EQUIVALENT_SET |      enable the equivalent set pruning technique in VEQ      |
|    ELABELED_GRAPH     |                  enable edge labeled graph                  |

> [1] Zhijie Zhang, Yujie Lu, Weiguo Zheng, and Xuemin Lin. 2024. A Comprehensive Survey and Experimental Study of Subgraph Matching: Trends, Unbiasedness, and Interaction. Proc. ACM Manag. Data 2, 1, Article 60 (February 2024), 29 pages.
> 
> [2] Shixuan Sun and Qiong Luo. 2020. In-Memory Subgraph Matching: An In-depth Study. In Proceedings of the 2020 ACM SIGMOD International Conference on Management of Data (SIGMOD '20). Association for Computing Machinery, New York, NY, USA, 1083–1098.
