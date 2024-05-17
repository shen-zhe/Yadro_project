#include "club.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

int timeInMinutes(const std::string& time) {
    int hours = std::stoi(time.substr(0, 2));
    int minutes = std::stoi(time.substr(3, 2));
    return hours * 60 + minutes;
}


std::string minutesToTime(int minutes) {
    int hours = minutes / 60;
    minutes %= 60;
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":" << std::setw(2) << std::setfill('0') << minutes;
    return oss.str();
}


bool isTimeValid(const std::string& time) {
    if (time.size() != 5 || time[2] != ':') return false;
    int hours = std::stoi(time.substr(0, 2));
    int minutes = std::stoi(time.substr(3, 2));
    return hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60;
}


bool isValidClientName(const std::string& name) {
    for (char c : name) {
        if (!isalnum(c) && c != '_' && c != '-') return false;
    }
    return true;
}


bool compareEvents(const Event& a, const Event& b) {
    int timeA = timeInMinutes(a.time);
    int timeB = timeInMinutes(b.time);
    if (timeA == timeB) {
        return a.type < b.type;  // ORIGINAL events come before OUTPUT events
    }
    return timeA < timeB;
}