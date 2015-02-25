#include "World.h"

#include "Building.h"
#include "Farming.h"

World::World(std::string _path, std::string _nick, std::string _world, std::string _loginLink)
: path(_path)
, nick(_nick)
, world(_world)
, loginLink(_loginLink)
, session(NULL)
, nextSessionAt(new TimePoint(GET_TIME_NOW))
, session_page(NULL)
, scene(NULL)
, menu(NULL)
, shade(NULL)
, top(NULL)
, menuHidden(true)
, runningFunc(NULL)
{
    cocos2d::FileUtils* FU =  cocos2d::FileUtils::getInstance();

    // create world directory if necessary
    if (! FU->isFileExist(path + "world.txt"))
    {
        createDirectory(path);

        std::ofstream of(path + "world.txt");
        if (! of.is_open()) std::cout << "ERROR: file " << path + "world.txt" << " can't be opened." << std::endl;
        of.close();
    }

    functionalities.push_back(new Building());
    functionalities.push_back(new Farming());

    runningFunc = functionalities[0];
}

World::~World()
{
    for (auto f: functionalities) {
        delete f;
    }
}

void World::update()
{
    // session
    TimePoint now = GET_TIME_NOW;
    if (session) // running session
    {
        if (session->ended) {

            DELETE(session);

        } else if (session->unlockedAt) {

            if (*session->unlockedAt < now) {
                DELETE(session->unlockedAt);
            }

        } else {

            (this->*session_page)();
        }

    } else if (*nextSessionAt < now) { // create new session

        session = new Session();

        session_page = &World::session_login;

        // next session in 1-4 hours
        nextSessionAt = new TimePoint(now + std::chrono::minutes(random(60,60*4)));
    }

    // scene update
    if (scene && scene->isRunning()) {

        if (shade) shade->setVisible(! menuHidden);
        if (menu) menu->setVisible(! menuHidden);
    }

    // fs updates
    for (auto f: functionalities) {
        f->update();
    }
}

void World::session_login()
{
    if (! session->loggedIn)
    {
        auto request = new cocos2d::network::HttpRequest();
        request->setUrl(loginLink.c_str());
        request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
        request->setResponseCallback( CC_CALLBACK_2(World::onLoggedIn, this));
        char* userData = "Mozilla/5.0 (X11; U; Linux i686; pl; rv:1.8.0.3) Gecko/20060426 Firefox/1.5.0.3";
        request->setUserData(&userData);
        cocos2d::network::HttpClient::getInstance()->send(request);
        request->release();

        session->loggedIn = true;
    }
}

void World::onLoggedIn(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response)
{
    if (! response->isSucceed()) {

        ERROR("Can't log in");
        session->ended = true;

    } else {

        std::vector<char> page = *(response->getResponseData());

        std::cout << "Page contains of " << page.size() << " characters\n\n";
        for (auto c: page) {
            std::cout << c;
        }
        std::cout << "\n\n";

        session_page = &World::session_overview;
    }
}

void World::session_overview()
{
    std::cout << "OVERVIEW of " << world << "\n";

    //session_page = &World::session_main;

    if (1) session->ended = true;
}

