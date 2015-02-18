#ifndef __FARMING_H__
#define __FARMING_H__

#include "Functionality.h"

class Farming : public Functionality
{
public:

    Farming();
    ~Farming();

    std::string getName();

    void update();

private:

    //
};

#endif // __FARMING_H__
