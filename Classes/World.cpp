#include "World.h"

#include "Building.h"
#include "Farming.h"

World::World(std::string _path, std::string _nick, std::string _world, std::string _market, std::string _loginLink)
: path(_path)
, nick(_nick)
, world(_world)
, market(_market)
, loginLink(_loginLink)
, session(NULL)
, nextSessionAt(GET_TIME_NOW)
/*old*///, session_page(NULL)
, scene(NULL)
, menu(NULL)
, shade(NULL)
, top(NULL)
, menuHidden(true)
, runningFunc(NULL)
{
    functionalities.push_back(new Building());
    functionalities.push_back(new Farming());
    runningFunc = functionalities[0];

    cocos2d::FileUtils* FU = cocos2d::FileUtils::getInstance();

    auto open_file = [&](std::string file)
    {
        if (! FU->isFileExist(path + file))
        {
            createDirectory(path);

            std::ofstream of(path + file);
            if (! of.is_open()) std::cout << "ERROR: file " << path + file << " can't be opened." << std::endl;
            of.close();
        }
    };

    open_file("world.txt");
    open_file("villages.txt");
    open_file("farms.txt");

    std::fstream file;

    // load villages
    file.open(path + "villages.txt");
    while (1)
    {
        std::string id;
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
        GET(id)
        #undef GET

        if (eof) break;

        villages.push_back(Village(path + "villages/" + id + "/", id));
        // create village directory if necessary
        if (! FU->isFileExist(path + "villages/" + id + "/village.txt"))
        {
            createDirectory(path + "villages");
            createDirectory(path + "villages/" + id);

            std::ofstream of(path + "villages/" + id + "/village.txt");
            if (! of.is_open()) std::cout
                << "ERROR: file "
                << path + "/villages/" + id + "/village.txt"
                << " can't be opened." << std::endl
            ;
            of.close();
        }
    }
    file.close();

    // load farms
    file.open(path + "farms.txt");
    for (std::string farm; std::getline(file, farm); farms.push_back(Farm(farm))) {}
    file.close();
}

World::~World()
{
    for (auto f: functionalities) {
        delete f;
    }
}

