#include "World.h"

void World::deviseStrategy()
{
    printData();

    buildEco();
    farm();

    std::cout << "Strategy devised ;)" << std::endl;
}

void World::printData()
{
    using namespace std;

    cout<<"\nDATA printing begin\n\n";

    /// farms
    cout << farms.size() << " farms: ";
    for (int i = 0; i < farms.size(); i++) {
        cout << std::string(farms[i].coords) << " ";
    }
    cout << "\n";

    for (int v = 0; v < villages.size(); v++)
    {
        cout << "\nVillage "<< v << " data:\n";

        cout << "Resources: ";
        for (int  i = 0; i < 3; i++) cout << villages[v].resources[i] << " ";
        cout << "Storage: " << villages[v].storageLimit << endl;
        cout << "Population: " << villages[v].resources[3] << "/" << villages[v].populationLimit << endl;

        cout << "Buildings: ";
        for (int i = 0; i < TW_BUILDING_SIZE; i++)
        {
            cout << villages[v].buildings[i] << " ";
        }
        cout << endl;

        cout << "Army: ";
        for (int i = 0; i < TW_UNIT_SIZE; i++) {
            cout << villages[v].army[i] << " ";
        }
        cout << endl;

        cout << "Technology: ";
        for (int i = 0; i < TW_UNIT_SIZE; i++) {
            cout << villages[v].isTech[i] << " ";
        }
        cout << endl;

        cout << "Attacks: ";
        for (auto c: villages[v].attackOrders) {
            cout << c.str() << " ";
        }
        cout << endl;

        cout << "Returns: ";
        for (auto c: villages[v].returnOrders) {
            cout << c.str() << " ";
        }
        cout << endl;
    }

    cout << "\nDATA printing end\n\n";
}

void World::farm()
{
    if (! farms.empty())
    {
        for (int i = 0; i < villages.size(); i++)
        {
            if (villages[i].army[0] == 0) continue;

            TWOrder o;
            o.isAttack = true;
            o.target = farms[0].coords;
            TWArmy a;
            a[0] = 1;
            o.army = a;

            session->villagesActions[i].order.push_back(o);
        }
    }
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
