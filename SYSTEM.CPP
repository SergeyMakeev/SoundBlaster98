static __segment     dmabufsel;
// ����প� � BIOS
static void biosdelay(unsigned long c)
{
 volatile unsigned long &biosclock = *(volatile unsigned long*) 0x46C;
 unsigned long t0 = biosclock;
 while ((biosclock - t0) < c)
 break;
}
// �뤥���� ������ DOS
// �ᯮ����⥫쭠� �㭪��
// �����頥�:        0-�� 㤠���� �뤥���� ������
//                    �����⥫� �� �뤥����� ������
// ���ணࠬ�஢��� DMA �� ��⠭���� ��।��

static void *dosmalloc(unsigned long len, void __far16 *&rmptr, __segment &pmsel)
{
 len = (len + 15) >> 4;
 REGS r;
 r.w.ax = 0x100;
 r.w.bx = len;
 int386(0x31, &r, &r);
 if (r.x.cflag) return 0;
 pmsel = r.w.dx;
 rmptr = (void __far16 *)((unsigned long)r.w.ax << 16);
 return (void *)((unsigned long)r.w.ax << 4);
}

// �᢮������ ������ DOS
// �ᯮ����⥫쭠� �㭪��
static void dosfree(__segment pmsel)
{
 REGS r;
 r.w.ax = 0x101;
 r.w.dx = pmsel;
 int386(0x31, &r, &r);
}

// �뤥���� DMA �����
// �����頥�:        0-�� 㤠���� �뤥���� DMA �����
//                    �����⥫� �� �뤥���� DMA �����
void *dmaAlloc(int &buflen, __segment &pmsel)
{
 if (buflen > 0x20000)
 buflen = 0x20000;
 buflen = (buflen + 15)&~15;
 void __far16 *rmptr;
 void *ptr = dosmalloc(buflen, rmptr, pmsel);
 if (!ptr) return 0;
 unsigned long a = 0x10000 - ((unsigned long)ptr&0xFFFF);
 if (a < buflen)
  if (a < (buflen >> 1))
   {
    buflen -= a;
    *(unsigned long*)&ptr += a;
   } else buflen = a;
 if (buflen > 0xFF00) buflen = 0xFF00;
 return ptr;
}

// �᢮������ DMA �����
void dmaFree(__segment pmseg)
{
 dosfree(pmseg);
}

// GET INTERRUPT VECTOR
// �ᯮ����⥫쭠� �㭪��
static void far *getvect(unsigned char intno)
{
 REGS r;
 SREGS sr;
 r.h.ah = 0x35;
 r.h.al = intno;
 sr.ds  = sr.es = 0;
 int386x(0x21, &r, &r, &sr);
 return MK_FP(sr.es, r.x.ebx);
}

// SET INTERRUPT VECTOR
// �ᯮ����⥫쭠� �㭪��
static void setvect(unsigned char intno, void far *vect)
{
 REGS r;
 SREGS sr;
 r.h.ah  = 0x25;
 r.h.al  = intno;
 r.x.edx = FP_OFF(vect);
 sr.ds   = FP_SEG(vect);
 sr.es   = 0;
 int386x(0x21, &r, &r, &sr);
}
// ��⠭�������� ����� ⠩���
static void SetTimer(void)
{
 _asm {
       mov al   ,0x36
       out 0x43 ,al
       mov al   ,LoTimer
       out 0x40 ,al
       mov al   ,HiTimer
       out 0x40 ,al
      }
}
// ��⠭�������� ����� ⠩��� 18.2
static void RestoreTimer(void)
{
 _asm {
       mov al   ,0x36
       out 0x43 ,al
       mov al   ,0x00
       out 0x40 ,al
       mov al   ,0x00
       out 0x40 ,al
      }
}