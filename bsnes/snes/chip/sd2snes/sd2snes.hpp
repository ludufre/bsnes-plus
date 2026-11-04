// CICLONE: chip de cartucho sd2snes - fina ponte do barramento SNES para o FpgaModel
// do host (via hooks C ciclone_chip_*). É mapeado SOMENTE quando o manifesto traz o
// elemento <sd2snes> (host_runner); a GUI carregando ROMs normais não o ativa.
//   - Memory: recebe o endereço SNES COMPLETO (mapeado em MapMode::Direct).
//   - MMIO:   cobre a janela SNESCMD $2A00-$2FFF.
class SD2SNES : public MMIO, public Memory {
public:
  void init();
  void enable();
  void power();
  void reset();

  uint8 read(unsigned addr);
  void  write(unsigned addr, uint8 data);

  uint8 mmio_read(unsigned addr);
  void  mmio_write(unsigned addr, uint8 data);
};

extern SD2SNES sd2snes;

// CICLONE: what the cart edge sees beyond its own address ranges. The real FPGA snoops the
// whole A/B bus: cheat.v spots the 4 stack pushes before an interrupt vector fetch and hijacks
// it (in-game hook), ctx.v mirrors PPU/CPU register writes into PSRAM, main.v watches $4200
// (auto-joypad) and $4016 (manual pad polling). bsnes hands those to the host through these;
// ciclone_snoop is only on while an sd2snes cart is mapped, so other carts pay one branch.
extern bool ciclone_snoop;
extern bool ciclone_necdsp_idle;   // NECDSP has no program yet (sd2snes.cpp)
extern "C" {
  void ciclone_chip_irq_vector(uint32_t vector, int native);   // CPU is about to fetch it
  void ciclone_chip_bus_snoop(uint32_t addr, uint8_t data, int write);
  void ciclone_chip_reset(void);                              // /RESET strobe
}
