#include <snes.hpp>
#include <stdint.h>

// CICLONE: hooks para o runtime do host (host_runner). Defaults FRACOS (weak) para
// que libsnes e a GUI linkem sem o runtime; o host_runner fornece versões fortes que
// encaminham ao FpgaModel ativo.
extern "C" {
  int     __attribute__((weak)) ciclone_chip_active(void) { return 0; }
  uint8_t __attribute__((weak)) ciclone_chip_snes_read(uint32_t addr) { (void)addr; return 0; }
  void    __attribute__((weak)) ciclone_chip_snes_write(uint32_t addr, uint8_t data) { (void)addr; (void)data; }
}

namespace SNES {

SD2SNES sd2snes;

void SD2SNES::init()   {}
void SD2SNES::enable() {}
void SD2SNES::power()  {}
void SD2SNES::reset()  {}

// Memory: endereço SNES completo (24 bits) via Direct map.
uint8 SD2SNES::read(unsigned addr)              { return ciclone_chip_snes_read(addr & 0xffffff); }
void  SD2SNES::write(unsigned addr, uint8 data) { ciclone_chip_snes_write(addr & 0xffffff, data); }

// MMIO: janela SNESCMD $2A00-$2FFF (addr já vem nesse range; o FpgaModel decodifica).
uint8 SD2SNES::mmio_read(unsigned addr)              { return ciclone_chip_snes_read(addr & 0xffffff); }
void  SD2SNES::mmio_write(unsigned addr, uint8 data) { ciclone_chip_snes_write(addr & 0xffffff, data); }

}
