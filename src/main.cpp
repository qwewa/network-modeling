#include <iostream>

#include "Data.hpp"
#include "Scheduler.hpp"
#include <chrono>
#include "openmp.hpp"

#define NUM_THREADS 8

int main()
{
    Data data;

    data.generate_all(5, 20, 5, 2, 2, 
                        1000, 0.5, 1000, 1000);

    data.save_graph("graph_5_20_5_2_2");

    data.save_tasks("tasks_1000_05_1000_1000");

    int amount_thread = 1;

    for(; amount_thread <= NUM_THREADS; amount_thread*=2)
    {    

        data.load_full("graph_5_20_5_2_2", "tasks_1000_05_1000_1000");

        Scheduler master(amount_thread, data.get_graph());

        auto tasks = data.get_tasks();

        auto start = std::chrono::high_resolution_clock::now();

        for (auto & t : tasks) {
            master.schedule_task(t);
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "my: "<< amount_thread << " " <<  duration << std::endl;

        data.load_full("graph_5_20_5_2_2", "tasks_1000_05_1000_1000");
        Graph g = data.get_graph();

        start = std::chrono::high_resolution_clock::now();

        for (auto & t : tasks) {
            WorkerResult out_best_result;
            run_openmp_task(t, g, amount_thread, out_best_result);
            finalize_openmp_task(g, out_best_result);
        }

        end = std::chrono::high_resolution_clock::now();

        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "openmp: "<< amount_thread << " " <<  duration << std::endl;
    }

    return 0;
}
