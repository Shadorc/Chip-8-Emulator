#pragma once
#include <string>

enum BitMask {
	NNN,	// 0000 XXXX XXXX XXXX (nnn or addr)
	N,		// 0000 0000 0000 XXXX (n or nibble)
	X,		// 0000 XXXX 0000 0000 (x)
	Y,		// 0000 0000 XXXX 0000 (y)
	KK,		// 0000 0000 XXXX XXXX (kk or byte)
	OP,		// XXXX 0000 0000 0000 (op code)
};

class Emulator
{
public:
	static const int DISPLAY_WIDTH = 64;
	static const int DISPLAY_HEIGHT = 32;
	static const int CPU_FREQUENCY = 700; // Original CPU was around 1MHz ~ 700op/s

	static const int MEMORY_SIZE = 4096;
	static const int STACK_SIZE = 16;

	uint8_t memory[MEMORY_SIZE];	// 0x000 to 0x1FF reserved for interpreter
	uint16_t i;						// memory address, only lowest 12 bits are used
	uint16_t stack[STACK_SIZE];		// store the address that the interpreter shoud return to when finished with a subroutine
	uint8_t sp;						// stack pointer, point to the topmost level of the stack
	uint8_t v[16];					// V[0xF] is used as a flag by some instructions
	uint8_t dt;						// delay timer, when non-zero, automatically decremented at a rate of 60Hz
	uint8_t st;						// sound timer, when non-zero, automatically decremented at a rate of 60Hz
	uint16_t pc;					// program counter, store the currently executing address

	uint16_t inputs_mask;			// 1 if the key corresponding to the bit index is pressed, 0 otherwise
	bool display[DISPLAY_WIDTH * DISPLAY_HEIGHT]; // true if pixel is black, false if white

	bool draw_flag;

	static uint16_t extract(uint16_t word, BitMask mask);

	Emulator();

	int init(const std::string program_path);
	bool cycle();

private:
	int read_program(const std::string& path);
	void init_sprites();

	uint16_t fetch_opcode() const;
	void decode_opcode(uint16_t word);

	void cls();
	void ret();
	void jp_addr(uint16_t nnn);
	void call_addr(uint16_t nnn);
	void se_vx_byte(uint16_t x, uint16_t kk);
	void sne_vx_byte(uint16_t x, uint16_t kk);
	void se_vx_vy(uint16_t x, uint16_t y);
	void ld_vx_byte(uint16_t x, uint16_t y);
	void add_vx_byte(uint16_t x, uint16_t kk);
	void ld_vx_vy(uint16_t x, uint16_t y);
	void or_vx_vy(uint16_t x, uint16_t y);
	void and_vx_vy(uint16_t x, uint16_t y);
	void xor_vx_vy(uint16_t x, uint16_t y);
	void add_vx_vy(uint16_t x, uint16_t y);
	void sub_vx_vy(uint16_t x, uint16_t y);
	void shr_vx_vy(uint16_t x, uint16_t y);
	void subn_vx_vy(uint16_t x, uint16_t y);
	void shl_vx_vy(uint16_t x, uint16_t y);
	void sne_vx_vy(uint16_t x, uint16_t y);
	void ld_i_addr(uint16_t nnn);
	void jp_v0_addr(uint16_t nnn);
	void rnd_vx_byte(uint16_t x, uint16_t kk);
	void drw_vx_vy_nibble(uint16_t x, uint16_t y, uint16_t n);
	void skp_vx(uint16_t x);
	void sknp_vx(uint16_t x);
	void ld_vx_dt(uint16_t x);
	void ld_vx_k(uint16_t x);
	void ld_dt_vx(uint16_t x);
	void ld_st_vx(uint16_t x);
	void add_i_vx(uint16_t x);
	void ld_f_vx(uint16_t x);
	void ld_b_vx(uint16_t x);
	void ld_i_vx(uint16_t x);
	void ld_vx_i(uint16_t x);
};
