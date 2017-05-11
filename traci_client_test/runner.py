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
    print(str(traci.simulation.getNetBoundary()))
    traci.simulation.subscribe([112, 115])
    traci.vehicle.subscribe("x",[0, 1])
##    print(str(traci.edge.getIDList()))
##    print(str(traci.edge.getIDCount()))
##    print(str(traci.junction.getIDList()))
##    print(str(traci.junction.getIDCount()))
    for i in range(0, 12100):
        traci.simulationStep()
        traci.simulation.getSubscriptionResults()
        traci.vehicle.getSubscriptionResults()
##        print(str(traci.simulation.getCurrentTime()))
##        print("Current SIM time: " + str(traci.simulation.getCurrentTime()))
##        print("N Departed vehicles: " + str(traci.simulation.getDepartedNumber()))
##        print("N Arrived vehicles: " + str(traci.simulation.getArrivedNumber()))
##        dep = traci.simulation.getDepartedIDList()
##        arr = traci.simulation.getArrivedIDList()
##        total_c = traci.vehicle.getIDCount()
##        total_l = traci.vehicle.getIDList()
##        total_l.sort()
##
##        for v in dep:
##            traci.vehicle.changeLane(v, 5, 10000)
##            traci.vehicle.slowDown(v, 0, 20000)
##            traci.vehicle.setColor(v, (255, 0, 0, 0))
##            traci.vehicle.setSpeed(v, 8.3)
##            traci.vehicle.setMaxSpeed(v, 8.3)
        
##        print("Departed: " + str(dep))
##        print("Arrived: " + str(arr))
##        print("N Total: " + str(total_c))
##        print("Total: " + str(total_l))

        #if len(total_l) > 0:
            #vhc = random.choice(total_l)
            #speed = traci.vehicle.getSpeed(vhc)
            #position = traci.vehicle.getPosition(vhc)

            #print("Vehicle " + vhc + "'s speed: " + str(speed) + "m/s")
            #print("Vehicle " + vhc + "'s position: " + str(position))
        
        #time.sleep(0.2)
    traci.close()

if __name__ == '__main__':
    run()
