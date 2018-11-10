//
// Created by benoit on 18/11/03.
//

#include <fstream>
#include <vector>
#include "chip_8.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace snooz {

constexpr std::uint8_t Chip8::chip8_fontset[80];

Chip8::Chip8():
        e1{r_()},
        pc_(0x200){ 

    // black screen at first
    for (auto& pixel: gfx_) pixel = 0;


    // Load fontset
    for(int i = 0; i < 80; ++i)
        memory_[i] = chip8_fontset[i];

    opcode_dispath_ = {
            {0x0000, [this] { op_0000();}},
            {0xA000, [this] { op_ANNN();}},
            {0x1000, [this] { op_1NNN();}},
            {0x2000, [this] { op_2NNN();}},
            {0x3000, [this] { op_3XNN();}},
            {0x4000, [this] { op_4XNN();}},
            {0x6000, [this] { op_6XNN();}},
            {0x7000, [this] { op_7XNN();}},
            {0x8000, [this] { op_8000();}},
            {0xC000, [this] { op_CXNN();}},
            {0xD000, [this] { op_DXYN();}},
            {0xF000, [this] { op_F000();}},
    };

    arithmetic_dispath_ = {
            {0x0000, [this] {op_8xy0();}},
            {0x0001, [this] {op_8xy1();}},
            {0x0002, [this] {op_8xy2();}},
            {0x0003, [this] {op_8xy3();}},
            {0x0004, [this] {op_8xy4();}},
            {0x0005, [this] {op_8xy5();}},
            {0x0006, [this] {op_8xy6();}},
            {0x0007, [this] {op_8xy7();}},
            {0x000E, [this] {op_8xyE();}},
    };

    input_dispatch_ = {
            {0x0007, [this] { op_FX07();}},
            {0x0015, [this] { op_FX15();}},
            {0x0018, [this] { op_FX18();}},
    };
}

void Chip8::decrease_timers() {
    if (delay_timer_ > 0) delay_timer_--;
    if (sound_timer_ > 0) sound_timer_--;
}

void Chip8::set_key_pressed(const size_t& index) {
    // No problem is overflow, std::array will scream.
    key_[index] = true;
}

void Chip8::set_key_released(const size_t& index) {
    // No problem is overflow, std::array will scream.
    key_[index] = false;
}
void Chip8::load_game(std::string source) {

    std::ifstream input(source, std::ios::binary);
    std::vector<std::uint8_t > v((std::istreambuf_iterator<char>(input)),
                         std::istreambuf_iterator<char>());
    assert(v.size() < memory_.size() - 512);

    for (size_t i = 0; i < v.size(); i++) {
        memory_[i+512] = v[i];
    }
}

bool Chip8::should_continue() const {
    return should_continue_;
}

void Chip8::emulateCycle() {
    next_opcode();
    if (opcode_dispath_.find(opcode_& 0xF000) != opcode_dispath_.end()) {
        opcode_dispath_[opcode_&0xF000]();
    } else {
        std::cerr << "Unknown opcode " << std::hex << opcode_ << std::endl;
    }
}

void Chip8::next_opcode() {
    opcode_ = (memory_[pc_] << 8) | memory_[pc_+1];
}

void Chip8::op_0000() {
    if ((opcode_ & 0x00FF) == 0x00EE) {
        op_00EE();
    } else {
        std::cerr << "Got 0xNNN...\n";
        pc_ += 2;
    }
}

// Flow control - return from a subroutine
// 00EE     Flow    return;     Returns from a subroutine. 
void Chip8::op_00EE() {
    // get the index from last stack and increase by 2 to jump to next instruction
    assert(sp_ > 0);
    pc_ = stack_[sp_-1]+2;
    sp_--;
}


void Chip8::op_ANNN() {
    I_ = opcode_ & 0x0FFF;
    pc_ += 2;
}

void Chip8::op_1NNN() {
    // just jump. Do not increase pc.
    pc_ = opcode_ & 0x0FFF;
}

void Chip8::op_2NNN() {
    // need to store the current pc.
    stack_[sp_] = pc_;
    sp_++;
    pc_ = opcode_ & 0x0FFF;

    // no need to increase pc here as we want to execute the next one.
}

void Chip8::op_3XNN() {
    //3XNN 	Cond 	if(Vx==NN) 	Skips the next instruction if VX equals NN.
    // (Usually the next instruction is a jump to skip a code block)

    auto constant = opcode_ & 0x00FF;
    if (V_[(opcode_ & 0x0F00) >> 8] == constant) {
        pc_ += 4;
    } else {
        pc_ += 2;
    }
}

void Chip8::op_4XNN() {
    auto constant = opcode_ & 0x00FF;
    if (V_[(opcode_ & 0x0F00) >> 8] != constant) {
        pc_ += 4;
    } else {
        pc_ += 2;
    }
}


