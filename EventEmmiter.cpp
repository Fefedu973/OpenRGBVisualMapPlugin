#include "EventEmmiter.h"

EventEmmiter* EventEmmiter::instance;

EventEmmiter::EventEmmiter(QObject *parent)
    : QObject(parent)
{

}

EventEmmiter* EventEmmiter::Get()
{
    if(!instance)
    {
        instance = new EventEmmiter();
    }

    return instance;
}

void EventEmmiter::ApplyImage(QImage* image)
{
    emit ImageApplied(image);
}
