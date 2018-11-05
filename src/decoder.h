//
// Created by benoit on 18/11/03.
// Just a utility to print the Opcodes from binary. Redundant with our chip vm... should be able to refactor

#pragma once

#include <unordered_map>
#include <functional>
#include <vector>

namespace snooz {
class Decoder {
public:

    Decoder();

    void load_game(std::string source);

    void decode();

private:

    void next_opcode();

    // Look at the first byte of the opcode and dispatch the correct function.
    std::unordered_map<uint16_t, std::function<void()>> opcode_dispath_;

    // for opcode such as 0x8...
    std::unordered_map<uint16_t, std::function<void()>> arithmetic_dispath_;

    void unknown();
    void print_with_desc(const std::string& msg);



        // 2bytes opcode
    std::uint16_t opcode_;

    // 4096 bytes memory
    // Layout is the following: 0x000 -> 0x200 interpreter. 0x200 -> 0xfff the program
    std::array<std::uint8_t, 4096> memory_;
    std::uint16_t pc_{0x200};
    size_t length_{0};

    /*
     * 0NNN 	Call 		Calls RCA 1802 program at address NNN. Not necessary for most ROMs.
        00E0 	Display 	disp_clear() 	Clears the screen.
        00EE 	Flow 	return; 	Returns from a subroutine.
1NNN 	Flow 	goto NNN; 	Jumps to address NNN.
2NNN 	Flow 	*(0xNNN)() 	Calls subroutine at NNN.
3XNN 	Cond 	if(Vx==NN) 	Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block)
4XNN 	Cond 	if(Vx!=NN) 	Skips the next instruction if VX doesn't equal NN. (Usually the next instruction is a jump to skip a code block)
5XY0 	Cond 	if(Vx==Vy) 	Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block)
     */
    void op_0000();
    void op_00E0();
    void op_00EE();
    void op_1NNN();
    void op_2NNN();
    void op_3XNN();
    void op_4XNN();
    void op_5XY0();

    /*
     * 6XNN 	Const 	Vx = NN 	Sets VX to NN.
       7XNN 	Const 	Vx += NN 	Adds NN to VX. (Carry flag is not changed)
     */
    void op_6XNN();
    void op_7XNN();

    /*
     * 8XY0 	Assign 	Vx=Vy 	Sets VX to the value of VY.
8XY1 	BitOp 	Vx=Vx|Vy 	Sets VX to VX or VY. (Bitwise OR operation)
8XY2 	BitOp 	Vx=Vx&Vy 	Sets VX to VX and VY. (Bitwise AND operation)
8XY3 	BitOp 	Vx=Vx^Vy 	Sets VX to VX xor VY.
8XY4 	Math 	Vx += Vy 	Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
8XY5 	Math 	Vx -= Vy 	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
8XY6 	BitOp 	Vx>>=1 	Stores the least significant bit of VX in VF and then shifts VX to the right by 1.[2]
8XY7 	Math 	Vx=Vy-Vx 	Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
8XYE 	BitOp 	Vx<<=1 	Stores the most significant bit of VX in VF and then shifts VX to the left by 1.[3]
     */
    void op_8000();
    void op_8XY0();
    void op_8XY1();
    void op_8XY2();
    void op_8XY3();
    void op_8XY4();
    void op_8XY5();
    void op_8XY6();
    void op_8XY7();
    void op_8XYE();

    /*
     * 9XY0 	Cond 	if(Vx!=Vy) 	Skips the next instruction if VX doesn't equal VY.
     *  (Usually the next instruction is a jump to skip a code block)
ANNN 	MEM 	I = NNN 	Sets I to the address NNN.
BNNN 	Flow 	PC=V0+NNN 	Jumps to the address NNN plus V0.
CXNN 	Rand 	Vx=rand()&NN 	Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
DXYN 	Disp 	draw(Vx,Vy,N) 	Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
            Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn’t change after the execution
            of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the
            sprite is drawn, and to 0 if that doesn’t happen
     */
    void op_9XY0();
    void op_ANNN();
    void op_BNNN();
    void op_CXNN();
    void op_DXYN();
};

}

