#include "Worker.hpp"
#include <random>
#include <queue>
#include <thread>

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

// Реализация Дейкстры
WorkerResult Worker::evaluate_node(const Task & task, int target_id) const
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

    double score = alpha * (data_ready - task.arrival_time) + beta * (comp_time) + gamma * (final_start - data_ready);

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


// Основной цикл worker
void Worker::run(const vector<unique_ptr<Worker>>& all_workers,
                atomic<bool>& stop_flag,
                atomic<bool>& start_task,
                atomic<int>& nodes_processed,
                 int& total_nodes,
                mutex& master_wait_mtx,
                condition_variable& cv_start,
                condition_variable& cv_finished) 
{

    while (!stop_flag) {

        {
            unique_lock<mutex> lock(master_wait_mtx);
            cv_start.wait(lock, [&] { return start_task.load() || stop_flag.load(); });
        }

        if (stop_flag) break;

        while (true) {
            auto node_id = pop_local();

            // if (!node_id) node_id = steal(all_workers);

            if (node_id) {
                WorkerResult current_res = evaluate_node(*current_task, *node_id);
            
                if (current_res.score < best_result.score) {
                    best_result = std::move(current_res);
                }

                // fetch_add возвращает значение до изменения, поэтому проверяем на total_nodes - 1
                if (nodes_processed.fetch_add(1) == total_nodes - 1)
                {
                    {
                        lock_guard<mutex> lock(master_wait_mtx); // Lost Wakeup
                    }
                    cv_finished.notify_one(); 
                }
            } else {
                // Если работы нет ни у кого — выходим в режим ожидания следующей задачи
                if (nodes_processed.load() >= total_nodes) {
                    break; 
                }
                std::this_thread::yield();
            }
        }
    }
}

optional<int> Worker::pop_local()
{
        lock_guard<mutex> lock(mtx);
        if (local_queue.empty()) return std::nullopt;
        int val = local_queue.front();
        local_queue.pop_front();
        return val;
}

optional<int> Worker::steal(const vector<unique_ptr<Worker>> & all_workers)
{
    size_t n = all_workers.size();

    if (n <= 1) return std::nullopt;

    static thread_local mt19937 gen(random_device{}());
    uniform_int_distribution<size_t> dist(0, n - 1);
    
    size_t start_idx = dist(gen);

    for (size_t i = 0; i < n; i++)
    {
        size_t idx = (start_idx + i) % n;
        auto & other = all_workers[idx];

        if (other -> id == this -> id) continue;

        unique_lock<mutex> lock(other -> mtx, std::try_to_lock);
        
        if (lock.owns_lock() && !(other -> local_queue.empty())) 
        {
            int val = other -> local_queue.back();
            other -> local_queue.pop_back();
            return val;
        }
    }

    return std::nullopt;
}