#pragma once
#include <string>
#include <stdexcept>

class Developer {
public:
    Developer(int id, const std::string& name) {
        if (name.empty()) {
            throw std::invalid_argument("Имя разработчика не может быть пустым");
        }
        id_ = id;
        name_ = name;
    }

    int id() const { return id_; }
    const std::string& name() const { return name_; }

private:
    int id_;
    std::string name_;
};
