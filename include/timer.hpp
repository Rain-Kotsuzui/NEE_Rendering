#ifndef TIMER_HPP
#define TIMER_HPP
#include <chrono>
#include <iomanip>
#include <iostream>

class Timer
{
public:
    Timer(long tot) : start(std::chrono::high_resolution_clock::now()), tot(tot), now(std::chrono::high_resolution_clock::now())
    {
        duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
        now_pixels = 0;
    }
    void update()
    {
        now_pixels++;
        now = std::chrono::high_resolution_clock::now();
        duration =
            std::chrono::duration_cast<std::chrono::microseconds>(now - start);
        time = duration.count() / 1e6;
        std::cout << std::fixed << std::setprecision(2)
                  << time * (tot * 1.0 / now_pixels - 1) << " seconds remain,now: "
                  << now_pixels*100.0/tot 
                  <<"%"
                  <<std::endl;
        std::cout << "\033[2J\033[1;1H";
    }
    float getTime() const
    {
        return time;
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::chrono::time_point<std::chrono::high_resolution_clock> now;
    std::chrono::duration<long long, std::micro> duration;
    long tot;
    long now_pixels;
    float time;
};

#endif
