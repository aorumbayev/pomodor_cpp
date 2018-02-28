//
//  main.cpp
//  pomodor_c
//
//  Created by Altynbek Orumbayev on 2/17/18.
//  Copyright © 2018 Altynbek Orumbayev. All rights reserved.
//

#include <iostream>
#include <string>
#include "argh.h"
#include "ao_tty_timer.hpp"
#include "NotificationCenter.hpp"

using namespace std;

PomodoroTimer *timer;

void handleNotification() {
    timer->restart(2);
}

int main(int argc, const char * argv[]) {
    // insert code here...
    auto cmdl = argh::parser(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
    
    float sh = 0.0f, lo = 0.0f, ti = 0.0f;
    
    cmdl("-short", 1) >> sh;
    cmdl("-long", 1) >> lo;
    cmdl("-time", 1) >> ti;
    timer = new PomodoroTimer(sh, lo, ti);
    
    NotificationCenter::defaultNotificationCenter()->addObserver(handleNotification, "My Observer");
    
    timer->start();
    
    return 0;
}
