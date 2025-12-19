#pragma once
#include "task.h"
#include <string>

inline std::string taskStatusToString(TaskStatus status) {
    switch (status) {
    case TaskStatus::Backlog: return "Backlog";
    case TaskStatus::Assigned: return "Assigned";
    case TaskStatus::InProgress: return "InProgress";
    case TaskStatus::Blocked: return "Blocked";
    case TaskStatus::Done: return "Done";
    default: return "Unknown";
    }
}

inline TaskStatus stringToTaskStatus(const std::string& str) {
    if (str == "Backlog") return TaskStatus::Backlog;
    if (str == "Assigned") return TaskStatus::Assigned;
    if (str == "InProgress") return TaskStatus::InProgress;
    if (str == "Blocked") return TaskStatus::Blocked;
    if (str == "Done") return TaskStatus::Done;
    throw std::runtime_error("Unknown TaskStatus string");
}
