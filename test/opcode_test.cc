//
// Created by benoit on 18/11/03.
//

#include "chip_8.h"
#include <gtest/gtest.h>

class Chip8FreeAccess: public snooz::Chip8 {

public:

    const std::array<std::uint16_t, 16>& stacks() { return stack_; }
    std::uint16_t  sp() { return sp_;}
    std::uint16_t  pc() { return pc_;}
    const std::array<std::uint8_t, 16>& V() { return V_;}
    const uint8_t delay_timer() const { return delay_timer_;}
    const uint8_t sound_timer() const { return sound_timer_;}
    std::uint16_t I() {
        return I_;
    }
};

TEST(opcode, op_annn) {
    Chip8FreeAccess chip8;

    // 0xA2F0 - mvi 2F0h - move 2F0 in I
    std::vector<uint8_t> source{0xA2, 0xF0};
    chip8.load_from_buffer(source);
    chip8.emulateCycle();

    auto i = chip8.I();
    ASSERT_EQ(0x2F0, i);
    ASSERT_EQ(0x202, chip8.pc());
}

TEST(opcode, op_2nnn) {
    Chip8FreeAccess chip8;

    // 0x2204 - execute subroutine at index 204.
    std::vector<uint8_t> source {0x22, 0x04, 0xA2, 0xF0, 0xA2, 0xFF};
    chip8.load_from_buffer(source);
    chip8.emulateCycle();

    // stack should be one. pc would be stored as 0x200. Current pc is 0x204.
    ASSERT_EQ(1, chip8.sp());
    ASSERT_EQ(0x204, chip8.pc());
    ASSERT_EQ(0x200, chip8.stacks()[0]);

    // to confirm execute the next cycle
    chip8.emulateCycle();
    auto i = chip8.I();
    ASSERT_EQ(0x2FF, i);
}

TEST(opcode, return_sub_op_00ee) {

    Chip8FreeAccess chip8;

    // 0x2204 - execute subroutine at index 204.
    std::vector<uint8_t> source {
            0x22, 0x04, // execute 0xA2FF subroutine
            0xA2, 0xF0, 
            0xA2, 0xFF, // subroutine start.
            0x00, 0xEE};
    chip8.load_from_buffer(source);
    chip8.emulateCycle();

    // stack should be one. pc would be stored as 0x200. Current pc is 0x204.
    ASSERT_EQ(1, chip8.sp());
    ASSERT_EQ(0x204, chip8.pc());
    ASSERT_EQ(0x200, chip8.stacks()[0]);

    // to confirm execute the next cycle
    chip8.emulateCycle();
    auto i = chip8.I();
    ASSERT_EQ(0x2FF, i);

    chip8.emulateCycle(); // return;
    ASSERT_EQ(0, chip8.sp());
    ASSERT_EQ(0x202, chip8.pc());

    chip8.emulateCycle();
    i = chip8.I();
    ASSERT_EQ(0x2F0, i);
}

TEST(opcode, assign_6xNN) {
    Chip8FreeAccess chip8;

    // Assign 4 to V[1].
    std::vector<uint8_t> source {0x61, 0x04};
    chip8.load_from_buffer(source);
    chip8.emulateCycle();

    ASSERT_EQ(0x202, chip8.pc());
    ASSERT_EQ(4, chip8.V()[1]);

}

TEST(opcode, add_8xy4_no_carry) {
    // Add y to x and store in x.
    Chip8FreeAccess chip8;

    // Assign 4 to V[1], assign 6 to v[2], add V[1] to v[2] and store in v[1]
    std::vector<uint8_t> source {0x61, 0x04, 0x62, 0x06, 0x81, 0x24};
    chip8.load_from_buffer(source);
    chip8.emulateCycle();
    ASSERT_EQ(4, chip8.V()[1]);
    chip8.emulateCycle();
    ASSERT_EQ(6, chip8.V()[2]);
    chip8.emulateCycle();

    ASSERT_EQ(0x206, chip8.pc());
    ASSERT_EQ(10, chip8.V()[1]);
    ASSERT_EQ(0, chip8.V()[0xF]);
}

