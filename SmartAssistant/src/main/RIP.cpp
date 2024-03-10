#include <vector>
#include <iostream>

struct Route {
    std::string destinationNetwork;
    int distance;
    std::string nextHop;
};

void updateRoutingTable(std::vector<Route>& routingTable, const std::vector<Route>& receivedRoutes, const std::string& sender) {
    for (const auto& receivedRoute : receivedRoutes) {
        Route updatedRoute = {receivedRoute.destinationNetwork, receivedRoute.distance + 1, sender};

        bool isRouteUpdated = false;
        for (auto& route : routingTable) {
            if (route.destinationNetwork == updatedRoute.destinationNetwork) {
                if (route.nextHop == sender) {
                    route.distance = updatedRoute.distance;
                } else if (updatedRoute.distance < route.distance) {
                    route.distance = updatedRoute.distance;
                    route.nextHop = updatedRoute.nextHop;
                }
                isRouteUpdated = true;
            }
        }

        if (!isRouteUpdated) {
            routingTable.push_back(updatedRoute);
        }
    }
}

int main() {
    std::vector<Route> routingTable;
    std::vector<Route> receivedRoutes = {{"192.168.1.0", 1, "Router_X"}};

    updateRoutingTable(routingTable, receivedRoutes, "Router_X");

    // Display the updated routing table.
    for (const auto& route : routingTable) {
        std::cout << "Destination Network: " << route.destinationNetwork
                  << ", Distance: " << route.distance
                  << ", Next Hop: " << route.nextHop << std::endl;
    }

    return 0;
}
