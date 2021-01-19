
//
// Disclaimer:
// ----------
//
// This code will work only if you selected window, graphics and audio.
//
// Note that the "Run Script" build phase will copy the required frameworks
// or dylibs to your application bundle so you can execute it on any OS X
// computer.
//
// Your resource files (images, sounds, fonts, ...) are also copied to your
// application bundle. To get the path to these resources, use the helper
// function `resourcePath()` from ResourcePath.hpp
//

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

// Here is a small helper for you! Have a look.
#include "ResourcePath.hpp"
#include <iostream>
#include <curl/curl.h>
#include <string>
#include <rapidjson/document.h>

size_t write_data(char *data, size_t memberSize, size_t memberCount, std::string *destination)
{
    size_t size = memberSize * memberCount;
    destination->append(data, size);
    return size;
}

int retrieve_file_from_URL(const std::string& url, std::string& fileBuffer)
{
    CURL *curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileBuffer);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        return res;
    }
    return 1;
}

class CollectionElement
{
public:
    CollectionElement(const std::string& title, const std::string& image_url) :
        title(title),
        image_url(image_url)
    {}

    std::string GetTitle() const
    {
        return title;
    }

    std::string GetImageUrl() const
    {
        return image_url;
    }
private:
    const std::string title;
    std::string image_url;
};

class Collection
{
public:
    Collection(const std::string& title) : 
        title(title)
    {}

    void AddElement(const CollectionElement& element)
    {
        elements.push_back(element);
    }

    std::string GetTitle() const
    {
        return title;
    }

    size_t GetElementCount() const
    {
        return elements.size();
    }

    CollectionElement GetElement(size_t index) const
    {
        return elements[index];
    }

private:
    std::string title;
    std::vector<CollectionElement> elements;
};

int main()
{
    std::string homeApiUrl("https://cd-static.bamgrid.com/dp-117731241344/home.json");
    std::string homeApiContents;
    if (retrieve_file_from_URL(homeApiUrl, homeApiContents) != 0)
    {
        return EXIT_FAILURE;
    }

    rapidjson::Document apiDoc;
    apiDoc.Parse(homeApiContents.c_str());
    auto& data = apiDoc["data"];
    auto& collection = data["StandardCollection"];
    auto& containers = collection["containers"];

    std::vector<Collection> collections;
    for (const auto& container : containers.GetArray())
    {
        auto& set = container["set"];
        auto& type = set["type"];
        if (strcmp(type.GetString(), "SetRef") != 0)
        {
            auto& text = set["text"];
            auto& title = text["title"];
            auto& full = title["full"];
            auto& title_set = full["set"];
            auto& title_default = title_set["default"];
            auto& title_content = title_default["content"];

            collections.emplace_back(Collection(title_content.GetString()));
            auto& items = set["items"];
            for (const auto& item : items.GetArray())
            {
                auto& item_type = item["type"];
                std::string title_type_string;
                std::string image_type_string;
                if (strcmp(item_type.GetString(), "DmcSeries") == 0)
                {
                    title_type_string = "series";
                    image_type_string = "series";
                }
                else if (strcmp(item_type.GetString(), "DmcVideo") == 0)
                {
                    title_type_string = "program";
                    image_type_string = "program";
                }
                else if (strcmp(item_type.GetString(), "StandardCollection") == 0)
                {
                    title_type_string = "collection";
                    image_type_string = "default";
                }
                else
                {
                    std::cout << "Unknown type" << std::endl;
                }

                auto& text = item["text"];
                auto& title = text["title"];
                auto& full = title["full"];
                auto& title_type = full[title_type_string.c_str()];
                auto& title_default = title_type["default"];
                auto& title_content = title_default["content"];

                auto& image = item["image"];
                auto& tile_image = image["tile"];
                auto& image_version = tile_image.MemberBegin()->value;
                auto& image_series = image_version[image_type_string.c_str()];
                auto& image_default = image_series["default"];
                auto& image_url = image_default["url"];

                CollectionElement element(title_content.GetString(), image_url.GetString());
                collections.back().AddElement(element);
            }
        }
    }
 
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(1600, 1200), "Disney+");

    // Set the Icon
    sf::Image icon;
    if (!icon.loadFromFile(resourcePath() + "DisneyPlus.png")) 
    {
        return EXIT_FAILURE;
    }
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    // Create a graphical text to display
    sf::Font font;
    if (!font.loadFromFile(resourcePath() + "Avenir.ttc")) 
    {
        return EXIT_FAILURE;
    }

    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed) 
            {
                window.close();
            }

            // Escape pressed: exit
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) 
            {
                window.close();
            }
        }

        // Clear screen
        window.clear();

        size_t collection_index = 0;
        const size_t row_size = 300;
        for (const auto& collection : collections)
        {
            sf::Text text(collection.GetTitle().c_str(), font, 24);
            text.setFillColor(sf::Color::White);
            text.setPosition(0.0f, 0.0f + collection_index++ * row_size);
            window.draw(text);
        }

        // Update the window
        window.display();
    }

    return EXIT_SUCCESS;
}
