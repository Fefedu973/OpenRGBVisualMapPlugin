# Persistence regression tests

Run `Build-Tests.cmd` with `QT_ROOT` pointing to the same Qt MSVC kit used for the
plugin. Build the plugin first with the repository's `Build-Room.cmd`.

This harness loads the actual DLL with `QPluginLoader` and drives its real editor
widgets offscreen. It uses a temporary configuration directory and a synthetic
two-LED device. No registry, hardware, capture API or user settings are accessed.
The sibling OpenRGB core provides its existing fake plugin API and real virtual
controller wrapper.

The tests verify decimal positions and autosave; preservation across a device-list
rebuild before the debounce fires; unplugged members and imported metadata;
canonical map references instead of geometry snapshots; exclusive image and LED
routing; legacy profile completion; and restoring the last active map when the
DLL is loaded again despite a different file's legacy auto-register flag.

The existing `tests/room-image-routing` suite remains the geometry, worker and
image-interface regression suite. See [ROOM-PERSISTENCE.md](../../ROOM-PERSISTENCE.md)
for the user-facing persistence and profile contract.
