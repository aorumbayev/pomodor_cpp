/*
 *      TTY-CLOCK headers file.
 *      Copyright © 2008 Martin Duquesnoy <xorg62@gmail.com>
 *      All rights reserved.
 *
 *      Redistribution and use in source and binary forms, with or without
 *      modification, are permitted provided that the following conditions are
 *      met:
 *
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following disclaimer
 *        in the documentation and/or other materials provided with the
 *        distribution.
 *      * Neither the name of the  nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TTYCLOCK_H_INCLUDED
#define TTYCLOCK_H_INCLUDED

#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <ncurses.h>
#include <unistd.h>
#include <getopt.h>
#include <map>

/* Macro */
#define NORMFRAMEW 35
#define SECFRAMEW  54
#define DATEWINH   3
/* Maximum number of digits in a time string, hh:mm:ss. */
#define N_TIME_DIGITS 6

/* Global ttyclock struct */
typedef struct ttyclock_t
{
    /* while() boolean */
    bool running;
    bool interupted;
    
    /* terminal variables */
    SCREEN *ttyscr;
    int bg;
    
    /* Running option */
    struct
    {
        bool box;
        int color;
        bool bold;
    } option;
    
    /* Clock geometry */
    struct
    {
        int x, y, w, h;
        /* For rebound use (see clock_rebound())*/
        int a, b;
    } geo;
    
    struct
    {
        int x, y, w, h;
        /* For rebound use (see clock_rebound())*/
        int a, b;
    } gep1;
    
    /* Date content ([2] = number by number) */
    int initial_digits[N_TIME_DIGITS];
    
    /* time.h utils */
    struct tm *tm;
    time_t lt;
    int num_of_timers;
    
    /* Clock member */
    //        WINDOW *framewin;
    //    WINDOW *newframewin;
    //    WINDOW *quotewin;
    
} ttyclock_t;


/* Global variable */
ttyclock_t *ttyclock;

/* Number matrix */
const bool number[][15] =
{
    {1,1,1,1,0,1,1,0,1,1,0,1,1,1,1}, /* 0 */
    {0,0,1,0,0,1,0,0,1,0,0,1,0,0,1}, /* 1 */
    {1,1,1,0,0,1,1,1,1,1,0,0,1,1,1}, /* 2 */
    {1,1,1,0,0,1,1,1,1,0,0,1,1,1,1}, /* 3 */
    {1,0,1,1,0,1,1,1,1,0,0,1,0,0,1}, /* 4 */
    {1,1,1,1,0,0,1,1,1,0,0,1,1,1,1}, /* 5 */
    {1,1,1,1,0,0,1,1,1,1,0,1,1,1,1}, /* 6 */
    {1,1,1,0,0,1,0,0,1,0,0,1,0,0,1}, /* 7 */
    {1,1,1,1,0,1,1,1,1,1,0,1,1,1,1}, /* 8 */
    {1,1,1,1,0,1,1,1,1,0,0,1,1,1,1}, /* 9 */
};


/* Prototypes */

// Forward declaration
void init(void);
void signal_handler(int signal);
void update_hour(int timer_id);
void draw_number(int n, int x, int y, unsigned int color);
void draw_clock(void);
void clock_move(int x, int y, int w, int h);
void set_second(void);
void set_center(void);
void set_box(bool b);
void key_event(void);

// MARK : Extended methods

// Stores multiple WINDOW instances for specific timer_ids
std::map<int, WINDOW*> timers_map;
// Stores multiple WINDOW instances representing quotes for specific timer_ids
std::vector<WINDOW*> quote_windows;
// Stores multiple boolean values indicating whether timer is running or not
std::vector<bool> timers_execution_vector;

struct date
{
    unsigned int hour[2];
    unsigned int minute[2];
    unsigned int second[2];
    char timestr[9];  /* hh:mm:ss */
};
std::vector<date *> dates_of_timers;

// End of forward declaration

bool time_is_zero(int timer_id) {
    
    date *cur_date = dates_of_timers[timer_id];
    
    return cur_date->hour[0] == 0
    && cur_date->hour[1] == 0
    && cur_date->minute[0] == 0
    && cur_date->minute[1] == 0
    && cur_date->second[0] == 0
    && cur_date->second[1] == 0;
}

