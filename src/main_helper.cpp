#include "main_helper.h"
using namespace t_sys;

bool t_sys::needInit() {
    if (FILE* file = fopen(USER_PATH, "r")) {
        fclose(file);
        return false;
    } else return true;
}

char t_sys::getOption(){
    char buf[4];
    std::cin>>buf;
    return buf[1];
}
