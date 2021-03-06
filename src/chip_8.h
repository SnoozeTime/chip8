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

    // for 0xE...
    std::unordered_map<uint16_t, std::function<void ()>> keyboard_dispatch_;;

    // --------------------------------------------------------------------
    // for opcodes.
    // --------------------------------------------------------------------
    // routing 0000 codes
    void op_0000();
    // Flow control - return from a subroutine
    // 00EE     Flow    return;     Returns from a subroutine. 
    void op_00EE();

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

    //  5XY0    Cond    if(Vx==Vy)  Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block)
    void op_5XY0();

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

    //   9XY0    Cond    if(Vx!=Vy)  Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block)
    void op_9XY0();

    // ANNN 	MEM 	I = NNN 	Sets I to the address NNN.
    void op_ANNN();

    // BNNN     Flow    PC=V0+NNN   Jumps to the address NNN plus V0. 
    void op_BNNN();

    // CXNN     Rand    Vx=rand()&NN    Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN. 
    void op_CXNN();

    // DXYN 	Disp 	draw(Vx,Vy,N) 	Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels
    // and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I;
    // I value doesn’t change after the execution of this instruction. As described above,
    // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
    // and to 0 if that doesn’t happen
    void op_DXYN();

    void op_E000();
    //  EX9E    KeyOp   if(key()==Vx)   Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block)
    void op_EX9E();
    //       EXA1    KeyOp   if(key()!=Vx)   Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block) 
    void op_EXA1();

    // F000 operations...
    void op_F000();

    // Timers
    // FX07     Timer   Vx = get_delay()    Sets VX to the value of the delay timer. 
    void op_FX07();

    // FX0A     KeyOp   Vx = get_key()  A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event) 
    void op_FX0A();
    // Only set when doing FX0A.
    bool wait_for_key_{false};
    bool key_pressed_{false};
    size_t key_pressed_idx_{0};

    // FX15     Timer   delay_timer(Vx)     Sets the delay timer to VX.
    void op_FX15();
    // FX18     Sound   sound_timer(Vx)     Sets the sound timer to VX.
    void op_FX18();

    // FX1E     MEM     I +=Vx  Adds VX to I.
    void op_FX1E();

    // set I to the address of the digit corresponding to value of V[X]
    // FX29     MEM     I=sprite_addr[Vx]   Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
    void op_FX29();

    // FX33     BCD     set_BCD(Vx);
    // *(I+0)=BCD(3);
    // *(I+1)=BCD(2);
    // *(I+2)=BCD(1);
    //  Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
    void op_FX33();

    // FX55    MEM     reg_dump(Vx,&I)     Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
    void op_FX55();

    // FX65     MEM     reg_load(Vx,&I)     Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
    void op_FX65();

    // helper to do bitwise op.
    std::uint8_t get_0X00(std::uint16_t opcode) const;
    std::uint8_t get_00Y0(std::uint16_t opcode) const;

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



