#pragma once
#include <nlohmann/json.hpp>
#include "scrumboard.h"
#include <string>

class BoardSerializer {
public:
    static nlohmann::json serialize(const ScrumBoard& board);
    static ScrumBoard deserialize(const nlohmann::json& json);
};

void saveBoardToFile(const ScrumBoard& board, const std::string& filename);
ScrumBoard loadBoardFromFile(const std::string& filename);
