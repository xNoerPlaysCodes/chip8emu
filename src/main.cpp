#include "beep.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <random>
#include <raylib.h>
#include <spdlog/spdlog.h>
#include <string>

#include "rlImGui.h"
#include "imgui.h"
#ifndef IMGUI_API
#define IMGUI_API
#endif
#include "misc/cpp/imgui_stdlib.h"

#include <cassert>

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


// std::<bool> play_audio
// std::thread g_timer_decr_thread;

constexpr size_t fb_x = 64;
constexpr size_t fb_y = 32;

size_t scale = 20;

namespace binary_part {
    constexpr int first = 0;
    constexpr int second = 1;
}

using sprite_t = uint8_t[8];

uint8_t rand_u8() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint8_t> distrib;
    return distrib(gen);
}

bool check_key(uint8_t key_index) {
    static constexpr KeyboardKey keymap[16] = {
        KEY_ZERO,
        KEY_ONE,
        KEY_TWO,
        KEY_THREE,
        KEY_FOUR,
        KEY_FIVE,
        KEY_SIX,
        KEY_SEVEN,
        KEY_EIGHT,
        KEY_NINE,
        KEY_A,
        KEY_B,
        KEY_C,
        KEY_D,
        KEY_E,
        KEY_F
    };

    return IsKeyDown(keymap[key_index]);
}

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

    uint8_t sp = 0;
    uint8_t dt {}, st {};

    bool waiting_for_key = false;
    uint8_t key_in_reg = 255;

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
        if (waiting_for_key) {
            for (uint8_t i = 0; i < 16; ++i) {
                if (check_key(i)) {
                    v[key_in_reg] = i;
                    waiting_for_key = false;
                    break;
                }
            }

            return;
        }

        uint16_t instr = read16(pc);
        pc += 2;

        // p = part
        uint8_t p0 = nibble(binary_part::first, instr >> 8);
        uint8_t p1 = nibble(binary_part::second, instr >> 8);
        uint8_t p2 = nibble(binary_part::first, instr & 0xFF);
        uint8_t p3 = nibble(binary_part::second, instr & 0xFF);

        // b = byte
        uint8_t b0 = instr >> 8;
        uint8_t b1 = instr & 0xFF;

        uint16_t last_12_bits = (p1 << 8) | b1;

        if (p0 == 0) {
            if (p2 == 0xE && p3 == 0) {
                memset(this->fb, 0, fb_x * fb_y);
            } else if (p2 == 0xE && p3 == 0xE) {
                pc = stack[--sp];
            }
        } else if (p0 == 0x1) {
            pc = last_12_bits;
        } else if (p0 == 0x2) {
            stack[sp++] = pc;
            pc = last_12_bits;
        } else if (p0 == 0x3) {
            uint8_t reg = p1;
            uint8_t val = b1;

            if (v[reg] == val) {
                pc += 2;
            }
        } else if (p0 == 0x4) {
            uint8_t reg = p1;
            uint8_t val = b1;

            if (v[reg] != val) {
                pc += 2;
            }
        } else if (p0 == 0x5) {
            uint8_t r1 = p1;
            uint8_t r2 = p2;

            if (v[r1] == v[r2]) {
                pc += 2;
            }
        } else if (p0 == 0x6) {
            uint8_t value = (p2 << 4) | p3;
            uint8_t &tgt_reg = v[p1];
            tgt_reg = value;
        } else if (p0 == 0x7) {
            uint8_t value = (p2 << 4) | p3;
            uint8_t &tgt_reg = v[p1];
            tgt_reg += value;
        } else if (p0 == 0x8) {
            uint8_t &vx = v[p1];
            uint8_t &vy = v[p2];
            if (p3 == 0) {
                vx = vy;
            } else if (p3 == 1) {
                vx |= vy;
            } else if (p3 == 2) {
                vx &= vy;
            } else if (p3 == 3) {
                vx ^= vy;
            } else if (p3 == 4) {
                vx += vy;
            } else if (p3 == 5) {
                v[0xf] = vx >= vy;
                vx = vx - vy;
            } else if (p3 == 6) {
                v[0xf] = vx & 1;
                vx >>= 1;
            } else if (p3 == 7) {
                v[0xf] = vx >= vy;
                vx = vy - vx;
            } else if (p3 == 0xE) {
                v[0xf] = bitindex(0, vx);
                vx <<= 1;
            }
        } else if (p0 == 0x9) {
            if (v[p1] != v[p2]) {
                pc += 2;
            }
        }
        else if (p0 == 0xA) {
            this->index = last_12_bits;
        } else if (p0 == 0xB) {
            pc = last_12_bits + v[0];
        } else if (p0 == 0xC) {
            v[p1] = rand_u8() & b1;
        } else if (p0 == 0xD) {
            uint8_t x = v[p1];
            uint8_t y = v[p2];
            uint8_t n = p3;

            v[0xf] = 0;
            for (uint16_t i = this->index; i < (this->index + n); ++i, ++y) {
                uint8_t byte = read8(i);
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
        } else if (p0 == 0xE) {
            if (b1 == 0x9E) {
                if (check_key(v[p1])) {
                    pc += 2;
                }
            } else if (b1 == 0xA1) {
                if (!check_key(v[p1])) {
                    pc += 2;
                }
            }
        } else if (p0 == 0xF) {
            uint8_t &vx = v[p1];
            if (b1 == 0x07) {
                vx = dt;
            } else if (b1 == 0x0A) {
                waiting_for_key = true;
                key_in_reg = p1;
            } else if (b1 == 0x15) {
                dt = vx;
            } else if (b1 == 0x18) {
                st = vx;
            } else if (b1 == 0x1E) {
                index += vx;
            } else if (b1 == 0x29) {
                index = vx * 5; // TODO: May be broken
            } else if (b1 == 0x33) {
                write8(index + 0, vx / 100);
                write8(index + 1, (vx / 10) % 10);
                write8(index + 2, vx % 10);
            } else if (b1 == 0x55) {
                for (uint16_t i = 0; i <= p1; ++i) {
                    write8(index + i, v[i]);
                }

                index += p1 + 1;
            } else if (b1 == 0x65) {
                for (uint16_t i = 0; i <= p1; ++i) {
                    v[i] = read8(index + i);
                }

                index += p1 + 1;
            }
        }
        else {
            spdlog::error("Unhandled Instruction: 0x{:x}{:x}{:x}{:x}", p0, p1, p2, p3);
        }
    }

    void st_logic() noexcept {
        static float last_time = GetTime();
        float now = GetTime();

        if (st == 0)
            return;

        if (now - last_time < (1./60.)) {
            return;
        }

        last_time = now;

        --st;
    }

    void dt_logic() noexcept {
        static float last_time = GetTime();
        float now = GetTime();

        if (dt == 0)
            return;

        if (now - last_time < (1./60.)) {
            return;
        }

        last_time = now;

        --dt;
    }
