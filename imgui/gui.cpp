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
	char savePath[4096] = "";

	std::optional<IsaacSave::SaveData> saveData;
	std::optional<IsaacSave::SaveData> saveData2;
	std::string error_messages;

	bool editMode = false;
}workspace;
void eval(std::function<void(void)> f, std::optional<std::function<void(void)> > err = nullptr) {
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

void RenderSaveInfo(IsaacSave::SaveData &saveData, bool editable) {
	auto achi = saveData.GetComponent<IsaacSave::AchievementBlock>();
	if (achi) {
		ImGui::Text(u8"全成就数量：%d", achi->achis.size());

		if (editable && ImGui::CollapsingHeader(u8"成就编辑")) {
			for (int i = 0;i < achi->achis.size(); i++) {
				if (i > 0 && i % 20 != 0)
					ImGui::SameLine();
				char buff[128];
				bool v = achi->achis[i];
				sprintf_s(buff, "c%d", i);
				ImGui::Checkbox(buff, &v);
				achi->achis[i] = v;
			}
		}
	}
	else {
		ImGui::Text(u8"异常：存档内没有成就信息");
	}

	auto counter = saveData.GetComponent<IsaacSave::CounterBlock>();
	if (counter) {
		ImGui::Text(u8"计数器数量：%d", counter->counters.size());
		if (editable && ImGui::CollapsingHeader(u8"计数器编辑")) {
			for (int i = 0;i < counter->counters.size(); i++) {
				if (i > 0 && i % 20 != 0)
					ImGui::SameLine();
				char buff[128];
				ImGui::InputInt(buff, (int*)&(counter->counters[i]));
			}
		}
	}
	else {
		ImGui::Text(u8"异常：存档内没有计数器信息");
	}
}

void RenderMainContent() {
	if (RenderDragDropThings())
		return;

	if (ImGui::Button(u8"中文")) {
		LANG.toZh();
	}
	ImGui::SameLine();
	if (ImGui::Button("English")) {
		LANG.toEng();
	}

	ImGui::Spacing();

	if (workspace.error_messages.size() > 0 && ImGui::CollapsingHeader(u8"报错", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text(u8"%s", workspace.error_messages.c_str());
		if (ImGui::Button(u8"清空")) {
			workspace.error_messages = "";
		}
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader(u8"输入文件", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text(u8"文件名：%s", workspace.currentFilePath.c_str());
		ImGui::SameLine();
		if (ImGui::Button(u8"选择文件")) {
			RequestDropFile([](std::string& file) {
				workspace.currentFilePath = file;
				std::string save_path = file + ".patched.dat";
				strcpy_s(workspace.savePath, save_path.c_str());

				eval([]() {
					workspace.saveData.reset();

					workspace.saveData.emplace(workspace.savePath);
					});

				}, []() {
					});
		}
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader(u8"副存档输入", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text(u8"文件名：%s", workspace.currentFilePath.c_str());
		ImGui::SameLine();
		if (ImGui::Button(u8"选择文件")) {
			RequestDropFile([](std::string& file) {
				eval([]() {
					workspace.saveData2.reset();

					workspace.saveData2.emplace(workspace.savePath);
					});
				}, []() {
					});
		}
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader(u8"存档信息", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushID("M");
		if (workspace.saveData.has_value()) {
			RenderSaveInfo(workspace.saveData.value(), workspace.editMode);
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
		ImGui::PopID();
	}

	if (workspace.saveData2.has_value() && ImGui::CollapsingHeader(u8"副存档信息", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushID("S");
		RenderSaveInfo(workspace.saveData2.value(), false);
		if (workspace.editMode) {
			ImGui::Text(u8"提示：编辑模式只能编辑主存档");
		}
		if (ImGui::Button(u8"合并忏悔+部分至存档")) {
			//TODO
		}
		ImGui::PopID();
	}

	if (workspace.saveData.has_value() && ImGui::CollapsingHeader(u8"输出文件", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::InputText(u8"输出文件名", workspace.savePath, 4095);
		ImGui::SameLine();
		if (ImGui::Button(u8"保存")) {
			workspace.saveData.value().WriteTo(workspace.savePath);
		}

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader(u8"其它设置")) {
		ImGui::Checkbox(u8"编辑模式", &workspace.editMode);
		ImGui::Checkbox(u8"打印详细日志", &IsaacSave::verbose);

	}
}




void RenderUI() {
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
	if (ImGui::Begin("full", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar)) {
		RenderMainContent();
		ImGui::End();
	}
}