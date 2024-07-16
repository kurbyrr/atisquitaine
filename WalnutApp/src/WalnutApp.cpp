#include "Walnut/Application.h"

#include <fstream>

#include "../cpp-httplib/out/httplib.h"

#include "ParseMetar.h"
#include "Quicktype.hpp"

class ExampleLayer : public Walnut::Layer
{
  public:
    std::string loadFile(const char *path)
    {
        std::ifstream inFile(path, std::ios::ate);
        if (!inFile)
            throw std::runtime_error("Could not open airac file");

        size_t file_size = inFile.tellg();
        inFile.seekg(0);

        std::string buffer(file_size, 0);
        inFile.read(buffer.data(), file_size);

        return buffer;
    }

    virtual void OnAttach() override
    {
        m_HttpClient = std::make_unique<httplib::Client>("https://api.ivao.aero");
        airports = nlohmann::json::parse(loadFile("./airac.json"));

        updateMetars();
    }

    template <std::size_t N> const char *stack_str_value_or(const std::array<char, N> &stack_arr, std::string_view alt)
    {
        return (stack_arr[0] == 0) ? alt.begin() : stack_arr.data();
    }

    virtual void OnUIRender() override
    {
        for (auto &airport : airports)
        {
            ImGui::Begin(airport.icao.data());
            if (!airport.isMetarReady)
            {
                ImGui::TextUnformatted("Updating...");
                ImGui::End();
                continue;
            }

            ImGui::TextUnformatted(airport.metar.c_str());
            for (int i = 0; i < airport.runways.size(); i++)
            {
                if (ImGui::RadioButton(airport.runways[i].name.data(), airport.activeRunwayIndex == i))
                {
                    airport.activeRunwayIndex = i;
                    genAtis();
                }
            }
            if (ImGui::Button("Auto Runway"))
                parseMetar(airport);
            const quicktype::Runway &runway = airport.runways[airport.activeRunwayIndex];
            ImGui::TextUnformatted("Procedures:");
            ImGui::Text("SID RNAV: %s\nSID CONV: %s", stack_str_value_or(runway.sidrnav, "N/A"),
                        stack_str_value_or(runway.sidconv, "N/A"));

            ImGui::Text("STAR RNAV: %s\nSTAR CONV: %s", runway.starrnav.data(),
                        stack_str_value_or(runway.starconv, "N/A"));
            ImGui::End();
        }

        ImGui::Begin("AutoGen");
        ImGui::TextUnformatted(m_AutoGenAtis.c_str());
        if (ImGui::Button("Copy"))
            ImGui::SetClipboardText(m_AutoGenAtis.c_str());
        ImGui::End();
    }

    virtual void OnDetach() override
    {
    }

  private:
    void genAtis()
    {
        m_AutoGenAtis.clear();
        for (const auto &airport : airports)
        {
            const auto &runway = airport.runways[airport.activeRunwayIndex];
            const char *sid = stack_str_value_or(runway.sidrnav, stack_str_value_or(runway.sidconv, "N/A"));
            const char *star = runway.starrnav.data();

            char loc_buff[32];
            std::snprintf(loc_buff, 32, "%s DEP-%s ARR-%s // ", airport.icao.data(), sid, star);
            m_AutoGenAtis += loc_buff;
        }

        m_AutoGenAtis.resize(m_AutoGenAtis.size() - 4); // Get rid of the final //
    }

    std::string fetchMetar()
    {
        constexpr bool use_dev_file = true;
        if (use_dev_file)
        {
            std::string raw_metar = loadFile("./metar.json");
            return raw_metar;
        }

        httplib::Headers headers = {{"accept", "application/json"}};
        uint8_t retries = 5;
        while (retries)
        {
            httplib::Result res = m_HttpClient->Get("/v2/airports/all/metar", headers);
            if (!res || res->status != 200) // Short circuiting
            {
                retries--;
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }

            return res->body;
        }

        throw std::runtime_error("Fetch error has occured");
    }

    void updateMetars()
    {
        for (auto &airport : airports)
            airport.isMetarReady = false;

        std::thread{[&]() {
            nlohmann::json metars = nlohmann::json::parse(fetchMetar());

            for (auto &airport : airports)
                for (auto it = metars.begin(); it != metars.end(); it++)
                {
                    std::string icao = it->at("airportIcao").get<std::string>();

                    if (std::memcmp(icao.c_str(), airport.icao.data(), sizeof(airport.icao)) == 0)
                    {
                        airport.metar = std::move(it->at("metar").get<std::string>());
                        airport.isMetarReady = true;
                        parseMetar(airport);
                    }
                }

            m_LastUpdate = std::chrono::steady_clock::now();
        }}.detach();
    }

  private:
    std::unique_ptr<httplib::Client> m_HttpClient;

    std::vector<quicktype::Airport> airports;
    std::chrono::time_point<std::chrono::steady_clock> m_LastUpdate;
    std::string m_AutoGenAtis;
};

Walnut::Application *Walnut::CreateApplication(int argc, char **argv)
{
    Walnut::ApplicationSpecification spec;
    spec.Name = "AtisQuitaine";

    Walnut::Application *app = new Walnut::Application(spec);
    app->PushLayer<ExampleLayer>();
    // app->SetMenubarCallback([app]() {
    //     if (ImGui::BeginMenu("File"))
    //     {
    //         if (ImGui::MenuItem("Open Airac"))
    //         {

    //         }
    //         ImGui::EndMenu();
    //     }
    // });
    return app;
}
