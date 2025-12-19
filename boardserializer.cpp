#include "boardserializer.h"
#include "taskutils.h"
#include <fstream>
#include <stdexcept>

nlohmann::json BoardSerializer::serialize(const ScrumBoard& board)
{
    nlohmann::json json;

    json["developers"] = nlohmann::json::array();
    auto devs = board.getAllDevelopers();
    for (auto it = devs.begin(); it != devs.end(); ++it) {
        nlohmann::json devJson;
        devJson["id"] = it->second.id();
        devJson["name"] = it->second.name();
        json["developers"].push_back(devJson);
    }

    json["tasks"] = nlohmann::json::array();
    auto tasks = board.getAllTasks();
    for (auto it = tasks.begin(); it != tasks.end(); ++it) {
        const Task& task = it->second;

        nlohmann::json taskJson;
        taskJson["id"] = task.id();
        taskJson["title"] = task.title();
        taskJson["description"] = task.description();
        taskJson["status"] = taskStatusToString(task.status());

        if (task.assignedDeveloper()) {
            taskJson["assignedDeveloperId"] = *task.assignedDeveloper();
        } else {
            taskJson["assignedDeveloperId"] = nullptr;
        }

        json["tasks"].push_back(taskJson);
    }

    return json;
}

ScrumBoard BoardSerializer::deserialize(const nlohmann::json& json)
{
    ScrumBoard board;

    int maxDevId = 0;
    int maxTaskId = 0;

    if (json.contains("developers")) {
        for (size_t i = 0; i < json["developers"].size(); ++i) {
            int id = json["developers"][i]["id"].get<int>();
            std::string name = json["developers"][i]["name"].get<std::string>();

            board.addDeveloper(Developer(id, name));
            if (id > maxDevId) maxDevId = id;
        }
    }

    if (json.contains("tasks")) {
        for (size_t i = 0; i < json["tasks"].size(); ++i) {
            int id = json["tasks"][i]["id"].get<int>();
            std::string title = json["tasks"][i]["title"].get<std::string>();
            std::string description = json["tasks"][i]["description"].get<std::string>();
            std::string statusStr = json["tasks"][i]["status"].get<std::string>();
            TaskStatus status = stringToTaskStatus(statusStr);

            board.addTask(Task(id, title, description));
            if (id > maxTaskId) maxTaskId = id;

            if (json["tasks"][i].contains("assignedDeveloperId")) {
                if (!json["tasks"][i]["assignedDeveloperId"].is_null()) {
                    int devId = json["tasks"][i]["assignedDeveloperId"].get<int>();
                    board.assignTask(id, devId);
                }
            }

            if (status != TaskStatus::Backlog) {
                board.changeTaskStatus(id, status);
            }
        }
    }

    board.setNextDeveloperId(maxDevId + 1);
    board.setNextTaskId(maxTaskId + 1);

    return board;
}

void saveBoardToFile(const ScrumBoard& board, const std::string& filename)
{
    std::ofstream file(filename.c_str());
    if (!file) {
        throw std::runtime_error("Невозможно открыть файл для записи");
    }

    nlohmann::json json = BoardSerializer::serialize(board);
    file << json.dump(4);
}

ScrumBoard loadBoardFromFile(const std::string& filename)
{
    std::ifstream file(filename.c_str());
    if (!file) {
        throw std::runtime_error("Невозможно открыть файл для чтения");
    }

    nlohmann::json json;
    file >> json;

    return BoardSerializer::deserialize(json);
}
