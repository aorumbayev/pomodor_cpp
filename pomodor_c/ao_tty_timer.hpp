//
//  ao_tty_timer.hpp
//  pomodor_c
//
//  Created by Altynbek Orumbayev on 2/27/18.
//  Copyright © 2018 Altynbek Orumbayev. All rights reserved.
//

#ifndef ao_tty_timer_hpp
#define ao_tty_timer_hpp

#include <stdio.h>
#include "ttytimer.h"
#include <string>
#include <iomanip>
#include "NotificationCenter.hpp"

std::string formatted_time(int minutes) {
    int hour, min, sec, time = minutes * 60;
    hour = time/3600;
    time = time%3600;
    min = time/60;
    time = time%60;
    sec = time;
    
    std::stringstream buffer;
    
    //TODO: - Fix
    buffer << std::setw(2) << std::setfill('0') << hour << ":"
    << std::setw(2) << std::setfill('0') << min << ":"
    << std::setw(2) << std::setfill('0') << sec;
    
    return buffer.str();
}

class PomodoroTimer {
private:
    
public:
    float sbreak_time, lbreak_time, task_time;
    PomodoroTimer(float sbreak, float lbreak, float task) : sbreak_time(sbreak), lbreak_time(lbreak), task_time(task) {};
    
    static void parse_time_arg(const char *time) {
        int digits[N_TIME_DIGITS];
        for (int i = 0; i < N_TIME_DIGITS; ++i) digits[i] = -1;
        
        int i = 0, remaining = 2;
        while (*time != '\0') {
            
            if (isdigit(*time)) {
                if (remaining == 0) {
                    puts("Too many digits in time argument");
                    exit(EXIT_FAILURE);
                }
                
                digits[i] = *time - '0';
                ++i;
                --remaining;
            } else if (*time == ':') {
                i += remaining;
                remaining = 2;
            } else {
                puts("Invalid character in time argument");
                exit(EXIT_FAILURE);
            }
            
            ++time;
        }
        
        fill_ttyclock_time(digits, ttyclock->date.hour);
        fill_ttyclock_time(digits + 2, ttyclock->date.minute);
        fill_ttyclock_time(digits + 4, ttyclock->date.second);
        memcpy(ttyclock->initial_digits, digits, N_TIME_DIGITS * sizeof(int));
        
        ttyclock->date.timestr[0] = ttyclock->date.hour[0] + '0';
        ttyclock->date.timestr[1] = ttyclock->date.hour[1] + '0';
        ttyclock->date.timestr[2] = ':';
        ttyclock->date.timestr[3] = ttyclock->date.minute[0] + '0';
        ttyclock->date.timestr[4] = ttyclock->date.minute[1] + '0';
        ttyclock->date.timestr[5] = ':';
        ttyclock->date.timestr[6] = ttyclock->date.second[0] + '0';
        ttyclock->date.timestr[7] = ttyclock->date.second[1] + '0';
        ttyclock->date.timestr[8] = '\0';
    }
    
    // starts timer instance with time in hh:mm:ss format
    void start() {
        
        const char *time = formatted_time(task_time).c_str();
        
        /* Alloc ttyclock */
        printf("%s", time);
        
        ttyclock = static_cast<ttyclock_t*>(malloc(sizeof(ttyclock_t)));
        assert(ttyclock != NULL);
        memset(ttyclock, 0, sizeof(ttyclock_t));
        
        /* Default color */
        ttyclock->option.color = COLOR_GREEN; /* COLOR_GREEN = 2 */
        
        parse_time_arg(time);
        /* Ensure input is anything but 0. */
        if (time_is_zero()) {
            puts("Time argument is zero");
            exit(EXIT_FAILURE);
        }
        
        init();
        attron(A_BLINK);
        while (ttyclock->running) {
            draw_clock();
            key_event();
            if (!time_is_zero()) update_hour();
            else {
                NotificationCenter::defaultNotificationCenter()->postNotification("My Observer");
                //break;
            }
        }
        
        endwin();
    }
    
    void restart(int time_c) {
        const char *time = formatted_time(time_c).c_str();
        parse_time_arg(time);
        
//        fill_ttyclock_time(ttyclock->initial_digits,
//                           ttyclock->date.hour);
//        fill_ttyclock_time(ttyclock->initial_digits + 2,
//                           ttyclock->date.minute);
//        fill_ttyclock_time(ttyclock->initial_digits + 4,
//                           ttyclock->date.second);
    }
    
};

#endif /* ao_tty_timer_hpp */
