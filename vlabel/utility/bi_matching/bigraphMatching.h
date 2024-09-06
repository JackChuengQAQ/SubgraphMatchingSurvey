#ifndef RELABEL_BIGRAPHMATCHING_H
#define RELABEL_BIGRAPHMATCHING_H

#include <algorithm>
#include "../../configuration/types.h"


#include <queue>
#include <iostream>

#define INF 0x3F3F3F3F
#define NIL 0x0

class HKmatch {
private:
    ui left_cnt;
    ui right_cnt;
    VertexID **adj;
    VertexID *adj_cnt;
    VertexID *pair;
    ui *distance;

    bool bfs();

    bool dfs(ui u);

public:

    HKmatch(ui left_cnt,
            ui right_cnt,
            VertexID **adj,
            ui *adj_cnt);

    ~HKmatch();

    ui MaxMatch();
};

#endif 
