#include <vector>
#include <iostream>

struct Interval 
{
    double start;
    double end;
};

class ResourceTimeline
{
    std::vector <Interval> busy;

    public:
        double find_available_start(double arrival, double duration) const;
        void reserve(double start, double end);
        double time_resource_is_free() const;

        const std::vector <Interval> & get_busy() const { return busy; }
        
        friend std::ostream& operator<<(std::ostream& os, const ResourceTimeline& rt) {
            os << "[";
            for (size_t i = 0; i < rt.busy.size(); i++) {
                os << "[" << rt.busy[i].start << ", " << rt.busy[i].end << "]";
            }
            os << "]";
            return os;
        }

};

class Resource
{
    protected:
        ResourceTimeline timeline;

    public:
        double find_available_start(double arrival, double duration) const {
            return timeline.find_available_start(arrival, duration);
        }

        void reserve(double start, double end) {
            timeline.reserve(start, end);
        }

        void print_busy() {
            std::cout << timeline << std::endl;
        }

        double time_resource_is_free() {
            return timeline.time_resource_is_free();
        }

        virtual ~Resource() = default;
};
