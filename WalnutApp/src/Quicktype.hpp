#pragma once

#include "json.hpp"

#ifndef NLOHMANN_OPT_HELPER
#define NLOHMANN_OPT_HELPER
namespace nlohmann
{
template <typename T> struct adl_serializer<std::shared_ptr<T>>
{
    static void to_json(json &j, const std::shared_ptr<T> &opt)
    {
        if (!opt)
            j = nullptr;
        else
            j = *opt;
    }

    static std::shared_ptr<T> from_json(const json &j)
    {
        if (j.is_null())
            return std::make_shared<T>();
        else
            return std::make_shared<T>(j.get<T>());
    }
};
template <typename T> struct adl_serializer<std::optional<T>>
{
    static void to_json(json &j, const std::optional<T> &opt)
    {
        if (!opt)
            j = nullptr;
        else
            j = *opt;
    }

    static std::optional<T> from_json(const json &j)
    {
        if (j.is_null())
            return std::make_optional<T>();
        else
            return std::make_optional<T>(j.get<T>());
    }
};
} // namespace nlohmann
#endif

namespace quicktype
{
using nlohmann::json;

#ifndef NLOHMANN_UNTYPED_quicktype_HELPER
#define NLOHMANN_UNTYPED_quicktype_HELPER
inline json get_untyped(const json &j, const char *property)
{
    if (j.find(property) != j.end())
    {
        return j.at(property).get<json>();
    }
    return json();
}

inline json get_untyped(const json &j, std::string property)
{
    return get_untyped(j, property.data());
}
#endif

#ifndef NLOHMANN_OPTIONAL_quicktype_HELPER
#define NLOHMANN_OPTIONAL_quicktype_HELPER
template <typename T> inline std::optional<T> get_stack_optional(const json &j, const char *property)
{
    auto it = j.find(property);
    if (it != j.end() && !it->is_null())
    {
        return j.at(property).get<std::optional<T>>();
    }
    return std::optional<T>();
}

template <typename T> inline std::optional<T> get_stack_optional(const json &j, std::string property)
{
    return get_stack_optional<T>(j, property.data());
}
#endif

struct Runway
{
    std::array<char, 4> name;
    std::array<char, 4> app;
    std::array<char, 3> starrnav;
    // TODO: Make everything optional for other airports
    // Optionnal
    std::array<char, 3> starconv;
    std::array<char, 6> sidrnav;
    std::array<char, 6> sidconv;
    uint32_t qfu;
    float score;
};

struct Airport
{
    std::array<char, 5> icao = {};
    bool isMetarReady = false;
    uint8_t activeRunwayIndex = 0;
    std::string metar;
    std::vector<std::array<char, 6>> sidwpt; // TODO: Use them lol
    std::vector<std::array<char, 6>> starwpt;
    std::vector<Runway> runways;
};
} // namespace quicktype

namespace quicktype
{
void from_json(const json &j, Runway &x);
void to_json(json &j, const Runway &x);

void from_json(const json &j, Airport &x);
void to_json(json &j, const Airport &x);

template <std::size_t N> inline void stack_str_from_std(std::array<char, N> &stack_str, std::string_view value)
{
    memcpy(stack_str.data(), value.begin(), sizeof(char[N - 1]));
    stack_str[N - 1] = 0;
}

template <std::size_t N>
inline void stack_str_from_json(std::array<char, N> &stack_str, const json &j, std::string_view property)
{
    std::string value = j.at(property).get<std::string>();
    stack_str_from_std(stack_str, value);
}

template <std::size_t N>
inline void opt_str_from_json(std::array<char, N> &stack_str, const json &j, std::string_view property)
{
    auto it = j.find(property);
    if (it != j.end() && !it->is_null())
    {
        std::string data = it.value().get<std::string>();
        stack_str_from_std(stack_str, data);
        return;
    }
    memset(stack_str.data(), 0, sizeof(stack_str));
}

inline void from_json(const json &j, Runway &x)
{
    x.qfu = j.at("QFU").get<uint32_t>();
    stack_str_from_json(x.name, j, "Name");
    opt_str_from_json(x.sidrnav, j, "SIDRNAV");
    opt_str_from_json(x.sidconv, j, "SIDCONV");
    stack_str_from_json(x.starrnav, j, "STARRNAV");
    opt_str_from_json(x.starconv, j, "STARCONV");
    stack_str_from_json(x.app, j, "APP");
}

inline void to_json(json &j, const Runway &x)
{
    j = json::object();
    j["Name"] = x.name;
    j["QFU"] = x.qfu;
    j["SIDRNAV"] = x.sidrnav;
    j["SIDCONV"] = x.sidconv;
    j["STARRNAV"] = x.starrnav;
    j["STARCONV"] = x.starconv;
    j["APP"] = x.app;
}

inline void from_json(const json &j, Airport &x)
{
    stack_str_from_json(x.icao, j, "ICAO");

    auto sidwpts = j.at("SIDWPT").get<std::vector<std::string>>();
    x.sidwpt.resize(sidwpts.size());
    for (size_t i = 0; i < sidwpts.size(); i++)
        stack_str_from_std(x.sidwpt[i], sidwpts[i]);

    auto starwpts = j.at("STARWPT").get<std::vector<std::string>>();
    x.starwpt.resize(starwpts.size());
    for (size_t i = 0; i < starwpts.size(); i++)
        stack_str_from_std(x.starwpt[i], starwpts[i]);

    x.runways = j.at("Runways").get<std::vector<Runway>>();
}

inline void to_json(json &j, const Airport &x)
{
    j = json::object();
    j["ICAO"] = x.icao;
    j["SIDWPT"] = x.sidwpt;
    j["STARWPT"] = x.starwpt;
    j["Runways"] = x.runways;
}
} // namespace quicktype
