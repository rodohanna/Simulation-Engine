#ifndef PLAYER_h_
#define PLAYER_h_

#include "EventBus.h"
#include "GameTypes.h"

enum PlayerActions
{
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    TOTAL_PLAYER_ACTIONS
};

struct Player : IInputEventSubscriber
{
    Player(EventBus *, const V2 &, const Color &);
    ~Player();
    void update(double ts);
    void handle_input_events(const InputEvent *, size_t);
    EventBus *event_bus;
    Color color;
    bool actions[TOTAL_PLAYER_ACTIONS];
    size_t texture_index;
    V2 dimensions;
    V2 position;
};

#endif