short rightirq = 0;
#define IRQS  6
#define PORTS 8
#define LDMA  3
#define HDMA  3
short irqs[IRQS]  = {
                     0x00,
                     0x03,
                     0x05,
                     0x07,
                     0x09,
                     0xA
                    };
short ports[PORTS] = {
                     0x210,
                     0x220,
                     0x230,
                     0x240,
                     0x250,
                     0x260,
                     0x270,
                     0x280
                     };
char ldma[LDMA]   = {
                     0x00,
                     0x01,
                     0x03
                    };
char hdma[HDMA]   = {
                     0x05,
                     0x07,
                     0x9
                    };
static void resetSB16()
{
  inpSB(0xF);
}
static void resetSB()
{
  inpSB(0xE);
}
static void test()
{
  rightirq = 1;
}
static char TestPort(unsigned short port, int delay)
{
  sbPort = port;
  if (delay)
    InitSBp();
  else
    InitSB();
  int i;
  for (i = 0; i < 1000; i++)
    if (inpSB(0xE) & 0x80)
      return inpSB(0xA) == 0xAA;
  return 0;
}

static char TestIrq(unsigned char irq)
{
  irqInit(irq, test, 0);
  InitSB();
  setrateSB(0xD3);
  rightirq = 0;
  WriteDSP(0x80);
  WriteDSP(0x03);
  WriteDSP(0x00);
  for (unsigned short i=0; i != 0xFFFF; i++)
  if (rightirq)
    break;
  irqClose();
  InitSB();
  resetSB();
  return rightirq;
}

static char TestDMA(unsigned char irq, unsigned char dma)
{
  outp(0x0A,4);
  outp(0x0A,5);
  outp(0x0A,7);

  irqInit(irq, test, 0);
  dmaStart(dma, 0, 10, 0x48);

  InitSB();
  setrateSB(0xD3);
  rightirq=0;
  WriteDSP(0x14);
  WriteDSP(0x03);
  WriteDSP(0x00);
  for (unsigned short i = 0; i != 0xFFFF; i++)
  if (rightirq)
    break;
  irqClose();
  dmaStop();
  InitSB();
  resetSB();
  return rightirq;
}

static char TestDMA16(unsigned char irq, unsigned char dma)
{
  outp(0x0A,4);
  outp(0x0A,5);
  outp(0x0A,7);
  outp(0xD4,5);
  outp(0xD4,6);
  outp(0xD4,7);
  irqInit(irq, test, 0);
  dmaStart(dma, 0, 2, 0x48);
  InitSB();
  setrateSB(0xEA);
  rightirq=0;
  WriteDSP(0xB6);
  WriteDSP(0x10);
  WriteDSP(0x00);
  WriteDSP(0x00);
  for (unsigned i = 0; i != 0xFFFF; i++)
    if (rightirq)
      break;
  irqClose();
  dmaStop();
  InitSB();
  resetSB16();
  return rightirq;
}
short GetSettingsFromDevice()
    {
           sbPort  = -1,
           sbIRQ   = -1,
           sbDMA   = -1,
           sbDMA16 = -1;
     for (int i = 0;i < PORTS;i++)
        if (TestPort(ports[i],1))
          {
           sbPort = ports[i];
           break;
          }
    if (sbPort == -1) return -1;
    for (i = 0;i < IRQS;i++)
       if (TestIrq(irqs[i]))
         {
          sbIRQ = irqs[i];
          break;
         }
    if (sbIRQ == -1) return -1;
    for (i = 0;i < LDMA;i++)
       if (TestDMA(sbIRQ,ldma[i]))
         {
          sbDMA = ldma[i];
          break;
         }
    if (sbDMA == -1) return -1;
    for (i = 0;i < HDMA;i++)
       if (TestDMA16(sbIRQ,hdma[i]))
         {
          sbDMA16 = hdma[i];
          break;
         }
    return 0;
    }