TEST(opcode, add_8xy4_carry) {
    // Add y to x and store in x.
    Chip8FreeAccess chip8;

    // Assign 4 to V[1], assign 6 to v[2], add V[1] to v[2] and store in v[1]
    std::vector<uint8_t> source {0x61, 0xF4, 0x62, 0x10, 0x81, 0x24};
    chip8.load_from_buffer(source);
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();

    ASSERT_EQ(0x206, chip8.pc());

    // 0xF4 + 0x10 = 260 -> should have 4 in the register + carry flag
    ASSERT_EQ(4, chip8.V()[1]);
    ASSERT_EQ(1, chip8.V()[0xF]);
}

TEST(opcode, draw_dxyn) {
    /*
     * HEX    BIN        Sprite
        0x3C   00111100     ****
        0xC3   11000011   **    **
        0xFF   11111111   ********
     */

    // The sprite is at location I. Heigh is 3.
    Chip8FreeAccess chip8;

    // x = 4, y = 6
    std::vector<uint8_t> source{0x61, 0x04, 0x62, 0x06};
    source.push_back(0xA2);
    source.push_back(0x08); // push 208 in I
    source.push_back(0xD1); // draw at coord x = v[1]
    source.push_back(0x23); // y = v[2], heigh = 3
    // encode the sprite here. Most likely that will be done in other place...
    source.push_back(0x3C);
    source.push_back(0xC3);
    source.push_back(0xFF);

    chip8.load_from_buffer(source);
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();
    ASSERT_EQ(0x208, chip8.I());

    // then draw
    chip8.emulateCycle();

    auto gfx = chip8.gfx();

}

TEST(opcode, cond_3xnn) {
    Chip8FreeAccess chip8;

    // x = 4, y = 6
    std::vector<uint8_t> source1{0x61, 0x04, 0x31, 0x04};
    chip8.load_from_buffer(source1);
    chip8.emulateCycle();
    chip8.emulateCycle();
    ASSERT_EQ(0x206, chip8.pc());


    std::vector<uint8_t> source2{0x61, 0x04, 0x31, 0x05};
    Chip8FreeAccess chip82;
    chip82.load_from_buffer(source2);
    chip82.emulateCycle();
    chip82.emulateCycle();
    ASSERT_EQ(0x204, chip82.pc());
}

TEST(opcode, cond_4xnn) {
    Chip8FreeAccess chip8;

    std::vector<uint8_t> source1{0x61, 0x04, 0x41, 0x04};
    chip8.load_from_buffer(source1);
    chip8.emulateCycle();
    chip8.emulateCycle();
    ASSERT_EQ(0x204, chip8.pc());


    std::vector<uint8_t> source2{0x61, 0x04, 0x41, 0x05};
    Chip8FreeAccess chip82;
    chip82.load_from_buffer(source2);
    chip82.emulateCycle();
    chip82.emulateCycle();
    ASSERT_EQ(0x206, chip82.pc());
}

TEST(opcode, add_constant_7xnn) {
    
    Chip8FreeAccess chip8;

    // add 4 to V[1] which is 4
    std::vector<uint8_t> source{0x61, 0x04, 0x71, 0x04};
    chip8.load_from_buffer(source);

    chip8.emulateCycle();
    chip8.emulateCycle();
    ASSERT_EQ(8, chip8.V()[1]);
}

TEST(opcode, jump_1NNN) {
    Chip8FreeAccess chip8;

    // 0x1204 - jump at index 204.
    std::vector<uint8_t> source {0x12, 0x04, 0xA2, 0xF0, 0xA2, 0xFF};
    chip8.load_from_buffer(source);
    chip8.emulateCycle();

    // stack should be one. pc would be stored as 0x200. Current pc is 0x204.
    ASSERT_EQ(0, chip8.sp());
    ASSERT_EQ(0x204, chip8.pc());

    // to confirm execute the next cycle
    chip8.emulateCycle();
    auto i = chip8.I();
    ASSERT_EQ(0x2FF, i);
}

// set the delay timer to VX
TEST(opcode, delay_FX15) {
    Chip8FreeAccess chip8;

    std::vector<uint8_t> source{0x61, 0x04, 0xF1, 0x15};
    chip8.load_from_buffer(source);

    chip8.emulateCycle();
    chip8.emulateCycle();
    ASSERT_EQ(0x04, chip8.delay_timer());
}

