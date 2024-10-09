#include "emulator.h"
#include <conio.h>
#include <fstream>
#include <filesystem>
#include <iostream>

uint16_t Emulator::extract(uint16_t word, BitMask mask)
{
	switch (mask) {
	case NNN:
		return (word & 0xFFF);
	case N:
		return (word & 0xF);
	case X:
		return (word & 0xF00) >> 8;
	case Y:
		return (word & 0xF0) >> 4;
	case KK:
		return (word & 0xFF);
	case OP:
		return (word & 0xF000) >> 12;
	default:
		throw std::invalid_argument("unknown mask");
	}
}

Emulator::Emulator() :
	memory{ 0 },
	i(0x200),
	stack{ 0 },
	sp(0),
	v{ 0 },
	dt(0),
	st(0),
	pc(0x200),
	inputs_mask(0),
	display{ 0 },
	draw_flag(false)
{
}

int Emulator::init(const std::string program_path)
{
	std::srand(static_cast<unsigned int>(std::time(0)));
	init_sprites();
	return read_program(program_path);
}

bool Emulator::cycle()
{
	uint16_t opcode = fetch_opcode();
	pc += 2;

	decode_opcode(opcode);

	if (dt > 0) {
		--dt;
	}

	if (st > 0) {
		// Buzzing sound
		--st;
	}

	uint16_t next_opcode = fetch_opcode();
	return next_opcode != 0;
}

int Emulator::read_program(const std::string& program_path)
{
	std::ifstream input_file(program_path, std::ios_base::binary);

	if (input_file.fail()) {
		std::cerr << "Error opening program file: " << program_path << std::endl;
		return EXIT_FAILURE;
	}

	input_file.seekg(0, std::ios::end);
	auto file_size = input_file.tellg();
	input_file.seekg(0, std::ios::beg);

	// Load program at beginning of memory
	input_file.read(reinterpret_cast<char*>(memory + pc), file_size);
	input_file.close();

	if (input_file.fail()) {
		std::cerr << "Error reading program file: " << program_path << std::endl;
		return EXIT_FAILURE;
	}

	const auto& filename = std::filesystem::path(program_path).filename();
	std::cout << "Successfully read " << input_file.gcount() << " bytes from " << filename << std::endl;

	return EXIT_SUCCESS;
}

void Emulator::init_sprites()
{
	const uint8_t length = 80;
	uint8_t sprites[length] = {
		// '0'
		0xF0, 0x90, 0x90, 0x90, 0xF0,
		// '1'
		0x20, 0x60, 0x20, 0x20, 0x70,
		// '2'
		0xF0, 0x10, 0xF0, 0x80, 0xF0,
		// '3'
		0xF0, 0x10, 0xF0, 0x10, 0xF0,
		// '4'
		0x90, 0x90, 0xF0, 0x10, 0x10,
		// '5'
		0xF0, 0x80, 0xF0, 0x10, 0xF0,
		// '6'
		0xF0, 0x80, 0xF0, 0x90, 0xF0,
		// '7'
		0xF0, 0x10, 0x20, 0x40, 0x40,
		// '8'
		0xF0, 0x90, 0xF0, 0x90, 0xF0,
		// '9'
		0xF0, 0x90, 0xF0, 0x10, 0xF0,
		// 'A'
		0xF0, 0x90, 0xF0, 0x90, 0x90,
		// 'B'
		0xE0, 0x90, 0xE0, 0x90, 0xE0,
		// 'C'
		0xF0, 0x80, 0x80, 0x80, 0xF0,
		// 'D'
		0xE0, 0x90, 0x90, 0x90, 0xE0,
		// 'E'
		0xF0, 0x80, 0xF0, 0x80, 0xF0,
		// 'F'
		0xF0, 0x80, 0xF0, 0x80, 0x80,
	};

	for (int idx = 0; idx < length; ++idx) {
		memory[idx] = sprites[idx];
	}
}

uint16_t Emulator::fetch_opcode() const
{
	return memory[pc] << 8 | memory[pc + 1];
}

