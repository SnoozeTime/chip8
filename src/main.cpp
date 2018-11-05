#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include "chip_8.h"

using namespace snooz;

#include <SFML/Graphics.hpp>

constexpr int zoom = 10;
constexpr int WIDTH = 64 * zoom;
constexpr int HEIGHT = (32 + 10) * zoom;

void update_pixels(Chip8& chip, std::vector<sf::RectangleShape>& rectangles) {

    auto gfx = chip.gfx();

    std::array<std::array<bool, 64>, 32> pixel_state;
    for (int i = 0; i < pixel_state.size(); i++) {
        for (int col = 0; col < pixel_state[i].size(); col++) {
                pixel_state[i][col] = gfx[col + i*64];
        }
    }

    rectangles.clear();
    for (int row = 0; row < pixel_state.size(); row++) {
        for (int col = 0; col < pixel_state[row].size(); col++) {
            if (pixel_state[row][col]) {
                rectangles.emplace_back(sf::Vector2f(zoom, zoom));
                rectangles.back().setFillColor(sf::Color::White);
                rectangles.back().move(col * zoom, row * zoom);
            }
        }
    }

}

sf::Font FONT;

void setup_graphics();
void print_chip_state(Chip8& chip, sf::RenderWindow& window);

int main(int argc, char** argv)
{
    if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <SOURCE>\n";
            return -1;
    }
    setup_graphics();


    snooz::Chip8 chip8;
// x = 4, y = 6
    std::vector<uint8_t> source{0x61, 0x04, 0x62, 0x06};
    source.push_back(0xA0);
    source.push_back(0x00); // push 208 in I
    source.push_back(0xD1); // draw at coord x = v[1]
    source.push_back(0x25); // y = v[2], heigh = 3
    // encode the sprite here. Most likely that will be done in other place...
    source.push_back(0x3C);
    source.push_back(0xC3);
    source.push_back(0xFF);

    chip8.load_game(argv[1]);
    //chip8.load_from_buffer(source);
//    chip8.emulateCycle();
  //  chip8.emulateCycle();
   // chip8.emulateCycle();
    // then draw
   // chip8.emulateCycle();
    std::vector<sf::RectangleShape> rectangles;
    //update_pixels(chip8, rectangles);
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "SFML works!");

    while (window.isOpen())
    {
        
        chip8.emulateCycle();
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }


        window.clear();
        // Update the screen only if there is a change. Then redraw
        if (chip8.draw_flag()) {
            update_pixels(chip8, rectangles);
            // meh. save some cpu cycles.
            chip8.set_draw_flag(false);
        }
        for (auto& rectangle: rectangles) {
            window.draw(rectangle);
        }
        print_chip_state(chip8, window);
        window.display();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}

//int main() {
//
//    snooz::Chip8 emulator;
//    emulator.load_game("./MISSILE");
//
//    // main loop.
//    while (emulator.should_continue()) {
//
//    }
//
//    return 0;
//}
void setup_graphics() {
    assert(FONT.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"));
}

void print_chip_state(Chip8& chip, sf::RenderWindow& window) {
    std::string state = "Bonjour";
    sf::Text text;

    text.setFont(FONT);
    text.setString(state);
    //
    // // set the character size
    text.setCharacterSize(24); // in pixels, not points!
    //
    // // set the color
    text.setColor(sf::Color::White);
    //
    // // set the text style
    text.setStyle(sf::Text::Bold | sf::Text::Underlined);
    //
    // ...
    //
    // // inside the main loop, between window.clear() and window.display()
    text.move(0, HEIGHT - 30);
    window.draw(text);
}
