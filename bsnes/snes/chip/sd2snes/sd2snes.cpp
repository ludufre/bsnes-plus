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

void SD2SNES::init()   {}
void SD2SNES::enable() { ciclone_snoop = true; }
void SD2SNES::power()  { ciclone_chip_reset(); }
void SD2SNES::reset()  { ciclone_chip_reset(); }

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