void Emulator::decode_opcode(uint16_t opcode)
{
# if _DEBUG
	std::cout << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << opcode << std::dec << " - ";
#endif

	draw_flag = false;

	switch (extract(opcode, BitMask::OP)) {
	case 0x0:
		switch (opcode) {
		case 0x00E0:
			cls();
			break;
		case 0x00EE:
			ret();
			break;
		default:
			throw std::invalid_argument("unknown opcode");
		}
		break;
	case 0x1:
		jp_addr(extract(opcode, BitMask::NNN));
		break;
	case 0x2:
		call_addr(extract(opcode, BitMask::NNN));
		break;
	case 0x3:
		se_vx_byte(extract(opcode, BitMask::X), extract(opcode, BitMask::KK));
		break;
	case 0x4:
		sne_vx_byte(extract(opcode, BitMask::X), extract(opcode, BitMask::KK));
		break;
	case 0x5:
		se_vx_vy(extract(opcode, BitMask::X), extract(opcode, BitMask::Y));
		break;
	case 0x6:
		ld_vx_byte(extract(opcode, BitMask::X), extract(opcode, BitMask::KK));
		break;
	case 0x7:
		add_vx_byte(extract(opcode, BitMask::X), extract(opcode, BitMask::KK));
		break;
	case 0x8:
		switch (extract(opcode, BitMask::N)) {
		case 0x0:
			ld_vx_vy(extract(opcode, BitMask::X), extract(opcode, BitMask::Y));
			break;
		case 0x1:
			or_vx_vy(extract(opcode, BitMask::X), extract(opcode, BitMask::Y));
			break;
		case 0x2:
			and_vx_vy(extract(opcode, BitMask::X), extract(opcode, BitMask::Y));
			break;
		case 0x3:
			xor_vx_vy(extract(opcode, BitMask::X), extract(opcode, BitMask::Y));
			break;
		case 0x4:
			add_vx_vy(extract(opcode, BitMask::X), extract(opcode, BitMask::Y));
			break;
		case 0x5:
			sub_vx_vy(extract(opcode, BitMask::X), extract(opcode, BitMask::Y));
			break;
		case 0x6:
			shr_vx_vy(extract(opcode, BitMask::X), extract(opcode, BitMask::Y));
			break;
		case 0x7:
			subn_vx_vy(extract(opcode, BitMask::X), extract(opcode, BitMask::Y));
			break;
		case 0xE:
			shl_vx_vy(extract(opcode, BitMask::X), extract(opcode, BitMask::Y));
			break;
		default:
			throw std::invalid_argument("unknown opcode");
		}
		break;
	case 0x9:
		sne_vx_vy(extract(opcode, BitMask::X), extract(opcode, BitMask::Y));
		break;
	case 0xA:
		ld_i_addr(extract(opcode, BitMask::NNN));
		break;
	case 0xB:
		jp_v0_addr(extract(opcode, BitMask::NNN));
		break;
	case 0xC:
		rnd_vx_byte(extract(opcode, BitMask::X), extract(opcode, BitMask::KK));
		break;
	case 0xD:
		drw_vx_vy_nibble(extract(opcode, BitMask::X), extract(opcode, BitMask::Y), extract(opcode, BitMask::N));
		break;
	case 0xE:
		switch (extract(opcode, BitMask::KK)) {
		case 0x9E:
			skp_vx(extract(opcode, BitMask::X));
			break;
		case 0xA1:
			sknp_vx(extract(opcode, BitMask::X));
			break;
		default:
			throw std::invalid_argument("unknown opcode");
		}
		break;
	case 0xF:
		switch (extract(opcode, BitMask::KK)) {
		case 0x07:
			ld_vx_dt(extract(opcode, BitMask::X));
			break;
		case 0x0A:
			ld_vx_k(extract(opcode, BitMask::X));
			break;
		case 0x15:
			ld_dt_vx(extract(opcode, BitMask::X));
			break;
		case 0x18:
			ld_st_vx(extract(opcode, BitMask::X));
			break;
		case 0x1E:
			add_i_vx(extract(opcode, BitMask::X));
			break;
		case 0x29:
			ld_f_vx(extract(opcode, BitMask::X));
			break;
		case 0x33:
			ld_b_vx(extract(opcode, BitMask::X));
			break;
		case 0x55:
			ld_i_vx(extract(opcode, BitMask::X));
			break;
		case 0x65:
			ld_vx_i(extract(opcode, BitMask::X));
			break;
		default:
			throw std::invalid_argument("unknown opcode");
		}
		break;
	default:
		throw std::invalid_argument("unknown opcode");
	}
}

// Clear the display.
void Emulator::cls()
{
#if _DEBUG
	std::cout << "CLS | Clear display" << std::endl;
#endif

	for (uint8_t y = 0; y < DISPLAY_HEIGHT; ++y) {
		for (uint8_t x = 0; x < DISPLAY_WIDTH; ++x) {
			display[x + y * DISPLAY_WIDTH] = 0;
		}
	}
}

