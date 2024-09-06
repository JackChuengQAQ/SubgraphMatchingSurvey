
#include "bigraphMatching.h"

HKmatch::HKmatch(ui left_cnt, ui right_cnt, VertexID **adj, ui *adj_cnt):
    left_cnt(left_cnt), right_cnt(right_cnt),
    adj(adj), adj_cnt(adj_cnt) {
    pair = new VertexID [left_cnt + right_cnt + 1];
    std::fill(pair, pair + left_cnt + right_cnt + 1, NIL);
    distance = new ui [left_cnt + right_cnt + 1];

    for (int i = 0; i < left_cnt; i++) {
        for (int j = 0; j < adj_cnt[i]; ++j) {
            adj[i][j] += 1;
        }
    }
}

HKmatch::~HKmatch() {
    delete[] distance;
    delete[] pair;
}

bool HKmatch::bfs() {
    std::queue<ui> queue;
    for(int i = 1; i <= left_cnt; i++) {
        if(pair[i] == NIL) {
            distance[i] = 0;
            queue.push(i);
        } else {
            distance[i] = INF;
        }
    }
    distance[NIL] = INF;
    while(!queue.empty()) {
        ui u = queue.front();
        queue.pop();
        
        if(distance[u] < distance[NIL]) {
            for (int i = 0; i < adj_cnt[u-1]; ++i) {
                ui v = adj[u-1][i];
                if(distance[pair[v]] == INF) {
                    distance[pair[v]] = distance[u] + 1;
                    queue.push(pair[v]);
                }
            }
        }
    }
    return distance[NIL] != INF;
}

bool HKmatch::dfs(ui u) {
    if(u != NIL) {
        for (int i = 0; i < adj_cnt[u-1]; ++i) {
            ui v = adj[u-1][i];
            if (distance[pair[v]] == distance[u] + 1) {
                if (dfs(pair[v])) {
                    pair[v] = u;
                    pair[u] = v;
                    return true;
                }
            }
        }
        distance[u] = INF;
        return false;
    }
    return true;
}

ui HKmatch::MaxMatch() {
    ui matching = 0;
    while(bfs()) {
        for(int i = 1; i <= left_cnt; i++) {
            if(pair[i] == NIL && dfs(i)) {
                matching++;
            }
        }
    }
    return matching;
}