//
// Created by benoit on 18/11/03.
//

#pragma once

#include <cstdint>
#include <array>
#include <functional>
#include <unordered_map>
#include <vector>
#include <random>
#include "decoder.h"

namespace snooz {

/// https://en.wikipedia.org/wiki/CHIP-8#Virtual_machine_description
class Chip8 {
public:
    Chip8();

    void load_game(std::string source);
    void load_from_buffer(const std::vector<uint8_t>& buff);

    void emulateCycle();

    bool should_continue() const;

    const std::array<std::uint8_t, 64*32> gfx() const { return gfx_;}

    bool draw_flag() const;
    void set_draw_flag(bool draw_flag);

    std::string print_state();
    std::uint8_t register_value(size_t index) const;

    // Keyboard control. Either press or release. Press will put to 1, release to 0.
    void set_key_pressed(const size_t& key_index);
    void set_key_released(const size_t& key_index);
    // timers are decreased in the main loop
    void decrease_timers();

protected:

    constexpr static std::uint8_t chip8_fontset[80] = {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    void next_opcode();
    bool should_continue_{true};

    // Look at the first byte of the opcode and dispatch the correct function.
    std::unordered_map<uint16_t, std::function<void ()>> opcode_dispath_;

    // for opcode such as 0x8...
    std::unordered_map<uint16_t, std::function<void ()>> arithmetic_dispath_;

    // for 0xF...
    std::unordered_map<uint16_t, std::function<void ()>> input_dispatch_;

    // --------------------------------------------------------------------
    // for opcodes.
    // --------------------------------------------------------------------
    // routing 0000 codes
    void op_0000();
    // Flow control - return from a subroutine
    // 00EE     Flow    return;     Returns from a subroutine. 
    void op_00EE();

    // ANNN 	MEM 	I = NNN 	Sets I to the address NNN.
    void op_ANNN();

    // Jump to NNN
    // 1NNN     Flow    goto NNN;   Jumps to address NNN. 
    void op_1NNN();

    // Will execute the subroutine at address NNN. 16 subroutines call max.
    // 2NNN 	Flow 	*(0xNNN)() 	Calls subroutine at NNN.
    void op_2NNN();

    // 3XNN 	Cond 	if(Vx==NN) 	Skips the next instruction if VX equals NN.
    // (Usually the next instruction is a jump to skip a code block)
    void op_3XNN();

    // 4XNN 	Cond 	if(Vx!=NN) 	Skips the next instruction if VX doesn't equal NN.
    // (Usually the next instruction is a jump to skip a code block)
    void op_4XNN();

    // 6XNN 	Const 	Vx = NN 	Sets VX to NN.
    void op_6XNN();

    // 7XNN     Const   Vx += NN    Adds NN to VX. (Carry flag is not changed) 
    void op_7XNN();

    // Arithmetic
    void op_8000();
    // 8XY0 	Assign 	Vx=Vy 	Sets VX to the value of VY.
    void op_8xy0();
    // 8XY1 	BitOp 	Vx=Vx|Vy 	Sets VX to VX or VY. (Bitwise OR operation)
    void op_8xy1();
    // 8XY2 	BitOp 	Vx=Vx&Vy 	Sets VX to VX and VY. (Bitwise AND operation)
    void op_8xy2();
    // 8XY3 	BitOp 	Vx=Vx^Vy 	Sets VX to VX xor VY.
    void op_8xy3();
    // 8XY4 	Math 	Vx += Vy 	Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
    void op_8xy4();
    // 8XY5 	Math 	Vx -= Vy 	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
    void op_8xy5();
    // 8XY6 	BitOp 	Vx>>=1 	Stores the least significant bit of VX in VF and then shifts VX to the right by 1.[2]
    void op_8xy6();
    // 8XY7 	Math 	Vx=Vy-Vx 	Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
    void op_8xy7();
    // 8XYE 	BitOp 	Vx<<=1 	Stores the most significant bit of VX in VF and then shifts VX to the left by 1.[3]
    void op_8xyE();

    // CXNN     Rand    Vx=rand()&NN    Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN. 
    void op_CXNN();

    // DXYN 	Disp 	draw(Vx,Vy,N) 	Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels
    // and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I;
    // I value doesn’t change after the execution of this instruction. As described above,
    // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
    // and to 0 if that doesn’t happen
    void op_DXYN();

    // F000 operations...
    void op_F000();

    // Timers
    // FX07     Timer   Vx = get_delay()    Sets VX to the value of the delay timer. 
    void op_FX07();

    // FX15     Timer   delay_timer(Vx)     Sets the delay timer to VX.
    void op_FX15();
    // FX18     Sound   sound_timer(Vx)     Sets the sound timer to VX.
    void op_FX18();

    // --------------------------------------------------------------------
    // state
    // --------------------------------------------------------------------

    // random number generator
    std::random_device r_{};

    // Choose a random mean between 1 and 6
    std::default_random_engine e1;
    std::uniform_int_distribution<int> uniform_dist{0, 0xFF};

    // 2bytes opcode
    std::uint16_t opcode_;

    // 4096 bytes memory
    // Layout is the following: 0x000 -> 0x200 interpreter. 0x200 -> 0xfff the program
    std::array<std::uint8_t, 4096> memory_;

    // CPU registers. last one is for carry flag for arithmetic
    std::array<std::uint8_t, 16> V_;

    // index register and program counter
    std::uint16_t I_{0};
    std::uint16_t pc_;

    // display
    std::array<std::uint8_t, 64*32> gfx_;

    // Interupts and hardware registers. The Chip 8 has none, but there are two timer registers that count at 60 Hz. When set above zero they will count down to zero.
    std::uint8_t delay_timer_{0};
    std::uint8_t sound_timer_{0};

    // when calling subroutines.
    std::array<std::uint16_t, 16> stack_;
    std::uint16_t sp_{0};

    // Hex-based keypad.
    std::array<bool, 0xF> key_;

    bool draw_flag_{false};
    
    Decoder decoder_;
};
}



