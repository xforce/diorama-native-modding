#pragma once

#include "event.h"
#include "vector.h"
#include "dio_std_types.h"

namespace dio
{
    struct ToServerPlayerStateEvent : public Event
    {
        virtual ~ToServerPlayerStateEvent() {};
        ToServerPlayerStateEvent();

        math::Vector3 chunk_id_;
        math::Vector3f xyz_;
        math::Vector3f ypr_;
        bool was_left_mouse_clicked_;
        bool was_right_mouse_clicked_;
        int block_id_;
        bool is_crouching_;
        string sign_text_;
    };
}