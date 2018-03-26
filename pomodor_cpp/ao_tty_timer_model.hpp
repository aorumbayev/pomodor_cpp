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
    std::vector<std::string> quote_strings;
    
    // Float values storing the timer's time intervals
    // for work and breaks.
    float sbreak_time, lbreak_time, task_time;
    
    // Counter used to identify when to shoot long break
    int sbreak_counter, num_of_timers;
    
    // Enum that defines different types of time options.
    enum TimeOptions {ShortBreak, LongBreak, WorkTime};
    
    // Instance of enum used for identifying in which
    // mode the timer is currently running.
    TimeOptions time_options;
    
    // Vectors storing simple quotes for work and break modes
    std::vector<std::string> work_quotes {
        "Keep pushing!",
        "You can do it!",
        "Do work. Now!",
        "Just do it!",
        "Keep pushing!"
    };
    
    std::vector<std::string> break_quotes {
        "Chill. Now!",
        "Get some coffee!",
        "Relax for a bit.",
        "Go have a power nap!"
        "Look in the mirror and relax."
    };
    
    // This method updates the random quotes vector depending on current timer mode
    void update_random_quotes() {
        quotes_vector.clear();
        for (int i = 0; i < num_of_timers; i++) {
            std::string quote = time_options == TimeOptions::WorkTime ?
            work_quotes[rand() % work_quotes.size()] :
            break_quotes[rand() % break_quotes.size()];
            quote_strings.push_back(quote);
        }
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
    
    int get_random_color() {
        return (int)rand() % 7;
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
    
    PomodoroTimer(float sbreak, float lbreak, float task, int num_of_timers) : sbreak_time(sbreak), lbreak_time(lbreak), task_time(task), num_of_timers(num_of_timers) {};
    
    //MARK: - Main public methods
    
    // Starts timer instance by initializing the console and printing values
    void start() {
        // ====================================
        // =======Legacy Code================
        
        const char *time = formatted_time(task_time).c_str();
        time_options = TimeOptions::WorkTime;
        
        update_random_quotes();
        
        /* Alloc ttyclock */
        printf("%s", time);
        
        ttyclock = static_cast<ttyclock_t*>(malloc(sizeof(ttyclock_t)));
        assert(ttyclock != NULL);
        memset(ttyclock, 0, sizeof(ttyclock_t));
        
        /* Set number of timers*/
        ttyclock->num_of_timers = this->num_of_timers;
        
        /* Default color */
        ttyclock->option.color = COLOR_GREEN; /* COLOR_GREEN = 2 */
        
        atexit(cleanup);
        
        parse_time_arg(time);
        
        /* Ensure input is anything but 0. */
        for (auto const& x : timers_map) {
            if (time_is_zero(x.first)) {
                puts("Time argument is zero");
                exit(EXIT_FAILURE);
            }
        }
        
        init();
        attron(A_BLINK);
        while (ttyclock->running) {
            draw_clock();
            key_event();
            for (auto const& x : timers_map)
            {
                bool status = timers_execution_vector[x.first];
                if (!time_is_zero(x.first)) update_hour(x.first);
            }
            // ====================================
            // =======Modified Part================
            update_time_option();
            update_random_quotes();
            restart();
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
        
        for (auto const& x : timers_map)
        {
            WINDOW *window = x.second;
            bool window_execution_status = timers_execution_vector[x.first];
            WINDOW *quote_window = quotes_vector[x.first];
            date *cur_date = dates_of_timers[x.first];
            
            /* Change the colours to blink at certain times. */
            if (time(NULL) % 2 == 0) {
                dotcolor = COLOR_PAIR(2);
                if (time_is_zero(x.first)) numcolor = 2;
            }
            
            if (window_execution_status) {
                /* Draw hour numbers */
                draw_number(cur_date->hour[0], 1, 1, numcolor);
                draw_number(cur_date->hour[1], 1, 8, numcolor);
                
                /* 2 dot for number separation */
                //            wbkgdset(window, dotcolor);
                mvwaddstr(window, 2, 16, "  ");
                mvwaddstr(window, 4, 16, "  ");
                
                /* Draw minute numbers */
                draw_number(cur_date->minute[0], 1, 20, numcolor);
                draw_number(cur_date->minute[1], 1, 27, numcolor);
                /* Draw the date */
                if (ttyclock->option.bold) wattron(window, A_BOLD);
                else wattroff(window, A_BOLD);
                
                /* Draw second frame. */
                /* Again 2 dot for number separation */
                wbkgdset(window, dotcolor);
                mvwaddstr(window, 2, NORMFRAMEW, "  ");
                mvwaddstr(window, 4, NORMFRAMEW, "  ");
                
                /* Draw second numbers */
                draw_number(cur_date->second[0], 1, 39, numcolor);
                draw_number(cur_date->second[1], 1, 46, numcolor);
            }
            
            wbkgdset(quote_window, (COLOR_PAIR(2)));
            mvwprintw(quote_window, (DATEWINH / 2), 1, "                        ");
            mvwprintw(quote_window, (DATEWINH / 2), 1, quote_strings[x.first].c_str());
            wrefresh(quote_window);
        }
    }
    
#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif
    
    void key_event(void) {
        int i, c;
        
        // s delay and ns delay.
        struct timespec length = { 1, 0 };
        //        char ch = getch(); endwin(); printf("KEY NAME : %s - %d\n", keyname(ch),ch);
        
        
        switch(c = wgetch(stdscr)) {
            case 'q':
            case 'Q':
                ttyclock->running = false;
                ttyclock->interupted = true;
                break;
                
            case CTRL('o'):
                ttyclock->running = false;
                break;
            case 'r':
            case 'R':
                for (auto const& x : timers_map)
                {
                    date *cur_date = dates_of_timers[x.first];
                    fill_ttyclock_time(ttyclock->initial_digits,
                                       cur_date->hour);
                    fill_ttyclock_time(ttyclock->initial_digits + 2,
                                       cur_date->minute);
                    fill_ttyclock_time(ttyclock->initial_digits + 4,
                                       cur_date->second);
                }
                break;
                
            default:
                nanosleep(&length, NULL);
                
                for (i = 0; i < 8; ++i) {
                    if (c == (i + '0')) {
                        timers_execution_vector[i - 1] = !timers_execution_vector[i - 11];
                        //                        ttyclock->option.color = i;
                        //                        init_pair(1, ttyclock->bg, i);
                        //                        init_pair(2, i, ttyclock->bg);
                    }
                }
                
                break;
        }
    }
};

#endif /* ao_tty_timer_hpp */
