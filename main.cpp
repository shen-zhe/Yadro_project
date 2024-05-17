#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "club.h"


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile) {
        std::cerr << "Error opening file: " << argv[1] << std::endl;
        return 1;
    }

    int numTables;
    std::string startTime, endTime;
    int hourlyRate;
    std::vector<Event> events;
    std::unordered_map<std::string, Client> clients;
    std::unordered_map<int, Table> tables;
    std::queue<Client> waitingQueue;
    std::vector<Event> outputEvents;

    if (!(inputFile >> numTables) || numTables <= 0) {
        std::cerr << "Error in line 1" << std::endl;
        return 1;
    }

    if (!(inputFile >> startTime >> endTime) || !isTimeValid(startTime) || !isTimeValid(endTime) || timeInMinutes(startTime) >= timeInMinutes(endTime)) {
        std::cerr << "Error in line 2" << std::endl;
        return 1;
    }

    if (!(inputFile >> hourlyRate) || hourlyRate <= 0) {
        std::cerr << "Error in line 3" << std::endl;
        return 1;
    }

    std::string line;
    std::getline(inputFile, line); // Skip the remaining part of line 3
    int lineNumber = 4;

    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        std::string time;
        int id;
        std::vector<std::string> details;
        std::string detail;

        if (!(iss >> time >> id)) {
            std::cerr << "Error in line " << lineNumber << std::endl;
            return 1;
        }

        while (iss >> detail) {
            details.push_back(detail);
        }

        if (!isTimeValid(time)) {
            std::cerr << "Error in line " << lineNumber << std::endl;
            return 1;
        }

        if (id < 1 || id > 4 || (id == 1 && details.size() != 1) || (id == 2 && details.size() != 2) || (id == 3 && details.size() != 1) || (id == 4 && details.size() != 1)) {
            std::cerr << "Error in line " << lineNumber << std::endl;
            return 1;
        }

        if ((id == 1 || id == 3 || id == 4) && !isValidClientName(details[0])) {
            std::cerr << "Error in line " << lineNumber << std::endl;
            return 1;
        }

        events.push_back({time, id, details, ORIGINAL});
        lineNumber++;
    }

    std::sort(events.begin(), events.end(), compareEvents);

    for (int i = 1; i <= numTables; ++i) {
        tables[i] = {i, 0, 0, ""};
    }

    std::cout << startTime << std::endl;

    for (const auto& event : events) {
        outputEvents.push_back(event);
        if (event.id == 1) {
            std::string clientName = event.details[0];
            if (clients.find(clientName) != clients.end()) {
                outputEvents.push_back({event.time, 13, {"YouShallNotPass"}, OUTPUT});
                continue;
            }
            if (timeInMinutes(event.time) < timeInMinutes(startTime) || timeInMinutes(event.time) >= timeInMinutes(endTime)) {
                outputEvents.push_back({event.time, 13, {"NotOpenYet"}, OUTPUT});
                continue;
            }
            clients[clientName] = {clientName, event.time, -1, false};
        } else if (event.id == 2) {
            std::string clientName = event.details[0];
            int tableNumber = std::stoi(event.details[1]);
            if (clients.find(clientName) == clients.end()) {
                outputEvents.push_back({event.time, 13, {"ClientUnknown"}, OUTPUT});
                continue;
            }
            Client& client = clients[clientName];
            if (tables[tableNumber].occupiedSince.empty() || tables[tableNumber].occupiedSince == client.arrivalTime) {
                if (client.tableNumber != -1) {
                    Table& oldTable = tables[client.tableNumber];
                    int occupiedTime = timeInMinutes(event.time) - timeInMinutes(oldTable.occupiedSince);
                    oldTable.occupiedTime += occupiedTime;
                    oldTable.revenue += ((occupiedTime + 59) / 60) * hourlyRate;
                    oldTable.occupiedSince = "";
                }
                client.tableNumber = tableNumber;
                tables[tableNumber].occupiedSince = event.time;
            } else {
                outputEvents.push_back({event.time, 13, {"PlaceIsBusy"}, OUTPUT});
                continue;
            }
        } else if (event.id == 3) {
            std::string clientName = event.details[0];
            if (clients.find(clientName) == clients.end()) {
                outputEvents.push_back({event.time, 13, {"ClientUnknown"}, OUTPUT});
                continue;
            }
            bool freeTableFound = false;
            for (const auto& [number, table] : tables) {
                if (table.occupiedSince.empty()) {
                    freeTableFound = true;
                    break;
                }
            }
            if (freeTableFound) {
                outputEvents.push_back({event.time, 13, {"ICanWaitNoLonger!"}, OUTPUT});
                continue;
            }
            clients[clientName].waiting = true;
            waitingQueue.push(clients[clientName]);
        } else if (event.id == 4) {
            std::string clientName = event.details[0];
            if (clients.find(clientName) == clients.end()) {
                outputEvents.push_back({event.time, 13, {"ClientUnknown"}, OUTPUT});
                continue;
            }
            Client& client = clients[clientName];
            if (client.tableNumber != -1) {
                Table& table = tables[client.tableNumber];
                int occupiedTime = timeInMinutes(event.time) - timeInMinutes(table.occupiedSince);
                table.occupiedTime += occupiedTime;
                table.revenue += ((occupiedTime + 59) / 60) * hourlyRate;
                table.occupiedSince = "";
                if (!waitingQueue.empty()) {
                    Client nextClient = waitingQueue.front();
                    waitingQueue.pop();
                    nextClient.tableNumber = client.tableNumber;
                    nextClient.waiting = false;
                    tables[client.tableNumber].occupiedSince = event.time;
                    clients[nextClient.name] = nextClient;
                    outputEvents.push_back({event.time, 12, {nextClient.name, std::to_string(client.tableNumber)}, OUTPUT});
                }
            }
            clients.erase(clientName);
        }
    }

    for (const auto& [name, client] : clients) {
        outputEvents.push_back({endTime, 11, {name}, OUTPUT});
    }

    std::sort(outputEvents.begin(), outputEvents.end(), compareEvents);

    for (const auto& event : outputEvents) {
        std::cout << event.time << " " << event.id;
        for (const auto& detail : event.details) {
            std::cout << " " << detail;
        }
        std::cout << std::endl;
    }

    std::cout << endTime << std::endl;

    for (int i = 1; i <= numTables; ++i) {
        Table& table = tables[i];
        int occupiedTime = table.occupiedTime;
        if (!table.occupiedSince.empty()) {
            occupiedTime += timeInMinutes(endTime) - timeInMinutes(table.occupiedSince);
            table.revenue += ((occupiedTime + 59) / 60) * hourlyRate;
        }
        std::cout << table.number << " " << table.revenue << " " << minutesToTime(occupiedTime) << std::endl;
    }

    return 0;
}