// Return from a subroutine
void Emulator::ret()
{
#if _DEBUG
	std::cout << "RET | PC = " << stack[sp] << "; SP = " << (sp - 1) << std::endl;
#endif

	pc = stack[sp] + 2;
	--sp;
}

// Jump to location addr
void Emulator::jp_addr(uint16_t nnn)
{
#if _DEBUG
	std::cout << "JP | PC = " << nnn << std::endl;
#endif

	pc = nnn;
}

// Call subroutine at addr
void Emulator::call_addr(uint16_t nnn)
{
#if _DEBUG
	std::cout << "CALL | SP = " << static_cast<int>(sp + 1) << "; stack[SP] = " << (pc - 2) << "; PC = " << nnn << std::endl;
#endif

	++sp;
	stack[sp] = pc - 2;
	pc = nnn;
}

// Skip next instruction if Vx = kk
void Emulator::se_vx_byte(uint16_t x, uint16_t kk)
{
#if _DEBUG
	std::cout << "SE | Skip if V[" << x << "] (" << static_cast<int>(v[x]) << ") == " << kk << std::endl;
#endif

	if (v[x] == kk) {
		pc += 2;
	}
}

// Skip next instruction if Vx != kk
void Emulator::sne_vx_byte(uint16_t x, uint16_t kk)
{
#if _DEBUG
	std::cout << "SNE | Skip if V[" << x << "] (" << static_cast<int>(v[x]) << ") != " << kk << std::endl;
#endif

	if (v[x] != kk) {
		pc += 2;
	}
}

// Skip next instruction if Vx = Vy
void Emulator::se_vx_vy(uint16_t x, uint16_t y)
{
#if _DEBUG
	std::cout << "SNE | Skip if V[" << x << "] (" << static_cast<int>(v[x]) << ") == V[" << y << "] (" << static_cast<int>(v[y]) << ")" << std::endl;
#endif

	if (v[x] == v[y]) {
		pc += 2;
	}
}

// Set Vx = kk
void Emulator::ld_vx_byte(uint16_t x, uint16_t kk)
{
#if _DEBUG
	std::cout << "LD | V[" << x << "] = " << kk << std::endl;
#endif

	v[x] = static_cast<uint8_t>(kk);
}

// Set Vx = Vx + kk
void Emulator::add_vx_byte(uint16_t x, uint16_t kk)
{
#if _DEBUG
	std::cout << "ADD | V[" << x << "] = V[" << x << "] (" << static_cast<int>(v[x]) << ") + " << kk << std::endl;
#endif

	v[x] += kk;
}

// Set Vx = V
void Emulator::ld_vx_vy(uint16_t x, uint16_t y)
{
#if _DEBUG
	std::cout << "LD | V[" << x << "] = V[" << y << "] (" << static_cast<int>(v[y]) << ")" << std::endl;
#endif

	v[x] = v[y];
}

// Set Vx = Vx OR Vy
void Emulator::or_vx_vy(uint16_t x, uint16_t y)
{
#if _DEBUG
	std::cout << "OR | V[" << x << "] = V[" << x << "] (" << static_cast<int>(v[x]) << ") | V[" << y << "] (" << static_cast<int>(v[y]) << ")" << std::endl;
#endif

	v[x] |= v[y];
}

// Set Vx = Vx AND Vy
void Emulator::and_vx_vy(uint16_t x, uint16_t y)
{
#if _DEBUG
	std::cout << "AND | V[" << x << "] = V[" << x << "] (" << static_cast<int>(v[x]) << ") & V[" << y << "] (" << static_cast<int>(v[y]) << ")" << std::endl;
#endif

	v[x] &= v[y];
}

// Set Vx = Vx XOR Vy
void Emulator::xor_vx_vy(uint16_t x, uint16_t y)
{
#if _DEBUG
	std::cout << "XOR | V[" << x << "] = V[" << x << "] (" << static_cast<int>(v[x]) << ") ^ V[" << y << "] (" << static_cast<int>(v[y]) << ")" << std::endl;
#endif

	v[x] ^= v[y];
}

// Set Vx = Vx + Vy, set VF = carry
void Emulator::add_vx_vy(uint16_t x, uint16_t y)
{
#if _DEBUG
	std::cout << "ADD | V[" << x << "] = V[" << x << "] (" << static_cast<int>(v[x]) << ") + V[" << y << "] (" << static_cast<int>(v[y]) << ")" << std::endl;
#endif

	uint16_t result = v[x] + v[y];
	v[0xF] = result > 0xFF; // carry
	v[x] = (uint8_t)result;
}

