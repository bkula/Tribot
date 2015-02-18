#ifndef __WORLD_H__
#define __WORLD_H__

#include "common.h"

#include "Functionality.h"

class World
{
public:

    World(std::string _path);
    ~World();

    cocos2d::Scene* getScene();

    void update();

    std::vector<Functionality*> functionalities;

    Functionality* runningFunc; // running functionality

private:

    const std::string path; // path to world's directory

    class Session
    {
    public:

        Session()
        : ended(false)
        , unlockedAt(NULL)
        {}

        bool ended;

        TimePoint* unlockedAt; // NULL - unlocked now

        /// vector<Village*> unvisitedVillages;

        // from, to - min/max time in seconds
        void pause(int from, int to)
        {
            DELETE(unlockedAt);
            unlockedAt = new TimePoint(GET_TIME_NOW + std::chrono::seconds(from + (random() % (to-from))));
        }

    } *session; // NULL - no ongoing session

    TimePoint* nextSessionAt; // NULL - no incoming session

    void (World::*session_page)(); // pointer to current session function
    void session_login();
    void session_overview();
    void session_main();
    void session_recruit();
    void session_market();
    void session_place();
    void session_map();

    // scene
    cocos2d::Scene* scene;
    cocos2d::ui::Layout* menu;
    cocos2d::DrawNode* shade;
    cocos2d::DrawNode* top;
    bool menuHidden;
    void menuButtonCallback(cocos2d::Ref* sender);
};

#endif // __WORLD_H__
