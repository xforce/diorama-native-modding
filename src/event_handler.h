// Copyright 2015 the kNet authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
/*
TODO: improve
*/

#include <cstddef>
#include <functional>
#include <type_traits>
#include <mutex>
#include <algorithm>
#include <vector>

#include "function_traits.h"


namespace public_api
{
    class Event
    {
    private:
        void * owner = nullptr;
    public:

        void SetOwner(void * _owner)
        {
            this->owner = _owner;
        }

        void * GetOwner()
        {
            return owner;
        }
    };

    template<typename... Args>
    class EventN : public Event
    {
        typedef std::tuple<Args...> argTuple;
    private:
        std::function<bool(Args... arg)> callback;
    public:

        EventN(decltype(callback) callback)
            : callback(callback)
        {

        }


        bool Invoke(Args... args)
        {
            if (callback)
                return callback(args...);

            return false;
        }

        static EventN * cast(Event * event)
        {
            return static_cast<EventN*>(event);
        }
    };

    template<typename T>
    struct remove_first_type_event
    { };

    template<typename T, typename... Ts>
    struct remove_first_type_event < std::tuple<T, Ts...> >
    {
        using eventT = EventN < Ts... >;
    };

    // TEMPLATE FUNCTION bind (implicit return type)
    template<class _Fun>
    Event *
        mkEventN(_Fun && _Fx)
    {	// bind a function object

        typedef typename std::remove_reference<decltype(::std::forward<_Fun>(_Fx))>::type ltype_const;

        using Traits = function_traits<ltype_const>;
        return new typename remove_first_type_event<typename Traits::argT::args>::eventT(_Fx);
    }

    template<class _Rx,
        class... _Ftypes,
        class... _Types>
        Event *
        mkEventN(_Rx(*_Pfx)(_Ftypes...), _Types&&... _Args)
    {	// bind a function pointer

        auto bn = bind_helper::bind<_Rx, _Ftypes...>(_Pfx);
        return new EventN<_Ftypes...>(bn);
    }

    template<class _Rx,
        typename _Farg0,
        typename _Types>
        Event *
        mkEventN(_Rx _Farg0::* const _Pmd, _Types&& _Args)
    {	// bind a wrapped member object pointer
        using Traits = function_traits < _Rx _Farg0::* >;
        auto bn = this_bind_helper::bind<Traits::arity, _Rx, _Farg0, _Types>(_Pmd, _Args);
        return new typename remove_first_type_event<typename Traits::argT::args>::eventT(bn);
    }

    template<typename eventIds>
    class EventHandler
    {
    private:
        std::vector<std::pair<eventIds, std::vector<Event*>>> _events;
        std::recursive_mutex _vecMutex;

        void AddEventInternal(eventIds id, Event* pEvent, void * owner = nullptr)
        {
            if (!pEvent)
                return;

            pEvent->SetOwner(owner);

            std::lock_guard<std::recursive_mutex> lk{ _vecMutex };
            decltype(auto) events = GetEventById(id);
            if (events.empty())
            {
                _events.push_back({ id, std::vector < Event* > {pEvent} });
            }
            else
            {
                events.push_back(pEvent);
            }
        }

    public:
        enum CallResult : char
        {
            NO_EVENT = 0,
            ALL_FALSE,
            ALL_TRUE,
            BOTH,
            MAX_RESULT,
        };

        void RemoveEventsByOwner(void * owner)
        {
            std::lock_guard<std::recursive_mutex> lk{ _vecMutex };
            for (auto it = _events.begin(); it != _events.end();)
            {
                auto& eventCon = *it;
                eventCon.second.erase(std::remove_if(eventCon.second.begin(), eventCon.second.end(), [owner](Event * pEvent) -> bool
                {
                    if (pEvent->GetOwner() == owner)
                    {
                        delete pEvent;
                        return true;
                    }
                    else
                        return false;
                }), eventCon.second.end());

                if (eventCon.second.empty())
                    it = _events.erase(it);
                else
                    ++it;
            }
        }

        template<typename ...Args>
        void AddEvent(eventIds id, void* owner, Args&&... args)
        {
            AddEventInternal(id, mkEventN(std::forward<Args>(args)...), owner);
        }

        std::vector<Event *>& GetEventById(eventIds id)
        {
            std::lock_guard<std::recursive_mutex> lk{ _vecMutex };
            for (auto& entry : _events)
            {
                if (entry.first == id)
                    return entry.second;
            }

            static std::vector<Event*> emptyVec = std::vector<Event*>();
            return emptyVec;
        };

        template<typename... Args>
        CallResult Call(eventIds id, Args... args)
        {
            std::lock_guard<std::recursive_mutex> lk{ _vecMutex };
            decltype(auto) events = GetEventById(id);
            if (!events.empty())
            {
                CallResult ret = CallResult::MAX_RESULT;

                for (auto &event : events)
                {
                    if (EventN<Args...>::cast(event)->Invoke(args...))
                    {
                        // If nothing was set set it to ALL_TRUE;
                        // If all previous was FALSE set it to BOTH

                        if (ret == CallResult::MAX_RESULT)
                            ret = CallResult::ALL_TRUE;
                        else if (ret == CallResult::ALL_FALSE)
                            ret = CallResult::BOTH;
                    }
                    else
                    {
                        // If all were TRUE or BOTH was set, set it to BOTH
                        // If nothing was set, then set it to ALL_FALSE
                        if (ret == CallResult::ALL_TRUE || ret == CallResult::BOTH)
                            ret = CallResult::BOTH;
                        else
                            ret = CallResult::ALL_FALSE;
                    }
                }
                return ret;
            }
            return NO_EVENT;
        }

        explicit operator bool() const noexcept
        {
            return !(_events.size() == 0);
        };

        bool operator ()(eventIds id) noexcept
        {
            std::lock_guard<std::recursive_mutex> lk{ _vecMutex };
            decltype(auto) events = GetEventById(id);
            return !(events.size() == 0);
        };
    };
};