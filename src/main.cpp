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

void setup_imgui_style() {
	// Excellency style by gonzaloivan121 from ImThemes
	ImGuiStyle& style = ImGui::GetStyle();
	
	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.6f;
	style.WindowPadding = ImVec2(10.0f, 10.0f);
	style.WindowRounding = 0.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_None;
	style.ChildRounding = 6.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 6.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(8.0f, 6.0f);
	style.FrameRounding = 6.0f;
	style.FrameBorderSize = 1.0f;
	style.ItemSpacing = ImVec2(6.0f, 6.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 11.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 14.0f;
	style.ScrollbarRounding = 6.0f;
	style.GrabMinSize = 10.0f;
	style.GrabRounding = 6.0f;
	style.TabRounding = 6.0f;
	style.TabBorderSize = 1.0f;
	style.TabMinWidthBase = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
	
	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5019608f, 0.5019608f, 0.5019608f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08235294f, 0.08235294f, 0.08235294f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.19607843f, 0.19607843f, 0.19607843f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.101960786f, 0.101960786f, 0.101960786f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.05882353f, 0.05882353f, 0.05882353f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.09019608f, 0.09019608f, 0.09019608f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.05882353f, 0.05882353f, 0.05882353f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08235294f, 0.08235294f, 0.08235294f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08235294f, 0.08235294f, 0.08235294f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15294118f, 0.15294118f, 0.15294118f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.019607844f, 0.019607844f, 0.019607844f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30980393f, 0.30980393f, 0.30980393f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4117647f, 0.4117647f, 0.4117647f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50980395f, 0.50980395f, 0.50980395f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.7529412f, 0.7529412f, 0.7529412f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.50980395f, 0.50980395f, 0.50980395f, 0.7f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.65882355f, 0.65882355f, 0.65882355f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.21960784f, 0.21960784f, 0.21960784f, 0.784f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.27450982f, 0.27450982f, 0.27450982f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.21960784f, 0.21960784f, 0.21960784f, 0.588f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.18431373f, 0.18431373f, 0.18431373f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.18431373f, 0.18431373f, 0.18431373f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.18431373f, 0.18431373f, 0.18431373f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.101960786f, 0.101960786f, 0.101960786f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.15294118f, 0.7254902f, 0.9490196f, 0.588f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.15294118f, 0.7254902f, 0.9490196f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.9098039f, 0.9098039f, 0.9098039f, 0.25f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.8117647f, 0.8117647f, 0.8117647f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.45882353f, 0.45882353f, 0.45882353f, 0.95f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.08235294f, 0.08235294f, 0.08235294f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(1.0f, 0.88235295f, 0.5294118f, 0.118f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(1.0f, 0.88235295f, 0.5294118f, 0.235f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08235294f, 0.08235294f, 0.08235294f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(1.0f, 0.88235295f, 0.5294118f, 0.118f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6117647f, 0.6117647f, 0.6117647f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.43137255f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.9019608f, 0.7019608f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18431373f, 0.18431373f, 0.18431373f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.101960786f, 0.101960786f, 0.101960786f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.15294118f, 0.7254902f, 0.9490196f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.15294118f, 0.7254902f, 0.9490196f, 0.8f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
}

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
    SetTraceLogLevel(LOG_INFO);

    std::string rom = "rom.ch8";
    float tgt = 660;

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

    setup_imgui_style();

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

    using palette_t = std::array<Color, 2>;

    InitAudioDevice();
    Sound sound = LoadSoundFromWave(LoadWaveFromMemory(".ogg", beep_ogg, beep_ogg_len));
    SetSoundVolume(sound, 1.f);

    state.init(data);

    float acc = 0.0;
    float last = 0.0;

    bool start = false;

    RenderTexture2D rtex = LoadRenderTexture(fb_x * scale, fb_y * scale);

    // const palette_t default_palette = {153, 102, 1, 255, 255, 204, 1, 255};
    const palette_t default_palette = {0, 0, 0, 255, 255, 255, 255, 255};
    palette_t palette = default_palette;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Color(26, 26, 26));
        BeginTextureMode(rtex);
        ClearBackground(Color(26, 26, 26));
        {
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
                if (!IsSoundPlaying(sound))
                    PlaySound(sound);
                // DrawRectangle(0, 0, 100, 100, RED);
            }
        }
        EndTextureMode();

        rlImGuiBegin();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Emulator", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            // ImGui::SetWindowSize({ (float) fb_x * scale, (float) fb_y * scale });
            ImGui::Image(rtex.texture.id, { (float) rtex.texture.width + 1, (float) rtex.texture.height + 1 }, {0, 1}, {1, 0});
            auto window_sz = ImGui::GetWindowSize();
            if (window_sz.x > fb_x * scale && window_sz.y > fb_y * scale) {
                ++scale;
                UnloadRenderTexture(rtex);
                rtex = LoadRenderTexture(fb_x * scale, fb_y * scale);
            } else if (window_sz.x < fb_x * scale && window_sz.y < fb_y * scale) {
                --scale;
                UnloadRenderTexture(rtex);
                rtex = LoadRenderTexture(fb_x * scale, fb_y * scale);
            }

            ImGui::SetWindowSize({ (float) fb_x * scale, (float) fb_y * scale });
        }
        ImGui::End();
        ImGui::PopStyleVar();
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

                static float colorpicker_out_1[3] = { default_palette[0].r / 255.f, default_palette[0].g / 255.f, default_palette[0].b / 255.f };
                static float colorpicker_out_2[3] = { default_palette[1].r / 255.f, default_palette[1].g / 255.f, default_palette[1].b / 255.f };
                static bool color_picker_1 = false;
                static bool color_picker_2 = false;
                if (ImGui::Button("Set Color 1")) {
                    color_picker_1 = true;
                }
                if (ImGui::Button("Set Color 2")) {
                    color_picker_2 = true;
                }
                if (color_picker_1) {
                    ImGui::OpenPopup("Color Picker");

                    if (ImGui::BeginPopupModal("Color Picker", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("Choose color 1");

                        ImGui::ColorPicker3("Picker", colorpicker_out_1);

                        if (ImGui::Button("Close")) {
                            color_picker_1 = false;
                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::EndPopup();
                    }
                }
                if (color_picker_2) {
                    ImGui::OpenPopup("Color Picker");

                    if (ImGui::BeginPopupModal("Color Picker", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("Choose color 2");

                        ImGui::ColorPicker3("Picker", colorpicker_out_2);

                        if (ImGui::Button("Close")) {
                            color_picker_2 = false;
                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::EndPopup();
                    }
                }



                palette[0].r = colorpicker_out_1[0] * 255;
                palette[0].g = colorpicker_out_1[1] * 255;
                palette[0].b = colorpicker_out_1[2] * 255;

                palette[1].r = colorpicker_out_2[0] * 255;
                palette[1].g = colorpicker_out_2[1] * 255;
                palette[1].b = colorpicker_out_2[2] * 255;
            }
            state.pc = pc;
        }
        ImGui::End();
        rlImGuiEnd();
        EndDrawing();
    }

    rlImGuiShutdown();
}
