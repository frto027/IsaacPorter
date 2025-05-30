# IsaacSaveData reverter

This program revert your game's `rep+persistentgamedata1.dat` to a `rep_persistentgamedata1.dat`.

All it does is remove the achievement and counters data that online coop added. 

Don't use it, because it is not tested.

Help wanted.

Feedback wanted.

# Project

You can build the project with cmake. We have some cmake target here.

- `imgui` TBD, you need vcpkg to build this target. you can use the CMakePresets.json, and set VCPKG_ROOT properly.
- `cli` a commandline tools.
- `test` just some hard coded test for debug.