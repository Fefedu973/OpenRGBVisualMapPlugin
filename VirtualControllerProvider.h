#ifndef VIRTUALCONTROLLERPROVIDER_H
#define VIRTUALCONTROLLERPROVIDER_H

#include "VirtualController.h"

class VirtualControllerProvider
{
public:
    static VirtualControllerProvider* Get();

    void RegisterController();
    void UnregisterController();

    void UpdateSize(int,int);

    VirtualController* GetController();


private:
    VirtualControllerProvider();
    ~VirtualControllerProvider();

    static VirtualControllerProvider* instance;

    VirtualController* virtual_controller;
};

#endif // VIRTUALCONTROLLERPROVIDER_H