void init() {
    struct sigaction sig;
    ttyclock->bg = COLOR_BLACK;
    
    initscr();
    
    cbreak();
    noecho();
    keypad(stdscr, true);
    start_color();
    curs_set(false);
    clear();
    
    /* Init default terminal color */
    if (use_default_colors() == OK) ttyclock->bg = -1;
    
    /* Init color pair */
    init_pair(0, ttyclock->bg, ttyclock->bg);
    init_pair(1, ttyclock->bg, ttyclock->option.color);
    init_pair(2, ttyclock->option.color, ttyclock->bg);
    refresh();
    
    /* Init signal handler */
    sig.sa_handler = signal_handler;
    sig.sa_flags = 0;
    sigaction(SIGWINCH, &sig, NULL);
    sigaction(SIGTERM,  &sig, NULL);
    sigaction(SIGINT,   &sig, NULL);
    sigaction(SIGSEGV,  &sig, NULL);
    
    /* Init global struct */
    ttyclock->running = true;
    ttyclock->interupted = false;
    if(!ttyclock->geo.x) ttyclock->geo.x = 0;
    if(!ttyclock->geo.y) ttyclock->geo.y = 0;
    if(!ttyclock->geo.a) ttyclock->geo.a = 1;
    if(!ttyclock->geo.b) ttyclock->geo.b = 1;
    ttyclock->geo.w = SECFRAMEW;
    ttyclock->geo.h = 7;
    ttyclock->tm = localtime(&(ttyclock->lt));
    ttyclock->lt = time(NULL);
    
    for (int i = 0 ; i < ttyclock->num_of_timers; i++) {
        WINDOW *cur_window = newwin(ttyclock->geo.h,
                                    ttyclock->geo.w,
                                    ttyclock->geo.x,
                                    ttyclock->geo.y);
        
        if (ttyclock->option.box) box(cur_window, 0, 0);
        timers_map.emplace(i, cur_window);
        
        WINDOW *quotewin = newwin(DATEWINH, 16 + 2,
                                  ttyclock->geo.x + ttyclock->geo.h - 1,
                                  ttyclock->geo.y + (ttyclock->geo.w / 2) -
                                  (16 / 2) - 1);
        
        if (ttyclock->option.box) box(quotewin, 0, 0);
        
        clearok(quotewin, true);
        
        quote_windows.push_back(quotewin);
        
        if (ttyclock->option.bold) wattron(cur_window, A_BLINK);
        
        timers_execution_vector.push_back(true);
        
        //        date val;
        dates_of_timers.push_back(new date());
    }
    
    set_center();
    nodelay(stdscr, true);
    
    for (auto const& x : timers_map)
    {
        wrefresh(x.second);
        wrefresh(quote_windows[x.first]);
    }
}

void signal_handler(int signal) {
    switch(signal) {
        case SIGWINCH:
            endwin();
            init();
            break;
            /* Interruption signal */
        case SIGINT:
        case SIGTERM:
            ttyclock->running = false;
            /* Segmentation fault signal */
            break;
        case SIGSEGV:
            endwin();
            fprintf(stderr, "Segmentation fault.\n");
            exit(EXIT_FAILURE);
            break;
    }
}

void cleanup(void) {
    if (ttyclock->ttyscr) delscreen(ttyclock->ttyscr);
    if (ttyclock) free(ttyclock);
}

/* Decrements ttyclock's time by 1 second. */
void update_hour(int timer_id) {
    date *cur_date = dates_of_timers[timer_id];
    bool status = timers_execution_vector[timer_id];
    
    unsigned int seconds = cur_date->second[0] * 10
    + cur_date->second[1];
    unsigned int minutes = cur_date->minute[0] * 10
    + cur_date->minute[1];
    unsigned int hours = cur_date->hour[0] * 10
    + cur_date->hour[1];
    
    if (status) {
        if (minutes == 0 && seconds == 0) hours = hours == 0 ? 59 : hours - 1;
        if (seconds == 0) minutes = minutes == 0 ? 59 : minutes - 1;
        seconds = seconds == 0 ? 59 : seconds - 1;
    }
    /* Put it all back into ttyclock. */
    cur_date->hour[0] = hours / 10;
    cur_date->hour[1] = hours % 10;
    
    cur_date->minute[0] = minutes / 10;
    cur_date->minute[1] = minutes % 10;
    
    cur_date->second[0] = seconds / 10;
    cur_date->second[1] = seconds % 10;
}

void draw_number(int n, int x, int y, unsigned int color, int timer_id) {
    int i, sy = y;
    
    for(i = 0; i < 30; ++i, ++sy) {
        if(sy == y + 6) {
            sy = y;
            ++x;
        }
        WINDOW *cur_window = timers_map[timer_id];
        if (ttyclock->option.bold) wattron(cur_window, A_BLINK);
        else wattroff(cur_window, A_BLINK);
        
        wbkgdset(cur_window,
                 COLOR_PAIR(number[n][i/2] * color));
        mvwaddch(cur_window, x, sy, ' ');
        
        wrefresh(cur_window);
        wrefresh(quote_windows[timer_id]);
    }
}

