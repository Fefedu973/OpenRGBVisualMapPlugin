#ifndef GRIDSETTINGS_H
#define GRIDSETTINGS_H

struct GridSettings
{
    int w;
    int h;
    bool show_grid;
    bool show_bounds;
    int grid_size;

    static inline GridSettings defaults() {
        return {128, 128, false, false, 8};
    }
};

#endif // GRIDSETTINGS_H
