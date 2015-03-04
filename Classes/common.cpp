
#include "common.h"

cocos2d::Color4F globalColor = cocos2d::Color4F::BLUE;

/*old*///std::string mainDirPath = "";

std::string twBuildingName[TW_BUILDING_SIZE];
std::string twUnitName[TW_UNIT_SIZE];

void initGlobals()
{
    for (int i = 0; i < TW_BUILDING_SIZE; i++) twBuildingName[i] = "NO-NAME";
    twBuildingName[TW_BUILDING_HQ] = "main";
    twBuildingName[TW_BUILDING_WOOD] = "wood";
    twBuildingName[TW_BUILDING_STONE] = "stone";
    twBuildingName[TW_BUILDING_IRON] = "iron";
    twBuildingName[TW_BUILDING_STORAGE] = "storage";
    twBuildingName[TW_BUILDING_FARM] = "farm";
    // ... TODO

    for (int i = 0; i < TW_UNIT_SIZE; i++) twUnitName[i] = "NO-NAME";
    twUnitName[TW_UNIT_NOBLE] = "noble";
    // ... TODO
}
