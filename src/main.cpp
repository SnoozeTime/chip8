#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include "chip_8.h"

using namespace snooz;

#include <SFML/Graphics.hpp>
#define DEBUG 1

constexpr int zoom = 10;
constexpr int WIDTH = 64 * zoom;
constexpr int HEIGHT = (32 + 10) * zoom;
constexpr int loop_delta_time = 17; //ms ~60Hz

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
void print_text(std::string text_str, sf::RenderWindow& window);

int main(int argc, char** argv)
{
    if (argc != 3) {
            std::cerr << "Usage: " << argv[0] << "(run|print) <SOURCE>\n";
            return -1;
    }

    std::string mode(argv[1]);
    std::string game(argv[2]);

    if (mode == "print") {
        Decoder decoder;
        decoder.load_game(game);
        decoder.decode();
        return 0;
    }

    setup_graphics();


    snooz::Chip8 chip8;
    chip8.load_game(game);

    std::vector<sf::RectangleShape> rectangles;
    //update_pixels(chip8, rectangles);
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "SFML works!");

    bool is_debug = false;
    std::string debug_text = "";
    while (window.isOpen())
    {
        
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();


                //Respond to key pressed events
            if (event.type == sf::Event::EventType::KeyPressed){
                if (event.key.code == sf::Keyboard::Space){
                    // Do something here
                    is_debug = !is_debug;
                    if (!is_debug) debug_text = "";
                }

                // Debug prints
                if (is_debug) {
                    switch (event.key.code) {
                    case sf::Keyboard::Num0:
                            debug_text = std::to_string(chip8.register_value(0x00));
                            break;
                    default:
                        break;
                    }
                }
            }
        }

        if (!is_debug) {
            chip8.emulateCycle();


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

#ifdef DEBUG
            print_text(chip8.print_state(), window);
#endif
            window.display();

            chip8.decrease_timers();
            std::this_thread::sleep_for(std::chrono::milliseconds(loop_delta_time));
        } else {
            window.clear();
            print_text(debug_text, window);
            window.display();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
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

void print_text(std::string text_str, sf::RenderWindow& window) {
    sf::Text text;

    text.setFont(FONT);
    text.setString(text_str);
    //
    // // set the character size
    text.setCharacterSize(14); // in pixels, not points!
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
