#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "ResourcePath.hpp"
#include "CurlHelpers.h"
#include "Container.h"
#include <iostream>
#include <string>
#include <rapidjson/document.h>
#include <algorithm>

static const size_t max_row_tile_count { 4 };
static const size_t max_row_count { 4 };
static const double row_offset { 10 };
static const double row_width { 250 };
static const double column_offset { 10 };
static const double column_width{ 335 };
static const double font_size { 24 };

// image width and height based on an aspect ration 1.78
static const double image_width { 310 };
static const double image_height { 174.22 };

// factor used to scale up the currently selected tile
static const sf::Vector2f kScaleEnhancementFactor(1.033f, 1.033f);

int main()
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(1600, 1200), "Disney+");

    // Set the Icon
    sf::Image icon;
    if (!icon.loadFromFile(resourcePath() + "DisneyPlus.png"))
    {
        std::cout << "Failed to load icon image";
    }
    else
    {
        window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    }

    sf::Font font;
    if (!font.loadFromFile(resourcePath() + "Avenir.ttc"))
    {
        return EXIT_FAILURE;
    }

    std::string homeApiUrl("https://cd-static.bamgrid.com/dp-117731241344/home.json");
    std::string homeApiContents;
    try
    {
        curlhelpers::retrieve_file_from_URL(homeApiUrl, homeApiContents);
    }
    catch (std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    rapidjson::Document apiDoc;
    apiDoc.Parse(homeApiContents.c_str());

    std::vector<disneymagic::Container> containers;
    containers.reserve(max_row_count);
    const auto& container_array = apiDoc["data"]["StandardCollection"]["containers"].GetArray();
    int row_index { 0 };
    for (; row_index < max_row_count; ++row_index)
    {
        const auto& collection_set = container_array[row_index]["set"];
        containers.emplace_back(collection_set, window, font, image_width, image_height);
    }

    int cursor_position { 0 };

    std::vector<int> first_item_index_per_row(containers.size(), 0);
    int first_collection_index { 0 };
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                    case sf::Keyboard::Escape:
                    {
                        window.close();
                        break;
                    }
                    case sf::Keyboard::Left:
                    {
                        if (cursor_position % max_row_tile_count > 0)
                        {
                            --cursor_position;
                        }
                        else
                        {
                            int row_index = cursor_position / max_row_tile_count;
                            if (first_item_index_per_row[row_index] > 0)
                            {
                                --first_item_index_per_row[row_index];
                            }
                        }
                        break;
                    }
                    case sf::Keyboard::Right:
                    {
                        if (cursor_position % max_row_tile_count < (max_row_tile_count - 1) )
                        {
                            ++cursor_position;
                        }
                        else
                        {
                            int row_index = cursor_position / max_row_tile_count;
                            int current_row_size = containers.at(row_index).GetItemCount();
                            if ((first_item_index_per_row[row_index] + max_row_tile_count) < current_row_size)
                            {
                                ++first_item_index_per_row[row_index];
                            }
                        }
                        break;
                    }
                    case sf::Keyboard::Up:
                    {
                        if (cursor_position / max_row_tile_count > 0)
                        {
                            cursor_position -= max_row_tile_count;
                        }
                        else
                        {
                            if (first_collection_index > 0)
                            {
                                --first_collection_index;
                            }
                        }
                        std::cout << "Up: " << first_collection_index << std::endl;
                        break;
                    }
                    case sf::Keyboard::Down:
                    {
                        int current_cursor_row = cursor_position / max_row_tile_count;
                        if (current_cursor_row < (max_row_count - 1))
                        {
                            cursor_position += max_row_tile_count;
                        }
                        else
                        {
                            if (first_collection_index + max_row_count < containers.size())
                            {
                                ++first_collection_index;
                            }
                        }
                        std::cout << "Down: " << first_collection_index << std::endl;
                        break;
                    }
                    default: break;
                }
            }
        }

        // Clear the display
        window.clear();

        // Render row titles, tiles, and cursor
        size_t row_index { 0 };
        for (size_t collection_index = first_collection_index; collection_index < first_collection_index + max_row_tile_count; ++collection_index)
        {
            auto& collection = containers.at(collection_index);
            double collection_row { row_offset + row_index * row_width };

            // Render the title for current row
            sf::Text collection_text(collection.GetTitle().c_str(), font, font_size);
            collection_text.setFillColor(sf::Color::White);
            collection_text.setPosition(column_offset, collection_row);
            window.draw(collection_text);

            // Render the tiles and selection cursor
            for (size_t tile_index = 0; tile_index < std::min(max_row_tile_count, collection.GetItemCount()); ++tile_index)
            {
                double tile_row { collection_row + font_size + 10 };
                double tile_column { column_offset + tile_index * column_width };
                auto& item = collection.GetItem(tile_index + first_item_index_per_row[collection_index]);

                if (cursor_position == row_index * max_row_tile_count + tile_index)
                {
                    item.EnhanceScale(kScaleEnhancementFactor);

                    sf::RectangleShape selection_rect;
                    selection_rect.setFillColor(sf::Color::Transparent);
                    selection_rect.setOutlineColor(sf::Color::White);
                    selection_rect.setOutlineThickness(5.0f);
                    sf::Vector2f rect_size(image_width * kScaleEnhancementFactor.x, image_height * kScaleEnhancementFactor.y);
                    selection_rect.setSize(rect_size);
                    selection_rect.setPosition(tile_column, tile_row);
                    window.draw(selection_rect);
                }
                else
                {
                    item.ResetScale();
                }

                item.Draw(sf::Vector2f(tile_column, tile_row));
            }
            row_index++;
        }

        // Update display
        window.display();
    }

    return EXIT_SUCCESS;
}
