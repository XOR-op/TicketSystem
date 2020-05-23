#include "main_helper.h"
using namespace t_sys;

bool t_sys::needInit() {
    if (FILE* file = fopen(USER_PATH, "r")) {
        fclose(file);
        return false;
    } else return true;
}

char t_sys::getOption(std::istream& ifs){
    char buf[4];
    ifs>>buf;
    assert(buf[0]=='-');
    return buf[1];
}
