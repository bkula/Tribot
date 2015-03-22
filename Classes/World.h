#ifndef __WORLD_H__
#define __WORLD_H__

#include "common.h"

#include "Functionality.h"

class World
{
public:

    World(std::string _path, std::string _nick, std::string _world, std::string _market, std::string _loginLink);
    ~World();

    cocos2d::Scene* getScene();

    void update();

    std::vector<Functionality*> functionalities;

    Functionality* runningFunc; // running functionality

private:

    const std::string path; // path to world's directory
    const std::string nick;
    const std::string world;
    const std::string market;
    const std::string loginLink;

    class Village
    {
    public:

        #define USE_MEMORY_DUMPS 0
        #define ALLOW_PARTIAL_MEMORY_DUMPS 0

        Village(std::string _path, std::string _id)
        : id(_id)
        , path(_path)
        {
            #if USE_MEMORY_DUMPS
            // load data from file

            cocos2d::FileUtils* FU =  cocos2d::FileUtils::getInstance();

            if (! FU->isFileExist(path + "village.txt"))
            {
                std::ofstream of(path + "village.txt");
                if (! of.is_open()) std::cout << "ERROR: file " << path + "village.txt" << " can't be opened." << std::endl;
                of.close();
            }

            std::fstream file(path + "village.txt");
            std::string data;

            // zapisywanie pliku do data
            while (1)
            {
                char c;
                file.get(c);
                if (file.eof()) break;
                data += c;
            }

            // if file has incorrect size

            if (
                #if ALLOW_PARTIAL_MEMORY_DUMPS
                0
                #else
                sizeof(Village) > data.size()*sizeof(char)
                #endif // ALLOW_PARTIAL_MEMORY_DUMPS
            ) {

                ERROR("File " + path + "village.txt has incorrect size");
                // DEBUG std::cout << "Info: sizeof(Village) = " << sizeof(Village) << ", data.size()*sizeof(char) = " << data.size()*sizeof(char) << "\n";

            } else {

                /// TEN BLOK MOŻE POWODOWAĆ SEGMENTATION FAULT

                // create village from file

                #if ALLOW_PARTIAL_MEMORY_DUMPS
                memcpy(this, data.c_str(), std::max(sizeof(Village), data.size()*sizeof(char)));
                #else
                memcpy(this, data.c_str(), sizeof(Village));
                #endif // ALLOW_PARTIAL_MEMORY_DUMPS

                if (path != _path) ERROR("Podczas inicjalizacji wioski (path != _path)");
                if (id != _id) ERROR("Podczas inicjalizacji wioski (id != _id)");
                path = _path, id = _id; // for security
            }

            file.close();
            #endif // USE_MEMORY_DUMPS
        }

        ~Village()
        {
            #if USE_MEMORY_DUMPS
            // save data in file
            std::fstream file(path + "village.txt", std::ios::in | std::ios::out | std::ios::trunc);
            if (! file.is_open()) {
                std::cout << "ERROR: file " << path + "village.txt" << " can't be opened." << std::endl;
            }
            size_t s = sizeof(Village)/sizeof(char) + sizeof(char)*2;
            char data[s];
            #if ALLOW_PARTIAL_MEMORY_DUMPS
            memcpy(data, this, sizeof(this));
            #else
            memcpy(data, this, sizeof(Village));
            #endif // ALLOW_PARTIAL_MEMORY_DUMPS
            std::string str(data, s);
            //DEBUG while (str.size() < sizeof(Village)/sizeof(char) + sizeof(char)*2) str += "0";
            file << str;
            file.close();
            //DEBUG std::cout << "DEST: " << str.size() << " " << sizeof(this) << " " << s << "\n";
            #endif // USE_MEMORY_DUMPS
        }

        std::string path;
        std::string id;

        /// data
        TWCoordinates coordinates;
        int buildings[TW_BUILDING_SIZE];
        int buildingsInQueue;
        TWCost buildingsCosts[TW_BUILDING_SIZE];
        int army[TW_UNIT_SIZE];
        TWCost resources;
        std::string h_value;
        bool isTech[TW_UNIT_SIZE];
        std::vector<TWCoordinates> attackOrders;
        std::vector<TWCoordinates> returnOrders;
        int storageLimit;
        int populationLimit;
    };
    std::vector<Village> villages;

    class Farm
    {
    public:

        Farm(std::string _coords)
        : coords(_coords)
        {
        }

        TWCoordinates coords;
    };
    std::vector<Farm> farms;

    class Session
    {
    public:

        Session(int villagesNumber)
        : ended(false)
        , unlockedAt(NULL)
        , loggingInPhase(0)
        , globalReviewPhase(GRP_START)
        , villageReviewPhase(VRP_START)
        , startegyDevised(false)
        , currentVillage(0)
        , allVillagesReviewed(false)
        , currentOrder(0)
        , villageActionsExecuted(false)
        {
            for (int i = 0; i < villagesNumber; i++)
            {
                villagesActions.push_back(Actions());
            }
        }

        bool ended;
        TimePoint* unlockedAt; // NULL - unlocked now

        enum GLOBAL_REVIEW_PHASES // Prefix GRP
        {
            GRP_START,
            GRP_MAP,
            GRP_REPORTS,
            GRP_END,
            GRP_WAIT // always last
        };

        #define GRP Session::GLOBAL_REVIEW_PHASES

        enum VILLAGE_REVIEW_PHASES // Prefix VRP
        {
            VRP_START,
            VRP_OVERVIEW,
            VRP_HQ,
            VRP_SMITH,
            VRP_END,
            VRP_WAIT // always last
        };

        #define VRP Session::VILLAGE_REVIEW_PHASES

        int loggingInPhase; // 0 - not logged, 1 - logging in progress, 2 - logged in
        int/*GLOBAL_REVIEW_PHASES*/ globalReviewPhase;
        int/*VILLAGE_REVIEW_PHASES*/ villageReviewPhase; /*old*/// 0 - begin, 1 - overview in progress, 2 - overview finished, 3 - HQ review in progress, 4 - end
        bool startegyDevised;
        int currentVillage;
        bool allVillagesReviewed;
        std::list<cocos2d::network::HttpRequest*> actionRequests;
        int currentOrder;
        bool villageActionsExecuted;

        void nextActionRequest()
        {
            if (! actionRequests.empty()) {
                cocos2d::network::HttpClient::getInstance()->send(actionRequests.front());
                actionRequests.front()->release();
                actionRequests.pop_front();
                pause(1,2);
            }
        }

        struct Actions // orders & expensions for each village
        {
            std::vector<TWOrder> order;
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
    void executeVillageActions(int v /*villageIndex*/);

    TimePoint nextSessionAt;

    // http callbacks
    #define ARGS cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response
    void onLoggedIn(ARGS);
    void onVillageOverviewed(ARGS);
    void onVillageHQViewed(ARGS);
    void onVillageSmithViewed(ARGS);
    void onActionRequestEnded(ARGS);
    void onGoToPlaceEnded(ARGS);
    void onSendArmyEnded(ARGS);
    #undef ARGS

    // strategy functions
    void printData();
    void buildEco();
    void farm();

    /*old*//*
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
