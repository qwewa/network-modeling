#include "Resource.hpp"

using namespace std;

double ResourceTimeline::find_available_start(double arrival, double duration) const
{
    double t = arrival;

    for (const auto& interval : busy)
    {
        if (t + duration <= interval.start)
        {
            return t;
        }

        if (t < interval.end)
        {
            t = interval.end;
        }
    }

    return t;
}

void ResourceTimeline::reserve(double start, double end)
{
    Interval new_interval{start, end};

    vector<Interval> result;
    
    auto it = busy.begin();
    
    while (it != busy.end() && it->end < new_interval.start)
    {
        result.push_back(*it);
        it++;
    }
    
    while (it != busy.end() && it->start <= new_interval.end)
    {
        new_interval.start = min(new_interval.start, it -> start);
        new_interval.end = max(new_interval.end, it -> end);
        it++;
    }
    
    result.push_back(new_interval);
    
    while (it != busy.end())
    {
        result.push_back(*it);
        ++it;
    }
    
    busy = std::move(result);
}

double ResourceTimeline::time_resource_is_free() const
{
    if (busy.empty())
        return 0;

    return busy.back().end;
}