void World::update()
{
    sessionUpdate();

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

void World::sessionUpdate()
{
    // session
    TimePoint now = GET_TIME_NOW;
    if (session) // there is running session
    {
        // ending
        if (session->ended)
        {

            DELETE(session);
        }
        // waiting
        else if (session->unlockedAt)
        {
            if (*session->unlockedAt < now) {
                DELETE(session->unlockedAt);
            }
        }
        // logging in
        else if (session->loggingInPhase < 2)
        {
            if (session->loggingInPhase == 0)
            {
                auto request = new cocos2d::network::HttpRequest();
                request->setUrl(loginLink.c_str());
                request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
                request->setResponseCallback( CC_CALLBACK_2(World::onLoggedIn, this));
                char* userData = "Mozilla/5.0 (X11; U; Linux i686; pl; rv:1.8.0.3) Gecko/20060426 Firefox/1.5.0.3";
                request->setUserData(&userData);
                cocos2d::network::HttpClient::getInstance()->send(request);
                request->release();

                session->loggingInPhase = 1;
            }
        }
        // global review
        else if (session->globalReviewPhase != GRP::GRP_END)
        {
            // TODO: make global review same to village review

            switch (session->globalReviewPhase) {

            case GRP::GRP_START:
                std::cout << "Review phase started.\n";
                session->globalReviewPhase++;
                break;

            case GRP::GRP_MAP:
                // TODO
                session->globalReviewPhase++;
                break;

            case GRP::GRP_REPORTS:
                // TODO
                session->globalReviewPhase++;
                break;

            case GRP::GRP_END:
                //
                session->globalReviewPhase++;
                break;

            default:
                session->globalReviewPhase = GRP::GRP_END;
                break;
            }
        }
        // villages review
        else if (! session->allVillagesReviewed && session->currentVillage < villages.size())
        {
            if (session->villageReviewPhase == VRP::VRP_START)
            {
                session->villageReviewPhase++;
            }
            else if (session->villageReviewPhase == VRP::VRP_END)
            {
                std::cout << "Village " << session->currentVillage << " review completed.\n";
                session->currentVillage++;
                session->villageReviewPhase = VRP::VRP_START;
            }
            else if (session->villageReviewPhase != VRP::VRP_WAIT)
            {
                auto request = new cocos2d::network::HttpRequest();

                #define CREATE_REQUEST(villageScreen, callbackFunction) { \
                    request = new cocos2d::network::HttpRequest(); \
                    request->setUrl(std::string("http://" + world + ".plemiona." + market + "/game.php?village=" \
                        + villages[session->currentVillage].id + "&screen=" + villageScreen).c_str()); \
                    request->setResponseCallback(CC_CALLBACK_2(World::callbackFunction, this)); \
                }

                std::string villageScreen;

                Village& v = villages[session->currentVillage];
                auto bExist = [&](TWBuilding b)->bool { return v.buildings[b]; };

                switch (session->villageReviewPhase) {

                case VRP::VRP_OVERVIEW:
                    villageScreen = "overview";
                    request->setResponseCallback(CC_CALLBACK_2(World::onVillageOverviewed, this));
                    break;

                case VRP::VRP_HQ:
                    villageScreen = "main";
                    request->setResponseCallback(CC_CALLBACK_2(World::onVillageHQViewed, this));
                    break;

                case VRP::VRP_SMITH:
                    for (int i = 0; i < TW_UNIT_SIZE; i++) v.isTech[i] = false;
                    if (bExist(TW_BUILDING_STATUE)) v.isTech[TW_UNIT_PALADIN] = true;
                    if (bExist(TW_BUILDING_BARRACKS)) v.isTech[TW_UNIT_SPEAR] = true;
                    if (bExist(TW_BUILDING_ACADEMY)) v.isTech[TW_UNIT_NOBLE] = true;
                    if (bExist(TW_BUILDING_SMITH)) {
                        villageScreen = "smith";
                        request->setResponseCallback(CC_CALLBACK_2(World::onVillageSmithViewed, this));
                    }
                    break;

                default:
                    session->villageReviewPhase = VRP::VRP_END;
                    break;
                }

                #undef CREATE_REQUEST

                if (! villageScreen.empty()) {

                    if (market != "pl") ERROR("Tribot nie wspiera światów nie polsko-języcznych"); // #ONLY_POLISH
                    request->setUrl(std::string("http://" + world + ".plemiona." + market + "/game.php?village="
                        + villages[session->currentVillage].id + "&screen=" + villageScreen).c_str());
                    request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
                    char* userData = "Mozilla/5.0 (X11; U; Linux i686; pl; rv:1.8.0.3) Gecko/20060426 Firefox/1.5.0.3";
                    request->setUserData(&userData);

                    session->villageReviewPhase = VRP::VRP_WAIT;

                    cocos2d::network::HttpClient::getInstance()->send(request);
                    request->release();
                }
                else
                {
                    session->villageReviewPhase = VRP::VRP_END;
                }
            }

            /*old*//*
            if (session->villagePhase == 0) // start village overview
            {
                session->villagePhase = 1;
                villageReview(&villages[session->currentVillage]);
            }
            else if (session->villagePhase == 2) // start HQ review
            {
                session->villagePhase = 3;
                villageHQReview(&villages[session->currentVillage]);
            }
            else if (session->villagePhase == 4) // review completed
            {
                std::cout << "Village " << session->currentVillage << " review completed.\n";
                session->currentVillage++;
                session->villagePhase = 0;
            }
            */
        }
        // devising strategy
        else if (! session->allVillagesReviewed)
        {
            session->allVillagesReviewed = true;
            session->currentVillage = 0;

            deviseStrategy();
        }
        // and finally... playing (orders & expensions)
        else if (session->currentVillage < villages.size())
        {
            if (! session->villageActionsExecuted)
            {
                session->villageActionsExecuted = true;
                session->currentOrder = 0;
                executeVillageActions(session->currentVillage);
            }
            /*old*/// else if (session->villagePhase == 2)
            else if (session->actionRequests.empty())
            {
                std::cout << "All actions of village " << session->currentVillage << " executed.\n";
                session->currentVillage++;
                session->villageActionsExecuted = false;
            }
        }
        // when there is nothing to do...
        else
        {
            session->ended = true;
            std::cout << "Session ended normally.\n";

            std::time_t t = std::chrono::system_clock::to_time_t(nextSessionAt);
            std::cout << "Next session at: " << std::ctime(&t);
        }

    } else if (nextSessionAt < now) { // create new session

        session = new Session(villages.size());

        /*old*///session_page = &World::session_login;

        // next session in 1-4 hours
        nextSessionAt = now + std::chrono::minutes(random(17,37));
        //nextSessionAt = now + std::chrono::minutes(random(60,60*4));
    }
}

