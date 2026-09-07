#include <iostream>

#include "../lib/Data.hpp"
#include "../lib/Scheduler.hpp"
#include <chrono>
#include <fstream>
#include <thread>

int main()
{
    Data data;

    // data.generate_all(5, 20, 5, 2, 2, 
    //                     1000, 0.5, 1000, 1000);

    // data.save_graph("graph_5_20_5_2_2");

    // data.save_tasks("tasks_1000_05_1000_1000");


    std::ofstream file("output_time.txt", std::ios::app);

    auto all_time = 0;
    int amount_thread = 10;

    for(; amount_thread <= 10; amount_thread*=2)
    {    
        all_time = 0;
        for(int i = 0; i < 5; i++)
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

            all_time += duration;
            std::cout << amount_thread << " " <<  duration << " " << all_time / 5 << std::endl;

        }

        file << amount_thread << " " << all_time / 5 << std::endl;
    }

    return 0;
}




// std::ofstream file("output.txt", std::ios::app);

// if (file.is_open()) {
//     file << "alpha  beta    gamma   avr_tranfer_time    avr_compute_time    avr_wait_time   avr_task_time   max_time" << std::endl;

// for (double alpha = 1; alpha < 2; alpha++)
// {
//     for (double beta = 1; beta < 2; beta++)
//     {
//         for (double gamma = 1; gamma < 2; gamma++)
//         {
//             data.load_full("graph_5_20_5_2_2", "tasks_1000_05_1000_1000");

//             Scheduler master(8, data.get_graph(), alpha, beta, gamma);

//             auto tasks = data.get_tasks();

//             auto start = std::chrono::high_resolution_clock::now();

//             for (auto & t : tasks) {
//                 master.schedule_task(t);
//             }

//             auto end = std::chrono::high_resolution_clock::now();

//             auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

//             std::cout << "\n\nprogram running time: " << duration << " мс\n";

//             master.print_statistics(tasks);
//         }
//     }
// }