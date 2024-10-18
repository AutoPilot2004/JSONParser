#pragma once

#include <vector>
#include <unordered_map>
#include <fstream>

#include "JSON.h"

[[nodiscard]] JSON parse(std::ifstream& f);