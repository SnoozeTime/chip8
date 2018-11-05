//
// Created by benoit on 18/11/03.
//

#include <iostream>
#include "decoder.h"
#include <fstream>
#include <cassert>
#include <sstream>

namespace snooz {
void Decoder::unknown() {
    //std::hex
    std::cout << "Unknown Opcode 0x" << std::hex << opcode_ << '\n';
}

Decoder::Decoder() {
    memory_.fill(0);

    opcode_dispath_ = {
            {0x0000, [this] { op_0000(); }},
            {0x1000, [this] { op_1NNN(); }},
            {0x2000, [this] { op_2NNN(); }},
            {0x3000, [this] { op_3XNN(); }},
            {0x4000, [this] { op_4XNN(); }},
            {0x5000, [this] { op_5XY0(); }},
            {0x6000, [this] { op_6XNN(); }},
            {0x7000, [this] { op_7XNN(); }},
            {0x8000, [this] { op_8000(); }},
            {0x9000, [this] { op_9XY0(); }},
            {0xA000, [this] { op_ANNN(); }},
            {0xB000, [this] { op_BNNN(); }},
            {0xC000, [this] { op_CXNN(); }},
            {0xD000, [this] { op_DXYN(); }},

    };

    arithmetic_dispath_ = {
            {0x0000, [this] {op_8XY0();}},
            {0x0001, [this] {op_8XY1();}},
            {0x0002, [this] {op_8XY2();}},
            {0x0003, [this] {op_8XY3();}},
            {0x0004, [this] {op_8XY4();}},
            {0x0005, [this] {op_8XY5();}},
            {0x0006, [this] {op_8XY6();}},
            {0x0007, [this] {op_8XY7();}},
            {0x000E, [this] {op_8XYE();}},
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

        if (opcode_dispath_.find(opcode_ & 0xF000) != opcode_dispath_.end()) {
            opcode_dispath_[opcode_ & 0xF000]();
        } else {
            unknown();
        }

        pc_ += 2;
    }
}

void Decoder::next_opcode() {
    opcode_ = (memory_[pc_] << 8) | memory_[pc_ + 1];
}

void Decoder::op_0000() {

    if ((opcode_ & 0x000F) == 0) {
        op_00E0();
    } else {
        op_00EE();
    }

}


void Decoder::op_00E0() {
    print_with_desc("Clears the screen");
}

void Decoder::op_00EE() {
    print_with_desc("Return from subroutine");
}

void Decoder::op_1NNN() {
    std::stringstream ss;
    ss << "Jump to address 0x" << std::hex << (opcode_ & 0x0FFF);
    print_with_desc(ss.str());
}

void Decoder::op_2NNN() {
    std::stringstream ss;
    ss << "Call subroutine at 0x" << std::hex << (opcode_ & 0x0FFF);
    print_with_desc(ss.str());
}

void Decoder::op_3XNN() {
    // Skips the next instruction if VX equals NN
    std::stringstream ss;
    ss << "Skips next instruction if V" << ((opcode_ & 0x0F00) >> 8) << " equals " << (opcode_ & 0x00FF);
    print_with_desc(ss.str());
}

void Decoder::op_4XNN() {
    std::stringstream ss;
    ss << "Skips next instruction if V" << ((opcode_ & 0x0F00) >> 8) << " not equal to " << (opcode_ & 0x00FF);
    print_with_desc(ss.str());
}

void Decoder::op_5XY0() {
    std::stringstream ss;
    ss << "Skips next instruction if V" << ((opcode_ & 0x0F00) >> 8) << " equals V" << ((opcode_ & 0x00F0) >> 4);
    print_with_desc(ss.str());
}

void Decoder::print_with_desc(const std::string &msg) {
    std::cout << std::dec << pc_ << "\t[0x" << std::hex << opcode_ << "]\t" << msg << std::endl;

}

void Decoder::op_6XNN() {
    std::stringstream ss;
    ss << "Set V" << ((opcode_ & 0x0F00) >> 8) << " to" << (opcode_ & 0x00FF);
    print_with_desc(ss.str());
}

void Decoder::op_7XNN() {
    std::stringstream ss;
    ss << "Add " <<  (opcode_ & 0x00FF) << " to V" << ((opcode_ & 0x0F00) >> 8);
    print_with_desc(ss.str());
}

void Decoder::op_8000() {
    if (arithmetic_dispath_.find(opcode_& 0x000F) != arithmetic_dispath_.end()) {
        arithmetic_dispath_[opcode_&0x000F]();
    } else {
        unknown();
    }
}

void Decoder::op_8XY0() {
    //Vx=Vy
    std::stringstream ss;
    ss << "Assign: V" << ((opcode_ & 0x0F00) >> 8) << " = V" << ((opcode_ & 0x00F0) >> 4);
    print_with_desc(ss.str());
}

void Decoder::op_8XY1() {
    // Sets VX to VX or VY. (Bitwise OR operation)
    auto x = (opcode_ & 0x0F00) >> 8;
    auto y = (opcode_ & 0x00F0) >> 4;
    std::stringstream ss;
    ss << "Sets V" << x << " to V" << x << " or V" << y << ". (Bitwise OR operation)";
    print_with_desc(ss.str());
}

void Decoder::op_8XY2() {
    // Sets VX to VX and VY. (Bitwise AND operation)
    auto x = (opcode_ & 0x0F00) >> 8;
    auto y = (opcode_ & 0x00F0) >> 4;
    std::stringstream ss;
    ss << "Sets V" << x << " to V" << x << " and V" << y << ". (Bitwise AND operation)";
    print_with_desc(ss.str());
}

void Decoder::op_8XY3() {
    // Sets VX to VX xor VY.
    auto x = (opcode_ & 0x0F00) >> 8;
    auto y = (opcode_ & 0x00F0) >> 4;
    std::stringstream ss;
    ss << "Sets V" << x << " to V" << x << " xor V" << y << ". (Bitwise XOR operation)";
    print_with_desc(ss.str());
}

void Decoder::op_8XY4() {
    // Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
    auto x = (opcode_ & 0x0F00) >> 8;
    auto y = (opcode_ & 0x00F0) >> 4;
    std::stringstream ss;
    ss << "Adds V" << y << " to V" << x;
    print_with_desc(ss.str());
}

void Decoder::op_8XY5() {
    auto x = (opcode_ & 0x0F00) >> 8;
    auto y = (opcode_ & 0x00F0) >> 4;
    std::stringstream ss;
    ss << "V" << y << " is substracted from V" << x;
    print_with_desc(ss.str());
}

void Decoder::op_8XY6() {
    // Stores the least significant bit of VX in VF and then shifts VX to the right by 1.[2]
    auto x = (opcode_ & 0x0F00) >> 8;
    std::stringstream ss;
    ss << "Store the least significant bit of V" << x << " in VF and then shifts VX to the right by 1.";
    print_with_desc(ss.str());
}

void Decoder::op_8XY7() {
    // Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
    auto x = (opcode_ & 0x0F00) >> 8;
    auto y = (opcode_ & 0x00F0) >> 4;
    std::stringstream ss;
    ss << "Sets V" << x << " to V" << y << " minus V" << x;
    print_with_desc(ss.str());
}

void Decoder::op_8XYE() {
    // Stores the most significant bit of VX in VF and then shifts VX to the left by 1.[3]
    auto x = (opcode_ & 0x0F00) >> 8;
    std::stringstream ss;
    ss << "Store the most significant bit of V" << x << " in VF and then shifts VX to the left by 1.";
    print_with_desc(ss.str());
}

void Decoder::op_9XY0() {
    std::stringstream ss;
    ss << "Skips next instruction if V" << ((opcode_ & 0x0F00) >> 8) << " not equal to V" << ((opcode_ & 0x00F0) >> 4);
    print_with_desc(ss.str());
}

void Decoder::op_ANNN() {
    std::stringstream ss;
    ss << "Set I to " << (opcode_ & 0x0FFF);
    print_with_desc(ss.str());
}

void Decoder::op_BNNN() {
    std::stringstream ss;
    ss << "Jump to the address " << (opcode_ & 0x0FFF) << " + V0";
    print_with_desc(ss.str());
}

void Decoder::op_CXNN() {
    // Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
    auto x = (opcode_ & 0x0F00) >> 8;
    auto N = (opcode_ & 0x00FF);
    std::stringstream ss;
    ss << "Sets V" << x << " to the result of a bitwise and operation on a random number and " << N;
    print_with_desc(ss.str());
}

void Decoder::op_DXYN() {
    // Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels
    auto x = (opcode_ & 0x0F00) >> 8;
    auto y = (opcode_ & 0x00F0) >> 4;
    std::stringstream ss;
    ss << "Draw a sprite at coordinate (V" << x << ", V" << y << ")";
    print_with_desc(ss.str());
}

}