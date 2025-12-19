#pragma once

#include <string>


enum class TaskStatus {
    Backlog,
    Assigned,
    InProgress,
    Blocked,
    Done
};

inline std::string toString(TaskStatus status) {
    switch(status) {
    case TaskStatus::Assigned: return "Assigned";
    case TaskStatus::InProgress: return "InProgress";
    case TaskStatus::Blocked: return "Blocked";
    case TaskStatus::Done: return "Done";
    default: return "Unknown";
    }
}
