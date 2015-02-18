
#include "AppTribot.h"

AppTribot::AppTribot()
: path(cocos2d::FileUtils::getInstance()->getWritablePath() + "TribotData/")
{
    cocos2d::FileUtils* FU =  cocos2d::FileUtils::getInstance();
std::cout << path + "Tribot.txt" << std::endl;
    // init Tribot Directory if necessary
    if (! FU->isFileExist(path + "Tribot.txt"))
    {
        std::fstream file(path + "Tribot.txt");
        std::cout << file.is_open() << "\n";
        file.close();
    }

    // temporarily
    worlds.push_back(new World(path + "Kulawy22 pl77/"));
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
