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

    std::string interpret(std::uint16_t opcode);
private:

    void next_opcode();

    // Look at the first byte of the opcode and dispatch the correct function.
    std::unordered_map<uint16_t, std::function<std::string(std::uint16_t) >> opcode_dispath_;

    // for opcode such as 0x8...
    std::unordered_map<uint16_t, std::function<std::string(std::uint16_t) >> arithmetic_dispath_;

    std::string unknown() const;
    std::string print_with_desc(std::uint16_t opcode, const std::string& msg) const;



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
    std::string op_0000(std::uint16_t opcode) const;
    std::string op_00E0(std::uint16_t opcode) const;
    std::string op_00EE(std::uint16_t opcode) const;
    std::string op_1NNN(std::uint16_t opcode) const;
    std::string op_2NNN(std::uint16_t opcode) const;
    std::string op_3XNN(std::uint16_t opcode) const;
    std::string op_4XNN(std::uint16_t opcode) const;
    std::string op_5XY0(std::uint16_t opcode) const;

    /*
     * 6XNN 	Const 	Vx = NN 	Sets VX to NN.
       7XNN 	Const 	Vx += NN 	Adds NN to VX. (Carry flag is not changed)
     */
    std::string op_6XNN(std::uint16_t opcode) const;
    std::string op_7XNN(std::uint16_t opcode) const;

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
    std::string op_8000(std::uint16_t opcode);
    std::string op_8XY0(std::uint16_t opcode) const;
    std::string op_8XY1(std::uint16_t opcode) const;
    std::string op_8XY2(std::uint16_t opcode) const;
    std::string op_8XY3(std::uint16_t opcode) const;
    std::string op_8XY4(std::uint16_t opcode) const;
    std::string op_8XY5(std::uint16_t opcode) const;
    std::string op_8XY6(std::uint16_t opcode) const;
    std::string op_8XY7(std::uint16_t opcode) const;
    std::string op_8XYE(std::uint16_t opcode) const;

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
    std::string op_9XY0(std::uint16_t opcode) const;
    std::string op_ANNN(std::uint16_t opcode) const;
    std::string op_BNNN(std::uint16_t opcode) const;
    std::string op_CXNN(std::uint16_t opcode) const;
    std::string op_DXYN(std::uint16_t opcode) const;
};

}

