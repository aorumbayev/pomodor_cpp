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
    int num_of_timers;
    
    // Enum that defines different types of time options.
    enum TimeOptions {ShortBreak, LongBreak, WorkTime};
    
    // Map storing quotes and current execution mode per timer
    std::map<int, TimeOptions> timer_state;
    std::vector<int> timer_break_conters;
    
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
    
    void init_timer_states() {
        timer_state.clear();
        timer_break_conters.clear();
        for (int i = 0; i < num_of_timers; i++) {
            timer_state.emplace(i, TimeOptions::ShortBreak);
            timer_break_conters.push_back(0);
        }
    }
    
    // This method updates the random quotes vector depending on current timer mode
    void init_random_quotes() {
        quotes_vector.clear();
        for (int i = 0; i < num_of_timers; i++) {
            std::string quote = work_quotes[rand() % work_quotes.size()];
            quote_strings.push_back(quote);
        }
    }
    
    void update_random_quotes(int timer_id) {
        std::string quote = timer_state[timer_id] == TimeOptions::WorkTime ?
        work_quotes[rand() % work_quotes.size()] :
        break_quotes[rand() % break_quotes.size()];
        quote_strings[timer_id] = quote;
    }
    
    // This method gets the current float value of current timer mode
    float get_current_timer(int timer_id) {
        switch (timer_state[timer_id]) {
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
    void update_time_option(int timer_id) {
        TimeOptions time_options = timer_state[timer_id];
        switch (time_options) {
            case ShortBreak:
                time_options = TimeOptions::WorkTime;
                timer_break_conters[timer_id] = timer_break_conters[timer_id]++;
                break;
                
            case LongBreak:
                time_options = TimeOptions::WorkTime;
                break;
                
            case WorkTime:
                if (timer_break_conters[timer_id] == 3) {
                    time_options = TimeOptions::LongBreak;
                    timer_break_conters[timer_id] = 0;
                } else {
                    time_options = TimeOptions::ShortBreak;
                }
                break;
        }
        timer_state[timer_id] = time_options;
    }
    
public:
    
    //MARK: - Constructors
    
    PomodoroTimer(float sbreak, float lbreak, float task, int num_of_timers) : sbreak_time(sbreak), lbreak_time(lbreak), task_time(task), num_of_timers(num_of_timers) {};
    
    //MARK: - Main public methods
    
    // Starts timer instance by initializing the console and printing values
    void start() {
        // ====================================
        // =======Legacy Code================
        
        init_random_quotes();
        init_timer_states();
        
        const char *time = formatted_time(task_time).c_str();
        
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
        
        /* Ensure input is anything but 0. */
        for (auto const& x : timers_map) {
            update_random_quotes(x.first);
            parse_time_arg(time, x.first);
            
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
                if (!time_is_zero(x.first)) update_hour(x.first);
                else {
                    update_time_option(x.first);
                    update_random_quotes(x.first);
                    restart(x.first);
                }
            }
        }
        
        endwin();
        // ====================================
    }
    
    // Resets the clock and activate next timer mode
    void restart(int timer_id) {
        const char *time = formatted_time(get_current_timer(timer_id)).c_str();
        parse_time_arg(time, timer_id);
    }
    
    // Modified code from ttytimer.h that sets the custom quotes
    // instead of extra row of digits under the clock
    void draw_clock(void) {
        chtype dotcolor = COLOR_PAIR(1);
        unsigned int numcolor = 1;
        
        for (auto const& x : timers_map)
        {
            WINDOW *window = x.second;
            WINDOW *quote_window = quotes_vector[x.first];
            date *cur_date = dates_of_timers[x.first];
            
            if (time(NULL) % 2 == 0) {
                dotcolor = COLOR_PAIR(2);
                if (time_is_zero(x.first)) numcolor = 2;
            }
            
            /* Draw hour numbers */
            draw_number(cur_date->hour[0], 1, 1, numcolor, x.first);
            draw_number(cur_date->hour[1], 1, 8, numcolor, x.first);
            
            /* 2 dot for number separation */
            wbkgdset(window, dotcolor);
            mvwaddstr(window, 2, 16, "  ");
            mvwaddstr(window, 4, 16, "  ");
            
            /* Draw minute numbers */
            draw_number(cur_date->minute[0], 1, 20, numcolor, x.first);
            draw_number(cur_date->minute[1], 1, 27, numcolor, x.first);
            /* Draw the date */
            if (ttyclock->option.bold) wattron(window, A_BOLD);
            else wattroff(window, A_BOLD);
            
            /* Draw second frame. */
            /* Again 2 dot for number separation */
            wbkgdset(window, dotcolor);
            mvwaddstr(window, 2, NORMFRAMEW, "  ");
            mvwaddstr(window, 4, NORMFRAMEW, "  ");
            
            /* Draw second numbers */
            draw_number(cur_date->second[0], 1, 39, numcolor, x.first);
            draw_number(cur_date->second[1], 1, 46, numcolor, x.first);
            
            
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
        
        struct timespec length = { 1, 0 };
        
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
                    bool status = timers_execution_vector[x.first];
                    if (!status) break;
                    
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
                        timers_execution_vector[i - 1] = !timers_execution_vector[i - 1];
                    }
                }
                
                break;
        }
    }
};

#endif /* ao_tty_timer_hpp */
