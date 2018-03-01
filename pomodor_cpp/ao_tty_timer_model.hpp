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
#include <string>
#include <iomanip>
#include <sstream>
#include <vector>

// Third-Party frameworks
#include "ttytimer.h"

//MARK : - Helpers

/**
 * This method converts integer minutes into an std::string with format hh:mm:ss
 * @param minutes gets time in minutes.
 */
std::string formatted_time(int minutes) {
    int hour, min, sec, time = minutes * 60;
    hour = time/3600;
    time = time%3600;
    min = time/60;
    time = time%60;
    sec = time;
    
    std::stringstream buffer;
    
    buffer << std::setw(2) << std::setfill('0') << hour << ":"
    << std::setw(2) << std::setfill('0') << min << ":"
    << std::setw(2) << std::setfill('0') << sec;
    
    return buffer.str();
}

//MARK: - PomodoroTimer class declaration
class PomodoroTimer {

private:
    // Used to represent current motivational quote
    // that needs to be used as a label under clock.
    std::string current_quote;
    
    // Float values storing the timer's time intervals
    // for work and breaks.
    float sbreak_time, lbreak_time, task_time;
    
    // Counter used to identify when to shoot long break
    int sbreak_counter;
    
    // Enum that defines different types of time options.
    enum TimeOptions {ShortBreak, LongBreak, WorkTime};
    
    // Instance of enum used for identifying in which
    // mode the timer is currently running.
    TimeOptions time_options;
    
    // Vectors storing simple quotes for work and break modes
    std::vector<std::string> work_quotes {
        "Keep pushing!",
        "You can do it!",
        "Do work. Now!"
    };
    
    std::vector<std::string> break_quotes {
        "Chill. Now!",
        "Get some coffee!",
        "Relax for a bit."
    };
    
    // This method gets the random quote depending on current timer mode
    std::string get_random_quote() {
        return time_options == TimeOptions::WorkTime ?
        work_quotes[rand() % work_quotes.size()] :
        break_quotes[rand() % break_quotes.size()];
    }
    
    // This method gets the current float value of current timer mode
    float get_current_timer() {
        switch (time_options) {
            case ShortBreak:
                return sbreak_time;
                
            case LongBreak:
                return lbreak_time;
                
            case WorkTime:
                return task_time;
        }
    }
    
    // This method gets the random quote depending on current timer mode
    void update_time_option() {
        switch (time_options) {
            case ShortBreak:
                time_options = TimeOptions::WorkTime;
                sbreak_counter++;
                break;
                
            case LongBreak:
                time_options = TimeOptions::WorkTime;
                break;
                
            case WorkTime:
                if (sbreak_counter == 3) {
                    time_options = TimeOptions::LongBreak;
                    sbreak_counter = 0;
                } else {
                    time_options = TimeOptions::ShortBreak;
                }
                break;
        }
    }
    
public:
    
    //MARK: - Constructors
    
    PomodoroTimer(float sbreak, float lbreak, float task) : sbreak_time(sbreak), lbreak_time(lbreak), task_time(task) {};
  
    //MARK: - Main public methods
    
    // Starts timer instance by initializing the console and printing values
    void start() {
        // ====================================
        // =======Legacy Code================
    
        const char *time = formatted_time(task_time).c_str();
        time_options = TimeOptions::WorkTime;
        
        current_quote = get_random_quote();
        
        /* Alloc ttyclock */
        printf("%s", time);
        
        ttyclock = static_cast<ttyclock_t*>(malloc(sizeof(ttyclock_t)));
        assert(ttyclock != NULL);
        memset(ttyclock, 0, sizeof(ttyclock_t));
        
        /* Default color */
        ttyclock->option.color = COLOR_GREEN; /* COLOR_GREEN = 2 */
        
        atexit(cleanup);
        
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
            // ====================================
            // =======Modified Part================
            else {
                update_time_option();
                current_quote = get_random_quote();
                restart();
            }
            // ====================================
        }
        
        endwin();
        // ====================================
    }
    
    // Resets the clock and activate next timer mode
    void restart() {
        const char *time = formatted_time(get_current_timer()).c_str();
        parse_time_arg(time);
    }
    
    // Modified code from ttytimer.h that sets the custom quotes
    // instead of extra row of digits under the clock
    void draw_clock(void) {
        chtype dotcolor = COLOR_PAIR(1);
        unsigned int numcolor = 1;
        
        /* Change the colours to blink at certain times. */
        if (time(NULL) % 2 == 0) {
            dotcolor = COLOR_PAIR(2);
            if (time_is_zero()) numcolor = 2;
        }
        
        /* Draw hour numbers */
        draw_number(ttyclock->date.hour[0], 1, 1, numcolor);
        draw_number(ttyclock->date.hour[1], 1, 8, numcolor);
        
        /* 2 dot for number separation */
        wbkgdset(ttyclock->framewin, dotcolor);
        mvwaddstr(ttyclock->framewin, 2, 16, "  ");
        mvwaddstr(ttyclock->framewin, 4, 16, "  ");
        
        /* Draw minute numbers */
        draw_number(ttyclock->date.minute[0], 1, 20, numcolor);
        draw_number(ttyclock->date.minute[1], 1, 27, numcolor);
        
        /* Draw the date */
        if (ttyclock->option.bold) wattron(ttyclock->quotewin, A_BOLD);
        else wattroff(ttyclock->quotewin, A_BOLD);
        
        wbkgdset(ttyclock->quotewin, (COLOR_PAIR(2)));
        mvwprintw(ttyclock->quotewin, (DATEWINH / 2), 1, "                        ");
        mvwprintw(ttyclock->quotewin, (DATEWINH / 2), 1, current_quote.c_str());
        wrefresh(ttyclock->quotewin);
        
        /* Draw second frame. */
        /* Again 2 dot for number separation */
        wbkgdset(ttyclock->framewin, dotcolor);
        mvwaddstr(ttyclock->framewin, 2, NORMFRAMEW, "  ");
        mvwaddstr(ttyclock->framewin, 4, NORMFRAMEW, "  ");
        
        /* Draw second numbers */
        draw_number(ttyclock->date.second[0], 1, 39, numcolor);
        draw_number(ttyclock->date.second[1], 1, 46, numcolor);
    }
};

#endif /* ao_tty_timer_hpp */