// Set Vx = Vx - Vy, set VF = NOT borrow
void Emulator::sub_vx_vy(uint16_t x, uint16_t y)
{
#if _DEBUG
	std::cout << "SUB | V[" << x << "] = V[" << x << "] (" << static_cast<int>(v[x]) << ") - V[" << y << "] (" << static_cast<int>(v[y]) << ")" << std::endl;
#endif

	v[0xF] = v[x] > v[y]; // NOT borrow
	v[x] -= v[y];
}

// Set Vx = Vx SHR 1
void Emulator::shr_vx_vy(uint16_t x, uint16_t y)
{
#if _DEBUG
	std::cout << "SHR | V[15] = " << ((v[x] & 0x1) ? 1 : 0) << "; V[" << x << "] = V[" << x << "] / 2 = " << static_cast<int>(v[x] >> 1) << std::endl;
#endif

	v[0xF] = (v[x] & 0x1) ? 1 : 0;	// if the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0
	v[x] >>= 1;						// divide by 2
}

// Set Vx = Vy - Vx, set VF = NOT borrow
void Emulator::subn_vx_vy(uint16_t x, uint16_t y)
{
#if _DEBUG
	std::cout << "SUBN | V[" << x << "] = V[" << y << "] (" << static_cast<int>(v[y]) << ") - V[" << x << "] (" << static_cast<int>(v[x]) << ")" << std::endl;
#endif

	v[0xF] = v[x] < v[y];
	v[x] = v[y] - v[x];
}

// Set Vx = Vx SHL 1
void Emulator::shl_vx_vy(uint16_t x, uint16_t y)
{
#if _DEBUG
	std::cout << "SHL | V[15] = " << ((v[x] & 0x80) ? 1 : 0) << "; V[" << x << "] = V[" << x << "] (" << static_cast<int>(v[x]) << ") x 2" << std::endl;
#endif

	v[0xF] = (v[x] & 0x80) ? 1 : 0;	// if the most-significant bit of Vx is 1, then VF is set to 1, otherwise 0
	v[x] <<= 1;						// multiply by 2
}

// Skip next instruction if Vx != Vy
void Emulator::sne_vx_vy(uint16_t x, uint16_t y)
{
#if _DEBUG
	std::cout << "SNE | Skip if V[" << x << "] (" << static_cast<int>(v[x]) << ") != V[" << y << "] (" << static_cast<int>(v[y]) << ")" << std::endl;
#endif

	if (v[x] != v[y]) {
		pc += 2;
	}
}

// Set I = nnn
void Emulator::ld_i_addr(uint16_t nnn)
{
#if _DEBUG
	std::cout << "LD | I = " << nnn << std::endl;
#endif

	i = nnn;
}

// Jump to location nnn + V0
void Emulator::jp_v0_addr(uint16_t nnn)
{
#if _DEBUG
	std::cout << "JP | PC = " << nnn << " + V[0] (" << static_cast<int>(v[0]) << ")" << std::endl;
#endif

	pc = nnn + v[0];
}

// Set Vx = random byte AND kk
void Emulator::rnd_vx_byte(uint16_t x, uint16_t kk)
{
	uint8_t value = (rand() % 256) & kk;
#if _DEBUG
	std::cout << "RND | V[" << x << "] = " << static_cast<int>(value) << std::endl;
#endif

	v[x] = value;
}

// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
void Emulator::drw_vx_vy_nibble(uint16_t x, uint16_t y, uint16_t n)
{
#if _DEBUG
	std::cout << "DRW | START=(V[" << x << "], V[" << y << "]) = (" << static_cast<uint16_t>(v[x]) << "," << static_cast<uint16_t>(v[y]) << "); N = " << n << std::endl;
#endif

	uint16_t start_x_pos = v[x];
	uint16_t y_pos = v[y];

	v[0xF] = 0;

	for (uint16_t byte_idx = 0; byte_idx < n; ++byte_idx) {
		uint8_t curr_byte = memory[i + byte_idx];

		uint16_t x_pos = start_x_pos;
		for (uint8_t bit_idx = 0; bit_idx < 8; ++bit_idx) {
			if ((curr_byte & (0x80 >> bit_idx)) != 0) {
				uint16_t display_pos = x_pos + y_pos * DISPLAY_WIDTH;
				if (display[display_pos]) {
					v[0xF] = 1;
				}

				display[display_pos] ^= 1;
			}

			++x_pos;
			if (x_pos >= DISPLAY_WIDTH) {
				x_pos = 0;
			}
		}

		++y_pos;
		if (y_pos >= DISPLAY_HEIGHT) {
			y_pos = 0;
		}
	}

	draw_flag = true;
}

