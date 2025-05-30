# IsaacPorter

Supported game version:
- 1.7.9b the latest repentence
- 1.9.7.11 or maybe newer online beta test

Feature:

- Downgrade the rep+ savedata to rep
- Merge a repentence savedata with a repentence plus save data.
- Lock/Unlock any achievement.
- Edit any counters inside the save data.
- Edit the collectible info inside the save data.

# Project

You can build the project with cmake. We have some cmake target here.

- `imgui` you need vcpkg to build this target. you can use the CMakePresets.json, and set VCPKG_ROOT properly.
- `cli` a commandline tools.
- `test` just some hard coded test for debug.
