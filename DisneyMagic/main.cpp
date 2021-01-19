
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

    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window");

    // Set the Icon
    sf::Image icon;
    if (!icon.loadFromFile(resourcePath() + "icon.png")) {
        return EXIT_FAILURE;
    }
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    // Load a sprite to display
    sf::Texture texture;
    if (!texture.loadFromFile(resourcePath() + "cute_image.jpg")) {
        return EXIT_FAILURE;
    }
    sf::Sprite sprite(texture);

    // Create a graphical text to display
    sf::Font font;
    if (!font.loadFromFile(resourcePath() + "sansation.ttf")) {
        return EXIT_FAILURE;
    }
    sf::Text text("Hello SFML", font, 50);
    text.setFillColor(sf::Color::Black);

    // Load a music to play
    sf::Music music;
    if (!music.openFromFile(resourcePath() + "nice_music.ogg")) {
        return EXIT_FAILURE;
    }

    // Play the music
    music.play();

    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Escape pressed: exit
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
        }

        // Clear screen
        window.clear();

        // Draw the sprite
        window.draw(sprite);

        // Draw the string
        window.draw(text);

        // Update the window
        window.display();
    }

    return EXIT_SUCCESS;
}
