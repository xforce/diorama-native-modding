#include "event.h"

#include <dio_std_types.h>

namespace dio
{
    struct ToServerChatEvent : public Event
    {
        string text_;

        virtual ~ToServerChatEvent() {};
        ToServerChatEvent();
    };
}