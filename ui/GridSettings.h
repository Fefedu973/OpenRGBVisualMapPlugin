#ifndef GRIDSETTINGS_H
#define GRIDSETTINGS_H

struct GridSettings
{
    int w;
    int h;
    bool show_grid;
    bool show_bounds;
    int grid_size;
    bool snap_to_grid;
    bool auto_load = false;
    bool auto_register = false;
    bool hide_members = false;
};

#endif // GRIDSETTINGS_H
