#include "VirtualControllerProvider.h"
#include "OpenRGBVisualMapPlugin.h"

VirtualControllerProvider* VirtualControllerProvider::instance;

VirtualControllerProvider::VirtualControllerProvider() :
    virtual_controller(new VirtualController()) {}

VirtualControllerProvider::~VirtualControllerProvider()
{
    delete virtual_controller;
}

VirtualControllerProvider* VirtualControllerProvider::Get()
{
    if(!instance)
    {
        instance = new VirtualControllerProvider();
    }

    return instance;
}

void VirtualControllerProvider::RegisterController()
{
    OpenRGBVisualMapPlugin::RMPointer->RegisterRGBController(virtual_controller);
}

void VirtualControllerProvider::UpdateSize(int w, int h)
{
    virtual_controller->UpdateSize(w, h);
}

VirtualController* VirtualControllerProvider::GetController()
{
    return virtual_controller;
}
