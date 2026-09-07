#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

#include "Graph.hpp"
#include "Task.hpp"

class DataGraph
{
    Graph graph;

    public:
        DataGraph() = default;

        void generate_clustered_graph(
            int num_clusters,
            int nodes_per_cluster,
            int intra_degree,
            int num_neighbors,
            int edges_per_connection
        );

        void save(const std::string & filename) const;
        void load(const std::string & filename);

        Graph & get_graph() { return graph; }
        const Graph & get_graph() const { return graph; }

        void clear() { graph.clear(); }
};

class DataTask
{
    std::vector <Task> tasks;

public:
    DataTask() = default;

    void generate(
        const Graph & graph,
        int num_tasks,
        double average_interval,
        double max_compute_load,
        double max_data_size
    );

    void save(const std::string & filename) const;
    void load(const std::string & filename);

    std::vector<Task>& get_tasks() { return tasks; }
    const std::vector<Task>& get_tasks() const { return tasks; }

    void clear() { tasks.clear(); }
};

class Data {
    DataGraph graph_manager;
    DataTask task_manager;

public:
    Data() = default;

    void generate_all(
        int n_clusters, int n_nodes, int intra_deg, int num_neighbors, int edges_per_conn, // Параметры графа
        int n_tasks, double avg_arrival, double max_l, double max_d                     // Параметры задач
    );

    void load_graph(const std::string & graph_file) {
        graph_manager.load(graph_file);
    }

    void load_task(const std::string & tasks_file) {
        task_manager.load(tasks_file);
    }

    void load_full(const std::string & graph_file, const std::string & tasks_file) {
        graph_manager.load(graph_file);
        task_manager.load(tasks_file);
    }

    void save_full(const std::string & file) {
        graph_manager.save(file + "_graph");
        task_manager.save(file + "_tasks");
    }

    void save_graph(const std::string & filename) const { graph_manager.save(filename); }
    void save_tasks(const std::string & filename) const { task_manager.save(filename); }


    Graph& get_graph() { return graph_manager.get_graph(); }
    const Graph& get_graph() const { return graph_manager.get_graph(); }
    
    std::vector<Task>& get_tasks() { return task_manager.get_tasks(); }
    const std::vector<Task>& get_tasks() const { return task_manager.get_tasks(); }
};
