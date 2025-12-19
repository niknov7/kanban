#pragma once
#include <map>
#include <stdexcept>
#include <unordered_map>
#include "task.h"
#include "developer.h"

class ScrumBoard {
public:
    ScrumBoard() : nextDeveloperId(1), nextTaskId(1) {}

    void setNextDeveloperId(int next) { nextDeveloperId = next; }
    void setNextTaskId(int next) { nextTaskId = next; }

    void addDeveloper(const Developer& developer) {
        if (developers_.find(developer.id()) != developers_.end()) {
            throw std::runtime_error("Разработчик с этим ID уже существует");
        }
        developers_.emplace(developer.id(), developer);
    }

    void addTask(const Task& task) {
        if (tasks_.find(task.id()) != tasks_.end()) {
            throw std::runtime_error("Задача с этим ID уже существует");
        }
        tasks_.emplace(task.id(), task);
    }

    void assignTask(int taskId, int developerId) {
        auto& task = getTask(taskId);
        ensureDeveloperExists(developerId);
        task.assignDeveloper(developerId);
    }

    void changeTaskStatus(int taskId, TaskStatus newStatus) {
        auto& task = getTask(taskId);
        task.changeStatus(newStatus);
    }

    const Task& getTask(int taskId) const {
        return tasks_.at(taskId);
    }

    const Developer& getDeveloper(int developerId) const {
        return developers_.at(developerId);
    }

    void removeDeveloper(int id) {
        auto it = developers_.find(id);
        if (it != developers_.end()) {
            developers_.erase(it);
        } else {
            throw std::runtime_error("Разработчик не найден");
        }
    }

    int getNextDeveloperId() {
        return nextDeveloperId++;
    }

    int getNextTaskId() {
        return nextTaskId++;
    }

    void removeTask(int taskId) {
        auto it = tasks_.find(taskId);
        if (it == tasks_.end()) {
            throw std::runtime_error("Задача не найдена");
        }
        tasks_.erase(it);
    }

    Task& getTask(int taskId) {
        auto it = tasks_.find(taskId);
        if (it == tasks_.end()) {
            throw std::out_of_range("Задача не найдена");
        }
        return it->second;
    }

    const std::unordered_map<int, Developer>& getAllDevelopers() const { return developers_; }
    const std::map<int, Task>& getAllTasks() const { return tasks_; }
    int peekNextDeveloperId() const { return nextDeveloperId; }
    int peekNextTaskId() const { return nextTaskId; }

private:
    std::unordered_map<int, Developer> developers_;
    int nextDeveloperId;

    std::map<int, Task> tasks_;
    int nextTaskId;

    void ensureDeveloperExists(int developerId) const {
        if (developers_.find(developerId) == developers_.end()) {
            throw std::out_of_range("Разработчик не найден");
        }
    }
};
