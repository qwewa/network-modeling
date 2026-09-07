#include "Graph.hpp"
#include "Task.hpp"
#include "Worker.hpp"
#include <thread>

class Scheduler {
    int num_threads;
    Graph& graph;
    std::vector<std::unique_ptr<Worker>> workers;
    std::vector<std::thread> threads;

    // Состояние текущей задачи
    std::atomic<bool> stop_flag{false};
    std::atomic<bool> start_task{false};
    std::atomic<int> nodes_processed{0};
    int total_nodes = 0;

    // Примитивы синхронизации
    std::mutex master_mtx;
    std::condition_variable cv_start;
    std::condition_variable cv_finished;

    // Для статистики
    double all_transfer_time = 0;
    double all_compute_time = 0;
    double all_wait_time = 0;

    // Параметры
    double alpha;
    double beta;
    double gamma;
    
    public:
        Scheduler(int n, Graph & g, double alpha = 1, double beta = 1, double gamma = 1);

        void schedule_task(const Task & task);

        void print_statistics(std::vector<Task> & tasks) const;

        ~Scheduler() {
            stop_flag = true;
            cv_start.notify_all();
            for (auto & t : threads) t.join();
        };
    private:
        void finalize_task(Graph & graph);
};

