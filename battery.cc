/* Prints a battery bar to stdout, e.g.
 * ● ▣▣▣▣▣▢▢▢▢▢
 *
 * To compile: g++ -std=c++11 battery-bar.cpp -o battery-bar
 *
 * Copyright 2014 Emil Edholm <emil@edholm.it>
 */
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "battery.hh"
#include "common.hh"

using namespace std;

Battery::Battery() {
    init();
}

void Battery::init()
{
    string state = Common::read_first_line(BAT0_PATH + STATUS_FILE);
    if (state == "Charging")
        bat0.state = CHARGING;
    else if (state == "Discharging")
        bat0.state = DISCHARGING;
    else if (state == "Full")
        bat0.state = FULL;
    else
        bat0.state = UNKNOWN;

    long now = atol(Common::read_first_line(BAT0_PATH + ENERGY_NOW_FILE).c_str());
    long full = atol(Common::read_first_line(BAT0_PATH + ENERGY_FULL_FILE).c_str());
    bat0.percent = static_cast<int>(((double)now / full) * 100 + 0.5);
}

string Battery::battery_json() {
    string status;
    switch(bat0.state) {
        case CHARGING:
            status.append("  ");
            break;
        case DISCHARGING:
            status.append("◯ ");
            break;
        default:
            status.append("● ");
            break;
    }
    return status.append(Common::make_bar(bat0.percent, 10));
}


