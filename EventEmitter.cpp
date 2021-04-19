#include "EventEmitter.h"

EventEmitter* EventEmitter::instance;

EventEmitter* EventEmitter::Get()
{
    if(!instance)
    {
        instance = new EventEmitter();
    }

    return instance;
}

void EventEmitter::ApplyImage(QImage* image)
{
    emit ImageApplied(image);
}