public:
    void init(const std::vector<uint8_t> &prog) noexcept {
        if (prog.size() > 0xFFF - 0x200) {
            std::cout << "Too big\n";
            return;
        }

        memcpy(mem + 0x200, prog.data(), prog.size());
        memcpy(mem, sprites, sizeof(sprites));
    }

    uint8_t nibble(int part, uint8_t byte) {
        if (part == 0) {
            return byte >> 4;
        } else {
            return byte & 0xF;
        }
    } 

    void step() noexcept {
        logic();
        st_logic();
        dt_logic();
    }
};

int main(int argc, char **argv) {
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");
    SetTraceLogCallback(raylib_spdlog_hook);
    SetTraceLogLevel(LOG_WARNING);

    std::string rom = "rom.ch8";
    float tgt = 500;

    for (size_t i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--rom") == 0) {
            if (i + 1 < argc) {
                rom = argv[i + 1];
            } else {
                spdlog::error("--rom didn't get a value provided with it");
            }
        } else if (strcmp(argv[i], "--speed") == 0) {
            if (i + 1 < argc) {
                tgt = std::atoi(argv[i + 1]);
            } else {
                spdlog::error("--speed didn't get a value provided with it");
            }
        }
    }

    if (!std::filesystem::exists(rom)) {
        spdlog::critical("Path: {} does not exist!", rom);
        return 1;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(fb_x * scale, fb_y * scale, "chip8emu");
    SetTargetFPS(60);

    rlImGuiSetup(true);

    state_t state;
    std::vector<uint8_t> data;
    {
        std::ifstream input(rom, std::ios::binary);

        input.seekg(0, std::ios::end);
        size_t sz = input.tellg();
        input.seekg(0, std::ios::beg);

        data.resize(sz);

        input.read(reinterpret_cast<char*>(data.data()), sz);
    }

    spdlog::info("ROM Loaded: {}", rom);

    using palette_t = Color[2];

    palette_t palettes[] = {
        {{153, 102, 1, 255}, {255, 204, 1, 255}},
        {WHITE, BLACK},
        {BLACK, GREEN}
    };

    InitAudioDevice();
    Sound sound = LoadSoundFromWave(LoadWaveFromMemory(".ogg", beep_ogg, beep_ogg_len));
    SetSoundVolume(sound, 1.f);

    state.init(data);

    float acc = 0.0;
    float last = 0.0;

    bool start = false;

    RenderTexture2D rtex = LoadRenderTexture(fb_x * scale, fb_y * scale);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Color(26, 26, 26));
        BeginTextureMode(rtex);
        ClearBackground(Color(26, 26, 26));
        {
            auto palette = palettes[0];
            for (size_t y = 0; y < fb_y; ++y) {
                for (size_t x = 0; x < fb_x; ++x) {
                    DrawRectangle(x * scale, y * scale, scale, scale, state.fb[y * fb_x + x] == 0 ? palette[0] : palette[1]);
                }
            }

            if (start) {
                if (last == 0.0) {
                    last = GetTime();
                }
                float now = GetTime();
                float dt = now - last;
                last = now;

                acc += dt;

                float step = 1. / tgt;

                while (acc >= step) {
                    state.step();
                    acc -= step;
                }
            } else {
                last = 0.0;
            }

            if (state.st > 0) {
                PlaySound(sound);
                // DrawRectangle(0, 0, 100, 100, RED);
            }
        }
        EndTextureMode();

        rlImGuiBegin();
        ImGui::Begin("Emulator", nullptr, ImGuiWindowFlags_NoResize);
        {
            ImGui::SetWindowSize({ (float) fb_x * scale, (float) fb_y * scale });
            ImGui::Image(rtex.texture.id, { (float) rtex.texture.width, (float) rtex.texture.height }, {0, 1}, {1, 0});
        }
        ImGui::End();
        DrawFPS(10, 10);
        ImGui::Begin("Settings");
        {
            ImGui::SliderFloat("Speed", &tgt, 1.f, 1000.f, "%.1f");
            ImGui::Checkbox("Started", &start);
            int pc = state.pc;
            ImGui::InputInt("Program Counter", &pc);
            if (ImGui::BeginTable("stats", 4)) {
                for (size_t i = 0; i < 16; ++i) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("v%01X", (int) i);
                    ImGui::TableNextColumn();
                    ImGui::Text("0x%02X", state.v[i]);
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("I");
                ImGui::TableNextColumn();
                ImGui::Text("0x%03X", state.index);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("ST");
                ImGui::TableNextColumn();
                ImGui::Text("%d", state.st);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("DT");
                ImGui::TableNextColumn();
                ImGui::Text("%d", state.dt);

                ImGui::EndTable();

                ImGui::InputText("ROM", &rom, ImGuiInputTextFlags_CallbackEdit, [](ImGuiInputTextCallbackData *idata) -> int {
                    if (!std::filesystem::exists(idata->Buf) || !std::filesystem::is_regular_file(idata->Buf))
                        return 1;
                    std::vector<uint8_t> data;
                    std::ifstream input(idata->Buf, std::ios::binary);

                    input.seekg(0, std::ios::end);
                    size_t sz = input.tellg();
                    input.seekg(0, std::ios::beg);

                    state_t *state = reinterpret_cast<state_t*>(idata->UserData);

                    data.resize(sz);

                    input.read(reinterpret_cast<char*>(data.data()), sz);

                    input.close();

                    spdlog::info("ROM CHANGE");

                    state->init(data);
                    state->pc = 0;
                    memset(state->v, 0, 16);
                    memset(state->fb, 0, sizeof(state->fb));

                    return 0;
                }, &state);
            }
            state.pc = pc;
        }
        ImGui::End();
        rlImGuiEnd();
        EndDrawing();
    }

    rlImGuiShutdown();
}
