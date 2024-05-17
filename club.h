#ifndef CLUB_H
#define CLUB_H

#include <string>
#include <vector>
#include <unordered_map>
#include <queue>

enum EventType {
    ORIGINAL,
    OUTPUT
};

struct Event {
    std::string time;
    int id;
    std::vector<std::string> details;
    EventType type;
};

struct Client {
    std::string name;
    std::string arrivalTime;
    int tableNumber;
    bool waiting;
};

struct Table {
    int number;
    int revenue;
    int occupiedTime;
    std::string occupiedSince;
};

int timeInMinutes(const std::string& time);
std::string minutesToTime(int minutes);
bool isTimeValid(const std::string& time);
bool isValidClientName(const std::string& name);
bool compareEvents(const Event& a, const Event& b);

#endif // CLUB_H