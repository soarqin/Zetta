#pragma once

#pragma pack(push, 1)
struct UnitInfo {
    uint64_t exp;
    uint8_t unk1[0x240];
    wchar_t name[16];
    uint8_t nameterm;
    wchar_t job[16];
    uint8_t jobterm;
    uint8_t unk2[0x22];
    uint16_t PSN;
    uint16_t SLP;
    uint16_t PAR;
    uint16_t FGT;
    uint16_t DPR;
    uint8_t unk3[0x6E];
    uint32_t skillExp[0x60];
    uint16_t skillID[0x60];
    uint8_t skillLevel[0x60];
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
    uint8_t unk4[0x04];
    uint32_t Mana;
    uint8_t unk5[0x18];
    uint8_t FistLevel;
    uint8_t SwordLevel;
    uint8_t SpearLevel;
    uint8_t BowLevel;
    uint8_t GunLevel;
    uint8_t AxeLevel;
    uint8_t StaffLevel;
    uint8_t unk6[1];
    uint8_t FistClass;
    uint8_t SwordClass;
    uint8_t SpearClass;
    uint8_t BowClass;
    uint8_t GunClass;
    uint8_t AxeClass;
    uint8_t StaffClass;
    uint8_t unk7[1];
    uint8_t HPGrowth;
    uint8_t SPGrowth;
    uint8_t ATKGrowth;
    uint8_t DEFGrowth;
    uint8_t INTGrowth;
    uint8_t SPDGrowth;
    uint8_t HITGrowth;
    uint8_t RESGrowth;
    uint16_t lvl;
    uint8_t unk8[0x15];
    uint8_t FireRes;
    uint8_t WindRes;
    uint8_t IceRes;
    uint8_t JmMx;
    uint8_t Jm;
    uint8_t MvMx;
    uint8_t Mv;
    uint8_t CountMx;
    uint8_t Counter;
    uint8_t unk9[0x01];
	uint8_t skillCount;
	uint8_t unk10[0x0B];
	uint16_t Rank;
    uint8_t unk11[0x29];
};
#pragma pack(pop)

/*
exp     0x0        8: 0A 00 00 00 00 00 00 00
name    0x248      16
job     0x269      16
PSN     0x2ac      2: 01 00
SLP     0x2ae      2: 01 00
PAR     0x2b0      2: 01 00
FGT     0x2b2      2: 01 00
DPR     0x2b4      2: 01 00
SkillE  0x324-0x4a4
SkillT  0x4a5-0x564
SkillL  0x564-0x5c4
HP      0x5c4      4: A6 00 00 00
SP      0x5c8      4: 8E 00 00 00
HPMx    0x5cc      4: A6 00 00 00
SPMx    0x5d0      4: 8E 00 00 00
ATK     0x5d4      4: BF 00 00 00
DEF     0x5d8      4: 8E 00 00 00
INT     0x5dc      4: 94 00 00 00
SPD     0x5e0      4: 92 00 00 00
HIT     0x5e4      4: 94 00 00 00
RES     0x5e8      4: 95 00 00 00
HPBase  0x5ec      4: 92 00 00 00
SPBase  0x5f0      4: 84 00 00 00
ATKBase 0x5f4      4: 93 00 00 00
DEFBaSE 0x5f8      4: 84 00 00 00
INTBase 0x5fc      4: 84 00 00 00
SPDBase 0x600      4: 88 00 00 00
HITBase 0x604      4: 8A 00 00 00
RESBase 0x608      4: 85 00 00 00
FistE   0x60c      4: 01 00 00 00
SworE   0x610      4: 9D 00 00 00
SpeaE   0x614      4: 01 00 00 00
BowE    0x618      4: 01 00 00 00
GunE    0x61c      4: 01 00 00 00
AxeE    0x620      4: 01 00 00 00
StafE   0x624      4: 01 00 00 00
Mana    0x62c      4: 0A 00 00 00
FistL   0x648      1: 01
SworL   0x649      1: 01
SpeaL   0x64a      1: 01
BowL    0x64b      1: 01
GunL    0x64c      1: 01
AxeL    0x64d      1: 01
StafL   0x64e      1: 01
FistC   0x650      1: 19
SworC   0x651      1: 14
SpeaC   0x652      1: 0F
BowC    0x653      1: 0A
GunC    0x654      1: 05
AxeC    0x655      1: 01
StafC   0x656      1: 00
HPG     0x658      1: 30
SPG     0x659      1: 20
ATKG    0x65a      1: 31
DEFG    0x65b      1: 20
INTG    0x65c      1: 22
SPDG    0x65d      1: 25
HITG    0x65e      1: 27
RESG    0x65f      1: 22
lvl     0x660      2: 0B 00
Fire    0x677      1: 01
Wind    0x678      1: 01
Ice     0x679      1: 01
JmMx    0x67a      1: 78
Jm      0x67b      1: 78
MvMx    0x67c      1: 3D
Mv      0x67d      1: 3D
CountMx 0x67e      1: 15
Counter 0x67f      1: 15
Rank    0x68d      2: 0A 00
*/