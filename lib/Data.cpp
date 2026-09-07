#include "Data.hpp"

#include <random>
#include <unordered_set>
#include <fstream>

using namespace std;
using json = nlohmann::json;

void DataGraph::generate_clustered_graph(
    int num_clusters,
    int nodes_per_cluster,
    int intra_degree,
    int num_neighbors,
    int edges_per_connection
)
{
    mt19937 gen(random_device{}());

    auto rand_node = [&](int max) {
        uniform_int_distribution<> dist(0, max - 1);
        return dist(gen);
    };

    auto rand_perf = [&]() {
        uniform_real_distribution<> dist(20.0, 80.0);
        return dist(gen);
    };

    auto high_bw = [&]() {
        uniform_real_distribution<> dist(40.0, 120.0);
        return dist(gen);
    };

    auto low_bw = [&]() {
        uniform_real_distribution<> dist(5.0, 25.0);
        return dist(gen);
    };

    graph.clear();

    int total_nodes = num_clusters * nodes_per_cluster;

    vector<shared_ptr<Node>> nodes;

    nodes.reserve(total_nodes);

    unordered_map<int, shared_ptr<Node>> node_map;

    for (int i = 0; i < total_nodes; i++)
    {
        auto node = graph.add_node(i, i / nodes_per_cluster, rand_perf());

        nodes.push_back(node);
        node_map[i] = node;
    }

    vector<vector<int>> clusters(num_clusters);
    
    for (int i = 0; i < total_nodes; i++)
    {
        int cluster_id = i / nodes_per_cluster;

        clusters[cluster_id].push_back(i);
    }

    unordered_set<long long> edge_set;

    auto make_key = [](int u, int v)
    {
        if (u > v) std::swap(u, v);
        return (static_cast<long long>(u) << 32) | v;
    };

    // Создание внутренних связей в кластере

    for (const auto& cluster : clusters)
    {
        int size = cluster.size();

        for (int idx = 0; idx < size; ++idx)
        {
            int u = cluster[idx];

            vector<int> candidates = cluster;
            shuffle(candidates.begin(), candidates.end(), gen);

            int connections = 0;

            for (int v : candidates)
            {
                if (u == v) continue;

                long long key = make_key(u, v);
                if (edge_set.count(key)) continue;

                graph.add_edge(node_map[u], node_map[v], high_bw());
                edge_set.insert(key);

                connections++;
                if (connections >= intra_degree)
                    break;
            }
        }
    }

    // Создание цепочки между класстерами (Гарантирует связь)

    for (int c = 1; c < num_clusters; ++c)
    {
        int u = clusters[c - 1][rand_node(clusters[c - 1].size())];
        int v = clusters[c][rand_node(clusters[c].size())];

        graph.add_edge(node_map[u], node_map[v], low_bw());

        long long key = make_key(u, v);

        edge_set.insert(key);
    }

    // Создание дополнительных связей между класстерами

    for (int c1 = 0; c1 < num_clusters; ++c1)
    {

        vector<int> candidates;

        for (int c2 = 0; c2 < num_clusters; ++c2)
        {
            if (c2 != c1)
                candidates.push_back(c2);
        }

        shuffle(candidates.begin(), candidates.end(), gen);

        int connections = 0;

        for (int c2 : candidates)
        {
            if (connections >= num_neighbors)
                break;

            for (int k = 0; k < edges_per_connection; ++k)
            {
                int u = clusters[c1][rand_node(clusters[c1].size())];
                int v = clusters[c2][rand_node(clusters[c2].size())];

                long long key = make_key(u, v);
                if (edge_set.count(key)) continue;

                graph.add_edge(node_map[u], node_map[v], low_bw());
                edge_set.insert(key);
            }

            connections++;
        }
    }
}

