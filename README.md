![Logo](https://raw.githubusercontent.com/AOrumbaev/pomodor_cpp/master/pomo_icon.png)

# pomodor_cpp
A Simple CLI pomodoro timer implemented in C++ using `ncurses`

### HOW-TO
Execute the following command in terminal from pomodor_cpp directory:

`g++ -std=c++11 -lncurses -o timer ao_tty_timer_controller.cpp`

#### Ubuntu
If you are using Ubuntu, execute the following command before compiling pomodoro.

`sudo apt-get install libncurses-dev`

### USAGE
```
./timer [-short MINUTES] [-long MINUTES] [-time MINUTES]

arguments description:
-short - time in minutes for short brake (default 5 minutes)
-long - time in minutes for short brake (default 30 minutes)
-time - time in minutes for short brake (default 25 minutes)
```

## Credits
• [tty-timer](https://github.com/mbarbar/ttytimer) - A modification of tty-clock that implements timer instead of clock.

• [Argh!](https://github.com/adishavit/argh) - A minimalist, frustration-free, header-only argument handler.
