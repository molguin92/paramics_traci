#!/usr/bin/env python

import os
import sys
import optparse
import subprocess
import random
import time

SIMTIME = 0x70
DEPARTEDVHCLST = 0x74
ARRIVEDVHCLST = 0x7a

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
import random
PORT = 8245

affected = []

def run():
    """execute the TraCI control loop"""
    traci.init(PORT)
    print("Server version: " + str(traci.getVersion()))
    print("Server timestep: " + str(traci.simulation.getDeltaT()))
    print(str(traci.simulation.getNetBoundary()))
    traci.simulation.subscribe([SIMTIME, DEPARTEDVHCLST, ARRIVEDVHCLST])
    traci.vehicle.subscribe("x",[0, 1])
    for i in range(0, 12100):
        
        traci.simulationStep()
        simsubs = traci.simulation.getSubscriptionResults()
        vehsubs = traci.vehicle.getSubscriptionResults("x")

        print("Current SIM time: " + str(simsubs[SIMTIME]))
        
        dep = simsubs[DEPARTEDVHCLST]
        arr = simsubs[ARRIVEDVHCLST]
        carsinsim = vehsubs[0]

        for car in carsinsim:
            #traci.vehicle.setSpeed(car, 5.0)
            road = traci.vehicle.getRoadID(car)
            if road == "26:2" and car not in affected:
                #traci.vehicle.slowDown(car, 3.0, 10000)
                traci.vehicle.setSpeed(car, 5.0)
                affected.append(car)
            elif road != "26:2" and car in affected:
                traci.vehicle.setSpeed(car, -1.0)
                affected.remove(car)
        
        
        #time.sleep(0.1)
    traci.close()

if __name__ == '__main__':
    run()
