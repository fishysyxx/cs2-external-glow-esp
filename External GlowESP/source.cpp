#include "memory.h"
#include <conio.h>
#include <stdio.h>
#include <iostream>
#include <Windows.h>

using namespace std;

struct Offsets {
    //offsets.cs
    const uintptr_t dwEntityList = 0x1A1F730;
    const uintptr_t dwLocalPlayerPawn = 0x1874050;

    //client_dll.cs
    const uintptr_t m_Glow = 0xC00;
    const uintptr_t m_iGlowType = 0x30;
    const uintptr_t m_glowColorOverride = 0x40;
    const uintptr_t m_bGlowing = 0x51;
    const uintptr_t m_iHealth = 0x344;
    const uintptr_t m_iTeamNum = 0x3E3;
    const uintptr_t m_hPlayerPawn = 0x814;
}Offsets;

void SetConsoleContent(bool isMemory, bool client) {
    HWND hWnd = GetConsoleWindow();
    HMENU hm = GetSystemMenu(hWnd, FALSE);
    RemoveMenu(hm, SC_SIZE, MF_BYCOMMAND | MF_REMOVE);
    RemoveMenu(hm, SC_MAXIMIZE, MF_BYCOMMAND | MF_REMOVE);
    DrawMenuBar(hWnd);

    SetConsoleTitle("External GlowESP");
    system("mode con cols=40 lines=13");

    cout << "[ABOUT]" << endl;
    cout << "1: fishysyxx" << endl;
    cout << "2: update offsets from https://github.com/a2x/cs2-dumper/blob/main/output/offsets.hpp" << endl << endl;
    cout << "[HOW TO USE]" << endl;
    cout << "1: run cs2.exe" << endl;
    cout << "2: run cheat.exe" << endl << endl;
    cout << "[LAUNCHING CHEAT]" << endl;
    if (!isMemory) { cout << "1: cs2.exe not found" << endl; }
    else { cout << "1: cs2.exe found" << endl; }
    if (!client) { cout << "2: client.dll not found" << endl; }
    else { cout << "2: client.dll found" << endl; }
    if (!isMemory || !client) { cout << "3: cheat launch failed" << endl; }
    else { cout << "3: cheat launched successfully" << endl; }
    return;
}

int main() {
    Memory memory("cs2.exe");
    uintptr_t client = memory.GetModuleAddress("client.dll");

    SetConsoleContent(memory.IsValid(), client);
    if (!memory.IsValid() || !client) { _getch(); return 0; }

    while (true) {
        uintptr_t dwLocalPlayerPawn = memory.Read<uintptr_t>(client + Offsets.dwLocalPlayerPawn);
        if (!dwLocalPlayerPawn) continue;
        uintptr_t dwEntityList = memory.Read<uintptr_t>(client + Offsets.dwEntityList);
        if (!dwEntityList) continue;

        for (int i = 0; i < 65; i++) {
            uintptr_t dwEntityPtr = memory.Read<uintptr_t>(dwEntityList + (8 * (i & 0x7FFF) >> 9) + 16);
            if (!dwEntityPtr) continue;
            uintptr_t dwControllerPtr = memory.Read<uintptr_t>(dwEntityPtr + 120 * (i & 0x1FF));
            if (!dwControllerPtr) continue;
            uintptr_t m_hPlayerPawn = memory.Read<uintptr_t>(dwControllerPtr + Offsets.m_hPlayerPawn);
            if (!m_hPlayerPawn) continue;
            uintptr_t dwListEntityPtr = memory.Read<uintptr_t>(dwEntityList + 0x8 * ((m_hPlayerPawn & 0x7FFF) >> 9) + 16);
            if (!dwListEntityPtr) continue;
            uintptr_t dwEntityPawn = memory.Read<uintptr_t>(dwListEntityPtr + 120 * (m_hPlayerPawn & 0x1FF));
            if (!dwEntityPawn || dwEntityPawn == dwLocalPlayerPawn) continue;
            uint32_t m_iHealth = memory.Read<uint32_t>(dwEntityPawn + Offsets.m_iHealth);
            if (!(m_iHealth > 0)) continue;
            uint32_t m_iTeamNum = memory.Read<uint32_t>(dwEntityPawn + Offsets.m_iTeamNum);

            uintptr_t glowColor = ((uintptr_t)(0) << 24) | ((uintptr_t)(0) << 16) | ((uintptr_t)(0) << 8) | ((uintptr_t)(0));
            uint32_t glowType = 1;
            uint32_t glowEnabled = 1;

            if (m_iTeamNum == 2) { glowColor = ((uintptr_t)(255) << 24) | ((uintptr_t)(0) << 16) | ((uintptr_t)(0) << 8) | ((uintptr_t)(255)); }
            if (m_iTeamNum == 3) { glowColor = ((uintptr_t)(255) << 24) | ((uintptr_t)(255) << 16) | ((uintptr_t)(0) << 8) | ((uintptr_t)(0)); }

            memory.Write<uint32_t>(dwEntityPawn + Offsets.m_Glow + Offsets.m_iGlowType, glowType);
            memory.Write<uintptr_t>(dwEntityPawn + Offsets.m_Glow + Offsets.m_glowColorOverride, glowColor);
            memory.Write<uint32_t>(dwEntityPawn + Offsets.m_Glow + Offsets.m_bGlowing, glowEnabled);
        }
        Sleep(1);
    }

    return 0;
}
