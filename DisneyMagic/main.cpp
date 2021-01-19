
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
#include <algorithm>

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
        auto& collection_set = container["set"];
        auto& collection_type = collection_set["type"];
        if (strcmp(collection_type.GetString(), "SetRef") != 0)
        {
            auto& collection_text = collection_set["text"];
            auto& collection_title = collection_text["title"];
            auto& collection_full = collection_title["full"];
            auto& collection_title_set = collection_full["set"];
            auto& collection_title_default = collection_title_set["default"];
            auto& collection_title_content = collection_title_default["content"];

            collections.emplace_back(Collection(collection_title_content.GetString()));
            auto& items = collection_set["items"];
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
                auto& image_element = tile_image["1.78"]; // ASSUMPTION: every "tile" contains an image with aspect ratio 1.78
                auto& image_series = image_element[image_type_string.c_str()];
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

        size_t collection_index{ 0 };
        const double row_offset{ 10 };
        const double row_size{ 250 };
        const double collection_origin_column{ 10 };
        for (const auto& collection : collections)
        {
            double collection_origin_row{ row_offset + collection_index * row_size };
            unsigned int font_size { 24 };
            sf::Text collection_text(collection.GetTitle().c_str(), font, font_size);
            collection_text.setFillColor(sf::Color::White);
            collection_text.setPosition(collection_origin_column, collection_origin_row);
            window.draw(collection_text);

            const double element_width{ 325 };
            for (size_t element_index = 0; element_index < std::min((size_t)4, collection.GetElementCount()); ++element_index)
            {
                double element_origin_row{ collection_origin_row + font_size + 10 };
                double element_origin_column{ collection_origin_column + element_index * element_width };

                const auto& element = collection.GetElement(element_index);
                std::string image_buffer;
                retrieve_file_from_URL(element.GetImageUrl().c_str(), image_buffer);

                sf::Texture image;
                if (image.loadFromMemory(image_buffer.data(), image_buffer.size()))
                {
                    sf::Sprite sprite(image);
                    sprite.setPosition(element_origin_column, element_origin_row);
                    sprite.setScale(0.6f, 0.6f);
                    window.draw(sprite);
                }
                else
                {
                    // If image fails to load, fall back to displaying the program title
                    sf::Text element_text(element.GetTitle(), font, font_size);
                    element_text.setFillColor(sf::Color::White);
                    element_text.setPosition(element_origin_column, element_origin_row);
                    window.draw(element_text);
                }
            }
            collection_index++;
        }

        sf::RectangleShape selection_rect;
        selection_rect.setFillColor(sf::Color::Transparent);
        selection_rect.setOutlineColor(sf::Color::White);
        selection_rect.setOutlineThickness(5.0f);
        selection_rect.setSize(sf::Vector2f(120.f, 50.f));   
        selection_rect.setPosition(collection_origin_column, row_offset);
        window.draw(selection_rect);

        // Update the window
        window.display();
    }

    return EXIT_SUCCESS;
}
