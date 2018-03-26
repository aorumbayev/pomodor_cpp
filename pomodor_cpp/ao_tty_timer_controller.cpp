//
//  main.cpp
//  pomodor_c
//
//  Created by Altynbek Orumbayev on 2/17/18.
//  Copyright © 2018 Altynbek Orumbayev. All rights reserved.
//

#include "ao_tty_timer_model.hpp"
#include <iostream>
#include <getopt.h>

using namespace std;

//MARK: - Local Timer Instance Declaration

void PrintHelp()
{
    std::cout <<
    "--num_timers <n>:    Set number of timers to be executed\n"
    "--user:              Specify short break, long break and timer time in following format {$0:$1:$2}\n"
    "--help:              Show help\n";
    exit(1);
}

//MARK: - Main function
bool IsTerminalAvailable = false;

int main(int argc, char ** argv) {
    
    // Number of timers
    int number_of_timers = 0;
    
    // Setup for `getopt_long` usage
    const char* const short_opts = "n:u";
    const option long_opts[] = {
        {"num_timers", 1, nullptr, 'n'},
        {"user", 1, nullptr, 'u'},
        {"help", 0, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };
    
    // Vector of users, each user is a simple struct holding individual time options.
    vector<User> users;
    
    while (true)
    {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);
        
        if (-1 == opt)
            break;
        
        switch (opt)
        {
            case 'n':
                number_of_timers = stoi(optarg);
                if (number_of_timers > 5) {
                    cout << "This CLI is not optimized for adding more than 5 timers simultaneosly" << endl;
                    return 1;
                }
                break;
                
            case 'u': {
                
                if (users.size() > number_of_timers) {
                    cout << "Number of users is exceeding number of timers!" << endl;
                    return 1;
                }
                
                users.push_back(User(optarg));
                break;
            }
                
            case 'h': // -h or --help
            case '?': // Unrecognized option
            default:
                PrintHelp();
                break;
        }
    }
    
    if (number_of_timers == 0) {
        cout << "Please, specify number of timers to be executed!" << endl;
        return 1;
    }
    
    if (users.size() != number_of_timers) {
        for (size_t i = users.size() ; i < number_of_timers; i++) {
            users.push_back(User());
        }
    }

    for (auto t : users) {
        cout << t.sh << " : " << t.lo << " : " << t.ti << " : " << endl;
    }
    
    PomodoroTimer timer = PomodoroTimer(number_of_timers, users);
    timer.start();
    
    return 0;
}
