//
// Created by Romain on 27/07/2023.
//

#ifndef GOBOUEMU_IO_RAM_H
#define GOBOUEMU_IO_RAM_H

#include "types.h"
#include "units/mmu.h"

Struct {
	u8
reg_P1	    , //0xFF00
reg_SB	    , //0xFF01
reg_SC	    , //0xFF02
reg_FF03    ,
reg_DIV	    , //0xFF04
reg_TIMA	, //0xFF05
reg_TMA	    , //0xFF06
reg_TAC	    , //0xFF07
reg_FF08[7] ,
reg_IF	    , //0xFF0F
reg_NR10	, //0xFF10
reg_NR11	, //0xFF11
reg_NR12	, //0xFF12
reg_NR13	, //0xFF13
reg_NR14	, //0xFF14
reg_FF15    ,
reg_NR21	, //0xFF16
reg_NR22	, //0xFF17
reg_NR23	, //0xFF18
reg_NR24	, //0xFF19
reg_NR30	, //0xFF1A
reg_NR31	, //0xFF1B
reg_NR32	, //0xFF1C
reg_NR33	, //0xFF1D
reg_NR34	, //0xFF1E
reg_FF1F    ,
reg_NR41	, //0xFF20
reg_NR42	, //0xFF21
reg_NR43	, //0xFF22
reg_NR44	, //0xFF23
reg_NR50	, //0xFF24
reg_NR51	, //0xFF25
reg_NR52	, //0xFF26
reg_FF27[9] ,
reg_WaveRAM[16], //0xFF30
reg_LCDC	, //0xFF40
reg_STAT	, //0xFF41
reg_SCY     , //0xFF42
reg_SCX     , //0xFF43
reg_LY      , //0xFF44
reg_LYC     , //0xFF45
reg_DMA     , //0xFF46
reg_BGP     , //0xFF47
reg_OBP0	, //0xFF48
reg_OBP1	, //0xFF49
reg_WY	    , //0xFF4A
reg_WX	    , //0xFF4B
reg_FF4C    ,
reg_KEY1	, //0xFF4D
reg_VBK	    , //0xFF4F
reg_FF50    ,
reg_HDMA1	, //0xFF51
reg_HDMA2	, //0xFF52
reg_HDMA3	, //0xFF53
reg_HDMA4	, //0xFF54
reg_HDMA5	, //0xFF55
reg_RP	    , //0xFF56
reg_FF57[17],
reg_BCPS	, //0xFF68
reg_BCPD	, //0xFF69
reg_OCPS	, //0xFF6A
reg_OCPD	, //0xFF6B
reg_OPRI	, //0xFF6C
reg_FF6D[3] ,
reg_SVBK	, //0xFF70
reg_FF71[5] ,
reg_PCM12	, //0xFF76
reg_PCM34	, //0xFF77
reg_FF78[8] ;
} IO_RAM;

extern IO_RAM ioRam;

// IO Addresses

#define P1	    0xFF00
#define SB	    0xFF01
#define SC	    0xFF02
#define DIV	    0xFF04
#define TIMA	0xFF05
#define TMA	    0xFF06
#define TAC	    0xFF07
#define IF	    0xFF0F
#define NR10	0xFF10
#define NR11	0xFF11
#define NR12	0xFF12
#define NR13	0xFF13
#define NR14	0xFF14
#define NR21	0xFF16
#define NR22	0xFF17
#define NR23	0xFF18
#define NR24	0xFF19
#define NR30	0xFF1A
#define NR31	0xFF1B
#define NR32	0xFF1C
#define NR33	0xFF1D
#define NR34	0xFF1E
#define NR41	0xFF20
#define NR42	0xFF21
#define NR43	0xFF22
#define NR44	0xFF23
#define NR50	0xFF24
#define NR51	0xFF25
#define NR52	0xFF26
#define WaveRAM	0xFF30
#define LCDC	0xFF40
#define STAT	0xFF41
#define SCY     0xFF42
#define SCX     0xFF43
#define LY      0xFF44
#define LYC     0xFF45
#define DMA     0xFF46
#define BGP     0xFF47
#define OBP0	0xFF48
#define OBP1	0xFF49
#define WY	    0xFF4A
#define WX	    0xFF4B
#define KEY1	0xFF4D
#define VBK	    0xFF4F
#define HDMA1	0xFF51
#define HDMA2	0xFF52
#define HDMA3	0xFF53
#define HDMA4	0xFF54
#define HDMA5	0xFF55
#define RP	    0xFF56
#define BCPS	0xFF68
#define BCPD	0xFF69
#define OCPS	0xFF6A
#define OCPD	0xFF6B
#define OPRI	0xFF6C
#define SVBK	0xFF70
#define PCM12	0xFF76
#define PCM34	0xFF77