// assignment :)
void Chip8::op_6XNN() {
    V_[(opcode_ & 0x0F00) >> 8] = opcode_ & 0xFF;
    pc_ += 2;
}

void Chip8::op_7XNN() {
    V_[(opcode_ & 0X0F00) >> 8] += opcode_ & 0xFF;
    pc_ += 2;
}

void Chip8::op_8000() {
    if (arithmetic_dispath_.find(opcode_& 0x000F) != arithmetic_dispath_.end()) {
        arithmetic_dispath_[opcode_&0x000F]();
    } else {
        std::cerr << "Unknown opcode " << std::hex << opcode_ << std::endl;
    }
}

void Chip8::op_8xy0() {

    auto X = (opcode_ & 0x0F00) >> 8;
    auto Y = (opcode_ & 0x00F0) >> 4;

    V_[X] = V_[Y];
    pc_++;
}

void Chip8::op_8xy1() {

}

void Chip8::op_8xy2() {

}

void Chip8::op_8xy3() {

}

void Chip8::op_8xy4() {


    auto X = (opcode_ & 0x0F00) >> 8;
    auto Y = (opcode_ & 0x00F0) >> 4;

    if (V_[X] > 0xFF - V_[Y]) {
        V_[0xF] = 1;
    } else {
        V_[0xF] = 0;
    }


    V_[X] += V_[Y];
    pc_ += 2;
}

void Chip8::op_8xy5() {

}

void Chip8::op_8xy6() {

}

void Chip8::op_8xy7() {

}

void Chip8::op_8xyE() {

}

void Chip8::op_CXNN() {
    // CXNN     Rand    Vx=rand()&NN    Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN. 
    auto rand = static_cast<std::uint8_t>(uniform_dist(e1) & 0xFF); 
    V_[(opcode_ & 0x0F00) >> 8] = rand & (opcode_ & 0x0FF);
    pc_ += 2;
}

void Chip8::op_DXYN() {
    // DXYN 	Disp 	draw(Vx,Vy,N)
    // Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
    // Each row of 8 pixels is read as bit-coded starting from memory location I;
    // I value doesn’t change after the execution of this instruction. As described above, VF is set
    // to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
    // and to 0 if that doesn’t happen

    auto x = V_[(opcode_ & 0x0F00) >> 8];
    auto y = V_[(opcode_ & 0x00F0) >> 4];
    auto height = opcode_ & 0x000F;

    V_[0xF] = 0;

    // (64)(64)(64).... 32 times.
    // index in the array is
    // 0 ...... 63
    // 64 ......     y * 64 + x
    // 128 ......
    for (int yline = 0; yline < height; yline++) {
        auto pixel = memory_[I_ + yline];

        for (int xline = 0; xline < 8; xline++) {

            // nice little trick. Try it.
            if ((pixel & (0x80 >> xline)) > 0) {
                // flipped from set ot unset.
                if (gfx_[x + xline + (y + yline) * 64] == 1) {
                    V_[0xF] = 1;
                }

                gfx_[x + xline + (y + yline) * 64] ^= 1;
            }

        }
    }
    draw_flag_ = true;
    pc_ += 2;
}

void Chip8::op_F000() {
    if (input_dispatch_.find(opcode_& 0x00FF) != input_dispatch_.end()) {
        input_dispatch_[opcode_&0x00FF]();
    } else {
        std::cerr << "Unknown opcode " << std::hex << opcode_ << std::endl;
    }

}

void Chip8::op_FX07() {
    V_[(opcode_ & 0x0F00) >> 8] = delay_timer_;
    pc_ += 2;
}
// FX15     Timer   delay_timer(Vx)     Sets the delay timer to VX.
void Chip8::op_FX15() {
    delay_timer_ = V_[(opcode_ & 0x0F00) >> 8]; 
    pc_ += 2;
}

// FX18     Sound   sound_timer(Vx)     Sets the sound timer to VX.
void Chip8::op_FX18() {
    sound_timer_ = V_[(opcode_ & 0x0F00) >> 8]; 
    pc_ += 2;
}


void Chip8::load_from_buffer(const std::vector<uint8_t> &buff) {
    for (int i = 0; i < buff.size(); i++) {
       memory_[512+i] = buff[i];
    }
}

bool Chip8::draw_flag() const {
    return draw_flag_;
}

void Chip8::set_draw_flag(bool flag) {
    draw_flag_ = flag;
}

std::string Chip8::print_state() {
    std::stringstream ss;
    ss << "I: " << I_ << '\n';
    ss << "Registers:\n";
    for (size_t i=0; i < V_.size(); i++) {
        ss << i << ": " << std::to_string(V_[i]) << " - ";
} 
ss << '\n' << "pc: " << pc_  << " - opcode: " << std::hex << opcode_ << '\t' << decoder_.interpret(opcode_);
    return ss.str();
}

std::uint8_t Chip8::register_value(size_t index) const {
    return V_[index];
}

// END OF NAMESPACE
}