void World::executeVillageActions(int v /*villageIndex*/)
{
    /// BUILD
    for (auto &b: session->villagesActions[v].build)
    {
        auto r = new cocos2d::network::HttpRequest();
        if (market != "pl") ERROR("Tribot nie wspiera światów nie polsko-języcznych"); // #ONLY_POLISH
        r->setUrl(std::string(
            "http://"+world+".plemiona."+market+"/game.php?village="+villages[v].id+"&action=upgrade_building&h="
            +villages[v].h_value+"&id="+twBuildingName[b]+"&type="+twBuildingName[b]+"&screen=main"
        ).c_str());
        r->setRequestType(cocos2d::network::HttpRequest::Type::GET);
        r->setResponseCallback( CC_CALLBACK_2(World::onActionRequestEnded, this));
        char* userData = "Mozilla/5.0 (X11; U; Linux i686; pl; rv:1.8.0.3) Gecko/20060426 Firefox/1.5.0.3";
        r->setUserData(&userData);
        session->actionRequests.push_back(r);

        std::cout << "Adding " << twBuildingName[b] << " to building queue\n";
    }

    /// ORDERS
    for (auto &o: session->villagesActions[v].order)
    {
        auto r = new cocos2d::network::HttpRequest();
        if (market != "pl") ERROR("Tribot nie wspiera światów nie polsko-języcznych"); // #ONLY_POLISH
        r->setUrl(std::string("http://"+world+".plemiona."+market+"/game.php?village="+villages[v].id+"&screen=place").c_str());
        r->setRequestType(cocos2d::network::HttpRequest::Type::GET);
        r->setResponseCallback( CC_CALLBACK_2(World::onGoToPlaceEnded, this));
        char* userData = "Mozilla/5.0 (X11; U; Linux i686; pl; rv:1.8.0.3) Gecko/20060426 Firefox/1.5.0.3";
        r->setUserData(&userData);
        session->actionRequests.push_back(r);

        std::cout << "Adding " << (o.isAttack ? "attack" : "support") << " order to " << o.target.str() << "\n";
    }

    /// RECRUIT
    for (auto &r: session->villagesActions[v].recruit)
    {
        //
        std::cout << "Recruiting ...\n";
    }

    /// TECH
    for (auto &t: session->villagesActions[v].tech)
    {
        //
        std::cout << "Adding " << "" << " to tech queue\n";
    }

    // must be called!
    session->nextActionRequest();
}

void World::onLoggedIn(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response)
{
    if (! response->isSucceed()) {

        ERROR("Can't log in to " + nick + "|" + world);
        ERROR("Are you sure that this link is valid? " + loginLink);
        session->ended = true;

    } else {

        session->loggingInPhase = 2;
        std::cout << "Logged to: " << nick << "|" << world << "\n";
        session->pause(3,6);

        /*
        std::vector<char> page = *(response->getResponseData());

        std::cout << "Page contains of " << page.size() << " characters\n\n";
        for (auto c: page) {
            std::cout << c;
        }
        std::cout << "\n\n";

        session_page = &World::session_overview;
        */
    }
}

