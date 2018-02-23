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
#include "ttytimer.h"
#include <thread>
#include <chrono>

using namespace std;

#define WORLD_WIDTH 50
#define WORLD_HEIGHT 20

string convert(string s, int numRows);

class token {
public:
    virtual bool is_settings() {
        return false;
    }
    
    virtual bool is_add_task() {
        return false;
    }
};

//class PomodoroTimer {
//private:
//    float
//
//public:
//    PomodoroTimer() {
//
//    }
//}

int main(int argc, const char * argv[]) {
    // insert code here...
    auto cmdl = argh::parser(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
    
    float sh = 0.0f, lo = 0.0f, ti = 0.0f;
    
    cmdl("-short", 1) >> sh;
    cmdl("-long", 1) >> lo;
    cmdl("-time", 1) >> ti;
    
    cout << sh << endl << lo << endl << ti << endl;
    
    char *t = "0:0:09";
    
    char *t1 = "0:0:02";
    cout << "starting first" << endl;
    start(t);
    
    int cnt = 0;
    while(cnt <= 10000000) {
        cout << "starting second" << endl;
        if (cnt == 10000000) {
        start(t1);
        }
        cnt++;
    }

    return 0;
}


