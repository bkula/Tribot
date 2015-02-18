#ifndef __FUNCTIONALITY_H__
#define __FUNCTIONALITY_H__

#include "common.h"

class Functionality
{
public:

    Functionality();
    ~Functionality();

    //virtual cocos2d::Node* getNode();

    virtual std::string getName() = 0;
    #define SETNAME(cls,name) inline std::string cls::getName() {return name;}

    virtual void update() = 0;

    cocos2d::Node* getConteiner(bool createNew = false);

    cocos2d::Menu* getTopButtons(bool createNew = false); // for example: add, search, menu

protected:

    cocos2d::Node* container; // conteiner for all Func's Nodes
    virtual void initContainer();

    cocos2d::Menu* topButtons;
    virtual void initTopButtons();
    //void addTopButton(std::string image, cocos2d::ccMenuCallback callback);
    int numberOfTopButtons;

    #define ADD_BUTTON_START(image) \
    if (topButtons) { \
    USING_NS_CC; \
    int width = float(topH) * 0.6f; \
    auto button = MenuItemImage::create(image, image,

    #define ADD_BUTTON_END \
    );\
    button->setPosition(Vec2(SCREEN.width/2 - width - (float(width)*1.5f)*numberOfTopButtons, \
        SCREEN.height/2 - topH/2)); \
    auto scaleTo = ScaleTo::create(0, width / std::max(button->getContentSize().width, button->getContentSize().height)); \
    button->runAction(scaleTo); \
    topButtons->addChild(button); \
    numberOfTopButtons++; \
    }

    /* TODO
    std::string file_path;
    TimePoint next_action;
    //void play();
    //void read_file();
    //void save();
    //vector<T> addational_buttons;
    int notificationsNumber;
    */
};

#endif // __FUNCTIONALITY_H__
