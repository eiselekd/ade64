// ADE64 2.03c -- advanced 16/32-bit opcode assembler/disassembler engine
// this stuff is used to get instruction length and parse it into
// prefix, opcode, modregr/m, address, immediate, etc.

#include <string.h>

int verbose = 0;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef int            BOOL;
#define FALSE          0
#define TRUE           1
#define IN
#define OUT

#define C_ERROR   0xFFFFFFFF
#define C_ADDR1   0x00000001
#define C_ADDR2   0x00000002
#define C_ADDR4   0x00000004
#define C_ADDR8   0x00000008
#define C_LOCK    0x00000010
#define C_67      0x00000020
#define C_66      0x00000040
#define C_REP     0x00000080
#define C_SEG     0x00000100
#define C_ANYPREFIX  (C_66+C_67+C_LOCK+C_REP+C_SEG+C_REX)
#define C_DATA1   0x00001000
#define C_DATA2   0x00002000
#define C_DATA4   0x00004000
#define C_DATA8   0x00008000
#define C_SIB     0x00010000
#define C_ADDR67  0x00020000
#define C_DATA66  0x00040000
#define C_MODRM   0x00080000
#define C_BAD     0x00100000
#define C_OPCODE2 0x00200000
#define C_REL     0x00400000
#define C_STOP    0x00800000
#define C_DATAQ   0x01000000

#define C_REX     0x02000000
#define REX_W     0x8

struct disasm64_struct
{
  BYTE  disasm_defaddr;         // 00
  BYTE  disasm_defdata;         // 01
  DWORD disasm_len;             // 02 03 04 05
  DWORD disasm_flag;            // 06 07 08 09
  DWORD disasm_addrsize;        // 0A 0B 0C 0D
  DWORD disasm_datasize;        // 0E 0F 10 11
  BYTE  disasm_rep;             // 12
  BYTE  disasm_seg;             // 13
  BYTE  disasm_opcode;          // 14
  BYTE  disasm_opcode2;         // 15
  BYTE  disasm_modrm;           // 16
  BYTE  disasm_sib;             // 17
  union
  {
  BYTE  disasm_addr_b[8];       // 18 19 1A 1B  1C 1D 1E 1F
  WORD  disasm_addr_w[4];
  DWORD disasm_addr_d[2];
  char  disasm_addr_c[8];
  short disasm_addr_s[4];
  long  disasm_addr_l[2];
  };
  union
  {
  BYTE  disasm_data_b[8];       // 20 21 22 23  24 25 26 27
  WORD  disasm_data_w[4];
  DWORD disasm_data_d[2];
  char  disasm_data_c[8];
  short disasm_data_s[4];
  long  disasm_data_l[2];
  };
  BYTE  disasm_rex;             // 28
}; // disasm64_struct

