#include "Scheduler.hpp"
#include <cmath>
#include <fstream>

using namespace std;


Scheduler::Scheduler(int n, Graph& g, double alpha, double beta, double gamma): num_threads(n), graph(g), alpha(alpha), beta(beta), gamma(gamma)
{
    for (int i = 0; i < num_threads; i++) {
        workers.push_back(std::make_unique<Worker>(i, g, alpha, beta, gamma));
    }

    // Запускаем потоки один раз
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(&Worker::run, workers[i].get(), 
                                std::ref(workers), std::ref(stop_flag), 
                                std::ref(start_task), std::ref(nodes_processed),
                                std::ref(total_nodes), std::ref(master_mtx),
                                std::ref(cv_start), std::ref(cv_finished));
    }
}

void Scheduler::schedule_task(const Task & task)
{
    {
        std::lock_guard<std::mutex> lock(master_mtx);

        auto& all_nodes = graph.get_nodes();
        total_nodes = all_nodes.size();
        nodes_processed = 0;
        // task.print_info();

        for (auto & w : workers)
        {
            w -> reset();
            w -> set_task(task); 
        }

        for (int i = 0; i < total_nodes; ++i) {
            workers[i % num_threads] -> push_node(all_nodes[i] -> id);
        }

        start_task = true;
    }

    cv_start.notify_all();

    // Ждем пока cv_finished сработает (когда nodes_processed == total_nodes)
    {
        std::unique_lock<std::mutex> lock(master_mtx);
        cv_finished.wait(lock, [&] { return nodes_processed >= total_nodes; });

        start_task = false;
    }

    finalize_task(graph);
}

void Scheduler::finalize_task(Graph& graph)
{
    WorkerResult absolute_best;

    for (auto& w : workers) {
        WorkerResult res = w -> get_best();

        // std::cout << "The best from woker_id " << w -> get_id() << " is node_id " <<
        //     res.node_id << " score = " << res.score
        //     << " computed start " << res.compute_start << " computed end " << res.compute_end
        //     << "\n";

        if (res.isValid() && res.score < absolute_best.score) {
            absolute_best = std::move(res);
        }
    }

    if (absolute_best.isValid())
    {
        all_compute_time += absolute_best.compute_end - absolute_best.compute_start;
        all_transfer_time += absolute_best.transfer_time;
        all_wait_time += absolute_best.wait_time;
        // Бронируем ребра
        for (auto & segment : absolute_best.transfer_path) {
            segment.edge -> reserve(segment.start_time, segment.end_time);
        }

        // Бронируем узел
        auto target_node = graph.get_node(absolute_best.node_id);
        target_node -> reserve(absolute_best.compute_start, absolute_best.compute_end);
        (target_node -> amount_task)++;

        // std::cout << "The absolute best is node_id " << absolute_best.node_id << " resource was reserve" << endl;
    }
}

void Scheduler::print_statistics(vector<Task> & tasks) const
{
    auto nodes = graph.get_nodes();

    int amount_tasks, max_tasks = -1, min_tasks = nodes[0] -> amount_task;

    double amount_time, max_time = -1, min_time = nodes[0] -> time_resource_is_free();


    double average_tasks = (double)tasks.size() / nodes.size(), std_dev_tasks = 0;

    double total_time = 0;

    for (auto n : nodes)
    {   
        amount_tasks = n -> amount_task;
        amount_time = n -> time_resource_is_free();

        max_tasks = std::max(amount_tasks, max_tasks);
        min_tasks = std::min(amount_tasks, min_tasks);
        std_dev_tasks += pow(amount_tasks - average_tasks, 2);

        max_time = std::max(amount_time, max_time);
        min_time = std::min(amount_time, min_time);

        total_time += amount_time;

    }

    std_dev_tasks = std::sqrt(std_dev_tasks / nodes.size());

    double average_time = total_time / nodes.size(), std_dev_time = 0;

    for (auto n : nodes)
    {
        amount_time = n -> time_resource_is_free();

        std_dev_time += std::pow(amount_time - average_time, 2);
    }

    std_dev_time = std::sqrt(std_dev_time / nodes.size());

    cout << "\nTASK BALANCE:\n"
    << "\namount: " << tasks.size()
    << "\nmin per node: " << min_tasks
    << "\nmax per node: " << max_tasks
    << "\naverage: " << (int)average_tasks
    << "\nimbalance: " << (double)(max_tasks) / average_tasks
    << "\nstandard deviation: " << std_dev_tasks << endl;

    cout << "\n\nTIME BALANCE:\n"
    << "\nmin per node: " << min_time
    << "\nmax per node: " << max_time
    << "\naverage: " << (int)average_time
    << "\nstandard deviation: " << std_dev_time << endl;

    double avr_tranfer_time = all_transfer_time / tasks.size();
    double avr_compute_time = all_compute_time / tasks.size();
    double avr_wait_time = all_wait_time / tasks.size();
    double avr_task_time = avr_tranfer_time + avr_compute_time + avr_wait_time;

    cout << "\n\nSCHEDULE:\n"
    << "\nschedule finish time: " << max_time
    << "\naverage tranfer time: " << avr_tranfer_time
    << "\naverage compute time: " << avr_compute_time
    << "\naverage wait time: " << avr_wait_time
    << "\naverage completion time: " << avr_task_time << endl;

    std::ofstream file("output.txt", std::ios::app);

    if (file.is_open()) {
        file << alpha << "        " << beta << "        " << gamma << "      " << avr_tranfer_time << "     " << avr_compute_time << "      " << avr_wait_time << "     " <<  avr_task_time << "    " << max_time <<  endl;
        file.close();
    }
}