#!/usr/bin/env python
import os
import random
import sys
import time

# we need to import python modules from the $SUMO_HOME/tools directory
try:
    sys.path.append(os.path.join(os.path.dirname(
        __file__), '..', '..', '..', '..', "tools"))  # tutorial in tests
    sys.path.append(os.path.join(os.environ.get("SUMO_HOME", os.path.join(
        os.path.dirname(__file__), "..", "..", "..")), "tools"))  # tutorial in docs
    from sumolib import checkBinary
except ImportError:
    sys.exit(
        "please declare environment variable 'SUMO_HOME' as the root directory of your sumo installation (it should contain folders 'bin', 'tools' and 'docs')")

import traci
PORT = 8245
new_route = ["2:6c", "6c:25", "25:15"]
affected_cars = [] 

def run():
    """execute the TraCI control loop"""
    traci.init(PORT)
    print("Server version: " + str(traci.getVersion()))
    traci.vehicle.subscribe("x",[0]) # sub to list of vehicles in sim
    for i in range(0, 10000):
        traci.simulationStep()
        car_list = traci.vehicle.getSubscriptionResults("x")[0] # get list
        for car in car_list:
            current_road = traci.vehicle.getRoadID(car) # get road
            if (current_road == new_route[0]) and (car not in affected_cars):
                print("route change for " + str(car))
                traci.vehicle.setColor(car, (255, 0, 0, 0))
                traci.vehicle.setRoute(car, new_route)
                affected_cars.append(car)
    traci.close()

if __name__ == '__main__':
    run()