void World::onVillageOverviewed(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response)
{
    /*old*///if (session->villagePhase != 1) ERROR("session->villagePhase != 1");

    if (! response->isSucceed()) {

        ERROR("Can't review village" + session->currentVillage);
        ERROR("Are you sure that this village ID is valid? " + villages[session->currentVillage].id);

    } else {

        // reading the page
        std::string page;
        std::vector<char> resp = *(response->getResponseData());
        for (auto c: resp) {
            page += c;
        }

        Village& v = villages[session->currentVillage];

        /*********************************
        * Reading data from village page *
        *********************************/

        // zmienne pomocnicze
        std::size_t found;

        /// coordinates

        std::string coord = "coord\":\"";
        found = page.find(coord);
        if (found == std::string::npos) ERROR("Nie znaleziono coord");

        coord = "";
        for (int i = 0; i < 7; i++) {
            if (i != 3) coord += page[found + coord.size() + i];
        }
        v.coordinates = coord;

        /// h_value

        std::string h_declaration = "var csrf_token = '";
        found = page.find(h_declaration);
        if (found == std::string::npos) ERROR("Nie znaleziono csrf_token");
        v.h_value = "";
        for (int i = found + h_declaration.size(); 1; i++) {
            if (page[i] == '\'') break;
            v.h_value += page[i];
        }

        /// resources

        auto res = [&](std::string res_name)->int{
            std::string s = "\"" + res_name + "\":";
            if (res_name == "pop") s += "\"";
            found = page.find(s);
            if (found == std::string::npos) {
                ERROR("Nie znaleziono " + res_name);
                return -1;
            }
            std::string result;
            for (int i = 0; 1; i++) {
                if (page[found + s.size() + i] == ',' || page[found + s.size() + i] == '"') break;
                result += page[found + s.size() + i];
            }
            return std::stoi(result);
        };

        v.resources = TWCost(res("wood"), res("stone"), res("iron"), res("pop"));

        /// buildings' levels

        auto lvl = [&](std::string name)->int {
            std::string s = "\"" + name + "\":\"";
            found = page.find(s, page.find("\"buildings\":{\"village\":\"" + v.id + "\","));
            if (found == std::string::npos) {
                ERROR("Nie znaleziono " + name);
                return -1;
            }
            std::string result;
            for (int i = 0; 1; i++) {
                if (page[found + s.size() + i] == '"') break;
                result += page[found + s.size() + i];
            }
            return std::stoi(result);
        };

        for (int i = 0; i < TW_BUILDING_SIZE; i++) {
            v.buildings[i] = lvl(twBuildingName[i]);
        }

        /// army

        found = 0;
        auto bound = page.find("screen=train");
        bound = page.find("screen=train", bound+1);
        for (int i = 0; i < TW_UNIT_SIZE; i++)
        {
            found = page.find("unit_" + twUnitName[i], found);
            if (found == std::string::npos || found > bound) {
                v.army[i] = 0;
                found = 0;
                continue;
            }
            std::string s = "<strong>";
            found = page.find(s, found);
            std::string result;
            for (int j = found + s.size(); 1; j++) {
                if (page[j] == '<') break;
                result += page[j];
            }
            v.army[i] = std::stoi(result);
        }

        /// storage

        auto readInt = [&](std::string name)->int {
            found = page.find("},\"village\":{\"id\":" + v.id + ",\"name\":\"");
            std::string s = name + "\":\"";
            found = page.find(s, found);
            if (found == std::string::npos) {
                ERROR("Nie znaleziono " + name);
                return -1;
            }
            std::string result;
            for (int i = 0; 1; i++) {
                if (page[found + s.size() + i] == '"') break;
                result += page[found + s.size() + i];
            }
            return std::stoi(result);
        };

        v.storageLimit = readInt("storage_max");
        v.populationLimit = readInt("pop_max");

        /// orders

        found = 0;
        while (1)
        {
            found = page.find("own_command", found);
            if (found == std::string::npos) {
                break;
            }
            std::string s = "data-command-type=\"";
            found = page.find(s, found);
            if (found == std::string::npos) {
                ERROR("Can't find " + s);
                break;
            }
            std::string type;
            for (size_t i = found + s.size(); 1; i++) {
                if (page[i] == '"') break;
                type += page[i];
            }
            found = page.find("</span>", found) + 1;
            found = page.find("</span>", found);
            bool opened = 0;
            std::string coo;
            while (1)
            {
                if (page[found] == '(') break;
                else if (page[found] == ')') opened = 1;
                else if (opened && page[found] != '|') {
                    coo += page[found];
                }
                found--;
            }
            if (coo.size() != 6) ERROR("Order has invalid coords: " + coo);
            std::reverse(coo.begin(), coo.end());
            if (type == "attack") {
                v.attackOrders.push_back(coo);
            } else if (type == "return") {
                v.returnOrders.push_back(coo);
            } else if ((type != "cancel") && (type != "support")) {
                ERROR("Unknown command: " + type);
            }
        }

        /* old
        /// building queue
        found = page.find("gif\" alt=\"Ratusz\""); // #ONLY_POLISH
        if (found == std::string::npos) v.buildingsInQueue = 0;
        else v.buildingsInQueue = 2; // non-zero
        */

        // at the end
        session->pause(1,4);
    }

    session->villageReviewPhase = VRP::VRP_OVERVIEW + 1;
}

