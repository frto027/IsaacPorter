#pragma once
struct Lang{
#define V(name, value) const char * name = value;
#include "langs/zh.inl"
#undef V

void toEng(){
    #define V(name, value) this->name = value;
    #include "langs/en.inl"
    #undef V    
}

void toZh(){
    #define V(name, value) this->name = value;
    #include "langs/zh.inl"
    #undef V    
}

};

extern Lang LANG;