// set the sound timer to VX
TEST(opcode, sound_FX18) {

    Chip8FreeAccess chip8;

    std::vector<uint8_t> source{0x61, 0x04, 0xF1, 0x18};
    chip8.load_from_buffer(source);

    chip8.emulateCycle();
    chip8.emulateCycle();
    ASSERT_EQ(0x04, chip8.sound_timer());
}
// vx = delay timer
TEST(opcode, delay_FX07) {
    Chip8FreeAccess chip8;

    std::vector<uint8_t> source{0x61, 0x04, 0xF1, 0x15, 0xFB, 0x07};
    chip8.load_from_buffer(source);

    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.decrease_timers(); // delay timer will beb 0x03
    ASSERT_EQ(0x03, chip8.delay_timer());
    chip8.emulateCycle();
    ASSERT_EQ(0x03, chip8.V()[0xB]);
}

TEST(opcode, keyboard_notpressed_op_EXA1_keypressed) {
    // will skip next instrution if key stored in VX is pressed.
    Chip8FreeAccess chip8;

    std::vector<uint8_t> source{
            0x61, 0x04, // VX = 4
            0xE1, 0xA1, // skip if 4 is not pressed
            0x62, 0x01, // set v2 to 1
            0x62, 0x00}; // set V2 to 0
    chip8.load_from_buffer(source);

    chip8.emulateCycle();
    // do notpress
    chip8.set_key_pressed(4);
    chip8.emulateCycle();
    chip8.emulateCycle();
    ASSERT_EQ(0x01, chip8.V()[0x2]);
}

TEST(opcode, keyboard_notpressed_op_EXA1_keynotpressed) {
    Chip8FreeAccess chip8;

    std::vector<uint8_t> source{
            0x61, 0x04, // VX = 4
            0xE1, 0xA1, // skip if v1=4 is not pressed
            0x62, 0x01, // set v2 to 1
            0x62, 0x00}; // set V2 to 0
    chip8.load_from_buffer(source);
 
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();
    ASSERT_EQ(0x00, chip8.V()[0x2]);
}


TEST(opcode, keyboard_pressed_op_EX9E_keynotpressed) {
    // will skip next instrution if key stored in VX is pressed.
    Chip8FreeAccess chip8;

    std::vector<uint8_t> source{
            0x61, 0x04, // VX = 4
            0xE1, 0x9E, // skip if 4 is pressed
            0x62, 0x01, // set v2 to 1
            0x62, 0x00}; // set V2 to 0
    chip8.load_from_buffer(source);
 
    chip8.emulateCycle();
    // do notpress
    // chip8.set_key_pressed(4);
    chip8.emulateCycle();
    chip8.emulateCycle();
    ASSERT_EQ(0x01, chip8.V()[0x2]);
}

TEST(opcode, keyboard_pressed_op_EX9E_keypressed) {
    // will skip next instrution if key stored in VX is pressed.
    Chip8FreeAccess chip8;

    std::vector<uint8_t> source{
            0x61, 0x04, // VX = 4
            0xE1, 0x9E, // skip if 4 is pressed
            0x62, 0x01, // set v2 to 1
            0x62, 0x00}; // set V2 to 0
    chip8.load_from_buffer(source);
 
    chip8.emulateCycle();
    // press
    chip8.set_key_pressed(4);
    chip8.emulateCycle();
    chip8.emulateCycle();
    ASSERT_EQ(0x00, chip8.V()[0x2]);
}

TEST(opcode, wait_key_op_FX0A) {
    Chip8FreeAccess chip8;

    std::vector<uint8_t> source{
            0xF1, 0x0A, // wait for key and store value in V1
    };

    chip8.load_from_buffer(source);
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();
    chip8.emulateCycle();

    // key not pressed so nothing happens.
    ASSERT_EQ(0x200, chip8.pc());
    chip8.set_key_pressed(5);
    chip8.emulateCycle();
    ASSERT_EQ(0x202, chip8.pc());
    ASSERT_EQ(0x05, chip8.V()[0x01]);
}

// flow control
TEST(opcode, op_BNNN) {

    Chip8FreeAccess chip8;

    std::vector<uint8_t> source{
            0x60, 0x0A, // V0 = A
            0xB2, 0x00, // set pc = 200 + v0
            };
    chip8.load_from_buffer(source);

    chip8.emulateCycle();
    chip8.emulateCycle();
    ASSERT_EQ(0x20A, chip8.pc());
}
