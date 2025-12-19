#pragma once
#include <string>
#include <optional>
#include <stdexcept>
#include "taskstatus.h"

class Task {
public:
    Task(int id, std::string title, std::string description)
        : id_(id),
        title_(std::move(title)),
        description_(std::move(description)),
        status_(TaskStatus::Backlog)
    {
        if (title_.empty()) {
            throw std::invalid_argument("Название задачи не может быть пустым");
        }
    }

    int id() const noexcept { return id_; }
    const std::string& title() const noexcept { return title_; }
    const std::string& description() const noexcept { return description_; }
    TaskStatus status() const noexcept { return status_; }

    const std::optional<int>& assignedDeveloper() const noexcept {
        return assignedDeveloperId_;
    }

    void assignDeveloper(int developerId) {
        assignedDeveloperId_ = developerId;

        if (status_ == TaskStatus::Backlog) {
            status_ = TaskStatus::Assigned;
        }
    }

    void changeStatus(TaskStatus newStatus) {
        validateStatusTransition(newStatus);
        status_ = newStatus;
    }

private:
    void validateStatusTransition(TaskStatus newStatus) const {
        if ((newStatus == TaskStatus::Assigned ||
             newStatus == TaskStatus::InProgress) &&
            !assignedDeveloperId_) {
            throw std::logic_error(
                "Для перемещения задачи в выполнение назначьте сначала разработчика");
        }
    }

private:
    int id_;
    std::string title_;
    std::string description_;
    TaskStatus status_;
    std::optional<int> assignedDeveloperId_;
};
