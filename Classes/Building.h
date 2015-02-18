#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "Functionality.h"

class Building : public Functionality
{
public:

    Building();
    ~Building();

    std::string getName();

    void update();

    void initContainer();
    void initTopButtons();

private:

    void add(cocos2d::Ref* sender);
};

#endif // __BUILDING_H__
