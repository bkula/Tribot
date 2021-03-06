#ifndef __COMMON_H__
#define __COMMON_H__

/// HASHTAGS LIST:
/*
#ONLY_POLISH
*/

/// Comments' types:
/*old*/
/*TODO*/
/*info*/
/*return*/

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "network/HttpClient.h"

// STL
#include "string"
#include "vector"
#include "algorithm"
#include "iostream"
#include "cmath"
#include "chrono"
#include "fstream"
#include "queue"
#include "list"

void initGlobals();

// global defines & constants
// prefix "g" means "global"

#define SCREEN cocos2d::Director::getInstance()->getVisibleSize()
#define ORIGIN cocos2d::Director::getInstance()->getVisibleOrigin()

#define gColor cocos2d::Color4F(0.1f, 0.2f, 0.7f, 1.0f)

const std::string gFont =  "fonts/Marker Felt.ttf";
const int topH = 80;

// useful macros
#define DELETE(ptr) if (ptr) {delete (ptr); (ptr) = NULL;}

inline void ERROR(std::string message)
{
    std::cout << "Tribot ERROR: " << message << std::endl;
}

#define GET_TIME_NOW std::chrono::system_clock::now()

// typedefs
typedef std::chrono::system_clock::time_point TimePoint;
typedef long long LL;

inline LL random(LL from, LL to)
{
    return from + (random() % (to - from));
}

// 0 - good, non-zero result - error
inline int createDirectory(std::string name)
{
    #if (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        return system(std::string("mkdir \"" + name + "\"").c_str());
    #else
        ERROR("Nie obsługiwana platforma");
        return -1;
    #endif
}

// dictionary
// errors

/* REMEMBER: if you use network...
Note that if you environment is Android do not forget adding permissions on your app’s Manifest :
<uses-permission android:name="android.permission.INTERNET"/>
*/

class PageReader // TODO
{
    PageReader(std::vector<char> source)
    {
        // TODO
    }

    int readInt();
    std::string readStr();

    struct Location
    {
        // list
    };
};

///
/// Tribal Wars' types
///

enum TWBuilding
{
    TW_BUILDING_HQ,
    TW_BUILDING_BARRACKS,
    TW_BUILDING_ACADEMY,
    TW_BUILDING_SMITH,
    TW_BUILDING_STATUE,
    TW_BUILDING_MARKET,
    TW_BUILDING_WOOD,
    TW_BUILDING_STONE,
    TW_BUILDING_IRON,
    TW_BUILDING_FARM,
    TW_BUILDING_STORAGE,
    TW_BUILDING_HIDE,
    // ... TODO
    TW_BUILDING_SIZE
};

enum TWUnit
{
    TW_UNIT_SPEAR,
    TW_UNIT_SWORD,
    TW_UNIT_AXE,
    TW_UNIT_BOW,
    TW_UNIT_SCOUT,
    TW_UNIT_LC,
    TW_UNIT_MA,
    TW_UNIT_HC,
    TW_UNIT_RAM,
    TW_UNIT_CAT,
    TW_UNIT_PALADIN,
    TW_UNIT_NOBLE,
    TW_UNIT_SIZE
};

extern std::string twBuildingName[TW_BUILDING_SIZE];
extern std::string twUnitName[TW_UNIT_SIZE];

class TWArmy
{
public:

    TWArmy()
    {
        for (int i = 0; i < 12; i++) {
            units[i] = 0;
        }
    }

    int& operator[](int i)
    {
        if (i < 0 || i >= 12) {
            ERROR("Invalid argument given for TWArmy[]");
            return units[0];
        }
        return units[i];
    }

private:

    int units[12];
};

struct TWCoordinates
{
public:

    TWCoordinates()
    {
    }

    TWCoordinates(std::string _six)
    {
        if (_six.size() == 7) {
            _six.erase(3);
        } else if (_six.size() != 6) {
            ERROR("Invalid argument given for TWCoordinates(string): " + _six);
        }
        six = _six;
    }

    TWCoordinates(int x, int y)
    {
        std::string sx, sy;
        sx = std::to_string(x), sy = std::to_string(y);
        if (sx.size() != 3 || sy.size() != 3) {
            ERROR("Invalid args given for TWCoordinates(int,int): (" + sx + "," + sy + ")");
        }
        six = sx + sy;
    }

    TWCoordinates& operator=(const std::string &str) {

        *this = TWCoordinates(str);

        return *this;
    }

    operator std::string() const
    {
        if (six.size() != 6) {
            ERROR("Uninitializet TWCoordinates");
        }
        return six;
    }

    int x() const
    {
        return std::stoi(six.substr(0, 3));
    }

    int y() const
    {
        return std::stoi(six.substr(3, 3));
    }

    std::string str() const
    {
        return six;
    }

private:

    std::string six; // string contains six numbers
};

struct TWOrder
{
    TWArmy army;
    TWCoordinates target;
    bool isAttack;
};

struct TWCost
{
    int wood;
    int stone;
    int iron;
    int pop;

    int& operator[](int i)
    {
        if (i == 0) return wood;
        else if (i == 1) return stone;
        else if (i == 2) return iron;
        else if (i == 3) return pop;
        else {
            ERROR("Nieprawidłowy index TWCost");
            return wood;
        }
    }

    int operator[](int i) const
    {
        if (i == 0) return wood;
        else if (i == 1) return stone;
        else if (i == 2) return iron;
        else if (i == 3) return pop;
        else {
            ERROR("Nieprawidłowy index TWCost");
            return wood;
        }
    }

    explicit TWCost(int w = 0, int s = 0, int i = 0, int p = 0)
    : wood(w), stone(s), iron(i), pop(p)
    {
    }

    explicit TWCost(int src[4])
    : wood(src[0]), stone(src[1]), iron(src[2]), pop(src[3])
    {
    }

    TWCost& operator=(const TWCost &c) {

        if (this == &c) return *this;

        for (int i = 0; i < 4; i++) {
            (*this)[i] = c[i];
        }

        return *this;
    }

    friend TWCost operator+(const TWCost& a, const TWCost& b)
    {
        TWCost res;
        for (int i = 0; i < 4; i++) {
            res[i] = a[i] + b[i];
        }
        return res;
    }

    friend TWCost operator-(const TWCost& a, const TWCost& b)
    {
        TWCost res;
        for (int i = 0; i < 4; i++) {
            res[i] = a[i] - b[i];
        }
        return res;
    }

    TWCost& operator+=(const TWCost& c)
    {
        for (int i = 0; i < 4; i++) {
            (*this)[i] = (*this)[i] + c[i];
        }
        return *this;
    }

    TWCost& operator-=(const TWCost& c)
    {
        for (int i = 0; i < 4; i++) {
            (*this)[i] = (*this)[i] - c[i];
        }
        return *this;
    }

    bool operator<(const TWCost& c)
    {
        for (int i = 0; i < 4; i++) {
            if (!((*this)[i] < c[i])) return false;
        }
        return true;
    }

    bool operator>(const TWCost& c)
    {
        for (int i = 0; i < 4; i++) {
            if (!((*this)[i] > c[i])) return false;
        }
        return true;
    }

    bool operator<=(const TWCost& c)
    {
        for (int i = 0; i < 4; i++) {
            if (!((*this)[i] <= c[i])) return false;
        }
        return true;
    }

    bool operator>=(const TWCost& c)
    {
        for (int i = 0; i < 4; i++) {
            if (!((*this)[i] >= c[i])) return false;
        }
        return true;
    }
};

#endif // __COMMON_H__
