#include "FilterTools.h"




// x, y are in [0,1]

std::map<std::string, FilterFunc> filters;


void initFilter() {

    filters["radius"] = Filter_radius;
    filters["square"] = Filter_square;
    filters["exp"] = Filter_exp;
}


bool hasFilter(std::string name) {
    auto it = filters.find(name);
    if (it == filters.end()) {
        return false;
    }
    return true;
}

FilterFunc getFilter(std::string name) {
    return filters.find(name)-> second;
}


// filters 


Complex Filter_radius(double x, double y) {
    return Complex (1 - (x*x + y*y)/2, 0);
}

Complex Filter_square(double x, double y) {
    return Complex ((x > 0.5 || y > 0.5 ? 0 : 1), 0);
}

Complex Filter_exp(double x, double y) {
    return Complex (std::exp(- x - y), 0);
}