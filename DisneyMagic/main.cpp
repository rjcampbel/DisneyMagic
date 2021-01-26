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

static const std::string home_api_url {"https://cd-static.bamgrid.com/dp-117731241344/home.json"};

static std::string get_home_api()
{
    std::string home_api_contents;
    curlhelpers::retrieve_file_from_URL(home_api_url, home_api_contents);
    return home_api_contents;
}

static void populate_default_containers(
    const std::string& home_api_contents,
    sf::RenderWindow& window,
    const sf::Font& font,
    std::vector<disneymagic::Container>& containers)
{
    rapidjson::Document api_doc;
    api_doc.Parse(home_api_contents.c_str());

    const auto& container_array = api_doc["data"]["StandardCollection"]["containers"].GetArray();
    containers.reserve(container_array.Size());

    disneymagic::ContainerFactory container_factory(window, font, image_width, image_height);
    std::transform(
        container_array.begin(),
        container_array.begin() + std::min(max_row_count, (size_t)container_array.Size()),
        std::back_inserter(containers),
        container_factory);
}

static void initialize_display(sf::RenderWindow& window, sf::Font& font)
{
    window.create(sf::VideoMode(1600, 1200), "Disney+");

    sf::Image icon;
    if (!icon.loadFromFile(resourcePath() + "DisneyPlus.png"))
    {
        std::cout << "Failed to load icon image";
    }
    else
    {
        window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    }

    if (!font.loadFromFile(resourcePath() + "Avenir.ttc"))
    {
        throw std::runtime_error("Failed to load font");
    }
}

int main()
{
    sf::RenderWindow window;
    sf::Font font;
    std::vector<disneymagic::Container> containers;

    try
    {
        initialize_display(window, font);
        populate_default_containers(get_home_api(), window, font, containers);
    }
    catch(std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        std::cout << "Unknown error" << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<int> first_item_index_per_row(containers.size(), 0);
    int cursor_position { 0 };
    int first_container_index { 0 };

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
                            if (first_container_index > 0)
                            {
                                --first_container_index;
                            }
                        }
                        std::cout << "Up: " << first_container_index << std::endl;
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
                            if (first_container_index + max_row_count < containers.size())
                            {
                                ++first_container_index;
                            }
                        }
                        std::cout << "Down: " << first_container_index << std::endl;
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
        for (size_t container_index = first_container_index; container_index < first_container_index + max_row_tile_count; ++container_index)
        {
            auto& container = containers.at(container_index);
            double container_row { row_offset + row_index * row_width };

            // Render the title for current row
            sf::Text container_text(container.GetTitle().c_str(), font, font_size);
            container_text.setFillColor(sf::Color::White);
            container_text.setPosition(column_offset, container_row);
            window.draw(container_text);

            // Render the tiles and selection cursor
            for (size_t tile_index = 0; tile_index < std::min(max_row_tile_count, container.GetItemCount()); ++tile_index)
            {
                double tile_row { container_row + font_size + 10 };
                double tile_column { column_offset + tile_index * column_width };
                auto& item = container.GetItem(tile_index + first_item_index_per_row[container_index]);

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