cocos2d::Scene* World::getScene()
{
    if (scene) return scene;

    using namespace cocos2d;

    scene = Scene::create();

    auto screen = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // BACKGROUND
    auto background = DrawNode::create();
    Vec2 verts[] = {
        origin,
        Vec2(origin.x, origin.y + screen.height),
        Vec2(screen),
        Vec2(origin.x + screen.width, origin.y)
    };
    background->drawPolygon(verts, 4, Color4F(0.8, 0.8, 0.8, 1), 0, gColor);
    scene->addChild(background, -100);

    // TOP
    top = DrawNode::create();
    int h = topH;
    Vec2 verts1[] = {
        Vec2(origin.x, origin.y + screen.height),
        Vec2(origin.x + screen.width, origin.y + screen.height),
        Vec2(origin.x + screen.width, origin.y + screen.height - h),
        Vec2(origin.x, origin.y + screen.height - h)
    };
    top->drawPolygon(verts1, 4, gColor, 0, gColor);
    scene->addChild(top, 100);

    // MENU BUTTON
    auto menuButton = MenuItemImage::create("MenuButton.png", "MenuButton.png",
        CC_CALLBACK_1(World::menuButtonCallback, this));
    menuButton->setPosition(Vec2(origin.x + h/2, origin.y + screen.height - h/2));
    auto scaleTo = ScaleTo::create(0, (float(h)*0.7f) / menuButton->getContentSize().width);
    menuButton->runAction(scaleTo);
    auto m = Menu::create(menuButton, nullptr);
    m->setPosition(Vec2::ZERO);
    scene->addChild(m, 101);

    // SHADE
    if (! shade) shade = DrawNode::create();
    Vec2 verts2[] = {
        origin,
        Vec2(origin.x, origin.y + screen.height - h),
        Vec2(screen.width, screen.height - h),
        Vec2(origin.x + screen.width, origin.y)
    };
    shade->drawPolygon(verts2, 4, Color4F(0.0, 0.0, 0.0, 0.5), 0, gColor);
    shade->setVisible(! menuHidden);
    scene->addChild(shade, 99);

    // MENU
    menu = ui::Layout::create();
    menu->setContentSize(Size(float(screen.width)*float(3.0/4.0), screen.height - h));
    menu->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
    menu->setBackGroundColor(Color3B::WHITE);
    menu->setPosition(Vec2(0,0));
    menu->setVisible(! menuHidden);
    scene->addChild(menu, 100);

    // MENU ITEMS
    int font_size = 35;
    int interline = font_size / 2;
    int y = interline;
    for (auto f: functionalities)
    {
        auto text = ui::Text::create();
        text->setText(f->getName());
        text->setFontName("fonts/Marker Felt.ttf");
        text->setFontSize(30);
        text->setColor(Color3B::BLACK);
        text->setTextHorizontalAlignment(TextHAlignment::LEFT);
        text->setPosition(Point(float(screen.width)*float(3.0/4.0)/2.0, screen.height -h -font_size/2 -y));
        menu->addChild(text, 101);

        auto listener = EventListenerTouchOneByOne::create();
        listener->setSwallowTouches(true);
        listener->onTouchBegan = [=](Touch* touch, Event* event) {
            if (menuHidden) return false;
            #define IF_ON_TEXT \
            int pt = screen.height -h -y +interline/2; \
            Rect rect = Rect(0, pt, float(screen.width)*float(3.0/4.0), pt -font_size -interline); \
            if (touch->getLocation().x >= 0 && touch->getLocation().x <= float(screen.width)*float(3.0/4.0) \
                && touch->getLocation().y <= pt && touch->getLocation().y >= pt -font_size -interline)
            //if (rect.containsPoint(touch->getLocation()))
            IF_ON_TEXT
            {
                return true;
            }
            return false;
        };
        listener->onTouchEnded = [=](Touch* touch, Event* event) {
            if (menuHidden) return;
            IF_ON_TEXT
            {
                menuHidden = true;

                // changing functionality
                if (runningFunc != f)
                {
                    runningFunc->getConteiner()->removeFromParentAndCleanup(false);
                    runningFunc->getTopButtons()->removeFromParentAndCleanup(false);
                    runningFunc = f;

                    scene->addChild(f->getConteiner(true));
                    top->addChild(f->getTopButtons(true));
                }
            }
            #undef IF_ON_TEXT
        };
        Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, text);

        y += font_size + interline;
    }

    ///
    ///  Functionalities' nodes
    ///
    scene->addChild(runningFunc->getConteiner());
    top->addChild(runningFunc->getTopButtons());

    return scene;
}

void World::menuButtonCallback(cocos2d::Ref* sender)
{
    menuHidden = menuHidden == 1 ? 0 : 1;
}
