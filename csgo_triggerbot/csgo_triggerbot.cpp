#include <iostream>

#include "ExternalProcess.h"

struct offsets
{
    uintptr_t localPlayer = 0xD8C2CC;
    uintptr_t entityList = 0x4DA3F5C;
    uintptr_t crosshairId = 0xB3E4;
    uintptr_t forceAttack = 0x31D54B4;
    uintptr_t health = 0x100;
    uintptr_t team = 0xF4;
} offsets;

struct values
{
    uintptr_t localPlayer;
    uintptr_t clientDLL;
    uintptr_t myTeam;

    bool tBot;
    bool friendlyFire;
} values;

static ExternalProcess csgo(L"csgo.exe");

void Shoot()
{
    csgo.WriteMemory<DWORD>(values.clientDLL + offsets.forceAttack, 5);
    csgo.WriteMemory<DWORD>(values.clientDLL + offsets.forceAttack, 4);
}

bool ShouldShoot()
{
    if (values.tBot)
    {
        int crosshairId = csgo.ReadMemory<int>(values.localPlayer + offsets.crosshairId);

        if (crosshairId)
        {
            uintptr_t crosshairEntity = csgo.ReadMemory<uintptr_t>(values.clientDLL + offsets.entityList + ((crosshairId - 1) * 0x10));

            int entityTeam = csgo.ReadMemory<int>(crosshairEntity + offsets.team);
            int entityHealth = csgo.ReadMemory<int>(crosshairEntity + offsets.health);

            return
                (entityTeam != values.myTeam || values.friendlyFire) &&
                entityHealth > 0;
        }
    }
    return false;
}

int main()
{
    values.clientDLL = csgo.GetModuleBaseAddress(L"client.dll");

    while (!values.localPlayer)
    {
        values.localPlayer = csgo.ReadMemory<uintptr_t>(values.clientDLL + offsets.localPlayer);
    }

    std::cout << "LocalPlayer: " << values.localPlayer << "/" << std::hex << values.localPlayer << std::dec << "\n";

    values.myTeam = csgo.ReadMemory<int>(values.localPlayer + offsets.team);
    values.tBot = true;
    values.friendlyFire = false;

    while (true)
    {
        if (ShouldShoot())
        {
            Shoot();
        }

        // toggle tBot
        if (GetAsyncKeyState(VK_F1) & 1)
        {
            values.tBot = !values.tBot;
            std::cout << "Toggled TriggerBot.\n";
        }

        // shoot team mates toggle
        if (GetAsyncKeyState(VK_F2) & 1)
        {
            values.friendlyFire = !values.friendlyFire;
            values.myTeam = csgo.ReadMemory<int>(values.localPlayer + offsets.team);
            std::cout << "Toggled Friendly Fire.\n";
        }
    }
}