void World::onVillageHQViewed(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response)
{
    /*old*///if (session->villagePhase != 3) ERROR("session->villagePhase != 3");

    if (! response->isSucceed()) {

        ERROR("Can't review HQ" + session->currentVillage);
        ERROR("Are you sure that this village ID is valid? " + villages[session->currentVillage].id);

    } else {

        // reading the page
        std::string page;
        std::vector<char> resp = *(response->getResponseData());
        for (auto c: resp) {
            page += c;
        }

        Village& v = villages[session->currentVillage];

        /****************************
        * Reading data from HQ page *
        ****************************/
        std::size_t found;

        /// building queue

        std::string today_at = ">dzisiaj o"; // > is important // #ONLY_POLISH
        v.buildingsInQueue = 0;
        found = 0;
        while (1) {
            found = page.find(today_at, found + today_at.size());
            if (found == std::string::npos) break;
            v.buildingsInQueue++;
        }

        /// buildings' costs

        // #ONLY_POLISH
        auto cost = [&](std::string name, int building_id){

            found = page.find("\"name\":\"" + name + "\"");
            found = page.find("\"wood\"", found);
            if (found == std::string::npos) {
                ERROR("Nie znaleziono " + name);
                return;
            }
            std::string str;
            for (int i = found, res = 0, read = false; res < 4; i++)
            {
                if (page[i] == ':') { // read begin
                    read = true;
                    str = "";
                } else if (page[i] == ',') { // read end
                    read = false;
                    v.buildingsCosts[building_id][res] = std::stoi(str);
                    res++;
                } else if (read == true) { // reading
                    str += page[i];
                }
            }
        };

        // #ONLY_POLISH
        cost("Ratusz", TW_BUILDING_HQ);
        cost("Tartak", TW_BUILDING_WOOD);
        cost("Cegielnia", TW_BUILDING_STONE);
        cost("Huta \\u017celaza", TW_BUILDING_IRON); // be careful with Polish characters
        cost("Spichlerz", TW_BUILDING_STORAGE);
        cost("Zagroda", TW_BUILDING_FARM);
        // ... TODO

        // at the end
        session->pause(1,3);
    }

    session->villageReviewPhase = VRP::VRP_HQ + 1;
}

void World::onVillageSmithViewed(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response)
{
    if (! response->isSucceed()) {

        ERROR("Can't review HQ" + session->currentVillage);
        ERROR("Are you sure that this village ID is valid? " + villages[session->currentVillage].id);

    } else {

        // reading the page
        std::string page;
        std::vector<char> resp = *(response->getResponseData());
        for (auto c: resp) {
            page += c;
        }

        Village& v = villages[session->currentVillage];

        std::size_t found;

        /// technology levels

        found = page.find("BuildingSmith.techs");
        for (int i = 1; i < TW_UNIT_SIZE - 2; i++)
        {
            found = page.find(twUnitName[i], found);
            std::string s = "\"level\":\"";
            found = page.find(s, found) + s.size();
            if (page[found] == '0') v.isTech[i] = false;
            else if (page[found] == '1') v.isTech[i] = true;
            else {
                ERROR(std::string("Unknown technology level \"") += page[found] + "\"");
                v.isTech[i] = false;
            }
        }

        // at the end
        session->pause(1,3);
    }

    session->villageReviewPhase = VRP::VRP_SMITH + 1;
}

