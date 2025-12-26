#include"7.process_communication.h"

//
//ImageButton

void ProcessCommunication::on_enter()
{
	clear_message_box();
	str_tip_list.clear();
	process_idx = 1;
	sum_time = 0;
	algo_choose_id = 0;
	current_pcb = nullptr;
	can_execute = false;
	for (int i = 1; i <= 8; i++)
	{
		auto pcb = std::make_shared<PCB>();
		pcb->update_state(ProcessState::Execute);
		pcb->pname = u8"Process" + std::to_string(i);
		pcb_queue_normal.push_back(pcb);
	}
}

void ProcessCommunication::on_exit()
{
	ConfigManager::instance()->clear_resource();
	pcb_queue_normal.clear();
	str_tip_list.clear();
}

void ProcessCommunication::on_input(const SDL_Event* event)
{
	if (ImGui::GetIO().WantCaptureKeyboard)return;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_e)
	{
		show_text_tip_window = !show_text_tip_window;
	}
}

void ProcessCommunication::on_update(float delta)
{
	on_update_imgui_region1(delta);
	on_update_imgui_region2(delta);
	on_update_imgui_region3(delta);

	render_text_window();

	//if (!can_execute)return;
	sum_time += delta;
	on_update_process(delta);
}

void ProcessCommunication::on_update_imgui_region1(float delta)
{
	static const char* process_from[] = { u8"Process1",u8"Process2",u8"Process3" ,u8"Process4" ,u8"Process5" ,u8"Process6" ,u8"Process7" ,u8"Process8" };
	static const char* process_to[] = { u8"Process1",u8"Process2",u8"Process3" ,u8"Process4" ,u8"Process5" ,u8"Process6" ,u8"Process7" ,u8"Process8" };
	static const char* receive_from[] = { u8"Process1",u8"Process2",u8"Process3" ,u8"Process4" ,u8"Process5" ,u8"Process6" ,u8"Process7" ,u8"Process8" };
	static int from_id = 0, to_id = 0, receive_id = 0;

	if (current_pcb)
	{
		{
			ImGui::SetNextWindowSize({ 320, 220 }, ImGuiCond_Once);
			std::string title_str = current_pcb->pname + u8"--消息传递信箱--";
			ImGui::Begin(title_str.c_str(), nullptr, ImGuiWindowFlags_NoResize);

			if (ImGui::Combo("##process_communication_pcb_receive_msg", &receive_id, receive_from, IM_ARRAYSIZE(receive_from), 4))
			{
				strcpy_s(msg_receive_from, sizeof(msg_receive_from), receive_from[receive_id]);
			}ImGui::SameLine();

			static bool choose_id = false;
			static char msg_receive_to_cmp[64];
			if (ImGui::ImageButton(ResourcesManager::instance()->find_texture("msg_receive"), { 20,20 }))
			{
				bool is_find = false;
				if (choose_id)
				{
					for (auto& msg : current_pcb->msg_list)
					{
						if (!msg.get()->is_received
							&& !strcmp(msg.get()->id.c_str(), msg_receive_to_cmp)
							&& !strcmp(msg.get()->from.c_str(), receive_from[receive_id])
							&& !strcmp(msg.get()->to.c_str(), current_pcb->pname.c_str()))
						{
							is_find = true;
							msg->is_received = true;
							std::string tip = u8"消息接收成功";
							str_tip_list.push_back(TextString(tip, TextType::Info));
							auto new_msg = std::make_shared<Message>();
							new_msg->id = msg.get()->id;
							new_msg->content = msg.get()->content;
							new_msg->to = msg.get()->to;
							new_msg->from = msg.get()->from;
							new_msg->is_received = true;
							current_pcb->msg_list_received.push_back(new_msg);
							receive(new_msg->id, new_msg->from, new_msg->to);
						}
					}
				}
				else
				{
					for (auto& msg : current_pcb->msg_list)
					{
						if (!msg.get()->is_received
							&& !strcmp(msg.get()->from.c_str(), receive_from[receive_id])
							&& !strcmp(msg.get()->to.c_str(), current_pcb->pname.c_str()))
						{
							is_find = true;
							msg->is_received = true;
							std::string tip = u8"消息接收成功";
							str_tip_list.push_back(TextString(tip, TextType::Info));
							auto new_msg = std::make_shared<Message>();
							new_msg->id = msg.get()->id;
							new_msg->content = msg.get()->content;
							new_msg->to = msg.get()->to;
							new_msg->from = msg.get()->from;
							new_msg->is_received = true;
							current_pcb->msg_list_received.push_back(new_msg);
							receive(new_msg->id, new_msg->from, new_msg->to);
						}
					}
				}
				if (!is_find)
				{
					std::string tip = u8"消息接收失败：无可接收消息";
					str_tip_list.push_back(TextString(tip, TextType::Error));
				}
			}
			ImGui::Checkbox(u8"开启消息id接收", &choose_id); ImGui::SameLine();
			ImGui::InputText("##process_communication_msg_receive_to_cmp", msg_receive_to_cmp, 64);

			ImGui::SeparatorText(u8"消息列表");
			for (int i = 0; i < current_pcb->msg_list_received.size(); i++)
			{
				auto& msg = current_pcb->msg_list_received[i];
				std::string msg_str_id = u8"消息ID: " + msg.get()->id;
				ImGui::Text(msg_str_id.c_str());
				std::string msg_str_from = u8"发送方: " + msg.get()->from;
				ImGui::Text(msg_str_from.c_str());
				std::string msg_str_content = u8"内容: " + msg.get()->content;
				ImGui::Text(msg_str_content.c_str());

				if (ImGui::ImageButton(ResourcesManager::instance()->find_texture("icon-false"), { 20, 20 }))
				{
					current_pcb->msg_list_received.erase(current_pcb->msg_list_received.begin(), current_pcb->msg_list_received.begin() + i + 1);
				}
				ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
			}
			ImGui::Spacing();

			ImGui::End();
		}
	}

	ImGui::BeginChild("process_communication_region1", { ImGui::GetContentRegionAvail().x,120 });

	ImGui::BeginGroup();
	ImGui::RadioButton(u8"直接消息传递系统", &algo_choose_id, 0); ImGui::SameLine();
	ImGui::RadioButton(u8"信箱通信系统", &algo_choose_id, 1);
	ImGui::EndGroup();

	ImGui::Text(u8"发送方:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(150);
	if (ImGui::Combo("##process_communication_process_from_to_choose", &from_id, process_from, IM_ARRAYSIZE(process_from), 4))
	{
	}ImGui::SameLine();
	ImGui::Text(u8"接收方:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(150);
	if (ImGui::Combo("##process_communication_process_to_to_choose", &to_id, process_from, IM_ARRAYSIZE(process_to), 4))
	{
	}ImGui::SameLine();
	strcpy_s(msg_from, sizeof(msg_from), process_from[from_id]);
	strcpy_s(msg_to, sizeof(msg_to), process_to[to_id]);

	ImGui::Text(u8"消息ID:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(150);
	ImGui::InputText("##process_communication_msg_id_input", msg_id, 64);
	ImGui::Text(u8"消息内容:"); ImGui::SameLine();
	ImGui::InputTextMultiline("##process_communication_msg_content_input", msg_content, 1024, { 400,40 },
		ImGuiInputTextFlags_AllowTabInput |
		ImGuiInputTextFlags_CtrlEnterForNewLine); ImGui::SameLine();
	if (ImGui::ImageButton(ResourcesManager::instance()->find_texture("msg_send"), { 40,40 }))
	{
		if (algo_choose_id == 1)
		{
			send(msg_id, msg_from, msg_to, msg_content);
		}
		else
		{
			std::string tip = u8"时间：" + std::to_string(sum_time) + u8" 进程 " + msg_from + u8" 发送消息到进程 " + msg_to + u8" 消息内容：" + msg_content;
			str_tip_list.push_back(TextString(tip, TextType::Info));
		}
		auto msg = std::make_shared<Message>();
		msg->id = msg_id;
		msg->from = msg_from;
		msg->to = msg_to;
		msg->content = msg_content;
		msg->is_received = false;
		if (algo_choose_id == 1)
		{
			pcb_queue_normal[to_id]->msg_list.push_back(msg);
		}
		else
		{
			pcb_queue_normal[to_id]->msg_list_received.push_back(msg);
		}
	}

	ImGui::EndChild();
	ImGui::Separator();
}

void ProcessCommunication::on_update_imgui_region2(float delta)
{
	ImGui::BeginChild("process_communication_region2", { ImGui::GetContentRegionAvail().x,400 });

	static const float height_delta = 44;
	for (int i = 0; i < pcb_queue_normal.size(); i++)
	{
		auto& pcb = pcb_queue_normal[i];

		std::stringstream ss;
		ss << std::fixed << std::setprecision(1) << pcb->time_execute;

		std::string label = pcb_queue_normal[i]->pname + ": " + pcb_queue_normal[i]->state_name
			+ u8"--处理时间:" + ss.str();

		ImGui::Button(label.c_str(), { 600,40 });
		if (ImGui::IsItemClicked())
		{
			current_pcb = pcb_queue_normal[i].get();
		}

		pcb_queue_normal[i]->draw_state({ 850,162 + height_delta * i }, 10);
	}

	ImGui::EndChild();

	ImGui::Separator();
}

void ProcessCommunication::on_update_imgui_region3(float delta)
{
	ImGui::BeginChildFrame(1, { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y });
	ImGui::Button(u8"清空日志", { ImGui::GetContentRegionAvail().x,30 });
	if (ImGui::IsItemClicked())
	{
		str_tip_list.clear();
	}

	// 显示str_tip_list中的所有提示
	for (const auto& textString : str_tip_list)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, static_cast<ImVec4>(ConfigManager::instance()->get_text_color(textString.type)));
		ImGui::TextUnformatted(textString.text_info.c_str());
		ImGui::PopStyleColor();
	}

	static size_t last_tip_count = 0;
	if (last_tip_count != str_tip_list.size())
	{
		ImGui::SetScrollY(ImGui::GetScrollMaxY() + 2000);
		last_tip_count = str_tip_list.size();
	}

	ImGui::EndChildFrame();
}

void ProcessCommunication::on_update_process(float delta)
{
	switch (algo_choose_id)
	{
	case 0:
		break;
	case 1:
		break;
	default:
		break;
	}
}

void ProcessCommunication::send(const std::string& id, const std::string& from, const std::string& to, const std::string& message)
{
	if (file_message_box_read.is_open() || file_message_box_write.is_open())
	{
		std::string tip = u8"Error：信箱已被占用，发送消息失败";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		return;
	}

	// 首先读取现有的消息内容
	std::vector<cJSON*> existing_messages;
	cJSON* root_array = nullptr;

	// 尝试读取现有文件
	file_message_box_read.open("message_box.json");
	if (file_message_box_read.good())
	{
		std::stringstream buffer;
		buffer << file_message_box_read.rdbuf();
		std::string file_content = buffer.str();
		file_message_box_read.close();

		if (!file_content.empty())
		{
			root_array = cJSON_Parse(file_content.c_str());
			if (root_array && cJSON_IsArray(root_array))
			{
				// 提取现有消息
				cJSON* item = nullptr;
				cJSON_ArrayForEach(item, root_array)
				{
					existing_messages.push_back(cJSON_Duplicate(item, 1));
				}
			}
			if (root_array)
			{
				cJSON_Delete(root_array);
			}
		}
	}

	// 创建新的消息
	cJSON* new_message = cJSON_CreateObject();
	cJSON_AddStringToObject(new_message, "id", id.c_str());
	cJSON_AddStringToObject(new_message, "from", from.c_str());
	cJSON_AddStringToObject(new_message, "to", to.c_str());
	cJSON_AddStringToObject(new_message, "message", message.c_str());

	// 创建新的根数组
	cJSON* new_root_array = cJSON_CreateArray();

	// 添加所有现有消息
	for (cJSON* msg : existing_messages)
	{
		cJSON_AddItemToArray(new_root_array, msg);
	}

	// 添加新消息
	cJSON_AddItemToArray(new_root_array, new_message);

	// 写入文件
	file_message_box_write.open("message_box.json");
	if (!file_message_box_write.good())
	{
		std::string tip = u8"Error：信箱打开失败，发送消息失败";
		str_tip_list.push_back(TextString(tip, TextType::Error));

		// 清理内存：由于文件打开失败，需要删除 new_root_array
		// 注意：new_root_array 中已经包含了 existing_messages 中的消息和 new_message
		cJSON_Delete(new_root_array);
		return;
	}

	char* json_string = cJSON_PrintUnformatted(new_root_array);
	file_message_box_write << json_string;
	file_message_box_write.close();

	// 清理内存
	free(json_string);
	cJSON_Delete(new_root_array); // 这会删除 new_root_array 及其所有子项（包括 existing_messages 中的副本和 new_message）

	std::string tip = u8"时间：" + std::to_string(sum_time)
		+ u8" 进程 " + from + u8" 发送消息到进程 " + to
		+ u8" 消息内容：" + message;
	str_tip_list.push_back(TextString(tip, TextType::Info));
}

void ProcessCommunication::receive(const std::string& id, const std::string& from, const std::string& to)
{
	if (file_message_box_read.is_open() || file_message_box_write.is_open())
	{
		std::string tip = u8"Error：信箱已被占用，接收消息失败";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		return;
	}

	file_message_box_read.open("message_box.json");
	if (!file_message_box_read.good())
	{
		std::string tip = u8"Error：信箱打开失败，接收消息失败";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		return;
	}

	std::stringstream ss;
	ss << file_message_box_read.rdbuf();
	file_message_box_read.close();

	std::string file_content = ss.str();
	if (file_content.empty())
	{
		std::string tip = u8"Error：信箱为空，接收消息失败";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		return;
	}

	// 添加调试信息
	std::cout << "DEBUG: Searching for message - id:" << id
		<< " from:" << from << " to:" << to << std::endl;
	std::cout << "DEBUG: File content: " << file_content << std::endl;

	cJSON* root_array = cJSON_Parse(file_content.c_str());
	if (!root_array || !cJSON_IsArray(root_array))
	{
		std::string tip = u8"Error：信箱内容解析失败，接收消息失败";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		if (root_array) cJSON_Delete(root_array);
		return;
	}

	// 查找匹配的消息
	cJSON* found_message = nullptr;
	int message_index = -1;
	int array_size = cJSON_GetArraySize(root_array);

	std::cout << "DEBUG: Array size: " << array_size << std::endl;

	for (int i = 0; i < array_size; i++)
	{
		cJSON* item = cJSON_GetArrayItem(root_array, i);
		if (item && cJSON_IsObject(item))
		{
			cJSON* msg_id = cJSON_GetObjectItem(item, "id");
			cJSON* msg_from = cJSON_GetObjectItem(item, "from");
			cJSON* msg_to = cJSON_GetObjectItem(item, "to");

			std::string current_id = msg_id ? (msg_id->valuestring ? msg_id->valuestring : "") : "";
			std::string current_from = msg_from ? (msg_from->valuestring ? msg_from->valuestring : "") : "";
			std::string current_to = msg_to ? (msg_to->valuestring ? msg_to->valuestring : "") : "";

			std::cout << "DEBUG: Checking item " << i
				<< " - id:" << current_id
				<< " from:" << current_from
				<< " to:" << current_to << std::endl;

			if (current_id == id && current_from == from && current_to == to)
			{
				found_message = item;
				message_index = i;
				std::cout << "DEBUG: Found match at index " << i << std::endl;
				break;
			}
		}
	}

	if (!found_message)
	{
		std::string tip = u8"Error：未找到匹配的消息，接收消息失败";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		cJSON_Delete(root_array);
		return;
	}

	// 获取消息内容
	cJSON* msg_message = cJSON_GetObjectItem(found_message, "message");
	if (!msg_message || !msg_message->valuestring)
	{
		std::string tip = u8"Error：消息格式错误，接收消息失败";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		cJSON_Delete(root_array);
		return;
	}

	std::string message_content = msg_message->valuestring;

	// 从数组中移除该消息
	cJSON_DeleteItemFromArray(root_array, message_index);

	std::cout << "DEBUG: Removed item at index " << message_index << std::endl;

	// 检查删除后的数组
	int new_size = cJSON_GetArraySize(root_array);
	std::cout << "DEBUG: New array size: " << new_size << std::endl;

	// 将更新后的数组写回文件
	file_message_box_write.open("message_box.json");
	if (!file_message_box_write.good())
	{
		std::string tip = u8"Error：信箱打开失败，无法更新消息列表";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		cJSON_Delete(root_array);
		return;
	}

	char* json_string = cJSON_PrintUnformatted(root_array);
	std::cout << "DEBUG: New JSON content: " << json_string << std::endl;

	file_message_box_write << json_string;
	file_message_box_write.close();

	free(json_string);
	cJSON_Delete(root_array);

	std::string tip = u8"时间：" + std::to_string(sum_time)
		+ u8" 进程 " + to + u8" 接收来自进程 " + from
		+ u8" 的消息内容：" + message_content;
	str_tip_list.push_back(TextString(tip, TextType::Info));
}

void ProcessCommunication::clear_message_box()
{
	if (file_message_box_read.is_open() || file_message_box_write.is_open())
	{
		std::string tip = u8"Error：信箱已被占用，清空消息失败";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		return;
	}

	file_message_box_write.open("message_box.json");
	if (!file_message_box_write.good())
	{
		std::string tip = u8"Error：信箱打开失败，清空消息失败";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		return;
	}

	// 写入一个空数组
	cJSON* empty_array = cJSON_CreateArray();
	char* json_string = cJSON_PrintUnformatted(empty_array);
	file_message_box_write << json_string;
	file_message_box_write.close();

	free(json_string);
	cJSON_Delete(empty_array);

	std::string tip = u8"时间：" + std::to_string(sum_time)
		+ u8" 信箱消息已清空";
	str_tip_list.push_back(TextString(tip, TextType::Info));
}

void ProcessCommunication::render_text_window()
{
	if (show_text_tip_window)
	{
		ImGui::SetNextWindowSize({ 800, 400 });
		if (ImGui::Begin(u8"进程通信系统说明", &show_text_tip_window))
		{
			ImGui::BeginChild("ProcessCommunicationTipContent", ImVec2(0, 0), true);
			{
				ImGui::PushTextWrapPos(0.0f);

				ImGui::Text(u8"系统概述：");
				ImGui::BulletText(u8"本系统模拟操作系统中的进程通信机制，支持两种消息传递方式");
				ImGui::BulletText(u8"通过可视化界面展示进程间通信的完整流程");
				ImGui::Spacing();

				ImGui::Text(u8"技术实现：");
				ImGui::BulletText(u8"定义Message结构体，包含：消息标识id、发送方from、接收方to、消息内容content、接收状态is_received");
				ImGui::BulletText(u8"每个进程维护两个消息列表：待处理消息列表(msg_list)和已接收消息列表(msg_list_received)");
				ImGui::BulletText(u8"进程状态可视化展示，支持点击选择当前操作进程");
				ImGui::Spacing();

				ImGui::Text(u8"直接消息传递系统：");
				ImGui::BulletText(u8"消息直接从发送进程传输到接收进程的消息队列");
				ImGui::BulletText(u8"接收进程可以立即查看和处理收到的消息");
				ImGui::BulletText(u8"实现简单，但需要发送和接收进程同时运行");
				ImGui::Spacing();

				ImGui::Text(u8"信箱通信系统：");
				ImGui::BulletText(u8"使用message_box.json文件作为共享信箱");
				ImGui::BulletText(u8"发送进程将消息写入信箱，接收进程从信箱读取消息");
				ImGui::BulletText(u8"支持消息的持久化存储，不要求进程同时运行");
				ImGui::BulletText(u8"使用cJSON库进行JSON格式的序列化和反序列化");
				ImGui::Spacing();

				ImGui::Text(u8"操作指南：");
				ImGui::BulletText(u8"选择通信方式：直接通信或信箱通信");
				ImGui::BulletText(u8"发送消息：选择发送方、接收方，填写消息ID和内容，点击发送按钮");
				ImGui::BulletText(u8"接收消息：在进程消息窗口中选择发送方，点击接收按钮");
				ImGui::BulletText(u8"指定接收：可开启消息ID过滤，只接收特定ID的消息");
				ImGui::BulletText(u8"管理消息：查看已接收消息列表，可删除已处理的消息");
				ImGui::Spacing();

				ImGui::Text(u8"核心功能：");
				ImGui::BulletText(u8"send()：发送消息到目标进程或信箱，包含完整的错误处理");
				ImGui::BulletText(u8"receive()：从信箱接收指定消息，支持消息ID和发送方过滤");
				ImGui::BulletText(u8"clear_message_box()：清空信箱中的所有消息");
				ImGui::BulletText(u8"实时日志：显示所有通信操作的详细记录和时间戳");
				ImGui::Spacing();

				ImGui::Text(u8"应用场景：");
				ImGui::BulletText(u8"直接通信：适用于实时性要求高、进程耦合紧密的场景");
				ImGui::BulletText(u8"信箱通信：适用于异步通信、进程解耦、消息持久化场景");
				ImGui::PopTextWrapPos();

				// 使用技巧
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

				ImGui::Text(u8"使用技巧：");
				ImGui::BulletText(u8"按E键可快速打开/关闭本提示窗口");
				ImGui::BulletText(u8"点击进程按钮可切换到该进程的消息管理界面");
				ImGui::BulletText(u8"在信箱通信中，消息会持久化保存，重启程序后仍可读取");
				ImGui::BulletText(u8"使用消息ID可以确保消息被特定处理逻辑接收");

				ImGui::PopStyleColor();
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}
}