import json
import networkx as nx
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

with open("build/graph_5_20_5_2_2") as f:
    data = json.load(f)

G = nx.Graph()
node_clusters = {}

for node in data["nodes"]:
    node_id = node["id"]
    G.add_node(node_id, performance=node["performance"])
    node_clusters[node_id] = node.get("cluster_id", "None")

for edge in data["edges"]:
    G.add_edge(edge["n1"], edge["n2"], weight=edge["bandwidth"])

unique_clusters = sorted(set(node_clusters.values()), key=lambda x: str(x))
color_scheme = plt.cm.tab10
cluster_colors = {
    cluster: color_scheme(i / max(1, len(unique_clusters) - 1))
    for i, cluster in enumerate(unique_clusters)
}

node_colors = [cluster_colors[node_clusters[node]] for node in G.nodes()]
pos = nx.spring_layout(G, k=1.5, iterations=100, seed=42)

intra_edges = []
inter_edges = []

for u, v in G.edges():
    if node_clusters[u] == node_clusters[v]:
        intra_edges.append((u, v))
    else:
        inter_edges.append((u, v))

plt.figure(figsize=(12, 8))

nx.draw_networkx_nodes(
    G, pos,
    node_color=node_colors,
    node_size=550,
    alpha=0.9
)

nx.draw_networkx_labels(
    G, pos,
    font_size=9
)

nx.draw_networkx_edges(
    G, pos,
    edgelist=intra_edges,
    alpha=0.45,
    width=1.5
)

nx.draw_networkx_edges(
    G, pos,
    edgelist=inter_edges,
    alpha=0.8,
    width=2.2,
    style="dashed"
)

edge_labels = {
    (u, v): f"{G[u][v]['weight']:.1f}"
    for u, v in G.edges()
}

nx.draw_networkx_edge_labels(
    G, pos,
    edge_labels=edge_labels,
    font_size=12
)

legend_patches = [
    mpatches.Patch(color=cluster_colors[cluster], label=f"Cluster {cluster}")
    for cluster in unique_clusters
]

plt.legend(handles=legend_patches, loc="upper right", title="Clusters")
plt.title("Generated clustered network topology")
plt.axis("off")
plt.tight_layout()
plt.savefig("network_graph_3x3.png", dpi=300)
plt.show()