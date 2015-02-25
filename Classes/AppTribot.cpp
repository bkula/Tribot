
#include "AppTribot.h"

AppTribot::AppTribot()
: path(cocos2d::FileUtils::getInstance()->getWritablePath() + "TribotData/")
{
    cocos2d::FileUtils* FU =  cocos2d::FileUtils::getInstance();

    // create Tribot Directory if necessary
    if (! FU->isFileExist(path + "worlds.txt"))
    {
        createDirectory(path);

        std::ofstream of(path + "worlds.txt");
        if (! of.is_open()) std::cout << "ERROR: file " << path + "worlds.txt" << " can't be opened." << std::endl;
        of.close();
    }

    std::fstream file(path + "worlds.txt");

    while (1)
    {
        std::string nick, world, link;
        bool eof = false;

        #define GET(str) \
        if (eof) break; \
        while (! eof) { \
            if (std::getline(file, str)) { \
                if (! str.empty()) break; /* ignore empty lines */ \
            } else { \
                eof = true; \
            } \
        }
        GET(nick)
        GET(world)
        GET(link)
        #undef GET

        if (eof) break;

        worlds.push_back(new World(path + nick + "|" + world + "/", nick, world, link));
    }
    file.close();

    // init HttpClient
    cocos2d::network::HttpClient::getInstance()->enableCookies(std::string(path + "cookies.txt").c_str());
    cocos2d::network::HttpClient::getInstance()->setTimeoutForConnect(10);
    cocos2d::network::HttpClient::getInstance()->setTimeoutForRead(10);
}

AppTribot::~AppTribot()
{
    for (auto w: worlds) {
        delete w;
    }
}

cocos2d::Scene* AppTribot::getLauncher()
{
    // temporarily
    return worlds[0]->getScene();
}

void AppTribot::update(float delta)
{
    for (auto w: worlds) {
        w->update();
    }
}
