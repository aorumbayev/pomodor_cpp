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
#include <memory>

/* Macro */
#define NORMFRAMEW 35
#define SECFRAMEW  54
#define DATEWINH   3
/* Maximum number of digits in a time string, hh:mm:ss. */
#define N_TIME_DIGITS 6


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

using namespace std;

/* Global ttyclock struct */
class ttyclock_t {
public:
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
    
    /* Date content ([2] = number by number) */
    int initial_digits[N_TIME_DIGITS];
    struct
    {
        unsigned int hour[2];
        unsigned int minute[2];
        unsigned int second[2];
        char timestr[9];  /* hh:mm:ss */
    } date;
    
    /* time.h utils */
    struct tm *tm;
    time_t lt;
    
    /* Clock member */
    WINDOW *framewin;
    WINDOW *quotewin;
    
    ttyclock_t() {
        
        /* Init global struct */
        running = true;
        interupted = false;
        if(!geo.x) geo.x = 0;
        if(!geo.y) geo.y = 0;
        if(!geo.a) geo.a = 1;
        if(!geo.b) geo.b = 1;
        geo.w = SECFRAMEW;
        geo.h = 7;
        tm = localtime(&(lt));
        lt = time(NULL);
        
        /* Create clock win */
        framewin = newwin(geo.h,
                          geo.w,
                          geo.x,
                          geo.y);
        if (option.box) box(framewin, 0, 0);
        
        if (option.bold) wattron(framewin, A_BLINK);
        
        /* Create the date win */
        quotewin = newwin(DATEWINH, 16 + 2,
                          geo.x + geo.h - 1,
                          geo.y + (geo.w / 2) -
                          (16 / 2) - 1);
        
        if (option.box) box(quotewin, 0, 0);
        
        clearok(quotewin, true);
        
        set_center();
        
        nodelay(stdscr, true);
        
        wrefresh(quotewin);
        
        wrefresh(framewin);
    }
    
    
    /* Decrements ttyclock's time by 1 second. */
    void update_hour(void) {
        unsigned int seconds = date.second[0] * 10
        + date.second[1];
        unsigned int minutes = date.minute[0] * 10
        + date.minute[1];
        unsigned int hours = date.hour[0] * 10
        + date.hour[1];
        
        if (minutes == 0 && seconds == 0) hours = hours == 0 ? 59 : hours - 1;
        if (seconds == 0) minutes = minutes == 0 ? 59 : minutes - 1;
        seconds = seconds == 0 ? 59 : seconds - 1;
        
        /* Put it all back into ttyclock. */
        date.hour[0] = hours / 10;
        date.hour[1] = hours % 10;
        
        date.minute[0] = minutes / 10;
        date.minute[1] = minutes % 10;
        
        date.second[0] = seconds / 10;
        date.second[1] = seconds % 10;
    }
    
    void draw_number(int n, int x, int y, unsigned int color) {
        int i, sy = y;
        
        for(i = 0; i < 30; ++i, ++sy) {
            if(sy == y + 6) {
                sy = y;
                ++x;
            }
            
            if (option.bold) wattron(framewin, A_BLINK);
            else wattroff(framewin, A_BLINK);
            
            wbkgdset(framewin,
                     COLOR_PAIR(number[n][i/2] * color));
            mvwaddch(framewin, x, sy, ' ');
        }
        
        wrefresh(framewin);
    }
    