void clock_move(int x, int y, int w, int h) {
    ttyclock->geo.x = ttyclock->num_of_timers <= 1 ? x : LINES / (ttyclock->num_of_timers + 6);
    ttyclock->geo.y = y;
    ttyclock->geo.w = w;
    ttyclock->geo.h = h;
    
    int cnt = 0;
    
    for (auto const& val : timers_map) {
        WINDOW *cur_window = val.second;
        WINDOW *quote_window = quote_windows[val.first];
        
        /* Erase border for a clean move */
        wbkgdset(cur_window, COLOR_PAIR(0));
        wborder(cur_window, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
        werase(cur_window);
        wrefresh(cur_window);
        
        /* Frame win move */
        mvwin(cur_window, ttyclock->geo.x + cnt, ttyclock->geo.y);
        wresize(cur_window, ttyclock->geo.h , ttyclock->geo.w);
        
        
        /* Date win move */
        mvwin(quote_window,
              ttyclock->geo.x + cnt + ttyclock->geo.h - 1,
              ttyclock->geo.y + (ttyclock->geo.w / 2)
              - (16 / 2) - 1);
        wresize(quote_window, DATEWINH, 16 + 2);
        
        
        cnt += (ttyclock->geo.h * ttyclock->num_of_timers + 6) / ttyclock->num_of_timers;
        if (ttyclock->option.box) {
            box(cur_window, 0, 0);
            box(quote_window,  0, 0);
        }
        wrefresh(cur_window);
    }
}

void set_second(void) {
    int new_w = SECFRAMEW;
    int y_adj;
    
    for(y_adj = 0; (ttyclock->geo.y - y_adj) > (COLS - new_w - 1); ++y_adj);
    
    clock_move(ttyclock->geo.x, (ttyclock->geo.y - y_adj), new_w,
               ttyclock->geo.h);
    
    set_center();
}

void set_center(void) {
    clock_move((LINES / 2 - (ttyclock->geo.h / 2)),
               (COLS  / 2 - (ttyclock->geo.w / 2)),
               ttyclock->geo.w,
               ttyclock->geo.h);
}

void set_box(bool b) {
    ttyclock->option.box = b;
    for (auto const& val : timers_map) {
        WINDOW *cur_window = val.second;
        wbkgdset(cur_window, COLOR_PAIR(0));
        
        if(ttyclock->option.box) {
            wbkgdset(cur_window, COLOR_PAIR(0));
            box(cur_window, 0, 0);
        } else {
            wborder(cur_window, ' ', ' ', ' ', ' ', ' ', ' ',
                    ' ', ' ');
        }
        
        wrefresh(cur_window);
        
    }
}

/* Fills two elements from digits into time, handling the -1 case. */
static void fill_ttyclock_time(int *digits, unsigned int *time) {
    if (digits[1] == -1) {
        time[0] = 0;
        if (digits[0] == -1) time[1] = 0;
        else time[1] = digits[0];
    } else {
        time[0] = digits[0];
        time[1] = digits[1];
    }
}

static void parse_time_arg(const char *time, int timer_id) {
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
    
    date *cur_date = dates_of_timers[timer_id];
    
    fill_ttyclock_time(digits, cur_date->hour);
    fill_ttyclock_time(digits + 2, cur_date->minute);
    fill_ttyclock_time(digits + 4, cur_date->second);
    memcpy(ttyclock->initial_digits, digits, N_TIME_DIGITS * sizeof(int));
    
    cur_date->timestr[0] = cur_date->hour[0] + '0';
    cur_date->timestr[1] = cur_date->hour[1] + '0';
    cur_date->timestr[2] = ':';
    cur_date->timestr[3] = cur_date->minute[0] + '0';
    cur_date->timestr[4] = cur_date->minute[1] + '0';
    cur_date->timestr[5] = ':';
    cur_date->timestr[6] = cur_date->second[0] + '0';
    cur_date->timestr[7] = cur_date->second[1] + '0';
    cur_date->timestr[8] = '\0';
}

/* Parses time into cur_date->hour/minute/second. Exits with
 * an error message on bad time format. Sets timestr to what was
 * parsed.
 * time format: hh:mm:ss, where all but the colons are optional.
 */

/* Converts the name of a colour to its ncurses number. Case insensitive. */
int color_name_to_number(const char *color) {
    
    if (strcasecmp(color, "black") == 0) return COLOR_BLACK;
    else if (strcasecmp(color, "red") == 0) return COLOR_RED;
    else if (strcasecmp(color, "green") == 0) return COLOR_GREEN;
    else if (strcasecmp(color, "yellow") == 0) return COLOR_YELLOW;
    else if (strcasecmp(color, "blue") == 0) return COLOR_BLUE;
    else if (strcasecmp(color, "magenta") == 0) return COLOR_MAGENTA;
    else if (strcasecmp(color, "cyan") == 0) return COLOR_CYAN;
    else if (strcasecmp(color, "white") == 0) return COLOR_WHITE;
    else return -1;
}

#endif /* TTYCLOCK_H_INCLUDED */