void World::onActionRequestEnded(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response)
{
    if (! response->isSucceed()) {
        ERROR("Invalid response for village " + std::to_string(session->currentVillage) + " action request");
    }

    /*old*///session->villagePhase = 2; // for compatibility with old version

    session->nextActionRequest();
}

void World::onGoToPlaceEnded(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response)
{
    if (! response->isSucceed()) {
        ERROR("Invalid response for village " + std::to_string(session->currentVillage) + " go to the place request");
    }

    /*TODO*/// add more asserts

    /// reading the response

    std::string page;
    std::vector<char> resp = *(response->getResponseData());
    for (auto c: resp) {
        page += c;
    }

    std::size_t found;
    std::string s, codeName, codeValue;

    s = "<input type=\"hidden\" name=\"";
    found = page.find(s);
    for (int i = found + s.size(); 1; i++) {
        if (page[i] == '"') break;
        codeName += page[i];
    }

    s = " value=\"";
    found = page.find(s, found);
    for (int i = found + s.size(); 1; i++) {
        if (page[i] == '"') break;
        codeValue += page[i];
    }

    /// creating POST request (sending army)

    TWOrder order = session->villagesActions[session->currentVillage].order[session->currentOrder];
    session->currentOrder++;

    std::stringstream postData;
    postData << codeName << "=" << codeValue;
    postData << "&template_id=";
    for (int i = 0; i < TW_UNIT_SIZE; i++) {
        postData << "&" << twUnitName[i] << "=" << order.army[i];
    }
    postData << "&x=" << order.target.x();
    postData << "&y=" << order.target.y();
    postData << "&target_type=coord";
    postData << "&input=" << order.target.x() << "%7C" << order.target.y();
    postData << "&attack=" << (order.isAttack ? "Napad" : "Wsparcie"); // #ONLY_POLISH

    std::cout << "Sending POST: " << postData.str() << std::endl;

    auto r = new cocos2d::network::HttpRequest();
    if (market != "pl") ERROR("Tribot nie wspiera światów nie polsko-języcznych"); // #ONLY_POLISH
    r->setUrl(
        std::string("http://"+world+".plemiona."+market+"/game.php?village="
        +villages[session->currentVillage].id+"&try=confirm&screen=place").c_str()
    );
    r->setRequestType(cocos2d::network::HttpRequest::Type::POST);
    r->setRequestData(postData.str().c_str(), strlen(postData.str().c_str()));
    r->setResponseCallback(CC_CALLBACK_2(World::onSendArmyEnded, this));
    char* userData = "Mozilla/5.0 (X11; U; Linux i686; pl; rv:1.8.0.3) Gecko/20060426 Firefox/1.5.0.3";
    r->setUserData(&userData);
    session->actionRequests.push_front(r); // must be at front of the queue to be executed first

    session->nextActionRequest();
}

