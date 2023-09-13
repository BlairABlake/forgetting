#include <chrono>
#include <cmath>
#include <string.h>
#include "split.h"
using namespace std::chrono;

class ForgettingCurve {
public:
    ForgettingCurve() {
        update();
    }

    ForgettingCurve(time_t t, int s): start_date(t), S(s) {
        update();
    };

    float R() {
        float t = get_t();
        return exp(-1 * t/S);
    }

    void update() {
        start_date = system_clock::to_time_t(system_clock::now());
        S++;
    }

    std::string serialize() {
        return std::to_string(S) + " " + std::to_string(start_date);
    }

private:
    time_t start_date;
    int S = 0;

    float get_t() {
        time_t now =  system_clock::to_time_t(system_clock::now());
        time_t interval = now - start_date;
        return interval / (60 * 60 * 24);
    }
};