#pragma once

#include "Worker.hpp"

WorkerResult evaluate_node_openmp(const Task & task, int target_id, const Graph& graph);

void run_openmp_task(const Task & task, const Graph & graph, int num_threads, WorkerResult & out_best_result);

void finalize_openmp_task(Graph& graph, 
                          const WorkerResult& absolute_best);