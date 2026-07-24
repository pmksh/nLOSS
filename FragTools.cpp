#include "FragTools.h"


#include <queue>
#include <random>
#include <cmath>
#include <algorithm>
// #include <chrono> 






// Regular fragmentation
std::vector<struct frame> GridFrag (int height, int width, int sx, int sy){

    std::vector<struct frame> frames;
    
    for (int x = 0; x < width; x += sx) {

        int sx0 = std::min(sx, width - x);

        for (int y = 0; y < height; y += sy) {

            int sy0 = std::min(sy, height - y);
                
            frames.push_back({x, y, sx0, sy0});
        }
    }

    return frames;
}


// -----------------------------------------------------------------------------------------------------------------------------------



// Min-heap node for the priority queue
struct Node {
    int priority;
    int x;
    int y;
};

// Comparator for min-heap (smallest priority first)
struct CompareNode {
    bool operator()(const Node& a, const Node& b) const {
        return a.priority > b.priority;
    }
};


// Non-uniform fragmentation
std::vector<struct frame> nuFrag (int height, int width, int seed, int type){

    // for now type is useless
    type = 0;


    std::vector<std::vector<int>> filled(width, std::vector<int>(height, 0));

    // Random number generator
    std::mt19937 rng;
    if (seed != -1) {
        rng.seed(seed);
    } else {
        // Use a non-deterministic seed
        std::random_device rd;
        rng.seed(rd());
    }

    // Helper to generate random integer in [low, high] inclusive
    auto rand_int = [&rng](int low, int high) -> int {
        std::uniform_int_distribution<int> dist(low, high);
        return dist(rng);
    };

    std::vector<struct frame> frames;
    std::priority_queue<Node, std::vector<Node>, CompareNode> pq;

    pq.push({0, 0, 0});   // initial node (priority = 0, x = 0, y = 0)
    int count = 0;

    while (!pq.empty()) {
        Node cur = pq.top();
        pq.pop();
        int x = cur.x;
        int y = cur.y;

        // Maximum possible square size from this corner
        int bound = std::min(width - x, height - y);
        bound = std::min(bound, 25);
        // Bound Adjustment
        for (int i = 0; i < bound; ++i) {
            if (filled[x + i][y] == 1) {
                bound = i;
                break;
            }
        }

        int k = rand_int(1, bound);

        // Push the neighbor to the right (if any)
        if (y + k < height && (x == 0 || filled[x - 1][y + k] == 1)) {
            int priority = y + k - x;
            pq.push({priority, x, y + k});
        }

        // Push the neighbour below (if any)
        if (x + k < width) {
            for (int i = 0; i < k; ++i) {
                if (filled[x + k][y + i] == 0) {
                    int priority = y + i - x - k;
                    pq.push({priority, x + k, y + i});
                    break;
                }
            }
        }

        frames.push_back({x, y, k, k});
        
        for (int i = 0; i < k; ++i) {
            for (int j = 0; j < k; ++j) {
                filled[x + i][y + j] = 1;
            }
        }
    }

    return frames;
}