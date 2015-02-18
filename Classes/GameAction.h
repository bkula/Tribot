#ifndef __GAMEACTION_H__
#define __GAMEACTION_H__

#include "common.h"

class GameAction
{
public:

    GameAction();
    ~GameAction();

    static void update();

    void add();
    void remove();

protected:

    virtual void execute() = 0;

    TimePoint executionTime;

private:

    static std::list<GameAction> waitingActions;
};

#endif // __GAMEACTION_H__
