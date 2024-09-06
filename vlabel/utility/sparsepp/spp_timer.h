

#ifndef spp_timer_h_guard
#define spp_timer_h_guard

#include <chrono>

namespace spp
{
    template<typename time_unit = std::milli>
    class Timer 
    {
    public:
        Timer()                 { reset(); }
        void reset()            { _start = _snap = clock::now();  }
        void snap()             { _snap = clock::now();  }

        float get_total() const { return get_diff<float>(_start, clock::now()); }
        float get_delta() const { return get_diff<float>(_snap, clock::now());  }
        
    private:
        using clock = std::chrono::high_resolution_clock;
        using point = std::chrono::time_point<clock>;

        template<typename T>
        static T get_diff(const point& start, const point& end) 
        {
            using duration_t = std::chrono::duration<T, time_unit>;

            return std::chrono::duration_cast<duration_t>(end - start).count();
        }

        point _start;
        point _snap;
    };
}

#endif 
