#include "Order.h"
#include "Input.h"
#include "MessageBus.h"

Order::Manager::Manager() : state(Order::State::IDLE){};
void Order::Manager::update(ECS::Map *map, double ts)
{
    switch (this->state)
    {
    case Order::State::PLACING_ZONE:
    {
        if (!Input::is_input_active(Input::LEFT_MOUSE_PRESSED))
        {
            zone_manager.quit_and_save_zone_placement(map);
            this->state = Order::State::IDLE;
        }
        break;
    }
    case Order::State::PLACING_STRUCTURE:
    {
        if (Input::is_input_active(Input::LEFT_MOUSE_JUST_PRESSED))
        {
            this->build_manager.quit_and_save_structure_placement(map);
            this->state = Order::State::IDLE;
        }
        break;
    }
    default:
    {
        // no op
    }
    }
    this->zone_manager.update(map, ts);
    this->build_manager.update(map, ts);
};

void Order::Manager::process_messages(ECS::Map *map)
{
    MBus::MessageQueue mq = MBus::get_queue(MBus::QueueType::ORDER);
    for (int i = 0; i < mq.length; ++i)
    {
        MBus::Message m = mq.queue[i];
        switch (m.type)
        {
        case MBus::Type::BEGIN_ZONE_PLACEMENT:
        {
            this->state = Order::State::PLACING_ZONE;
            this->zone_manager.begin_zone_placement(map);
            break;
        }
        case MBus::Type::BEGIN_STRUCTURE_PLACEMENT:
        {
            printf("Handling structure placement\n");
            this->state = Order::State::PLACING_STRUCTURE;
            this->build_manager.begin_structure_placement(&m.data.bsp.dimensions);
            break;
        }
        default:
        {
            this->state = Order::State::IDLE;
        }
        }
    }
}