    void clock_move(int x, int y, int w, int h) {
        /* Erase border for a clean move */
        wbkgdset(framewin, COLOR_PAIR(0));
        wborder(framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
        werase(framewin);
        wrefresh(framewin);
        
        wbkgdset(quotewin, COLOR_PAIR(0));
        wborder(quotewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
        werase(quotewin);
        wrefresh(quotewin);
        
        /* Frame win move */
        mvwin(framewin, (geo.x = x), (geo.y = y));
        wresize(framewin, (geo.h = h),
                (geo.w = w));
        
        /* Date win move */
        mvwin(quotewin,
              geo.x + geo.h - 1,
              geo.y + (geo.w / 2)
              - (16 / 2) - 1);
        wresize(quotewin, DATEWINH,
                16 + 2);
        
        if (option.box) box(quotewin,  0, 0);
        
        if (option.box) box(framewin, 0, 0);
        
        wrefresh(framewin);
        wrefresh(quotewin);
    }
    
    void set_second(void) {
        int new_w = SECFRAMEW;
        int y_adj;
        
        for(y_adj = 0; (geo.y - y_adj) > (COLS - new_w - 1); ++y_adj);
        
        clock_move(geo.x, (geo.y - y_adj), new_w,
                   geo.h);
        
        set_center();
    }
    
    void set_center(void) {
        clock_move((LINES / 2 - (geo.h / 2)),
                   (COLS  / 2 - (geo.w / 2)),
                   geo.w,
                   geo.h);
    }
    
    void set_box(bool b) {
        option.box = b;
        
        wbkgdset(framewin, COLOR_PAIR(0));
        wbkgdset(quotewin, COLOR_PAIR(0));
        
        if(option.box) {
            wbkgdset(framewin, COLOR_PAIR(0));
            wbkgdset(quotewin, COLOR_PAIR(0));
            box(framewin, 0, 0);
            box(quotewin,  0, 0);
        } else {
            wborder(framewin, ' ', ' ', ' ', ' ', ' ', ' ',
                    ' ', ' ');
            wborder(quotewin, ' ', ' ', ' ', ' ', ' ', ' ',
                    ' ', ' ');
        }
        
        wrefresh(quotewin);
        wrefresh(framewin);
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
    
    void parse_time_arg(const char *time) {
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
        
        fill_ttyclock_time(digits, date.hour);
        fill_ttyclock_time(digits + 2, date.minute);
        fill_ttyclock_time(digits + 4, date.second);
        memcpy(initial_digits, digits, N_TIME_DIGITS * sizeof(int));
        
        date.timestr[0] = date.hour[0] + '0';
        date.timestr[1] = date.hour[1] + '0';
        date.timestr[2] = ':';
        date.timestr[3] = date.minute[0] + '0';
        date.timestr[4] = date.minute[1] + '0';
        date.timestr[5] = ':';
        date.timestr[6] = date.second[0] + '0';
        date.timestr[7] = date.second[1] + '0';
        date.timestr[8] = '\0';
    }
    
    void key_event(void) {
        int i, c;
        
        // s delay and ns delay.
        struct timespec length = { 1, 0 };
        switch(c = wgetch(stdscr)) {
            case 'q':
            case 'Q':
                running = false;
                interupted = true;
                break;
                
            case 'r':
            case 'R':
                fill_ttyclock_time(initial_digits,
                                   date.hour);
                fill_ttyclock_time(initial_digits + 2,
                                   date.minute);
                fill_ttyclock_time(initial_digits + 4,
                                   date.second);
                break;
                
            default:
#ifdef TOOT
                if (time_is_zero() && time(NULL) % 2 == 0) toot(500, 1000);
                else
#endif
                    nanosleep(&length, NULL);
                
                for (i = 0; i < 8; ++i) {
                    if (c == (i + '0')) {
                        option.color = i;
                        init_pair(1, bg, i);
                        init_pair(2, i, bg);
                    }
                }
                
                break;
        }
    }
    
    
    bool time_is_zero(void) {
        return date.hour[0] == 0
        && date.hour[1] == 0
        && date.minute[0] == 0
        && date.minute[1] == 0
        && date.second[0] == 0
        && date.second[1] == 0;
    }
    
};


void reload();


/* Global variable */
//ttyclock_t *ttyclock;

//void signal_handler(int signal) {
//    switch(signal) {
//        case SIGWINCH:
//            endwin();
//            reload();
//            break;
//            /* Interruption signal */
//        case SIGINT:
//        case SIGTERM:
////            ttyclock->running = false;
//            /* Segmentation fault signal */
//            break;
//        case SIGSEGV:
//            endwin();
//            fprintf(stderr, "Segmentation fault.\n");
//            exit(EXIT_FAILURE);
//            break;
//    }
//}

void reload(unique_ptr<ttyclock_t> & ttyclock) {
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
//    sig.sa_handler = signal_handler;
//    sig.sa_flags   = 0;
//    sigaction(SIGWINCH, &sig, NULL);
//    sigaction(SIGTERM,  &sig, NULL);
//    sigaction(SIGINT,   &sig, NULL);
//    sigaction(SIGSEGV,  &sig, NULL);
}

/* Prototypes */

void cleanup(void) {
    //    if (ttyscr) delscreen(ttyscr);
    //    if (ttyclock) free(ttyclock);
    //    printf("hehe");
    //
}

/* Parses time into date.hour/minute/second. Exits with
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



int main(int argc, const char * argv[]) {

    ttyclock_t clock;
    
    return 0;
}
