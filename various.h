#pragma once

#include <string>
#include <sstream>
#include <utility>
#include <vector>

std::string intToString(int input);

class various{
    public:
        template <typename T> static std::string ToString(T input);
        static void iterateVectorToFile(std::vector<std::pair<double, double>> vectorInput, std::string fileName);
        static std::string fileNameErase(std::string fileName);
};

