#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Quicktype.hpp"

std::vector<std::string> splitString(const std::string &str, char sep);
void parseMetar(quicktype::Airport &airport);
