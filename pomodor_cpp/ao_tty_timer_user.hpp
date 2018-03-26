//
//  ao_tty_timer_user.hpp
//  pomodor_cpp
//
//  Created by Altynbek Orumbayev on 3/26/18.
//  Copyright © 2018 Altynbek Orumbayev. All rights reserved.
//

#ifndef ao_tty_timer_user_hpp
#define ao_tty_timer_user_hpp

#include <stdio.h>
#include <iostream>

static std::vector<std::string> splitted_string(const std::string &str, const char& ch) {
    std::string next;
    std::vector<std::string> result;
    
    // For each character in the string
    for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
        // If we've hit the terminal character
        if (*it == ch) {
            // If we have some characters accumulated
            if (!next.empty()) {
                // Add them to the result vector
                result.push_back(next);
                next.clear();
            }
        } else {
            // Accumulate the next character into the sequence
            next += *it;
        }
    }
    if (!next.empty())
        result.push_back(next);
    return result;
}

struct User {
public:
    float sh;
    float lo;
    float ti;
    
    User() {
        sh = 5;
        lo = 30;
        ti = 25;
    }
    
    User(char *arguments){
        std::string str_args(arguments);
        std::vector<std::string> args_array = splitted_string(str_args, ',');
        
        if (args_array.size() != 3) {
            sh = 5;
            lo = 30;
            ti = 25;
        }
        else {
            sh = stof(args_array[0]);
            lo = stof(args_array[1]);
            ti = stof(args_array[2]);
        }
    };
};

#endif /* ao_tty_timer_user_hpp */
