// this file is saved as UTF8 with BOM
#include "gui.h"
#include <imgui.h>
#include <string>
#include <functional>
#include <optional>
#include "../IsaacSave.h"
#include "lang.h"

struct DragDropFile {
    bool file_wanted = false;
    std::string file;
    bool file_accepted = false;
    std::function<void(std::string& file)> Callback;
    std::function<void(void)> Cancel;
}dragDropFile;

void RenderUIDrop(int path_count, const char* paths[]) {
    if (path_count > 0) {
        dragDropFile.file = paths[0];
        dragDropFile.file_accepted = true;
    }
}

void RequestDropFile(std::function<void(std::string& file)> Callback,
    std::function<void(void)> Cancel) {
    dragDropFile.Callback = Callback;
    dragDropFile.Cancel = Cancel;
    dragDropFile.file_accepted = false;
    dragDropFile.file_wanted = true;
}

bool RenderDragDropThings() {
    if (dragDropFile.file_wanted) {
        if (dragDropFile.file_accepted) {
            dragDropFile.file_wanted = false;
            dragDropFile.Callback(dragDropFile.file);
            return false;
        }
        ImGui::Text("\n\n");
        ImGui::Text(LANG.DRAG_FILE_HERE);
        ImGui::SameLine();
        if (ImGui::Button(LANG.CANCEL)) {
            dragDropFile.file_wanted = false;
            dragDropFile.Cancel();
        }
        return true;
    }
    return false;
}

struct {
    std::string currentFilePath;
    std::string currentFilePath2;
    char savePath[4096] = "";

    std::optional<IsaacSave::SaveData> saveData;
    std::optional<IsaacSave::SaveData> saveData2;
    std::string error_messages;

    bool editMode = false;
    bool mergeMode = false;
} workspace;

void LogUI(std::string msg) {
    workspace.error_messages += msg + "\n";
}

void eval(std::function<void(void)> f, std::optional<std::function<void(void)>> err = {}) {
    try {
        f();
    }
    catch (std::exception e) {
        if (err.has_value())
            err.value()();
        workspace.error_messages += e.what();
        workspace.error_messages += "\n";
    }
}
struct SaveInfoUI {
    bool achiEdit = false;
    bool counterEdit = false;
    bool collectibleEdit = false;
} MainUI, SideUI;