// Skip next instruction if key with the value of Vx is pressed
void Emulator::skp_vx(uint16_t x)
{
	bool key_pressed = static_cast<bool>(inputs_mask & (1 << v[x]));
#if _DEBUG
	std::cout << "SKP | Skip if " << x << " is pressed (" << key_pressed << ")" << std::endl;
#endif

	if (key_pressed) {
		pc += 2;
	}
}

// Skip next instruction if key with the value of Vx is not pressed
void Emulator::sknp_vx(uint16_t x)
{
	bool key_pressed = static_cast<bool>(inputs_mask & (1 << v[x]));
#if _DEBUG
	std::cout << "SKNP | Skip if " << x << " is not pressed (" << !key_pressed << ")" << std::endl;
#endif

	if (!key_pressed) {
		pc += 2;
	}
}

// Set Vx = delay timer value
void Emulator::ld_vx_dt(uint16_t x)
{
#if _DEBUG
	std::cout << "LD | V[" << x << "] = DT (" << static_cast<int>(dt) << ")" << std::endl;
#endif

	v[x] = dt;
}

// Wait for a key press, store the value of the key in Vx
void Emulator::ld_vx_k(uint16_t x)
{
#if _DEBUG
	std::cout << "LD | Wait for key press" << std::endl;
#endif

	if (!inputs_mask) {
		return;
	}

	for (uint8_t i = 15; i >= 0; --i) {
		if (inputs_mask & (1 << i)) {
			v[x] = i;
			return;
		}
	}
}

// Set delay timer = Vx
void Emulator::ld_dt_vx(uint16_t x)
{
#if _DEBUG
	std::cout << "LD | DT = V[" << x << "] (" << static_cast<int>(v[x]) << ")" << std::endl;
#endif

	dt = v[x];
}

// Set sound timer = Vx
void Emulator::ld_st_vx(uint16_t x)
{
#if _DEBUG
	std::cout << "LD | ST = V[" << x << "] (" << static_cast<int>(v[x]) << ")" << std::endl;
#endif

	st = v[x];
}

// Set I = I + Vx
void Emulator::add_i_vx(uint16_t x)
{
#if _DEBUG
	std::cout << "LD | I = I (" << static_cast<int>(i) << ") + V[" << x << "] (" << static_cast<int>(v[x]) << ")" << std::endl;
#endif

	i += v[x];
}

// Set I = location of sprite for digit Vx
void Emulator::ld_f_vx(uint16_t x)
{
#if _DEBUG
	std::cout << "LD | I = V[" << x << "] (" << static_cast<int>(v[x]) << ") x 5" << std::endl;
#endif

	i = v[x] * 5; // sprites are 5 bytes long
}

// Store BCD representation of Vx in memory locations I, I+1, and I+2
void Emulator::ld_b_vx(uint16_t x)
{
	uint8_t vx = v[x];
	uint8_t hundreds = vx / 100;
	uint8_t tens = (vx - hundreds * 100) / 10;
	uint8_t ones = vx - hundreds * 100 - tens * 10;

#if _DEBUG
	std::cout << "LD | MEM[" << i << "] = " << static_cast<int>(hundreds) << "; MEM[" << (i + 1) << "] = " << static_cast<int>(tens) << "; MEM[" << (i + 2) << "] = " << static_cast<int>(ones) << std::endl;
#endif

	memory[i] = hundreds;
	memory[i + 1] = tens;
	memory[i + 2] = ones;
}

// Store registers V0 through Vx in memory starting at location I
void Emulator::ld_i_vx(uint16_t x)
{
#if _DEBUG
	std::cout << "LD | ";
#endif

	for (int idx = 0; idx < x + 1; ++idx) {
		memory[i + idx] = v[idx];
#if _DEBUG
		std::cout << "MEM[" << (i + idx) << "] = V[" << idx << "] (" << static_cast<int>(v[idx]) << "); ";
#endif
	}

#if _DEBUG
	std::cout << std::endl;
#endif
}

// Read registers V0 through Vx from memory starting at location I
void Emulator::ld_vx_i(uint16_t x)
{
#if _DEBUG
	std::cout << "LD | ";
#endif

	for (int idx = 0; idx < x + 1; ++idx) {
		v[idx] = memory[i + idx];
#if _DEBUG
		std::cout << "V[" << idx << "] = MEM[" << (i + idx) << "] (" << static_cast<int>(memory[i + idx]) << "); ";
#endif
	}

#if _DEBUG
	std::cout << std::endl;
#endif
}



