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

void RequestDropFile(std::function< void (std::string& file)> Callback,
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
		ImGui::Text(u8"  将文件拖拽至此以接收");
		ImGui::SameLine();
		if (ImGui::Button(u8"取消")) {
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
}workspace;

void LogUI(std::string msg) {
	workspace.error_messages += msg + "\n";
}

void eval(std::function<void(void)> f, std::optional<std::function<void(void)> > err = {}) {
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
void RenderSaveInfo(IsaacSave::SaveData &saveData, bool editable, SaveInfoUI& ui) {
	auto achi = saveData.GetComponent<IsaacSave::AchievementBlock>();
	if (achi) {
		ImGui::Text(u8"全成就数量：%d", achi->achis.size());

		if (editable) {
			ImGui::SameLine();
			if (ImGui::Button(u8"编辑##cachi")) {
				ui.achiEdit = true;
			}
			if (ui.achiEdit) {
				if (ImGui::Begin(u8"成就编辑",NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
					if (ImGui::Button(u8"关闭##achi")) {
						ui.achiEdit = false;
					}

					int single_width = 100;
					int achi_per_line = (ImGui::GetWindowWidth() - 20) / single_width;
					if (achi_per_line == 0)
						achi_per_line = 1;
					for (int i = 0; i < achi->achis.size(); i++) {
						int line_index = i % achi_per_line;
						if (line_index != 0)
							ImGui::SameLine(line_index * single_width);
						char buff[128];
						bool v = achi->achis[i];
						sprintf_s(buff, u8"成就%d", i);
						ImGui::Checkbox(buff, &v);
						achi->achis[i] = v;
					}
				}
				ImGui::End();
			}
		}
	}
	else {
		ImGui::Text(u8"异常：存档内没有成就信息");
	}

	auto counter = saveData.GetComponent<IsaacSave::CounterBlock>();
	if (counter) {
		ImGui::Text(u8"计数器数量：%d", counter->counters.size());
		if (editable) {
			ImGui::SameLine();
			if (ImGui::Button(u8"编辑##counter")) {
				ui.counterEdit = true;
			}
			if (ui.counterEdit) {
				if (ImGui::Begin(u8"计数器编辑", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
					if (ImGui::Button(u8"关闭##counter")) {
						ui.counterEdit = false;
					}
					ImGui::SameLine();
					ImGui::Text(u8"范围：%d 至 %d", INT_MIN, INT_MAX);

					for (int i = 0; i < counter->counters.size(); i++) {
						char buff[128];
						sprintf_s(buff, u8"计数器%d", i);
						ImGui::SetNextItemWidth(300);
						ImGui::InputInt(buff, (int*)&(counter->counters[i]));
					}
				}
				ImGui::End();
			}
			
		}
	}
	else {
		ImGui::Text(u8"异常：存档内没有计数器信息");
	}

	auto collectible = saveData.GetComponent<IsaacSave::CollectibleBlock>();
	if (collectible) {
		ImGui::Text(u8"道具目击档案数量：%d", collectible->counters.size());
		if (editable) {
			ImGui::SameLine();
			if (ImGui::Button(u8"编辑##collectible")) {
				ui.collectibleEdit = true;
			}

			if (ui.collectibleEdit) {
				if (ImGui::Begin(u8"道具数量编辑", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
					if (ImGui::Button(u8"关闭##collectible")) {
						ui.collectibleEdit = false;
					}
					ImGui::SameLine();
					for (int i = 0; i < collectible->counters.size(); i++) {
						char buff[128];
						sprintf_s(buff, u8"道具%d", i);
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
		ImGui::Text(u8"异常：存档内没有道具信息");
	}

}

void MergeSaves() {
	if (!workspace.saveData.has_value() || !workspace.saveData2.has_value()) {
		LogUI(u8"需要加载两个存档才能进行合并");
		return;
	}
	auto achi = workspace.saveData.value().GetComponent<IsaacSave::AchievementBlock>();
	auto counter = workspace.saveData.value().GetComponent<IsaacSave::CounterBlock>();
	auto sachi = workspace.saveData2.value().GetComponent<IsaacSave::AchievementBlock>();
	auto scounter = workspace.saveData2.value().GetComponent<IsaacSave::CounterBlock>();
	int acount = 0;
	int ccount = 0;
	if (achi->block_size != achi->extra_number || achi->block_size != achi->achis.size()) {
		LogUI(u8"存档achievement数据不满足经验预期，无法继续合并");
		return;
	}
	if (counter->block_size != 4 * counter->extra_number || counter->extra_number != counter->counters.size()) {
		LogUI(u8"存档counter数据不满足经验预期，无法继续合并");
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
	sprintf_s(buff, u8"合并完成，共合并%d个成就和%d个计数器。", acount, ccount);
	LogUI(buff);
}

void RenderMainContent() {
	if (RenderDragDropThings())
		return;
	/*
	if (ImGui::Button(u8"中文")) {
		LANG.toZh();
	}
	ImGui::SameLine();
	// emmm, I'm lazy. no english
	if (ImGui::Button("English")) {
		LANG.toEng();
	}
	ImGui::Spacing();
	*/

	if (ImGui::CollapsingHeader(u8"输入文件", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushID("M");
		if (ImGui::Button(u8"选择文件")) {
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
			ImGui::Text(u8"文件名：%s", workspace.currentFilePath.c_str());

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Dummy({ 0,10 });
		ImGui::PopID();
	}


	if (workspace.saveData.has_value() && ImGui::CollapsingHeader(u8"存档信息", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushID("M");
		if (workspace.saveData.has_value()) {
			RenderSaveInfo(workspace.saveData.value(), workspace.editMode, MainUI);
			ImGui::Spacing();
			if (ImGui::Button(u8"裁剪至忏悔")) {
				eval([&]() {
					workspace.saveData.value().cut(IsaacSave::VER_REP);
					});
			}
		}
		else {
			ImGui::Text(u8"暂未加载存档");
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Dummy({ 0,10 });
		ImGui::PopID();
	}


	if (workspace.mergeMode && workspace.saveData.has_value() && ImGui::CollapsingHeader(u8"副存档输入（合并模式）", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushID("S");
		if (ImGui::Button(u8"选择文件")) {
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
			ImGui::Text(u8"文件名：%s", workspace.currentFilePath2.c_str());

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Dummy({ 0,10 });
		ImGui::PopID();
	}

	if (workspace.mergeMode && workspace.saveData2.has_value() && ImGui::CollapsingHeader(u8"副存档信息（合并模式）", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushID("S");
		RenderSaveInfo(workspace.saveData2.value(), false, SideUI);
		if (workspace.editMode) {
			ImGui::Text(u8"提示：编辑模式只能编辑主存档");
		}
		if (ImGui::Button(u8"合并忏悔+部分至主存档")) {
			MergeSaves();
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Dummy({ 0,10 });
		ImGui::PopID();
	}

	if (workspace.saveData.has_value() && ImGui::CollapsingHeader(u8"输出文件", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::InputText(u8"输出文件名", workspace.savePath, 4095);
		if (ImGui::Button(u8"保存")) {
			workspace.saveData.value().WriteTo(workspace.savePath);
			IsaacSave::FixChecksumForFile(workspace.savePath);
			LogUI(u8"已保存");
		}
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Dummy({ 0,10 });
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader(u8"其它设置")) {
		ImGui::Checkbox(u8"合并模式", &workspace.mergeMode);
		ImGui::Text(u8"      启用合并模式后，可以另外选择一个忏悔+的存档，");
		ImGui::Text(u8"      并将其中的忏悔+部分合并至当前存档中。");
		ImGui::Checkbox(u8"编辑模式", &workspace.editMode);
		ImGui::Text(u8"      编辑模式显示编辑按钮，可编辑存档内部数据。");

		ImGui::Checkbox(u8"打印详细日志", &IsaacSave::verbose);

	}

	if (workspace.error_messages.size() > 0 && ImGui::CollapsingHeader(u8"日志输出", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text(u8"%s", workspace.error_messages.c_str());
		if (ImGui::Button(u8"清空")) {
			workspace.error_messages = "";
		}
		ImGui::Spacing();
	}

}




void RenderUI() {
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
	if (ImGui::Begin("full", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
		ImGui::SetNextWindowPos(ImVec2(10, 10));
		auto subsize = ImGui::GetMainViewport()->Size;
		subsize.x -= 20;
		subsize.y -= 20;
		ImGui::SetNextWindowSize(subsize);

		RenderMainContent();
		ImGui::End();
	}
}