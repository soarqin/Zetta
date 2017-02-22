#pragma once

#pragma pack(push, 1)
struct ItemInfo {
    uint64_t habits[8]; // 0x000
    uint64_t exp;       // 0x040
    int32_t hp;         // 0x048
    int32_t sp;
    int32_t atk;
    int32_t def;
    int32_t inte;
    int32_t spd;
    int32_t hit;
    int32_t res;
    int32_t hpBase;     // 0x068
    int32_t spBase;
    int32_t atkBase;
    int32_t defBase;
    int32_t inteBase;
    int32_t spdBase;
    int32_t hitBase;
    int32_t resBase;
    uint16_t id;        // 0x088
    uint8_t unk2[0x02];
    uint8_t level;      // 0x08C
    uint8_t dungeonMax; // 0x08D
    uint8_t unk3[0x06];
    uint8_t dungeonCur; // 0x094
    uint8_t unk4[0x05];
    uint8_t rarity;     // 0x09A
    uint8_t type;       // 0x09B
    uint8_t icon;       // 0x09C
    uint8_t popMax;     // 0x09D
    uint8_t mv;         // 0x09E
    uint8_t jm;         // 0x09F
    uint8_t unk5[0x5];
    uint8_t elite;      // 0x0A5
    uint8_t unk6[0x12];
    uint8_t name[0x60]; // 0x0B8
    uint8_t unk7[0x68]; // 0x118
};
#pragma pack(pop)
