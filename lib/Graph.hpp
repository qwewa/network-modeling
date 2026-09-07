#pragma once

#include <vector>
#include <memory>
#include "Resource.hpp"

struct Edge;
struct Node;

struct Node: public Resource
{
    int id;
    int cluster_id;
    double performance;
    int amount_task = 0;

    std::vector <std::weak_ptr<Edge>> edges;

    Node(int node_id, int cluster_id, double perf):
        id(node_id), cluster_id(cluster_id), performance(perf) {}

    double computing_time(double data) const {
        return (data / performance);
    }
};

struct Edge: public Resource
{
    double bandwidth;

    std::shared_ptr<Node> node1;
    std::shared_ptr<Node> node2;

    Edge(double bw, std::shared_ptr<Node> n1, std::shared_ptr<Node> n2):
        bandwidth(bw), node1(n1), node2(n2) {}

    double transfer_time(double data) const {
        return data / bandwidth;
    }

    std::shared_ptr<Node> get_neighbor(std::shared_ptr<Node> node) const;
};

class Graph 
{
    std::vector<std::shared_ptr<Node>> nodes;
    std::vector<std::shared_ptr<Edge>> edges;

    public:

        std::shared_ptr<Node> add_node(int id, int cluster_id, double performance);
        std::shared_ptr<Edge> add_edge(std::shared_ptr<Node> n1, std::shared_ptr<Node> n2, double bandwidth);
        
        std::shared_ptr<Node> get_node(int id) const;
        
        std::vector<std::shared_ptr<Node>> & get_nodes() { return nodes; }
        const std::vector<std::shared_ptr<Node>> & get_nodes() const { return nodes; }
        
        std::vector<std::shared_ptr<Edge>> & get_edges() { return edges; }
        const std::vector<std::shared_ptr<Edge>> & get_edges() const { return edges; }

        std::size_t node_count() const { return nodes.size(); }
        void clear();
};