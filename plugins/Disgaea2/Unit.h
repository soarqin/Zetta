#pragma once

#pragma pack(push, 1)
struct UnitInfo {
	uint64_t exp;         // +8
	uint8_t equip[0x600]; // +0x608
	uint8_t name[0x40];   // +0x648
	uint8_t job[0x40];    // +0x688
	uint8_t unk1[0x48];   // +0x6D0
	uint16_t sres[5];     // +0x6DA
	uint8_t unk2[0x72];   // +0x74C
	uint32_t skillExp[0x70]; // +0x90C
	uint16_t skillID[0x70]; // +0x9EC
	uint8_t skillLevel[0x70]; // +0xA5C
	uint16_t skillIndex[0x70]; // +0xB3C
	uint16_t skillId2[0x70]; // +0xC1C
	uint32_t unk3; // +0xC20
	uint32_t HP;
	uint32_t SP;
	uint32_t HPMax;
	uint32_t SPMax;
	uint32_t ATK;
	uint32_t DEF;
	uint32_t INT;
	uint32_t SPD;
	uint32_t HIT;
	uint32_t RES;
	uint32_t HPBase;
	uint32_t SPBase;
	uint32_t ATKBase;
	uint32_t DEFBase;
	uint32_t INTBase;
	uint32_t SPDBase;
	uint32_t HITBase;
	uint32_t RESBase;
	uint32_t FistExp;
	uint32_t SwordExp;
	uint32_t SpearExp;
	uint32_t BowExp;
	uint32_t GunExp;
	uint32_t AxeExp;
	uint32_t StaffExp;
	uint32_t unk4;
	uint32_t Mana; // +0xC8C
	uint8_t unk5[0x18]; // +0xCA4
	uint8_t FistLevel;
	uint8_t SwordLevel;
	uint8_t SpearLevel;
	uint8_t BowLevel;
	uint8_t GunLevel;
	uint8_t AxeLevel;
	uint8_t StaffLevel;
	uint8_t unk6[1]; // +0xCAC
	uint8_t FistClass;
	uint8_t SwordClass;
	uint8_t SpearClass;
	uint8_t BowClass;
	uint8_t GunClass;
	uint8_t AxeClass;
	uint8_t StaffClass;
	uint8_t unk7[0x09];
    uint8_t HPGrowth;
	uint8_t SPGrowth;
	uint8_t ATKGrowth;
	uint8_t DEFGrowth;
	uint8_t INTGrowth;
	uint8_t SPDGrowth;
	uint8_t HITGrowth;
	uint8_t RESGrowth;
	uint8_t unk8[0x08]; // +0xCCC
	uint32_t lvl; // +0xCD0
	uint8_t unk9[0x14]; // +0xCE4
	uint8_t JmMx;
	uint8_t Jm;
	uint8_t MvMx;
	uint8_t Mv;
	uint8_t CountMx;
	uint8_t Counter; // +0xCEA
	uint8_t unk10; // +0xCEB
	uint8_t skillCount; //0xCEC
	uint8_t unk11[0x214];
};
#pragma pack(pop)