void RenderSaveInfo(IsaacSave::SaveData& saveData, bool editable, SaveInfoUI& ui) {
    auto achi = saveData.GetComponent<IsaacSave::AchievementBlock>();
    if (achi) {
        ImGui::Text(LANG.ACHI_SLOT_COUNT, achi->achis.size());

        if (editable) {
            ImGui::SameLine();
            if (ImGui::Button(LANG.EDIT_ACHI)) {
                ui.achiEdit = true;
            }
            if (ui.achiEdit) {
                if (ImGui::Begin(LANG.EDIT_ACHI_TITLE, NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
                    if (ImGui::Button(LANG.CLOSE_ACHI)) {
                        ui.achiEdit = false;
                    }
                    ImGui::Separator();

                    int single_width = 100;
                    int achi_per_line = ((int)ImGui::GetWindowWidth() - 20) / single_width;
                    if (achi_per_line == 0)
                        achi_per_line = 1;
                    for (int i = 0; i < achi->achis.size(); i++) {
                        int line_index = i % achi_per_line;
                        if (line_index != 0)
                            ImGui::SameLine((float)line_index * single_width);
                        char buff[128];
                        bool v = achi->achis[i];
                        sprintf_s(buff, LANG.ACHI_ITEM, i);
                        ImGui::Checkbox(buff, &v);
                        achi->achis[i] = v;
                    }
                }
                ImGui::End();
            }
        }
    }
    else {
        ImGui::Text(LANG.NO_ACHI);
    }

    auto counter = saveData.GetComponent<IsaacSave::CounterBlock>();
    if (counter) {
        ImGui::Text(LANG.COUNTER_SLOT_COUNT, counter->counters.size());
        if (editable) {
            ImGui::SameLine();
            if (ImGui::Button(LANG.EDIT_COUNTER)) {
                ui.counterEdit = true;
            }
            if (ui.counterEdit) {
                if (ImGui::Begin(LANG.EDIT_COUNTER_TITLE, NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
                    if (ImGui::Button(LANG.CLOSE_COUNTER)) {
                        ui.counterEdit = false;
                    }
                    ImGui::SameLine();
                    ImGui::Text(LANG.COUNTER_RANGE, INT_MIN, INT_MAX);
                    ImGui::Separator();

                    for (int i = 0; i < counter->counters.size(); i++) {
                        char buff[128];
                        sprintf_s(buff, LANG.COUNTER_ITEM, i);
                        ImGui::SetNextItemWidth(300);
                        ImGui::InputInt(buff, (int*)&(counter->counters[i]));
                    }
                }
                ImGui::End();
            }
        }
    }
    else {
        ImGui::Text(LANG.NO_COUNTER);
    }

    auto collectible = saveData.GetComponent<IsaacSave::CollectibleBlock>();
    if (collectible) {
        ImGui::Text(LANG.COLLECTIBLE_SLOT_COUNT, collectible->counters.size());
        if (editable) {
            ImGui::SameLine();
            if (ImGui::Button(LANG.EDIT_COLLECTIBLE)) {
                ui.collectibleEdit = true;
            }

            if (ui.collectibleEdit) {
                if (ImGui::Begin(LANG.EDIT_COLLECTIBLE_TITLE, NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
                    if (ImGui::Button(LANG.CLOSE_COLLECTIBLE)) {
                        ui.collectibleEdit = false;
                    }
                    ImGui::Separator();
                    for (int i = 0; i < collectible->counters.size(); i++) {
                        char buff[128];
                        sprintf_s(buff, LANG.COLLECTIBLE_ITEM, i);
                        ImGui::SetNextItemWidth(100);
                        bool v = collectible->counters[i];
                        ImGui::Checkbox(buff, &v);
                        collectible->counters[i] = (char)v;
                    }
                }
                ImGui::End();
            }
        }
    }
    else {
        ImGui::Text(LANG.NO_COLLECTIBLE);
    }
}

void MergeSaves() {
    if (!workspace.saveData.has_value() || !workspace.saveData2.has_value()) {
        LogUI(LANG.NEED_TWO_SAVES);
        return;
    }
    auto achi = workspace.saveData.value().GetComponent<IsaacSave::AchievementBlock>();
    auto counter = workspace.saveData.value().GetComponent<IsaacSave::CounterBlock>();
    auto sachi = workspace.saveData2.value().GetComponent<IsaacSave::AchievementBlock>();
    auto scounter = workspace.saveData2.value().GetComponent<IsaacSave::CounterBlock>();
    int acount = 0;
    int ccount = 0;
    if (achi->block_size != achi->extra_number || achi->block_size != achi->achis.size()) {
        LogUI(LANG.ACHI_MERGE_FAIL);
        return;
    }
    if (counter->block_size != 4 * counter->extra_number || counter->extra_number != counter->counters.size()) {
        LogUI(LANG.COUNTER_MERGE_FAIL);
        return;
    }
    while (achi->achis.size() < sachi->achis.size()) {
        acount++;
        achi->block_size++;
        achi->extra_number++;
        achi->achis.push_back(sachi->achis[achi->achis.size()]);
    }
    while (counter->counters.size() < scounter->counters.size()) {
        ccount++;
        counter->block_size += 4;
        counter->extra_number += 1;
        counter->counters.push_back(scounter->counters[counter->counters.size()]);
    }
    char buff[1024];
    sprintf_s(buff, LANG.MERGE_DONE, acount, ccount);
    LogUI(buff);
}

void RenderMainContent() {
    if (RenderDragDropThings())
        return;

    if (ImGui::CollapsingHeader(LANG.INPUT_FILE, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushID("M");
        if (ImGui::Button(LANG.SELECT_FILE)) {
            RequestDropFile([](std::string& file) {
                workspace.currentFilePath = file;
                std::string save_path = file + ".patched.dat";
                strcpy_s(workspace.savePath, save_path.c_str());

                eval([]() {
                    workspace.saveData.reset();
                    workspace.saveData.emplace(workspace.currentFilePath);
                });
            }, []() {
                workspace.saveData.reset();
                workspace.saveData2.reset();
                workspace.currentFilePath = "";
                workspace.savePath[0] = 0;
            });
        }
        ImGui::SameLine();
        if (workspace.currentFilePath[0])
            ImGui::Text(LANG.FILE_NAME, workspace.currentFilePath.c_str());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Dummy({ 0,10 });
        ImGui::PopID();
    }

    if (workspace.saveData.has_value() && ImGui::CollapsingHeader(LANG.SAVE_INFO, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushID("M");
        if (workspace.saveData.has_value()) {
            RenderSaveInfo(workspace.saveData.value(), workspace.editMode, MainUI);
            ImGui::Spacing();
            if (ImGui::Button(LANG.CUT_TO_REP)) {
                eval([&]() {
                    workspace.saveData.value().cut(IsaacSave::VER_REP);
                });
            }
        }
        else {
            ImGui::Text(LANG.NO_SAVE);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Dummy({ 0,10 });
        ImGui::PopID();
    }

    if (workspace.mergeMode && workspace.saveData.has_value() && ImGui::CollapsingHeader(LANG.MERGE_INPUT, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushID("S");
        if (ImGui::Button(LANG.SELECT_FILE)) {
            RequestDropFile([](std::string& file) {
                eval([&]() {
                    workspace.currentFilePath2 = file;
                    workspace.saveData2.reset();
                    workspace.saveData2.emplace(workspace.currentFilePath2);
                });
            }, []() {
                workspace.saveData2.reset();
                workspace.currentFilePath2 = "";
            });
        }
        ImGui::SameLine();
        if (workspace.currentFilePath2[0])
            ImGui::Text(LANG.FILE_NAME, workspace.currentFilePath2.c_str());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Dummy({ 0,10 });
        ImGui::PopID();
    }

    if (workspace.mergeMode && workspace.saveData2.has_value() && ImGui::CollapsingHeader(LANG.MERGE_INFO, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushID("S");
        RenderSaveInfo(workspace.saveData2.value(), false, SideUI);
        if (workspace.editMode) {
            ImGui::Text(LANG.EDIT_HINT);
        }
        if (ImGui::Button(LANG.MERGE_TO_MAIN)) {
            MergeSaves();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Dummy({ 0,10 });
        ImGui::PopID();
    }

    if (workspace.saveData.has_value() && ImGui::CollapsingHeader(LANG.OUTPUT_FILE, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::InputText(LANG.OUTPUT_FILE_NAME, workspace.savePath, 4095);
        if (ImGui::Button(LANG.SAVE)) {
            workspace.saveData.value().WriteTo(workspace.savePath);
            IsaacSave::FixChecksumForFile(workspace.savePath);
            LogUI(LANG.SAVED);
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Dummy({ 0,10 });
        ImGui::Spacing();
    }

    if (ImGui::CollapsingHeader(LANG.OTHER_SETTINGS)) {
        ImGui::Checkbox(LANG.MERGE_MODE, &workspace.mergeMode);
        ImGui::Text(LANG.MERGE_MODE_DESC1);
        ImGui::Text(LANG.MERGE_MODE_DESC2);
        ImGui::Checkbox(LANG.EDIT_MODE, &workspace.editMode);
        ImGui::Text(LANG.EDIT_MODE_DESC);
        ImGui::Checkbox(LANG.VERBOSE_LOG, &IsaacSave::verbose);
    }

    if (workspace.error_messages.size() > 0 && ImGui::CollapsingHeader(LANG.LOG_OUTPUT, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text(LANG.ERROR_MESSAGES, workspace.error_messages.c_str());
        if (ImGui::Button(LANG.CLEAR)) {
            workspace.error_messages = "";
        }
        ImGui::Spacing();
    }
}

void RenderUI() {
    static bool warning_display = true;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
    if (ImGui::Begin("full", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
        ImGui::SetNextWindowPos(ImVec2(10, 10));
        auto subsize = ImGui::GetMainViewport()->Size;
        subsize.x -= 20;
        subsize.y -= 20;
        ImGui::SetNextWindowSize(subsize);

        if (warning_display) {
            if (ImGui::Button(u8"中文")) {
                LANG.toZh();
            }
            ImGui::SameLine();
            if (ImGui::Button("English")) {
                LANG.toEng();
            }
            ImGui::Spacing();

            ImGui::Separator();

            ImGui::Text(LANG.WARNING);
            ImGui::Spacing();
            ImGui::Text(LANG.ABOUT_SOFT);
            ImGui::Spacing();
            ImGui::Text(LANG.AUTHOR);
            if (ImGui::Button(LANG.CONTINUE)) {
                warning_display = false;
            }
        }
        else {
            RenderMainContent();
        }

        ImGui::End();
    }
}
