#include "World.h"

void World::deviseStrategy()
{
    buildEco();
    farm();

    std::cout << "Strategy devised ;)" << std::endl;
}

void World::farm()
{
    // FIRST
}

void World::buildEco()
{
    for (int i = 0; i < villages.size(); i++)
    {
        Village &v = villages[i];

        auto mm = std::minmax({
            v.buildings[TW_BUILDING_WOOD],
            v.buildings[TW_BUILDING_STONE],
            v.buildings[TW_BUILDING_IRON],
            v.buildings[TW_BUILDING_STORAGE],
            v.buildings[TW_BUILDING_FARM]
        });

        auto build = [&](TWBuilding b)
        {
            if (
                (v.buildings[b] < mm.second // too low level
                || mm.first == mm.second) // all levels are equal
                && v.buildingsInQueue < 2 // queue isn't full
                && v.buildingsCosts[b] <= v.resources // enough rescources
            )
            {
                session->villagesActions[i].build.push_back(b);
                v.resources -= v.buildingsCosts[b];
                v.buildingsInQueue++;
                //TODO nextSessionAt = std::min(nextSessionAt, now + std::chrono::seconds(v.buildingTime[b]));
            }
        };

        build(TW_BUILDING_WOOD);
        build(TW_BUILDING_STONE);
        build(TW_BUILDING_IRON);
        build(TW_BUILDING_STORAGE);
        build(TW_BUILDING_FARM);
    }
}