DWORD ade64_table[256*3] = {
/* 00 */  C_MODRM,
/* 01 */  C_MODRM,
/* 02 */  C_MODRM,
/* 03 */  C_MODRM,
/* 04 */  C_DATA1,
/* 05 */  C_DATA66,
/* 06 */  C_BAD,
/* 07 */  C_BAD,
/* 08 */  C_MODRM,
/* 09 */  C_MODRM,
/* 0A */  C_MODRM,
/* 0B */  C_MODRM,
/* 0C */  C_DATA1,
/* 0D */  C_DATA66,
/* 0E */  C_BAD,
/* 0F */  C_OPCODE2,
/* 10 */  C_MODRM+C_BAD,
/* 11 */  C_MODRM,
/* 12 */  C_MODRM+C_BAD,
/* 13 */  C_MODRM,
/* 14 */  C_DATA1+C_BAD,
/* 15 */  C_DATA66+C_BAD,
/* 16 */  C_BAD,
/* 17 */  C_BAD,
/* 18 */  C_MODRM+C_BAD,
/* 19 */  C_MODRM,
/* 1A */  C_MODRM,
/* 1B */  C_MODRM,
/* 1C */  C_DATA1+C_BAD,
/* 1D */  C_DATA66+C_BAD,
/* 1E */  C_BAD,
/* 1F */  C_BAD,
/* 20 */  C_MODRM,
/* 21 */  C_MODRM,
/* 22 */  C_MODRM,
/* 23 */  C_MODRM,
/* 24 */  C_DATA1,
/* 25 */  C_DATA66,
/* 26 */  C_SEG+C_BAD,
/* 27 */  C_BAD,
/* 28 */  C_MODRM,
/* 29 */  C_MODRM,
/* 2A */  C_MODRM,
/* 2B */  C_MODRM,
/* 2C */  C_DATA1,
/* 2D */  C_DATA66,
/* 2E */  C_SEG+C_BAD,
/* 2F */  C_BAD,
/* 30 */  C_MODRM,
/* 31 */  C_MODRM,
/* 32 */  C_MODRM,
/* 33 */  C_MODRM,
/* 34 */  C_DATA1,
/* 35 */  C_DATA66,
/* 36 */  C_SEG+C_BAD,
/* 37 */  C_BAD,
/* 38 */  C_MODRM,
/* 39 */  C_MODRM,
/* 3A */  C_MODRM,
/* 3B */  C_MODRM,
/* 3C */  C_DATA1,
/* 3D */  C_DATA66,
/* 3E */  C_SEG+C_BAD,
/* 3F */  C_BAD,
/* 40 */  C_REX /*0*/,
/* 41 */  C_REX /*0*/,
/* 42 */  C_REX /*0*/,
/* 43 */  C_REX /*0*/,
/* 44 */  C_REX /*C_BAD*/,
/* 45 */  C_REX /*0*/,
/* 46 */  C_REX /*0*/,
/* 47 */  C_REX /*0*/,
/* 48 */  C_REX /*0*/,
/* 49 */  C_REX /*0*/,
/* 4A */  C_REX /*0*/,
/* 4B */  C_REX /*0*/,
/* 4C */  C_REX /*C_BAD*/,
/* 4D */  C_REX /*0*/,
/* 4E */  C_REX /*0*/,
/* 4F */  C_REX /*0*/,
/* 50 */  0,
/* 51 */  0,
/* 52 */  0,
/* 53 */  0,
/* 54 */  0,
/* 55 */  0,
/* 56 */  0,
/* 57 */  0,
/* 58 */  0,
/* 59 */  0,
/* 5A */  0,
/* 5B */  0,
/* 5C */  C_BAD,
/* 5D */  0,
/* 5E */  0,
/* 5F */  0,
/* 60 */  C_BAD,
/* 61 */  C_BAD,
/* 62 */  C_MODRM+C_BAD,
/* 63 */  C_MODRM+C_BAD,
/* 64 */  C_SEG,
/* 65 */  C_SEG+C_BAD,
/* 66 */  C_66,
/* 67 */  C_67,
/* 68 */  C_DATA66,
/* 69 */  C_MODRM+C_DATA66,
/* 6A */  C_DATA1,
/* 6B */  C_MODRM+C_DATA1,
/* 6C */  C_BAD,
/* 6D */  C_BAD,
/* 6E */  C_BAD,
/* 6F */  C_BAD,
/* 70 */  C_DATA1+C_REL+C_BAD,
/* 71 */  C_DATA1+C_REL+C_BAD,
/* 72 */  C_DATA1+C_REL,
/* 73 */  C_DATA1+C_REL,
/* 74 */  C_DATA1+C_REL,
/* 75 */  C_DATA1+C_REL,
/* 76 */  C_DATA1+C_REL,
/* 77 */  C_DATA1+C_REL,
/* 78 */  C_DATA1+C_REL,
/* 79 */  C_DATA1+C_REL,
/* 7A */  C_DATA1+C_REL+C_BAD,
/* 7B */  C_DATA1+C_REL+C_BAD,
/* 7C */  C_DATA1+C_REL,
/* 7D */  C_DATA1+C_REL,
/* 7E */  C_DATA1+C_REL,
/* 7F */  C_DATA1+C_REL,
/* 80 */  C_MODRM+C_DATA1,
/* 81 */  C_MODRM+C_DATA66,
/* 82 */  C_MODRM+C_DATA1+C_BAD,
/* 83 */  C_MODRM+C_DATA1,
/* 84 */  C_MODRM,
/* 85 */  C_MODRM,
/* 86 */  C_MODRM,
/* 87 */  C_MODRM,
/* 88 */  C_MODRM,
/* 89 */  C_MODRM,
/* 8A */  C_MODRM,
/* 8B */  C_MODRM,
/* 8C */  C_MODRM+C_BAD,
/* 8D */  C_MODRM,
/* 8E */  C_MODRM+C_BAD,
/* 8F */  C_MODRM,
/* 90 */  0,
/* 91 */  0,
/* 92 */  0,
/* 93 */  C_BAD,
/* 94 */  C_BAD,
/* 95 */  C_BAD,
/* 96 */  C_BAD,
/* 97 */  C_BAD,
/* 98 */  C_BAD,
/* 99 */  0,
/* 9A */  C_DATA66+C_DATA2+C_BAD,
/* 9B */  0,
/* 9C */  C_BAD,
/* 9D */  C_BAD,
/* 9E */  C_BAD,
/* 9F */  C_BAD,
/* A0 */  C_ADDR67,
/* A1 */  C_ADDR67,
/* A2 */  C_ADDR67,
/* A3 */  C_ADDR67,
/* A4 */  0,
/* A5 */  0,
/* A6 */  0,
/* A7 */  0,
/* A8 */  C_DATA1,
/* A9 */  C_DATA66,
/* AA */  0,
/* AB */  0,
/* AC */  0,
/* AD */  C_BAD,
/* AE */  0,
/* AF */  C_BAD,
/* B0 */  C_DATA1,
/* B1 */  C_DATA1,
/* B2 */  C_DATA1,
/* B3 */  C_DATA1,
/* B4 */  C_DATA1,
/* B5 */  C_DATA1,
/* B6 */  C_DATA1+C_BAD,
/* B7 */  C_DATA1+C_BAD,
/* B8 */  C_DATA66+C_DATAQ,
/* B9 */  C_DATA66+C_DATAQ,
/* BA */  C_DATA66+C_DATAQ,
/* BB */  C_DATA66+C_DATAQ,
/* BC */  C_DATA66+C_DATAQ/*+C_BAD*/,
/* BD */  C_DATA66+C_DATAQ,
/* BE */  C_DATA66+C_DATAQ,
/* BF */  C_DATA66+C_DATAQ,
/* C0 */  C_MODRM+C_DATA1,
/* C1 */  C_MODRM+C_DATA1,
/* C2 */  C_DATA2+C_STOP,
/* C3 */  C_STOP,
/* C4 */  C_MODRM+C_DATA2/*+C_BAD*/,
/* C5 */  C_MODRM+C_DATA1/*+C_BAD*/,
/* C6 */  C_MODRM+C_DATA1,
/* C7 */  C_MODRM+C_DATA66,
/* C8 */  C_DATA2+C_DATA1,
/* C9 */  0,
/* CA */  C_DATA2+C_STOP+C_BAD,
/* CB */  C_STOP+C_BAD,
/* CC */  C_BAD,
/* CD */  C_DATA1,
/* CE */  C_BAD,
/* CF */  C_STOP+C_BAD,
/* D0 */  C_MODRM,
/* D1 */  C_MODRM,
/* D2 */  C_MODRM,
/* D3 */  C_MODRM,
/* D4 */  C_DATA1+C_BAD,
/* D5 */  C_DATA1+C_BAD,
/* D6 */  C_BAD,
/* D7 */  C_BAD,
/* D8 */  C_MODRM,
/* D9 */  C_MODRM,
/* DA */  C_MODRM,
/* DB */  C_MODRM,
/* DC */  C_MODRM,
/* DD */  C_MODRM,
/* DE */  C_MODRM,
/* DF */  C_MODRM,
/* E0 */  C_DATA1+C_REL+C_BAD,
/* E1 */  C_DATA1+C_REL+C_BAD,
/* E2 */  C_DATA1+C_REL,
/* E3 */  C_DATA1+C_REL,
/* E4 */  C_DATA1+C_BAD,
/* E5 */  C_DATA1+C_BAD,
/* E6 */  C_DATA1+C_BAD,
/* E7 */  C_DATA1+C_BAD,
/* E8 */  C_DATA66+C_REL,
/* E9 */  C_DATA66+C_REL+C_STOP,
/* EA */  C_DATA66+C_DATA2+C_BAD,
/* EB */  C_DATA1+C_REL+C_STOP,
/* EC */  C_BAD,
/* ED */  C_BAD,
/* EE */  C_BAD,
/* EF */  C_BAD,
/* F0 */  C_LOCK+C_BAD,
/* F1 */  C_BAD,
/* F2 */  C_REP,
/* F3 */  C_REP,
/* F4 */  C_BAD,
/* F5 */  C_BAD,
/* F6 */  C_MODRM,
/* F7 */  C_MODRM,
/* F8 */  0,
/* F9 */  0,
/* FA */  C_BAD,
/* FB */  C_BAD,
/* FC */  0,
/* FD */  0,
/* FE */  C_MODRM,
/* FF */  C_MODRM,

/********************** 2 byte ********************************/

/* 00 */  C_MODRM,
/* 01 */  C_MODRM,
/* 02 */  C_MODRM,
/* 03 */  C_MODRM,
/* 04 */  C_ERROR,
/* 05 */  0 /*C_ERROR*/,
/* 06 */  0,
/* 07 */  C_ERROR,
/* 08 */  0,
/* 09 */  0,
/* 0A */  0,
/* 0B */  0,
/* 0C */  C_ERROR,
/* 0D */  C_MODRM,
/* 0E */  C_ERROR,
/* 0F */  C_ERROR,
/* 10 */  C_MODRM /*C_ERROR*/,
/* 11 */  C_MODRM /*C_ERROR*/,
/* 12 */  C_MODRM /*C_ERROR*/,
/* 13 */  C_MODRM /*C_ERROR*/,
/* 14 */  C_MODRM /*C_ERROR*/,
/* 15 */  C_MODRM /*C_ERROR*/,
/* 16 */  C_MODRM /*C_ERROR*/,
/* 17 */  C_MODRM /*C_ERROR*/,
/* 18 */  C_MODRM /*C_ERROR*/,
/* 19 */  C_MODRM /*C_ERROR*/,
/* 1A */  C_MODRM /*C_ERROR*/,
/* 1B */  C_MODRM /*C_ERROR*/,
/* 1C */  C_MODRM /*C_ERROR*/,
/* 1D */  C_MODRM /*C_ERROR*/,
/* 1E */  C_MODRM /*C_ERROR*/,
/* 1F */  C_MODRM /*C_ERROR*/,
/* 20 */  C_MODRM /*C_ERROR*/,
/* 21 */  C_MODRM /*C_ERROR*/,
/* 22 */  C_MODRM /*C_ERROR*/,
/* 23 */  C_MODRM /*C_ERROR*/,
/* 24 */  C_MODRM /*C_ERROR*/,
/* 25 */  C_MODRM /*C_ERROR*/,
/* 26 */  C_MODRM /*C_ERROR*/,
/* 27 */  C_MODRM /*C_ERROR*/,
/* 28 */  C_MODRM /*C_ERROR*/,
/* 29 */  C_MODRM /*C_ERROR*/,
/* 2A */  C_MODRM /*C_ERROR*/,
/* 2B */  C_MODRM /*C_ERROR*/,
/* 2C */  C_MODRM /*C_ERROR*/,
/* 2D */  C_MODRM /*C_ERROR*/,
/* 2E */  C_MODRM /*C_ERROR*/,
/* 2F */  C_MODRM /*C_ERROR*/,
/* 30 */  C_ERROR,
/* 31 */  0 /*C_ERROR*/,
/* 32 */  C_ERROR,
/* 33 */  C_ERROR,
/* 34 */  C_ERROR,
/* 35 */  C_ERROR,
/* 36 */  C_ERROR,
/* 37 */  C_ERROR,
/* 38 */  C_MODRM+C_DATA1 /*C_ERROR*/,
/* 39 */  C_ERROR,
/* 3A */  C_MODRM /*C_MODRM+C_DATA2+C_DATA66*/ /*+C_DATA2*/ /*C_ERROR*/,
/* 3B */  C_ERROR,
/* 3C */  C_ERROR,
/* 3D */  C_ERROR,
/* 3E */  C_ERROR,
/* 3F */  C_ERROR,
/* 40 */  C_MODRM,
/* 41 */  C_MODRM,
/* 42 */  C_MODRM,
/* 43 */  C_MODRM,
/* 44 */  C_MODRM,
/* 45 */  C_MODRM,
/* 46 */  C_MODRM,
/* 47 */  C_MODRM,
/* 48 */  C_MODRM,
/* 49 */  C_MODRM,
/* 4A */  C_MODRM,
/* 4B */  C_MODRM,
/* 4C */  C_MODRM,
/* 4D */  C_MODRM,
/* 4E */  C_MODRM,
/* 4F */  C_MODRM,
/* 50 */  C_MODRM /*C_ERROR*/,
/* 51 */  C_MODRM /*C_ERROR*/,
/* 52 */  C_MODRM /*C_ERROR*/,
/* 53 */  C_MODRM /*C_ERROR*/,
/* 54 */  C_MODRM /*C_ERROR*/,
/* 55 */  C_MODRM /*C_ERROR*/,
/* 56 */  C_MODRM /*C_ERROR*/,
/* 57 */  C_MODRM /*C_ERROR*/,
/* 58 */  C_MODRM /*C_ERROR*/,
/* 59 */  C_MODRM /*C_ERROR*/,
/* 5A */  C_MODRM /*C_ERROR*/,
/* 5B */  C_MODRM /*C_ERROR*/,
/* 5C */  C_MODRM /*C_ERROR*/,
/* 5D */  C_MODRM /*C_ERROR*/,
/* 5E */  C_MODRM /*C_ERROR*/,
/* 5F */  C_MODRM /*C_ERROR*/,
/* 60 */  C_MODRM /*C_ERROR*/,
/* 61 */  C_MODRM /*C_ERROR*/,
/* 62 */  C_MODRM /*C_ERROR*/,
/* 63 */  C_MODRM /*C_ERROR*/,
/* 64 */  C_MODRM /*C_ERROR*/,
/* 65 */  C_MODRM /*C_ERROR*/,
/* 66 */  C_MODRM /*C_ERROR*/,
/* 67 */  C_MODRM /*C_ERROR*/,
/* 68 */  C_MODRM /*C_ERROR*/,
/* 69 */  C_MODRM /*C_ERROR*/,
/* 6A */  C_MODRM /*C_ERROR*/,
/* 6B */  C_MODRM /*C_ERROR*/,
/* 6C */  C_MODRM /*C_ERROR*/,
/* 6D */  C_MODRM /*C_ERROR*/,
/* 6E */  C_MODRM /*C_ERROR*/,
/* 6F */  C_MODRM /*C_ERROR*/,
/* 70 */  C_MODRM+C_DATA1 /*C_ERROR*/,
/* 71 */  C_ERROR,
/* 72 */  C_ERROR,
/* 73 */  C_MODRM+C_DATA1 /*C_ERROR*/,
/* 74 */  C_MODRM /*C_ERROR*/,
/* 75 */  C_ERROR,
/* 76 */  C_MODRM /*C_ERROR*/,
/* 77 */  C_ERROR,
/* 78 */  C_ERROR,
/* 79 */  C_ERROR,
/* 7A */  C_ERROR,
/* 7B */  C_ERROR,
/* 7C */  C_MODRM /*C_ERROR*/,
/* 7D */  C_MODRM /*C_ERROR*/,
/* 7E */  C_MODRM /*C_ERROR*/,
/* 7F */  C_MODRM /*C_ERROR*/,
/* 80 */  C_DATA66+C_REL,
/* 81 */  C_DATA66+C_REL,
/* 82 */  C_DATA66+C_REL,
/* 83 */  C_DATA66+C_REL,
/* 84 */  C_DATA66+C_REL,
/* 85 */  C_DATA66+C_REL,
/* 86 */  C_DATA66+C_REL,
/* 87 */  C_DATA66+C_REL,
/* 88 */  C_DATA66+C_REL,
/* 89 */  C_DATA66+C_REL,
/* 8A */  C_DATA66+C_REL,
/* 8B */  C_DATA66+C_REL,
/* 8C */  C_DATA66+C_REL,
/* 8D */  C_DATA66+C_REL,
/* 8E */  C_DATA66+C_REL,
/* 8F */  C_DATA66+C_REL,
/* 90 */  C_MODRM,
/* 91 */  C_MODRM,
/* 92 */  C_MODRM,
/* 93 */  C_MODRM,
/* 94 */  C_MODRM,
/* 95 */  C_MODRM,
/* 96 */  C_MODRM,
/* 97 */  C_MODRM,
/* 98 */  C_MODRM,
/* 99 */  C_MODRM,
/* 9A */  C_MODRM,
/* 9B */  C_MODRM,
/* 9C */  C_MODRM,
/* 9D */  C_MODRM,
/* 9E */  C_MODRM,
/* 9F */  C_MODRM,
/* A0 */  0,
/* A1 */  0,
/* A2 */  0,
/* A3 */  C_MODRM,
/* A4 */  C_MODRM+C_DATA1,
/* A5 */  C_MODRM,
/* A6 */  C_ERROR,
/* A7 */  C_ERROR,
/* A8 */  0,
/* A9 */  0,
/* AA */  0,
/* AB */  C_MODRM,
/* AC */  C_MODRM+C_DATA1,
/* AD */  C_MODRM,
/* AE */  C_MODRM /*C_ERROR*/,
/* AF */  C_MODRM,
/* B0 */  C_MODRM,
/* B1 */  C_MODRM,
/* B2 */  C_MODRM,
/* B3 */  C_MODRM,
/* B4 */  C_MODRM,
/* B5 */  C_MODRM,
/* B6 */  C_MODRM,
/* B7 */  C_MODRM,
/* B8 */  C_MODRM /*C_ERROR*/,
/* B9 */  C_ERROR,
/* BA */  C_MODRM+C_DATA1,
/* BB */  C_MODRM,
/* BC */  C_MODRM,
/* BD */  C_MODRM,
/* BE */  C_MODRM,
/* BF */  C_MODRM,
/* C0 */  C_MODRM,
/* C1 */  C_MODRM,
/* C2 */  C_MODRM /*C_ERROR*/,
/* C3 */  C_MODRM /*C_ERROR*/,
/* C4 */  C_MODRM /*C_ERROR*/,
/* C5 */  C_MODRM /*C_ERROR*/,
/* C6 */  C_MODRM+C_DATA1 /*C_ERROR*/,
/* C7 */  C_MODRM /*C_ERROR*/,
/* C8 */  0,
/* C9 */  0,
/* CA */  0,
/* CB */  0,
/* CC */  0,
/* CD */  0/*C_DATA1*/,
/* CE */  0,
/* CF */  0,
/* D0 */  C_MODRM /*C_ERROR*/,
/* D1 */  C_MODRM /*C_ERROR*/,
/* D2 */  C_MODRM /*C_ERROR*/,
/* D3 */  C_MODRM /*C_ERROR*/,
/* D4 */  C_MODRM /*C_ERROR*/,
/* D5 */  C_MODRM /*C_ERROR*/,
/* D6 */  C_MODRM /*C_ERROR*/,
/* D7 */  C_MODRM /*C_ERROR*/,
/* D8 */  C_MODRM /*C_ERROR*/,
/* D9 */  C_MODRM /*C_ERROR*/,
/* DA */  C_MODRM /*C_ERROR*/,
/* DB */  C_MODRM /*C_ERROR*/,
/* DC */  C_MODRM /*C_ERROR*/,
/* DD */  C_MODRM /*C_ERROR*/,
/* DE */  C_MODRM /*C_ERROR*/,
/* DF */  C_MODRM /*C_ERROR*/,
/* E0 */  C_MODRM /*C_ERROR*/,
/* E1 */  C_MODRM /*C_ERROR*/,
/* E2 */  C_MODRM /*C_ERROR*/,
/* E3 */  C_MODRM /*C_ERROR*/,
/* E4 */  C_MODRM /*C_ERROR*/,
/* E5 */  C_MODRM /*C_ERROR*/,
/* E6 */  C_MODRM /*C_ERROR*/,
/* E7 */  C_MODRM /*C_ERROR*/,
/* E8 */  C_MODRM /*C_ERROR*/,
/* E9 */  C_MODRM /*C_ERROR*/,
/* EA */  C_MODRM /*C_ERROR*/,
/* EB */  C_MODRM /*C_ERROR*/,
/* EC */  C_MODRM /*C_ERROR*/,
/* ED */  C_MODRM /*C_ERROR*/,
/* EE */  C_MODRM /*C_ERROR*/,
/* EF */  C_MODRM /*C_ERROR*/,
/* F0 */  C_MODRM /*C_ERROR*/,
/* F1 */  C_MODRM /*C_ERROR*/,
/* F2 */  C_MODRM /*C_ERROR*/,
/* F3 */  C_MODRM /*C_ERROR*/,
/* F4 */  C_MODRM /*C_ERROR*/,
/* F5 */  C_MODRM /*C_ERROR*/,
/* F6 */  C_MODRM /*C_ERROR*/,
/* F7 */  C_MODRM /*C_ERROR*/,
/* F8 */  C_MODRM /*C_ERROR*/,
/* F9 */  C_MODRM /*C_ERROR*/,
/* FA */  C_MODRM /*C_ERROR*/,
/* FB */  C_MODRM /*C_ERROR*/,
/* FC */  C_MODRM /*C_ERROR*/,
/* FD */  C_MODRM /*C_ERROR*/,
/* FE */  C_MODRM /*C_ERROR*/,
/* FF */  C_ERROR,

/********************** 3 byte ********************************/

/* 00 */  C_ERROR,
/* 01 */  C_ERROR,
/* 02 */  C_ERROR,
/* 03 */  C_ERROR,
/* 04 */  C_ERROR,
/* 05 */  C_ERROR,
/* 06 */  C_ERROR,
/* 07 */  C_ERROR,
/* 08 */  0,
/* 09 */  0,
/* 0A */  0,
/* 0B */  0,
/* 0C */  0,
/* 0D */  0,
/* 0E */  0,
/* 0F */  C_DATA1,
/* 10 */  C_ERROR,
/* 11 */  C_ERROR,
/* 12 */  C_ERROR,
/* 13 */  C_ERROR,
/* 14 */  0,
/* 15 */  0,
/* 16 */  0,
/* 17 */  0,
/* 18 */  C_ERROR,
/* 19 */  C_ERROR,
/* 1A */  C_ERROR,
/* 1B */  C_ERROR,
/* 1C */  C_ERROR,
/* 1D */  C_ERROR,
/* 1E */  C_ERROR,
/* 1F */  C_ERROR,
/* 20 */  0,
/* 21 */  0,
/* 22 */  0,
/* 23 */  C_ERROR,
/* 24 */  C_ERROR,
/* 25 */  C_ERROR,
/* 26 */  C_ERROR,
/* 27 */  C_ERROR,
/* 28 */  C_ERROR,
/* 29 */  C_ERROR,
/* 2A */  C_ERROR,
/* 2B */  C_ERROR,
/* 2C */  C_ERROR,
/* 2D */  C_ERROR,
/* 2E */  C_ERROR,
/* 2F */  C_ERROR,
/* 30 */  C_ERROR,
/* 31 */  C_ERROR,
/* 32 */  C_ERROR,
/* 33 */  C_ERROR,
/* 34 */  C_ERROR,
/* 35 */  C_ERROR,
/* 36 */  C_ERROR,
/* 37 */  C_ERROR,
/* 38 */  C_ERROR,
/* 39 */  C_ERROR,
/* 3A */  C_ERROR,
/* 3B */  C_ERROR,
/* 3C */  C_ERROR,
/* 3D */  C_ERROR,
/* 3E */  C_ERROR,
/* 3F */  C_ERROR,
/* 40 */  C_ERROR,
/* 41 */  C_ERROR,
/* 42 */  C_ERROR,
/* 43 */  C_ERROR,
/* 44 */  C_ERROR,
/* 45 */  C_ERROR,
/* 46 */  C_ERROR,
/* 47 */  C_ERROR,
/* 48 */  C_ERROR,
/* 49 */  C_ERROR,
/* 4A */  C_ERROR,
/* 4B */  C_ERROR,
/* 4C */  C_ERROR,
/* 4D */  C_ERROR,
/* 4E */  C_ERROR,
/* 4F */  C_ERROR,
/* 50 */  C_ERROR,
/* 51 */  C_ERROR,
/* 52 */  C_ERROR,
/* 53 */  C_ERROR,
/* 54 */  C_ERROR,
/* 55 */  C_ERROR,
/* 56 */  C_ERROR,
/* 57 */  C_ERROR,
/* 58 */  C_ERROR,
/* 59 */  C_ERROR,
/* 5A */  C_ERROR,
/* 5B */  C_ERROR,
/* 5C */  C_ERROR,
/* 5D */  C_ERROR,
/* 5E */  C_ERROR,
/* 5F */  C_ERROR,
/* 60 */  C_ERROR,
/* 61 */  C_ERROR,
/* 62 */  C_ERROR,
/* 63 */  C_DATA1,
/* 64 */  C_ERROR,
/* 65 */  C_ERROR,
/* 66 */  C_ERROR,
/* 67 */  C_ERROR,
/* 68 */  C_ERROR,
/* 69 */  C_ERROR,
/* 6A */  C_ERROR,
/* 6B */  C_ERROR,
/* 6C */  C_ERROR,
/* 6D */  C_ERROR,
/* 6E */  C_ERROR,
/* 6F */  C_ERROR,
/* 70 */  C_ERROR,
/* 71 */  C_ERROR,
/* 72 */  C_ERROR,
/* 73 */  C_ERROR,
/* 74 */  C_ERROR,
/* 75 */  C_ERROR,
/* 76 */  C_ERROR,
/* 77 */  C_ERROR,
/* 78 */  C_ERROR,
/* 79 */  C_ERROR,
/* 7A */  C_ERROR,
/* 7B */  C_ERROR,
/* 7C */  C_ERROR,
/* 7D */  C_ERROR,
/* 7E */  C_ERROR,
/* 7F */  C_ERROR,
/* 80 */  C_ERROR,
/* 81 */  C_ERROR,
/* 82 */  C_ERROR,
/* 83 */  C_ERROR,
/* 84 */  C_ERROR,
/* 85 */  C_ERROR,
/* 86 */  C_ERROR,
/* 87 */  C_ERROR,
/* 88 */  C_ERROR,
/* 89 */  C_ERROR,
/* 8A */  C_ERROR,
/* 8B */  C_ERROR,
/* 8C */  C_ERROR,
/* 8D */  C_ERROR,
/* 8E */  C_ERROR,
/* 8F */  C_ERROR,
/* 90 */  C_ERROR,
/* 91 */  C_ERROR,
/* 92 */  C_ERROR,
/* 93 */  C_ERROR,
/* 94 */  C_ERROR,
/* 95 */  C_ERROR,
/* 96 */  C_ERROR,
/* 97 */  C_ERROR,
/* 98 */  C_ERROR,
/* 99 */  C_ERROR,
/* 9A */  C_ERROR,
/* 9B */  C_ERROR,
/* 9C */  C_ERROR,
/* 9D */  C_ERROR,
/* 9E */  C_ERROR,
/* 9F */  C_ERROR,
/* A0 */  C_ERROR,
/* A1 */  C_ERROR,
/* A2 */  C_ERROR,
/* A3 */  C_ERROR,
/* A4 */  C_ERROR,
/* A5 */  C_ERROR,
/* A6 */  C_ERROR,
/* A7 */  C_ERROR,
/* A8 */  C_ERROR,
/* A9 */  C_ERROR,
/* AA */  C_ERROR,
/* AB */  C_ERROR,
/* AC */  C_ERROR,
/* AD */  C_ERROR,
/* AE */  C_ERROR,
/* AF */  C_ERROR,
/* B0 */  C_ERROR,
/* B1 */  C_ERROR,
/* B2 */  C_ERROR,
/* B3 */  C_ERROR,
/* B4 */  C_ERROR,
/* B5 */  C_ERROR,
/* B6 */  C_ERROR,
/* B7 */  C_ERROR,
/* B8 */  C_ERROR,
/* B9 */  C_ERROR,
/* BA */  C_ERROR,
/* BB */  C_ERROR,
/* BC */  C_ERROR,
/* BD */  C_ERROR,
/* BE */  C_ERROR,
/* BF */  C_ERROR,
/* C0 */  C_ERROR,
/* C1 */  C_ERROR,
/* C2 */  C_ERROR,
/* C3 */  C_ERROR,
/* C4 */  C_ERROR,
/* C5 */  C_ERROR,
/* C6 */  C_ERROR,
/* C7 */  C_ERROR,
/* C8 */  C_ERROR,
/* C9 */  C_ERROR,
/* CA */  C_ERROR,
/* CB */  C_ERROR,
/* CC */  C_ERROR,
/* CD */  C_ERROR,
/* CE */  C_ERROR,
/* CF */  C_ERROR,
/* D0 */  C_ERROR,
/* D1 */  C_ERROR,
/* D2 */  C_ERROR,
/* D3 */  C_ERROR,
/* D4 */  C_ERROR,
/* D5 */  C_ERROR,
/* D6 */  C_ERROR,
/* D7 */  C_ERROR,
/* D8 */  C_ERROR,
/* D9 */  C_ERROR,
/* DA */  C_ERROR,
/* DB */  C_ERROR,
/* DC */  C_ERROR,
/* DD */  C_ERROR,
/* DE */  C_ERROR,
/* DF */  C_ERROR,
/* E0 */  C_ERROR,
/* E1 */  C_ERROR,
/* E2 */  C_ERROR,
/* E3 */  C_ERROR,
/* E4 */  C_ERROR,
/* E5 */  C_ERROR,
/* E6 */  C_ERROR,
/* E7 */  C_ERROR,
/* E8 */  C_ERROR,
/* E9 */  C_ERROR,
/* EA */  C_ERROR,
/* EB */  C_ERROR,
/* EC */  C_ERROR,
/* ED */  C_ERROR,
/* EE */  C_ERROR,
/* EF */  C_ERROR,
/* F0 */  C_ERROR,
/* F1 */  C_ERROR,
/* F2 */  C_ERROR,
/* F3 */  C_ERROR,
/* F4 */  C_ERROR,
/* F5 */  C_ERROR,
/* F6 */  C_ERROR,
/* F7 */  C_ERROR,
/* F8 */  C_ERROR,
/* F9 */  C_ERROR,
/* FA */  C_ERROR,
/* FB */  C_ERROR,
/* FC */  C_ERROR,
/* FD */  C_ERROR,
/* FE */  C_ERROR,
/* FF */  C_ERROR

}; // ade64_table[]

