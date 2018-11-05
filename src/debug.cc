//
// Created by benoit on 18/11/03.
//
#include "decoder.h"

int main() {
    snooz::Decoder decoder;
    decoder.load_game("../../games/MAZE");
    decoder.decode();
}