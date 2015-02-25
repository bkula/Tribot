#ifndef __WORLD_H__
#define __WORLD_H__

#include "common.h"

#include "Functionality.h"

class World
{
public:

    World(std::string _path, std::string _nick, std::string _world, std::string _loginLink);
    ~World();

    cocos2d::Scene* getScene();

    void update();

    std::vector<Functionality*> functionalities;

    Functionality* runningFunc; // running functionality

private:

    const std::string path; // path to world's directory
    const std::string nick;
    const std::string world;
    const std::string loginLink;

    class Session
    {
    public:

        Session()
        : ended(false)
        , unlockedAt(NULL)
        , loggedIn(false)
        {}

        bool ended;

        TimePoint* unlockedAt; // NULL - unlocked now

        /// Village* currentVillage;
        /// vector<Village*> unvisitedVillages;

        bool loggedIn;

        // from, to - min/max time in seconds
        void pause(int from, int to)
        {
            DELETE(unlockedAt);
            unlockedAt = new TimePoint(GET_TIME_NOW + std::chrono::seconds(from + (random() % (to-from))));
        }

    } *session; // NULL - no ongoing session

    // http callbacks
    void onLoggedIn(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response);

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
