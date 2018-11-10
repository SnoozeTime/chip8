//
// Created by benoit on 18/11/03.
//

#include <iostream>
#include "decoder.h"
#include <fstream>
#include <cassert>
#include <sstream>

namespace snooz {
std::string Decoder::unknown() const {
    //std::hex
    std::stringstream ss;
    ss << "Unknown Opcode 0x" << std::hex << opcode_;
    return ss.str();
}

Decoder::Decoder() {
    memory_.fill(0);

    opcode_dispath_ = {
            {0x0000, [this](std::uint16_t opcode) { return op_0000(opcode); }},
            {0x1000, [this](std::uint16_t opcode) { return op_1NNN(opcode); }},
            {0x2000, [this](std::uint16_t opcode) { return op_2NNN(opcode); }},
            {0x3000, [this](std::uint16_t opcode) { return op_3XNN(opcode); }},
            {0x4000, [this](std::uint16_t opcode) { return op_4XNN(opcode); }},
            {0x5000, [this](std::uint16_t opcode) { return op_5XY0(opcode); }},
            {0x6000, [this](std::uint16_t opcode) { return op_6XNN(opcode); }},
            {0x7000, [this](std::uint16_t opcode) { return op_7XNN(opcode); }},
            {0x8000, [this](std::uint16_t opcode) { return op_8000(opcode); }},
            {0x9000, [this](std::uint16_t opcode) { return op_9XY0(opcode); }},
            {0xA000, [this](std::uint16_t opcode) { return op_ANNN(opcode); }},
            {0xB000, [this](std::uint16_t opcode) { return op_BNNN(opcode); }},
            {0xC000, [this](std::uint16_t opcode) { return op_CXNN(opcode); }},
            {0xD000, [this](std::uint16_t opcode) { return op_DXYN(opcode); }},

    };

    arithmetic_dispath_ = {
            {0x0000, [this](std::uint16_t opcode) {return op_8XY0(opcode);}},
            {0x0001, [this](std::uint16_t opcode) {return op_8XY1(opcode);}},
            {0x0002, [this](std::uint16_t opcode) {return op_8XY2(opcode);}},
            {0x0003, [this](std::uint16_t opcode) {return op_8XY3(opcode);}},
            {0x0004, [this](std::uint16_t opcode) {return op_8XY4(opcode);}},
            {0x0005, [this](std::uint16_t opcode) {return op_8XY5(opcode);}},
            {0x0006, [this](std::uint16_t opcode) {return op_8XY6(opcode);}},
            {0x0007, [this](std::uint16_t opcode) {return op_8XY7(opcode);}},
            {0x000E, [this](std::uint16_t opcode) {return op_8XYE(opcode);}},
    };
}

void Decoder::load_game(std::string source) {
    std::ifstream input(source, std::ios::binary);
    assert(input.is_open());
    std::vector<std::uint8_t> v((std::istreambuf_iterator<char>(input)),
                                std::istreambuf_iterator<char>());
    assert(v.size() < memory_.size() - 512);

    for (size_t i = 0; i < v.size(); i++) {
        memory_[i + 512] = v[i];
    }

    length_ = v.size();
}

void Decoder::decode() {
    while (pc_ - 0x200 < length_) {
        next_opcode();
        std::cout << interpret(opcode_) << std::endl;
        pc_ += 2;
    }
}

std::string Decoder::interpret(std::uint16_t opcode) {
        if (opcode_dispath_.find(opcode & 0xF000) != opcode_dispath_.end()) {
            return opcode_dispath_[opcode & 0xF000](opcode);
        } else {
            return unknown();
        }
}

void Decoder::next_opcode() {
    opcode_ = (memory_[pc_] << 8) | memory_[pc_ + 1];
}

std::string Decoder::op_0000(std::uint16_t opcode) const {

    if ((opcode & 0x000F) == 0) {
        return op_00E0(opcode);
    } else {
        return op_00EE(opcode);
    }

}


std::string Decoder::op_00E0(std::uint16_t opcode) const {
    return print_with_desc(opcode, "Clears the screen");
}

std::string Decoder::op_00EE(std::uint16_t opcode) const {
    return print_with_desc(opcode, "Return from subroutine");
}

std::string Decoder::op_1NNN(std::uint16_t opcode) const {
    std::stringstream ss;
    ss << "Jump to address 0x" << std::hex << (opcode & 0x0FFF);
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_2NNN(std::uint16_t opcode) const {
    std::stringstream ss;
    ss << "Call subroutine at 0x" << std::hex << (opcode & 0x0FFF);
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_3XNN(std::uint16_t opcode) const {
    // Skips the next instruction if VX equals NN
    std::stringstream ss;
    ss << "Skips next instruction if V" << ((opcode & 0x0F00) >> 8) << " equals " << (opcode & 0x00FF);
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_4XNN(std::uint16_t opcode) const {
    std::stringstream ss;
    ss << "Skips next instruction if V" << ((opcode & 0x0F00) >> 8) << " not equal to " << (opcode & 0x00FF);
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_5XY0(std::uint16_t opcode) const {
    std::stringstream ss;
    ss << "Skips next instruction if V" << ((opcode & 0x0F00) >> 8) << " equals V" << ((opcode & 0x00F0) >> 4);
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::print_with_desc(std::uint16_t opcode, const std::string &msg) const {
    std::stringstream ss;
    ss << std::hex << pc_ << "\t[0x" << opcode << "]"<< "\t" << msg; 
    return ss.str();
}

std::string Decoder::op_6XNN(std::uint16_t opcode) const {
    std::stringstream ss;
    ss << "Set V" << ((opcode & 0x0F00) >> 8) << " to" << (opcode & 0x00FF);
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_7XNN(std::uint16_t opcode) const {
    std::stringstream ss;
    ss << "Add " <<  (opcode & 0x00FF) << " to V" << ((opcode & 0x0F00) >> 8);
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_8000(std::uint16_t opcode){
    if (arithmetic_dispath_.find(opcode & 0x000F) != arithmetic_dispath_.end()) {
        return arithmetic_dispath_[opcode & 0x000F](opcode);
    } else {
        return unknown();
    }
}

std::string Decoder::op_8XY0(std::uint16_t opcode) const {
    //Vx=Vy
    std::stringstream ss;
    ss << "Assign: V" << ((opcode & 0x0F00) >> 8) << " = V" << ((opcode & 0x00F0) >> 4);
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_8XY1(std::uint16_t opcode) const {
    // Sets VX to VX or VY. (Bitwise OR operation)
    auto x = (opcode & 0x0F00) >> 8;
    auto y = (opcode & 0x00F0) >> 4;
    std::stringstream ss;
    ss << "Sets V" << x << " to V" << x << " or V" << y << ". (Bitwise OR operation)";
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_8XY2(std::uint16_t opcode) const {
    // Sets VX to VX and VY. (Bitwise AND operation)
    auto x = (opcode & 0x0F00) >> 8;
    auto y = (opcode & 0x00F0) >> 4;
    std::stringstream ss;
    ss << "Sets V" << x << " to V" << x << " and V" << y << ". (Bitwise AND operation)";
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_8XY3(std::uint16_t opcode) const {
    // Sets VX to VX xor VY.
    auto x = (opcode & 0x0F00) >> 8;
    auto y = (opcode & 0x00F0) >> 4;
    std::stringstream ss;
    ss << "Sets V" << x << " to V" << x << " xor V" << y << ". (Bitwise XOR operation)";
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_8XY4(std::uint16_t opcode) const {
    // Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
    auto x = (opcode & 0x0F00) >> 8;
    auto y = (opcode & 0x00F0) >> 4;
    std::stringstream ss;
    ss << "Adds V" << y << " to V" << x;
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_8XY5(std::uint16_t opcode) const {
    auto x = (opcode & 0x0F00) >> 8;
    auto y = (opcode & 0x00F0) >> 4;
    std::stringstream ss;
    ss << "V" << y << " is substracted from V" << x;
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_8XY6(std::uint16_t opcode) const {
    // Stores the least significant bit of VX in VF and then shifts VX to the right by 1.[2]
    auto x = (opcode & 0x0F00) >> 8;
    std::stringstream ss;
    ss << "Store the least significant bit of V" << x << " in VF and then shifts VX to the right by 1.";
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_8XY7(std::uint16_t opcode) const {
    // Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
    auto x = (opcode & 0x0F00) >> 8;
    auto y = (opcode & 0x00F0) >> 4;
    std::stringstream ss;
    ss << "Sets V" << x << " to V" << y << " minus V" << x;
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_8XYE(std::uint16_t opcode) const {
    // Stores the most significant bit of VX in VF and then shifts VX to the left by 1.[3]
    auto x = (opcode & 0x0F00) >> 8;
    std::stringstream ss;
    ss << "Store the most significant bit of V" << x << " in VF and then shifts VX to the left by 1.";
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_9XY0(std::uint16_t opcode) const {
    std::stringstream ss;
    ss << "Skips next instruction if V" << ((opcode & 0x0F00) >> 8) << " not equal to V" << ((opcode & 0x00F0) >> 4);
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_ANNN(std::uint16_t opcode) const {
    std::stringstream ss;
    ss << "Set I to " << (opcode & 0x0FFF);
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_BNNN(std::uint16_t opcode) const {
    std::stringstream ss;
    ss << "Jump to the address " << (opcode & 0x0FFF) << " + V0";
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_CXNN(std::uint16_t opcode) const {
    // Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
    auto x = (opcode & 0x0F00) >> 8;
    auto N = (opcode & 0x00FF);
    std::stringstream ss;
    ss << "Sets V" << x << " to the result of a bitwise and operation on a random number and " << N;
    return print_with_desc(opcode, ss.str());
}

std::string Decoder::op_DXYN(std::uint16_t opcode) const {
    // Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels
    auto x = (opcode & 0x0F00) >> 8;
    auto y = (opcode & 0x00F0) >> 4;
    std::stringstream ss;
    ss << "Draw a sprite at coordinate (V" << x << ", V" << y << ")";
    return print_with_desc(opcode, ss.str());
}

}
