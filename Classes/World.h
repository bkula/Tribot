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

    class Village
    {
    public:

        Village(std::string _id)
        : id(_id)
        {
        }

        const std::string id;
        TWCoordinates coordinates;
    };
    std::vector<Village> villages;

    class Session
    {
    public:

        Session(int villagesNumber)
        : ended(false)
        , unlockedAt(NULL)
        , loggingInPhase(0)
        , reviewPhase(REVIEW_PHASE_START)
        , startegyDevised(false)
        , currentVillage(0)
        , allVillagesReviewed(false)
        {
            villagesActions.reserve(villagesNumber);
        }

        bool ended;
        TimePoint* unlockedAt; // NULL - unlocked now

        enum REVIEW_PHASES
        {
            REVIEW_PHASE_START, // always first
            REVIEW_PHASE_MAP,
            REVIEW_PHASE_REPORTS,
            REVIEW_PHASE_END // always last
        };

        int loggingInPhase; // 0 - not logged, 1 - logging in progress, 2 - logged in
        int/*REVIEW_PHASES*/ reviewPhase;
        bool startegyDevised;
        int currentVillage;
        bool allVillagesReviewed;

        struct Actions // orders & expensions for each village
        {
            struct Order
            {
                TWArmy army;
                TWCoordinates target;
                bool isAttack;
            };

            std::vector<Order> order;
            std::vector<TWBuilding> build;
            std::vector<TWArmy> recruit;
            std::vector<TWUnit> tech;
        };
        std::vector<Actions> villagesActions;

        // from, to - min/max time in seconds
        void pause(float from, float to)
        {
            int _from = from * 1000.0f;
            int _to = to * 1000.0f;
            DELETE(unlockedAt);
            unlockedAt = new TimePoint(
                GET_TIME_NOW
                + std::chrono::milliseconds(_from + (random() % (_to-_from)))
                + std::chrono::milliseconds(random() % 1000)
            );
        }

    } *session; // NULL - no ongoing session

    void sessionUpdate();
    void deviseStrategy();

    TimePoint* nextSessionAt; // NULL - no incoming session

    // http callbacks
    void onLoggedIn(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response);

    /*
    void (World::*session_page)(); // pointer to current session function
    void session_login();
    void session_overview();
    void session_main();
    void session_recruit();
    void session_market();
    void session_place();
    void session_map();
    */

    // scene
    cocos2d::Scene* scene;
    cocos2d::ui::Layout* menu;
    cocos2d::DrawNode* shade;
    cocos2d::DrawNode* top;
    bool menuHidden;
    void menuButtonCallback(cocos2d::Ref* sender);
};

#endif // __WORLD_H__