// ade64_disasm() -- returns opcode length or 0

int ade64_disasm(IN BYTE* opcode0, IN OUT disasm64_struct* diza)
{
  BYTE* opcode = opcode0;

  disasm64_struct temp_diza;                   // comment if NULL is never passed
  if (diza == NULL) diza = &temp_diza;       //

  memset(diza, 0x00, sizeof(disasm64_struct)); // comment these lines,
  diza->disasm_defdata = 4;                  // and fill structure before call
  diza->disasm_defaddr = 4;                  // -- to allow 16/32-bit disasm

  if (*(WORD*)opcode == 0x0000) return 0;
  if (*(WORD*)opcode == 0xFFFF) return 0;

  DWORD flag = 0;

repeat_prefix:

  BYTE c = *opcode++;

  DWORD t = ade64_table[ c ];

  if (t & C_ANYPREFIX)
  {

    if (flag & t) return 0;    // twice LOCK,SEG,REP,66,67

    flag |= t;

    if (t & C_67)
    {
      diza->disasm_defaddr ^= 2^4;
    }
    else
    if (t & C_66)
    {
      diza->disasm_defdata ^= 2^4;
    }
    else
    if (t & C_SEG)
    {
      diza->disasm_seg = c;
    }
    else
    if (t & C_REP)
    {
      diza->disasm_rep = c;
    }
    else
    if (t & C_REX)
    {
      diza->disasm_rex = c;
    }
    // LOCK

    goto repeat_prefix;

  } // C_ANYPREFIX

  flag |= t;

  diza->disasm_opcode = c;

  if (c == 0x0F)
  {
    c = *opcode++;

    diza->disasm_opcode2 = c;

    flag |= ade64_table[ 256 + c ]; // 2nd flagtable half

    if (flag == C_ERROR) return 0;

    if (c == 0x3a) {
      c = *opcode++;
      flag |= ade64_table[ (256*2) + c ]; // 3nd flagtable half

      if (flag == C_ERROR) return 0;

    }


  }
  else
  if (c == 0xF7)
  {
    if (((*opcode) & 0x38)==0)
      flag |= C_DATA66;
  }
  else
  if (c == 0xF6)
  {
    if (((*opcode) & 0x38)==0)
      flag |= C_DATA1;
  }

  if (flag & C_MODRM)
  {
    c = *opcode++;

    diza->disasm_modrm = c;

    if ((c & 0x38) == 0x20)
    if (diza->disasm_opcode == 0xFF)
      flag |= C_STOP;

    BYTE mod = c & 0xC0;
    BYTE rm  = c & 0x07;

    if (verbose) {
      printf("mod: 0x%x\n", mod);
      printf("rm : 0x%x\n", rm);
    }


    if (mod != 0xC0)
    {
      if (diza->disasm_defaddr == 4)
      {
        if (rm == 4)
        {
          flag |= C_SIB;
          c = *opcode++;
          diza->disasm_sib = c;
          rm = c & 0x07;
        }

        if (mod == 0x40)
        {
          flag |= C_ADDR1;
        }
        else
        if (mod == 0x80)
        {
          flag |= C_ADDR4;
        }
        else
        {
          if (rm == 5)
            flag |= C_ADDR4;
        }
      }
      else // MODRM 16-bit
      {

        if (mod == 0x40)
        {
          flag |= C_ADDR1;
        }
        else
        if (mod == 0x80)
        {
          flag |= C_ADDR2;
        }
        else
        {
          if (rm == 6)
            flag |= C_ADDR2;
        }
      }
    }
  } // C_MODRM

  diza->disasm_flag = flag;

  DWORD a =  flag & (C_ADDR1 | C_ADDR2 | C_ADDR4 | C_ADDR8);
  DWORD d = (flag & (C_DATA1 | C_DATA2 | C_DATA4 | C_DATA8)) >> 12;

  if (flag & C_ADDR67) a += diza->disasm_defaddr;
  if (flag & C_DATA66) {
    d += ((flag & C_DATAQ) && (diza->disasm_rex & REX_W)) ? 8 :  diza->disasm_defdata;
  }

  diza->disasm_addrsize = a;
  diza->disasm_datasize = d;

  DWORD i;
  for(i=0; i<a; i++)
    diza->disasm_addr_b[i] = *opcode++;

  for(i=0; i<d; i++)
    diza->disasm_data_b[i] = *opcode++;

  diza->disasm_len = opcode - opcode0;

  return diza->disasm_len;

} // ade64_disasm()

// ade64_asm() -- returns assembled opcode length

int ade64_asm(OUT BYTE* opcode, IN OUT disasm64_struct* s)
{
  BYTE* opcode0 = opcode;

  if (s->disasm_flag & C_SEG)     *opcode++ = s->disasm_seg;
  if (s->disasm_flag & C_LOCK)    *opcode++ = 0xF0;
  if (s->disasm_flag & C_REP)     *opcode++ = s->disasm_rep;
  if (s->disasm_flag & C_67)      *opcode++ = 0x67;
  if (s->disasm_flag & C_66)      *opcode++ = 0x66;
  *opcode++ = s->disasm_opcode;
  if (s->disasm_flag & C_OPCODE2) *opcode++ = s->disasm_opcode2;
  if (s->disasm_flag & C_MODRM)   *opcode++ = s->disasm_modrm;
  if (s->disasm_flag & C_SIB)     *opcode++ = s->disasm_sib;
  for (DWORD i=0; i<s->disasm_addrsize; i++)
    *opcode++ = s->disasm_addr_b[i];
  for (DWORD i=0; i<s->disasm_datasize; i++)
    *opcode++ = s->disasm_data_b[i];

  return opcode - opcode0;
} // ade64_asm