void World::onSendArmyEnded(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response)
{
    if (! response->isSucceed()) {
        ERROR("Invalid response for village " + std::to_string(session->currentVillage) + " go send army request");
    }

    /// reading data from page

    std::string page;
    std::vector<char> resp = *(response->getResponseData());
    for (auto c: resp) {
        page += c;
    }

    std::size_t found = 0;
    std::string s;
    std::string attack = "attack", ch = "ch", x = "x", y = "y", action_id = "action_id"; // post data
    TWArmy army;

    auto read = [&](std::string& name) {
        s = "<input type=\"hidden\" name=\"" + name + "\" value=\"";
        name = "";
        found = page.find(s, found);
        if (found == std::string::npos) ERROR("npos 1");
        for (int i = found + s.size(); 1; i++) {
            if (page[i] == '"') break;
            name += page[i];
        }
    };

    read(attack);
    read(ch);
    read(x);
    read(y);
    read(action_id);

    for (int i = 0; i < TW_UNIT_SIZE; i++) {
        s = "<input type=\"hidden\" name=\"" + twUnitName[i] + "\" value=\"";
        found = page.find(s, found);
        if (found == std::string::npos) ERROR("npos 2");
        std::string unitCount;
        for (int j = found + s.size(); 1; j++) {
            if (page[j] == '"') break;
            unitCount += page[j];
        }
        army[i] = std::stoi(unitCount);
    }

    std::string csrf;
    s = "var csrf_token = '";
    found = page.find(s);
    for (int i = found + s.size(); 1; i++) {
        if (page[i] == '\'') break;
        csrf += page[i];
    }

    /// creating POST request

    std::stringstream postData;
    postData << "attack=" << attack;
    postData << "&ch=" << ch;
    postData << "&x=" << x;
    postData << "&y=" << y;
    postData << "&action_id=" << action_id;
    for (int i = 0; i < TW_UNIT_SIZE; i++) {
        postData << "&" << twUnitName[i] << "=" << army[i];
    }

    std::cout << "Sending POST: " << postData.str() << std::endl;

    auto r = new cocos2d::network::HttpRequest();
    if (market != "pl") ERROR("Tribot nie wspiera światów nie polsko-języcznych"); // #ONLY_POLISH
    r->setUrl(
        std::string("http://"+world+".plemiona."+market+"/game.php?village="
        +villages[session->currentVillage].id+"&action=command&h="+csrf+"&screen=place").c_str()
    );
    r->setRequestType(cocos2d::network::HttpRequest::Type::POST);
    r->setRequestData(postData.str().c_str(), strlen(postData.str().c_str()));
    r->setResponseCallback( CC_CALLBACK_2(World::onActionRequestEnded, this));
    char* userData = "Mozilla/5.0 (X11; U; Linux i686; pl; rv:1.8.0.3) Gecko/20060426 Firefox/1.5.0.3";
    r->setUserData(&userData);
    session->actionRequests.push_front(r); // must be at front of the queue to be executed first

    session->nextActionRequest();
}

/* old
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
*/

/* old
void World::session_overview()
{
    std::cout << "OVERVIEW of " << world << "\n";

    //session_page = &World::session_main;

    if (1) session->ended = true;
}
*/

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

/*old*//*
void World::villageReview(Village* v)
{
    // start overeview
    auto request = new cocos2d::network::HttpRequest();
    if (market != "pl") ERROR("Tribot nie wspiera światów nie polsko-języcznych"); // #ONLY_POLISH
    request->setUrl(std::string("http://" + world + ".plemiona." + market + "/game.php?village=" + v->id + "&screen=overview").c_str());
    request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
    request->setResponseCallback( CC_CALLBACK_2(World::onVillageViewed, this));
    char* userData = "Mozilla/5.0 (X11; U; Linux i686; pl; rv:1.8.0.3) Gecko/20060426 Firefox/1.5.0.3";
    request->setUserData(&userData);
    cocos2d::network::HttpClient::getInstance()->send(request);
    request->release();
}

void World::villageHQReview(Village* v)
{
    // start HQ review
    auto request = new cocos2d::network::HttpRequest();
    if (market != "pl") ERROR("Tribot nie wspiera światów nie polsko-języcznych"); // #ONLY_POLISH
    request->setUrl(std::string("http://" + world + ".plemiona." + market + "/game.php?village=" + v->id + "&screen=main").c_str());
    request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
    request->setResponseCallback( CC_CALLBACK_2(World::onVillageHQViewed, this));
    char* userData = "Mozilla/5.0 (X11; U; Linux i686; pl; rv:1.8.0.3) Gecko/20060426 Firefox/1.5.0.3";
    request->setUserData(&userData);
    cocos2d::network::HttpClient::getInstance()->send(request);
    request->release();
}
*/
