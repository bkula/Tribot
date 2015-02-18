#ifndef __APP_TRIBOT_H__
#define __APP_TRIBOT_H__

#include "common.h"

#include "World.h"

class AppTribot
{
public:

    AppTribot();
    ~AppTribot();

    cocos2d::Scene* getLauncher();

    // called each frame
    void update(float delta);

    std::vector<World*> worlds;

private:

    const std::string path;

    cocos2d::Scene* launcher;
};

#endif // __APP_TRIBOT_H__
