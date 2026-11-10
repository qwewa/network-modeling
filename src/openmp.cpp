#include "openmp.hpp"
#include <unordered_map>
#include <queue>
#include <omp.h>

using namespace std;

#define ALPHA 1
#define BETA 1
#define GAMMA 1

struct BackPointer
{
    int parent_id;
    shared_ptr<Edge> edge;
    double start_time;
    double end_time;
};

WorkerResult evaluate_node_openmp(const Task & task, int target_id, const Graph& graph)
{    
    unordered_map<int, double> arrival_times;
    unordered_map<int, BackPointer> predecessors;
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> pq;

    arrival_times[task.source_id] = task.arrival_time;
    pq.push({task.arrival_time, task.source_id});

    while (!pq.empty()) {
        auto [t_curr, u_id] = pq.top();
        pq.pop();

        if (u_id == target_id) break;
        if (t_curr > arrival_times[u_id]) continue;

        auto u_node = graph.get_node(u_id);

        for (auto & weak_edge : u_node -> edges) {
            auto edge = weak_edge.lock();

            int v_id = edge -> get_neighbor(u_node) -> id;

            // Время на передачу данных по этому ребру
            double travel_time = edge -> transfer_time(task.data_size);
            
            // Время, когда передача сможет начаться
            double t_start_transfer = edge -> find_available_start(t_curr, travel_time);
            double t_finish_transfer = t_start_transfer + travel_time;

            if (arrival_times.find(v_id) == arrival_times.end() || t_finish_transfer < arrival_times[v_id]) {
                arrival_times[v_id] = t_finish_transfer;
                predecessors[v_id] = {u_id, edge, t_start_transfer, t_finish_transfer};
                pq.push({arrival_times[v_id], v_id});
            }
        }
    }

    if (arrival_times.find(target_id) == arrival_times.end())
        return WorkerResult();

    auto target_node = graph.get_node(target_id);

    double comp_time = target_node -> computing_time(task.compute_load);
    double data_ready = arrival_times[target_id];

    // Ищем окно для вычислений на самом узле
    double final_start = target_node -> find_available_start (data_ready, comp_time);

    double final_finish = final_start + comp_time;

    std::vector<PathSegment> full_path;
    int curr = target_id;

    double score = ALPHA * (data_ready - task.arrival_time) + BETA * (comp_time) + GAMMA * (final_start - data_ready);

    while (predecessors.count(curr))
    {
        auto & bp = predecessors[curr];
        full_path.push_back({bp.edge, bp.start_time, bp.end_time});
        curr = bp.parent_id;
    }

    std::reverse(full_path.begin(), full_path.end());

    WorkerResult res;
    res.finish_time = final_finish;
    res.score = score;
    res.node_id = target_id;
    res.compute_start = final_start;
    res.compute_end = final_finish;
    res.transfer_time = data_ready - task.arrival_time;
    res.wait_time = final_start - data_ready;
    res.transfer_path = std::move(full_path);

    return res;
}


void run_openmp_task(const Task & task, const Graph & graph, int num_threads, WorkerResult & out_best_result) {
    auto& all_nodes = graph.get_nodes();
    int total_nodes = all_nodes.size();

    omp_set_num_threads(num_threads);

    WorkerResult global_best;

    #pragma omp parallel
    {
        WorkerResult local_best;

        #pragma omp for schedule(dynamic, 1)
        for (int i = 0; i < total_nodes; ++i) {
            int target_id = all_nodes[i]->id;
            
            WorkerResult current_res = evaluate_node_openmp(task, target_id, graph);

            if (current_res.score < local_best.score) {
                local_best = std::move(current_res);
            }
        }

        #pragma omp critical
        {
            if (local_best.score < global_best.score) {
                global_best = local_best;
            }
        }
    }

    out_best_result = std::move(global_best);
}

void finalize_openmp_task(Graph& graph, 
                          const WorkerResult& absolute_best)
{
    if (absolute_best.isValid())
    {
        for (auto & segment : absolute_best.transfer_path) {
            segment.edge -> reserve(segment.start_time, segment.end_time);
        }

        auto target_node = graph.get_node(absolute_best.node_id);
        if (target_node) {
            target_node -> reserve(absolute_best.compute_start, absolute_best.compute_end);
            (target_node -> amount_task)++;
        }
    }
}