void DataGraph::save(const string & filename) const
{
    json j;

    for (const auto & node : graph.get_nodes())
    {
        j["nodes"].push_back({
            {"id", node -> id},
            {"cluster_id", node -> cluster_id},
            {"performance", node -> performance}
        });
    }

    for (const auto & edge : graph.get_edges())
    {
        j["edges"].push_back({
            {"n1", edge -> node1 -> id},
            {"n2", edge -> node2 -> id},
            {"bandwidth", edge -> bandwidth}
        });
    }

    ofstream file(filename);

    file << j.dump(4);
}

void DataGraph::load(const string& filename)
{
    ifstream file(filename);

    if (!file.is_open()) return;

    json j;
    file >> j;

    graph.clear();
    unordered_map<int, shared_ptr<Node>> id_to_node;
    
    // Восстанавление узлов
    for (const auto& n_json : j["nodes"])
    {
        auto node = graph.add_node(
            n_json["id"].get<int>(),
            n_json["cluster_id"].get<int>(),
            n_json["performance"].get<double>()
        );
        id_to_node[node->id] = node;
    }

    // Восстанавление ребер
    for (const auto& e_json : j["edges"])
    {
        int id1 = e_json["n1"].get<int>();
        int id2 = e_json["n2"].get<int>();
        double bw = e_json["bandwidth"].get<double>();

        if (id_to_node.count(id1) && id_to_node.count(id2))
        {
            graph.add_edge(id_to_node[id1], id_to_node[id2], bw);
        }
    }
}

void DataTask::generate(
    const Graph& graph,
    int num_tasks,
    double average_interval,
    double max_compute_load,
    double max_data_size
) 
{
    tasks.clear();
    mt19937 gen(random_device{}());
    
    vector<int> node_ids;

    for(const auto& n : graph.get_nodes()) node_ids.push_back(n->id);

    if (node_ids.empty()) return;

    auto rand_src = [&]() {
        uniform_int_distribution<> dist(0, node_ids.size() - 1);
        return node_ids[dist(gen)];
    };

    auto rand_load = [&]() {
        uniform_real_distribution<> dist(100.0, max_compute_load);
        return dist(gen);
    };

    auto rand_data = [&]() {
        uniform_real_distribution<> dist(100.0, max_data_size);
        return dist(gen);
    };

    double lambda = 1.0 / average_interval;

    auto rand_arrival = [&]() {
        exponential_distribution<> dist(lambda);
        return dist(gen);
    };

    for (int i = 0; i < num_tasks; ++i) {
        tasks.emplace_back(
            i, 
            rand_src(), 
            rand_load(), 
            rand_data(), 
            rand_arrival()
        );
    }

    sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
        return a.arrival_time < b.arrival_time;
    });
}

void DataTask::save(const string& filename) const
{
    json j;

    for (const auto& t : tasks) 
    {
        j.push_back({
            {"id", t.id},
            {"source_id", t.source_id},
            {"load", t.compute_load},
            {"data", t.data_size},
            {"arrival", t.arrival_time}
        });
    }
    ofstream file(filename);

    file << j.dump(4);
}

void DataTask::load(const string& filename)
{
    ifstream file(filename);
    if (!file.is_open()) return;

    json j;
    file >> j;

    tasks.clear();
    for (const auto & t_json : j) {
        Task t(
            t_json["id"].get<int>(),
            t_json["source_id"].get<int>(),
            t_json["load"].get<double>(),
            t_json["data"].get<double>(),
            t_json["arrival"].get<double>()
        );
        tasks.push_back(t);
    }
    
    sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
        return a.arrival_time < b.arrival_time;
    });
}

void Data::generate_all(
        int n_clusters, int n_nodes, int intra_deg, int n_neighbors, int edges_per_neigh,
        int n_tasks, double avg_arrival, double max_load, double max_data
)
{
    graph_manager.clear();
    task_manager.clear();

    graph_manager.generate_clustered_graph(n_clusters, n_nodes, intra_deg, n_neighbors, edges_per_neigh);
    task_manager.generate(graph_manager.get_graph(), n_tasks, avg_arrival, max_load, max_data);
}
