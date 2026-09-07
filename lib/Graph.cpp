#include "Graph.hpp"

using namespace std;

shared_ptr<Node> Edge::get_neighbor(shared_ptr<Node> node) const 
{
    if (node == node1) return node2;
    if (node == node2) return node1;

    return nullptr;
}

shared_ptr<Node> Graph::add_node(int id, int cluster_id, double performance) 
{
    auto node = make_shared<Node>(id, cluster_id, performance);
    nodes.push_back(node);
    return node;
}

shared_ptr<Edge> Graph::add_edge(shared_ptr<Node> node1, shared_ptr<Node> node2, double bandwidth)
{
    auto edge = make_shared<Edge>(bandwidth, node1, node2);

    node1 -> edges.push_back(edge);
    node2 -> edges.push_back(edge);

    edges.push_back(edge);

    return edge;
}

shared_ptr<Node> Graph::get_node(int id) const
{
    for (const auto & node : nodes)
    {
        if (node->id == id) 
        {
            return node;
        }
    }
    return nullptr;
}

void Graph::clear()
{
    for (auto & node : nodes) node->edges.clear();

    edges.clear();
    nodes.clear();
}