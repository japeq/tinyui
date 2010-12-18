#include "tiny_ui.h"

void Button::set_handler(ClickInterface *events)
{
    m_handler = events;
}

void IoWatch::set_handler(IoWatchInterface *events)
{
    m_handler = events;
}

void Timer::set_handler(TimerInterface *events)
{
    m_handler = events;
}

Application *Application::instance()
{
    return m_instance;
}

Application *Application::m_instance;

