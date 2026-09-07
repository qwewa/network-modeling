#pragma once 

#include <vector>
#include <memory>
#include <atomic>
#include <deque>
#include "Graph.hpp"
#include "Task.hpp"

struct PathSegment
{
    std::shared_ptr<Edge> edge;
    double start_time;
    double end_time;
};

struct WorkerResult
{
    double finish_time = 1e18;
    int node_id = -1; // Узел на который назначится задача
    double compute_start = 0;
    double compute_end = 0;
    double transfer_time = 1e18;
    double wait_time = 1e18;
    double score = 1e18; // Основной критерий оптимизации
    std::vector<PathSegment> transfer_path;
    
    bool isValid() const { return node_id != -1; }
};

class Worker
{
    int id;
    std::optional<Task> current_task;
    WorkerResult best_result;

    const Graph & graph;
    std::deque<int> local_queue;    // Локальная очередь ID узлов

    std::mutex mtx;

    double alpha;
    double beta;
    double gamma;

    public:
        Worker(int worker_id, const Graph & g, double alpha=1, double beta=1, double gamma=1) 
            : id(worker_id), graph(g), alpha(alpha), beta(beta), gamma(gamma) {}
        
        void run(const std::vector<std::unique_ptr<Worker>>& all_workers,
            std::atomic<bool> & stop_flag,
            std::atomic<bool> & start_task,
            std::atomic<int> & nodes_processed,
            int & total_nodes,
            std::mutex & master_wait_mtx,
            std::condition_variable & cv_start,
            std::condition_variable & cv_finished);

        void push_node(int node_id)
        {
            std::lock_guard<std::mutex> lock(mtx);

            local_queue.push_back(node_id);
        }

        void reset()
        {
            std::lock_guard<std::mutex> lock(mtx);
            local_queue.clear();
            best_result = WorkerResult();
            current_task = std::nullopt;
        }

        WorkerResult get_best() const { return best_result; }

        int get_id() const { return id; }

        void set_task(const Task & t) { current_task = t; }

    private:

        std::optional<int> pop_local();
        std::optional<int> steal(const std::vector<std::unique_ptr<Worker>> & all_workers);

        WorkerResult evaluate_node(const Task& task, int target_id) const;
};