#include "ParseMetar.h"

#include <regex>

// TODO: string_view
std::vector<std::string> splitString(const std::string &str, char sep)
{
    std::vector<std::string> result;
    std::string buffer;

    for (char ch : str)
    {
        if (ch == sep)
        {
            if (!buffer.empty())
            {
                result.push_back(buffer);
                buffer.clear();
            }
        }
        else
            buffer += ch;
    }
    if (!buffer.empty())
        result.push_back(buffer);

    return result;
}

struct Wind
{
    int dir;
    int speed;
};

void parseMetar(quicktype::Airport &airport)
{
    std::vector<std::string> metarArray = splitString(airport.metar, ' ');

    static std::regex windRegex("^(\\d\\d\\d|VRB)P?(\\d+)(?:G(\\d+))?(KT|MPS|KPH)");
    std::smatch match;
    for (auto &metarPart : metarArray)
    {
        if (std::regex_match(metarPart, match, windRegex))
        {
            Wind wind = {match[1] == "VRB" ? 180 : std::stoi(match[1]), std::stoi(match[2])}; // x: deg / y: speed

            for (auto &runway : airport.runways)
            {
                float frontWind = wind.speed * std::cos(glm::radians(static_cast<float>(wind.dir - runway.qfu)));
                float sideWind = wind.speed * std::sin(glm::radians(static_cast<float>(wind.dir - runway.qfu)));

                runway.score = frontWind - std::abs(sideWind);
            }

            std::sort(airport.runways.begin(), airport.runways.end(),
                      [](const auto &lhs, const auto &rhs) { return lhs.score > rhs.score; });
            airport.activeRunwayIndex = 0;
        }
    }
}
