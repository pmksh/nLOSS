#include "Commons.h"


#include <map>
#include <string>




void initFilter();



bool hasFilter(std::string name);

FilterFunc getFilter(std::string name);


Complex Filter_radius(double x, double y);

Complex Filter_square(double x, double y);

Complex Filter_exp(double x, double y);