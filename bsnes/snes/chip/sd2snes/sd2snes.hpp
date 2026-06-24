// CICLONE: chip de cartucho sd2snes — fina ponte do barramento SNES para o FpgaModel
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
