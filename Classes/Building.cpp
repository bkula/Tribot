#include "Building.h"

SETNAME(Building, "Building")

Building::Building()
{
}

Building::~Building()
{
}

void Building::update()
{
}

void Building::initContainer()
{
    //
}

void Building::initTopButtons()
{
    ADD_BUTTON_START("add.png")
    CC_CALLBACK_1(Building::add, this)
    ADD_BUTTON_END
}

void Building::add(cocos2d::Ref* sender)
{
    std::cout << "ADD" << std::endl;
}
