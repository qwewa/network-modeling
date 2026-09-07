#pragma once

#include <iostream>
#include "Graph.hpp"

struct Task 
{
    int id;
    int source_id;
    int assigned_node_id = -1;

    double compute_load;
    double data_size;
    double arrival_time;

    double start_time = 0.0;
    double finish_time = 0.0;

    Task(int id, int src, double load, double data, double arrival):
        id(id), source_id(src), compute_load(load), 
        data_size(data), arrival_time(arrival) {}

    void print_info() const {
        std::cout << "[Task " << id << "] "
                  << "Src: " << source_id << " | "
                  << "Load: " << compute_load << " | "
                  << "Data: " << data_size << " | "
                  << "Arr: " << arrival_time;

        if (assigned_node_id != -1)
        {
            std::cout << " -> Node: " << assigned_node_id;
        }
        std::cout << std::endl;
    }
};