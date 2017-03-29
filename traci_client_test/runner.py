#!/usr/bin/env python

import os
import sys
import optparse
import subprocess
import random
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
    

def run():
    """execute the TraCI control loop"""
    traci.init(PORT)
    print("Server version: " + str(traci.getVersion()))
    print("Server timestep: " + str(traci.simulation.getDeltaT()))
    for i in range(0, 1000):
        traci.simulationStep()
        print("Current SIM time: " + str(traci.simulation.getCurrentTime()))
        print("N Departed vehicles: " + str(traci.simulation.getDepartedNumber()))
        print("N Arrived vehicles: " + str(traci.simulation.getArrivedNumber()))
        dep = traci.simulation.getDepartedIDList()
        arr = traci.simulation.getArrivedIDList()
        total_c = traci.vehicle.getIDCount()
        total_l = traci.vehicle.getIDList()
        total_l.sort()

        for v in dep:
            traci.vehicle.changeLane(v, 5, 10000)
            traci.vehicle.slowDown(v, 70, 1000)
        
        print("Departed: " + str(dep))
        print("Arrived: " + str(arr))
        print("N Total: " + str(total_c))
        print("Total: " + str(total_l))

        if len(total_l) > 0:
            vhc = random.choice(total_l)
            speed = traci.vehicle.getSpeed(vhc)
            position = traci.vehicle.getPosition(vhc)

            print("Vehicle " + vhc + "'s speed: " + str(speed))
            print("Vehicle " + vhc + "'s speed: " + str(position))
        
        time.sleep(0.1)
    traci.close()

#if __name__ == '__main__':
#    run()
