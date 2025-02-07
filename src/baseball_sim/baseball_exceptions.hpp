#pragma once

#include <stdexcept>
#include <string>
#include <iostream>

class invalidYearException : std::exception {
    private:
        std::string stat_container_id;
        std::string stat_type;
        int attempted_year;
        std::string message;
    
    public:
        invalidYearException(std::string stat_container_ID, std::string attempted_stat_type, int year, std::string msg = "") : stat_container_id(stat_container_ID), stat_type(attempted_stat_type), attempted_year(year), message(msg) {
            std::string return_val = "There is no " + stat_type + " data available for stat_container " + stat_container_id + " from the year " + std::to_string(attempted_year) + ". " + message;
            std::cout << return_val;
        }

        char * what() {
            std::string return_val = "";
            return return_val.data();
        }
};

class invalidStatNameException : std::exception {
    private:
        std::string stat_container_id;
        std::string stat_type;
        std::string attempted_stat;
        std::string message;

    public:
        invalidStatNameException(std::string stat_container_ID, std::string attempted_stat_type, std::string stat_name, std::string msg = "") : stat_container_id(stat_container_ID), stat_type(attempted_stat_type), attempted_stat(stat_name), message(msg) {
            std::string return_val = "No " + stat_type + " stat called \"" + attempted_stat + "\" exists for stat_container " + stat_container_id + ". " + message;
            std::cout << return_val;
        }

        char * what() {
            std::string return_val = "";
            return return_val.data();
        }    
};