#define JOYP P1
#define BGPI BCPS
#define BGPD BCPD
#define OBPI OCPS
#define OBPD OCPD

// IO Direct Access

#define ioP1	    ioRam.reg_P1
#define ioSB	    ioRam.reg_SB
#define ioSC	    ioRam.reg_SC
#define ioDIV	    ioRam.reg_DIV
#define ioTIMA	ioRam.reg_TIMA
#define ioTMA	    ioRam.reg_TMA
#define ioTAC	    ioRam.reg_TAC
#define ioIF	    ioRam.reg_IF
#define ioNR10	ioRam.reg_NR10
#define ioNR11	ioRam.reg_NR11
#define ioNR12	ioRam.reg_NR12
#define ioNR13	ioRam.reg_NR13
#define ioNR14	ioRam.reg_NR14
#define ioNR21	ioRam.reg_NR21
#define ioNR22	ioRam.reg_NR22
#define ioNR23	ioRam.reg_NR23
#define ioNR24	ioRam.reg_NR24
#define ioNR30	ioRam.reg_NR30
#define ioNR31	ioRam.reg_NR31
#define ioNR32	ioRam.reg_NR32
#define ioNR33	ioRam.reg_NR33
#define ioNR34	ioRam.reg_NR34
#define ioNR41	ioRam.reg_NR41
#define ioNR42	ioRam.reg_NR42
#define ioNR43	ioRam.reg_NR43
#define ioNR44	ioRam.reg_NR44
#define ioNR50	ioRam.reg_NR50
#define ioNR51	ioRam.reg_NR51
#define ioNR52	ioRam.reg_NR52
#define ioWaveRAM	ioRam.reg_WaveRAM
#define ioLCDC	ioRam.reg_LCDC
#define ioSTAT	ioRam.reg_STAT
#define ioSCY     ioRam.reg_SCY
#define ioSCX     ioRam.reg_SCX
#define ioLY      ioRam.reg_LY
#define ioLYC     ioRam.reg_LYC
#define ioDMA     ioRam.reg_DMA
#define ioBGP     ioRam.reg_BGP
#define ioOBP0	ioRam.reg_OBP0
#define ioOBP1	ioRam.reg_OBP1
#define ioWY	    ioRam.reg_WY
#define ioWX	    ioRam.reg_WX
#define ioKEY1	ioRam.reg_KEY1
#define ioVBK	    ioRam.reg_VBK
#define ioHDMA1	ioRam.reg_HDMA1
#define ioHDMA2	ioRam.reg_HDMA2
#define ioHDMA3	ioRam.reg_HDMA3
#define ioHDMA4	ioRam.reg_HDMA4
#define ioHDMA5	ioRam.reg_HDMA5
#define ioRP	    ioRam.reg_RP
#define ioBCPS	ioRam.reg_BCPS
#define ioBCPD	ioRam.reg_BCPD
#define ioOCPS	ioRam.reg_OCPS
#define ioOCPD	ioRam.reg_OCPD
#define ioOPRI	ioRam.reg_OPRI
#define ioSVBK	ioRam.reg_SVBK
#define ioPCM12	ioRam.reg_PCM12
#define ioPCM34	ioRam.reg_PCM34

#define ioJOYP ioP1
#define ioBGPI ioBCPS
#define ioBGPD ioBCPD
#define ioOBPI ioOCPS
#define ioOBPD ioOCPD

#define ioLCDC0 (ioLCDC & 0x01)
#define ioLCDC1 (ioLCDC & 0x02)
#define ioLCDC2 (ioLCDC & 0x04)
#define ioLCDC3 (ioLCDC & 0x08)
#define ioLCDC4 (ioLCDC & 0x10)
#define ioLCDC5 (ioLCDC & 0x20)
#define ioLCDC6 (ioLCDC & 0x40)
#define ioLCDC7 (ioLCDC & 0x80)

// Methods

extern writer ** io_write_handlers;

#define direct_read_io(addr) direct_reader(io, addr)
#define read_io(addr) indirect_reader(io, addr, memoryMap.dma_lock)

// direct: check io mem access
// raw: write value only (emu side), then, the readable one

static inline void direct_raw_write_io(u16 addr, u8 value) {
	memoryMap.io[addr] = value; // ((u8*)ioRam)[addr & 0xff]
}

static inline void direct_write_io(u16 addr, u8 value) {
	io_write_handlers[addr](addr, value);
}

static inline void raw_write_io(u16 addr, u8 value) {
	if (!memoryMap.dma_lock) direct_raw_write_io(addr, value);
}

static inline void write_io(u16 addr, u8 value) {
	if (!memoryMap.dma_lock) direct_write_io(addr, value);
}

#endif //GOBOUEMU_IO_RAM_H
