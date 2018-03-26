![LOGO](https://raw.githubusercontent.com/AOrumbaev/pomodor_cpp/master/image_resources/pomo_icon.png)

# pomodor_cpp
A Simple CLI pomodoro timer implemented in C++ using `ncurses`.
Contains support for up to 5 parrallel timers execution. 
For best experience and due to fixed size of digits it is required to run the CLI in fullscreen terminal!

### HOW-TO

![GIF](https://raw.githubusercontent.com/AOrumbaev/pomodor_cpp/master/image_resources/tutorial_timer.gif)

Execute the following command in terminal from pomodor_cpp directory:

`g++ -std=c++11 -lncurses -o timer ao_tty_timer_controller.cpp`

#### Ubuntu
If you are using Ubuntu, execute the following command before compiling pomodoro.

`sudo apt-get install libncurses-dev`

### Example Usage
```
./timer --num_timers $0 --user 3,10,15 --user 5,50,30
```
##### Arguments description:
`--num_timers` - required argument for specifying number of timer instances to execute.
`--user` - optional argument to set specific short break, long break and time options for user. Input for time values are separated by `,` and entered in following format `$0,$1,$2`.

### Extra features
If you want to pause certain timer just type the corresponding number on your keyboard. For instance, if you want to stop second timer for second user just press `2` button on your keyboard. Pressing it again will resume the timer.

## Credits
• [tty-timer](https://github.com/mbarbar/ttytimer) - A modification of tty-clock that implements timer instead of clock.
