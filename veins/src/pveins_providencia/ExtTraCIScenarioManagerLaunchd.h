//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef SRC_VEINS_MODULES_APPLICATION_PVEINS_PROVIDENCIA_EXTTRACISCENARIOMANAGERLAUNCHD_H_
#define SRC_VEINS_MODULES_APPLICATION_PVEINS_PROVIDENCIA_EXTTRACISCENARIOMANAGERLAUNCHD_H_

#include "veins/modules/mobility/traci/TraCIScenarioManagerLaunchd.h"

class ExtTraCIScenarioManagerLaunchd: public Veins::TraCIScenarioManagerLaunchd {
public:
    ExtTraCIScenarioManagerLaunchd(): shut_down(false) {}
    virtual ~ExtTraCIScenarioManagerLaunchd(){}

    virtual void initialize(int stage);
    virtual void finish();

    bool shuttingDown();

protected:
    virtual void init_traci();

private:
    bool shut_down;
};

#endif /* SRC_VEINS_MODULES_APPLICATION_PVEINS_PROVIDENCIA_EXTTRACISCENARIOMANAGERLAUNCHD_H_ */
