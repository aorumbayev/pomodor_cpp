//
//  main.cpp
//  pomodor_c
//
//  Created by Altynbek Orumbayev on 2/17/18.
//  Copyright © 2018 Altynbek Orumbayev. All rights reserved.
//

#include "ao_tty_timer_model.hpp"

// Third-Party frameworks
#include "argh.h"

using namespace std;

//MARK: - Local Timer Instance Declaration

PomodoroTimer *timer;

//MARK: - Main function
bool IsTerminalAvailable = false;

int main(int argc, const char * argv[]) {
    
    // Declaring and initializing time intervals
    float sh = 0.0f, lo = 0.0f, ti = 0.0f;
    
    // Declaring and initializing argument parser
    auto cmdl = argh::parser(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
    
    // Reading parameters and parsing arguments (with default values)
    cmdl("-short", 5) >> sh;
    cmdl("-long", 30) >> lo;
    cmdl("-time", 25) >> ti;
    
    for (int argi = 1; argi < argc; argi++)
    {
        if (strcmp(argv[argi], "--debug-in-terminal") == 0)
        {
            printf("Debugging in terminal enabled\n");
            getchar(); // Without this call debugging will be skipped
            break;
        }
    }
    
    char *term = getenv("TERM");
    
    IsTerminalAvailable = (term != NULL);
    
    if (IsTerminalAvailable)
        IsTerminalAvailable = (initscr() != NULL);
    
    // ======================
    
    // Initializing and starting timer
    timer = new PomodoroTimer(sh, lo, ti, 2);
    timer->start();
    
    // ======================
    
    if (IsTerminalAvailable)
    {
        printw("Press any key to exit...");
        refresh();
        
        getch();
        
        endwin();
    }

    return 0;
}
