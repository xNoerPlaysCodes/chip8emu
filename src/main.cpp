#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <raylib.h>
#include <spdlog/spdlog.h>
#include <thread>

void raylib_spdlog_hook(int level, const char *text, va_list args) {
    spdlog::level::level_enum log_level = spdlog::level::trace;
    switch (level) {
        default: break;
        case LOG_DEBUG: log_level = spdlog::level::debug; break;
        case LOG_INFO: log_level = spdlog::level::info; break;
        case LOG_WARNING: log_level = spdlog::level::warn; break;
        case LOG_ERROR: log_level = spdlog::level::err; break;
        case LOG_FATAL: log_level = spdlog::level::critical; break;
    }

    char buf[4096] = {};
    std::vsnprintf(buf, 4096, text, args);

    spdlog::log(log_level, "{}", buf);
}


// std::atomic<bool> play_audio
// std::thread g_timer_decr_thread;

constexpr size_t fb_x = 64;
constexpr size_t fb_y = 32;

size_t scale = 20;

namespace binary_part {
    struct first {
        operator int() {
            return 0;
        }
    };

    struct second {
        operator int() {
            return 1;
        }
    };
}

using sprite_t = uint8_t[8];

constexpr sprite_t sprites[] = {
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

struct state_t {
    uint8_t mem[4096] = {};
    uint16_t stack[32] = {};

    uint16_t index {};
    uint16_t pc = 0x200;

    uint8_t v[16] = {};

    uint8_t sp {};
    std::atomic<uint8_t> dt {}, st {};

    bool fb[fb_x * fb_y] = {};
private:
    void write8(uint16_t addr, uint8_t value) noexcept {
        // TODO: MMIO
        mem[addr % 4096] = value;
    }

    uint8_t read8(uint16_t addr) const noexcept {
        // TODO: MMIO
        return mem[addr % 4096];
    }

    uint16_t read16(uint16_t addr) const noexcept {
        // TODO: MMIO
        return read8(addr) << 8 | read8(addr + 1);
    }

    uint8_t bitindex(uint8_t index, uint8_t byte) {
        return (byte >> (7 - index)) & 1;
    }

    void logic() noexcept {
        uint16_t instr = read16(pc);

        // p = part
        uint8_t p0 = nibble(binary_part::first{}, instr >> 8);
        uint8_t p1 = nibble(binary_part::second{}, instr >> 8);
        uint8_t p2 = nibble(binary_part::first{}, instr & 0xFF);
        uint8_t p3 = nibble(binary_part::second{}, instr & 0xFF);

        // b = byte
        uint8_t b0 = instr >> 8;
        uint8_t b1 = instr & 0xFF;

        uint16_t last_12_bits = (p1 << 8) | b1;

        bool auto_pc_increment = true;

        if (p0 == 0) {
            if (p2 == 0xE && p3 == 0) {
                memset(this->fb, 0, fb_x * fb_y);
            } else if (p2 == 0xE && p3 == 0xE) {
                pc = stack[sp--];
                auto_pc_increment = false;
            }
        } else if (p0 == 0x1) {
            pc = last_12_bits;
            auto_pc_increment = false;
        } else if (p0 == 0x6) {
            uint8_t value = (p2 << 4) | p3;
            uint8_t &tgt_reg = v[p1];
            tgt_reg = value;
        } else if (p0 == 0x7) {
            uint8_t value = (p2 << 4) | p3;
            uint8_t &tgt_reg = v[p1];
            tgt_reg += value;
        } else if (p0 == 0xA) {
            this->index = last_12_bits;
        } else if (p0 == 0xD) {
            uint8_t x = v[p1];
            uint8_t y = v[p2];
            uint8_t n = p3;

            v[0xf] = 0;
            for (uint16_t i = this->index; i < (this->index + n); ++i, ++y) {
                uint8_t byte = this->mem[i];
                for (uint8_t j = 0; j < 8; ++j, ++x) {
                    uint8_t bit = bitindex(j, byte);
                    size_t idx = (y % fb_y) * fb_x + (x % fb_x);
                    if (fb[idx] == 1 && bit == 1) {
                        this->v[0xf] = 1;
                    }
                    if ((fb[idx] == 1 && bit == 1) || (fb[idx] == 0 && bit == 0)) {
                        fb[idx] = 0;
                    } else if ((fb[idx] == 1 && bit == 0) || (fb[idx] == 0 && bit == 1)) {
                        fb[idx] = 1;
                    }
                }
                x = v[p1];
            }
        }
        else {
            std::cout << "Unhandled Instruction: ";
            std::cout << std::hex << (int) p0;
            std::cout << std::hex << (int) p1;
            std::cout << std::hex << (int) p2;
            std::cout << std::hex << (int) p3;
            std::cout << '\n';
        }

        std::cout << pc << '\n';

        if (auto_pc_increment)
            pc += 2;
    }
public:
    void init(const std::vector<uint8_t> &prog) noexcept {
        if (prog.size() > 0xFFF - 0x200) {
            std::cout << "Too big\n";
            return;
        }

        memcpy(mem + 0x200, prog.data(), prog.size());
        memcpy(mem, sprites, 5 * sizeof(sprites));
        // for (size_t i = 0; i < 0x80; i += 5) {
        //
        // }
    }

    uint8_t nibble(int part, uint8_t byte) {
        if (part == 0) {
            return byte >> 4;
        } else {
            return byte & 0xF;
        }
    } 

    void step() noexcept {
        static double last_time = GetTime();
        double now = GetTime();
        // if (now - last_time >= 0.1) {
        //     last_time = now;
            logic();
        // }
    }
};

int main(int argc, char **argv) {
    spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");
    SetTraceLogCallback(raylib_spdlog_hook);
    SetTraceLogLevel(LOG_WARNING);

    InitWindow(fb_x * scale, fb_y * scale, "chip8emu");

    SetTargetFPS(60); // it lines up with the  CHIP-8's intended speed which is also 60 hz so that's nice!

    state_t state;
    std::vector<uint8_t> data;
    {
        std::ifstream input("rom.ch8", std::ios::binary);

        input.seekg(0, std::ios::end);
        size_t sz = input.tellg();
        input.seekg(0, std::ios::beg);

        data.resize(sz);

        input.read(reinterpret_cast<char*>(data.data()), sz);
    }

    state.init(data);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        {
            for (size_t y = 0; y < fb_y; ++y) {
                for (size_t x = 0; x < fb_x; ++x) {
                    DrawRectangle(x * scale, y * scale, scale, scale, state.fb[y * fb_x + x] == 0 ? WHITE : BLACK);
                }
            }

            state.step();
        }
        DrawFPS(10, 10);
        EndDrawing();
    }
}
