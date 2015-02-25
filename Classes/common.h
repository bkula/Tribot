#ifndef __COMMON_H__
#define __COMMON_H__

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

/** WARNING: if you use network...
Note that if you environment is Android do not forget adding permissions on your app’s Manifest :
<uses-permission android:name="android.permission.INTERNET"/>
*/

#endif // __COMMON_H__
