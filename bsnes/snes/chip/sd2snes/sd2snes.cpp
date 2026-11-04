#include <snes.hpp>
#include <stdint.h>

// CICLONE: hooks para o runtime do host (host_runner). Defaults FRACOS (weak) para
// que libsnes e a GUI linkem sem o runtime; o host_runner fornece versões fortes que
// encaminham ao FpgaModel ativo.
extern "C" {
  int     __attribute__((weak)) ciclone_chip_active(void) { return 0; }
  uint8_t __attribute__((weak)) ciclone_chip_snes_read(uint32_t addr) { (void)addr; return 0; }
  void    __attribute__((weak)) ciclone_chip_snes_write(uint32_t addr, uint8_t data) { (void)addr; (void)data; }
  int     __attribute__((weak)) ciclone_chip_msu_enabled(void) { return 0; }
  void    __attribute__((weak)) ciclone_chip_irq_vector(uint32_t vector, int native) { (void)vector; (void)native; }
  void    __attribute__((weak)) ciclone_chip_bus_snoop(uint32_t addr, uint8_t data, int write) { (void)addr; (void)data; (void)write; }
  void    __attribute__((weak)) ciclone_chip_reset(void) {}
}

namespace SNES {

SD2SNES sd2snes;
bool ciclone_snoop = false;

// The cart's uPD7725 (fpga_dsp: DSP-1..4). NECDSP stays a coprocessor of every sd2snes cart,
// idle until the FPGA model hands it a program (ciclone_necdsp_load), so its thread always
// exists for the scheduler and costs one step per 8000 clocks while no DSP game runs.
bool ciclone_necdsp_idle = true;

static void necdsp_setup() {
  necdsp.revision = NECDSP::Revision::uPD7725;
  necdsp.frequency = 8000000;
  necdsp.power();
  necdsp.srmask = 1; necdsp.srtest = 1;   // the model passes A0: 1 = status, 0 = data
  necdsp.drmask = 1; necdsp.drtest = 0;
}

void SD2SNES::init()   {}
void SD2SNES::enable() { ciclone_snoop = true; }
void SD2SNES::power()  { ciclone_chip_reset(); necdsp_setup(); }
void SD2SNES::reset()  { ciclone_chip_reset(); necdsp.reset(); }

// Memory: endereço SNES completo (24 bits) via Direct map.
uint8 SD2SNES::read(unsigned addr)              { return ciclone_chip_snes_read(addr & 0xffffff); }
void  SD2SNES::write(unsigned addr, uint8 data) { ciclone_chip_snes_write(addr & 0xffffff, data); }

// MMIO: MSU-1 $2000-$2007, copier $2020-$202F, S-RTC $2800/$2801 + janela SNESCMD $2A00-$2FFF
// (o FpgaModel decodifica). $2000-$2007 sem FEAT_MSU1: ninguém dirige o barramento (open bus).
uint8 SD2SNES::mmio_read(unsigned addr) {
  if((addr & 0xfff8) == 0x2000 && !ciclone_chip_msu_enabled()) return cpu.regs.mdr;
  return ciclone_chip_snes_read(addr & 0xffffff);
}
void  SD2SNES::mmio_write(unsigned addr, uint8 data) { ciclone_chip_snes_write(addr & 0xffffff, data); }

}

extern "C" {
// FPGA model -> NECDSP (runs on the SNES thread, from the model's decode)
void ciclone_necdsp_load(const uint32_t *pgm, int npgm, const uint16_t *dat, int ndat) {
  using namespace SNES;
  for(unsigned n = 0; n < 2048; n++) necdsp.programROM[n] = n < (unsigned)npgm ? pgm[n] : 0;
  for(unsigned n = 0; n < 1024; n++) necdsp.dataROM[n] = n < (unsigned)ndat ? dat[n] : 0;
  necdsp.reset();
  ciclone_necdsp_idle = npgm == 0;
}
uint8_t ciclone_necdsp_read(int a0)           { return SNES::necdsp.read(a0 & 1); }
void    ciclone_necdsp_write(int a0, uint8_t d) { SNES::necdsp.write(a0 & 1, d); }
}
