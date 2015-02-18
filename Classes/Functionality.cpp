
#include "Functionality.h"

Functionality::Functionality()
: container(NULL)
, topButtons(NULL)
, numberOfTopButtons(0)
{
}

Functionality::~Functionality()
{
}

cocos2d::Node* Functionality::getConteiner(bool createNew)
{
    if (! container || createNew) {
        container = cocos2d::Node::create();
        initContainer();
    }

    return container;
}

void Functionality::initContainer()
{
    using namespace cocos2d;

    auto text = ui::Text::create();
    text->setText("Funkcja nieopracowana");
    text->setFontName(gFont);
    text->setFontSize(40);
    text->setColor(Color3B::BLACK);
    text->setTextHorizontalAlignment(TextHAlignment::CENTER);
    text->setPosition(Vec2(SCREEN.width/2, SCREEN.height/2));
    container->addChild(text);
}

cocos2d::Menu* Functionality::getTopButtons(bool createNew)
{
    if (! topButtons || createNew) {
        topButtons = cocos2d::Menu::create();
        initTopButtons();
    }

    return topButtons;
}

void Functionality::initTopButtons()
{
}

/*
void Functionality::addTopButton(std::string image, cocos2d::ccMenuCallback callback)
{
    if (! topButtons) {
        return;
    }

    USING_NS_CC;

    int x = float(topH) * 0.7f;

    auto button = MenuItemImage::create(image, image, callback);

    button->setPosition(Vec2(SCREEN.width - x/2 - x*numberOfTopButtons, 0));

    auto scaleTo = ScaleTo::create(0, x / std::max(button->getContentSize().width, button->getContentSize().height));
    button->runAction(scaleTo);

    topButtons->addChild(button);

    numberOfTopButtons++;
}
*/
