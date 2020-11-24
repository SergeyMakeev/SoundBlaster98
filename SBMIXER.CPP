/*                 REAL TIME DIGITAL SOUND MIXER                        */
/*             COPYRIGHT (c) 1998 DIGITAL WORLD Co.                     */
/*                  (Stereo and mono support)                           */
/*                                                                      */
//                             Написано
//***********Поддержка Sound Blaster'a********************************
//         Программист : Макеев Сергей (Joker)
//  Советы и подсказки : Кузнецов Александр (Alexander)
//
//**********RealTime Sound Mixing ver 1.0******************************
//         Программист : Горин Максим (Maxx)
//           Some Help : Кузнецов Александр (Alexander)
//
//
//**********RealTime Sound Mixing ver 2.000.13*************************
//         Программист : Макеев Сергей (Joker)
//                Идеи : МИКРОАРТ
//
//**********RealTime Sound Mixing ver 3.000.13*************************
//         Программист : Макеев Сергей (Joker)
//                Идеи : Горин Максим (Maxx)
//           Some Help : Кузнецов Александр (Alexander)
//  Идея 16 bit mixing : Ethan Brodsky
//  Идея Timer Restore : Niklas Beisert
//**********RealTime Sound Mixing ver 4.000.00 моно *******************
//        Программист  : Горин Максим (Maxx),
//               Идеи  : Макеев Сергей (Joker)

//**********RealTime Sound Mixing ver 5.000.00 стерео *****************
//        Программист  : Горин Максим (Maxx),
//               Идеи  : Макеев Сергей (Joker)
//********** RealTime Sound Mixing ver 5.001  стерео *****************
//        Динамическое изменение всех параметров.
//        Программист  : Горин Максим (Maxx),
//********** RealTime Sound Mixing ver 6.00  стерео *****************
//        Динамическое изменение всех параметров.
//        Программист  : Горин Максим (Maxx),
//        Any Rate

#include <dos.h>
#include <io.h>
//#include <FCNTL.H>
#include <string.H>
#include <stdio.h>
#include <stdlib.h>
#include <MALLOC.H>
#include "load.cpp"
#define LOBYTE(l)         ((unsigned char)(l))
#define HIBYTE(l)         ((unsigned char)((l) >> 8))
// Таблица DMA портов
static unsigned char dmaPorts[8][6] = {
                                        {0x00,0x01,0x0A,0x0B,0x0C,0x87},
                                        {0x02,0x03,0x0A,0x0B,0x0C,0x83},
                                        {0x04,0x05,0x0A,0x0B,0x0C,0x81},
                                        {0x06,0x07,0x0A,0x0B,0x0C,0x82},
                                        {0xC0,0xC2,0xD4,0xD6,0xD8,0x8F},
                                        {0xC4,0xC6,0xD4,0xD6,0xD8,0x8B},
                                        {0xC8,0xCA,0xD4,0xD6,0xD8,0x89},
                                        {0xCC,0xCE,0xD4,0xD6,0xD8,0x8A}

                                      };
//начальный размер 2 * DMA буффера при Hz >= 11025 Hz, 8 бит.
//начальный размер DMA буффера при Hz < 11025 Hz, 8 бит.
//При увеличении частоты, битности и т.д. размер дма буффера увеличивается!
#define               BEGSIZE 512
//системные переменные для работы с dma buffer'ом.
static int           dmasize        = 1,
                     halfdmasize    = 1,
                     quarterdmasize = 1;
char                 MaxChannel     = 8;  //Максимальное число каналов для MIXER'a
const unsigned short TimerRate      = 100, //Частота с которой работает таймер в герцах
                     OldRate        = TimerRate / 18.2;
unsigned short       ActiveChannel  = 0; //Кол-во активных каналов
char                 *buf           = 0; //Указатель на DMA буфер
static unsigned char dmaCh,
                     *dmaPort;
static int            dmaLen         = 1;
unsigned short        rate           = 11025;         //Частота для SB
unsigned char         stereo         = 0;       //0-mono,1-stereo
unsigned char         output16bit    = 0;  //0-8bit,1-16bit
static short          sbPort         = 0x220,//Sound Blaster Port
                      sbIRQ          = 0x5 , //Sound Blaster Interrupt
                      sbDMA          = 1,    //Sound Blaster DMA chanel 8 bit
                      sbDMA16        = 5,    //Sound Blaster DMA chanel 16 bit
                      sbUseVer       = 1,    //Версия DSP взятая из BLASTER Tx
                      sbModel        = 1;    //Номер модели Sound Blaster'a Tx
unsigned short        DSPver         = 0,    //Версия DSP взятая из DSP
                      sbTimerConst;
static unsigned long summa7,
                     shiftarray;
// Определяеться в главном теле программы
int                  len            = 1;
unsigned short       dmalen         = 1,
                     sblen          = 1,
                     *VirtualBuffer = 0,
                     *vbuf,
                     dmaoffset      = 0;
static unsigned char irqNum,
                     irqIntNum,
                     irqPort;
static void far      *irqOldInt;
static unsigned char irqOldMask,
                     irqPreEOI;
static unsigned long tmCount        = 1;
static unsigned char          HiTimer        = 0,
                     LoTimer        = 0,
                     part           = 1,
                     rl             = 0,
                     rr             = 1,
                     initactive     = 0;
static void (*playproc)();
static void (*irqRoutine)();
void VoidProcedure(void);
static void (*UserProcedure)()      = VoidProcedure;
//подменяет подготовку буффера
static void (*PrepareVirtualBuffer)();
static void (*CopyData)(char *,unsigned short *);
static void (__far __interrupt *tmOldTimer)();
static void (*tmTimerRoutine)();
void (__interrupt __far *oldVec)();
short GetSettingsFromDevice();
// Отключение прерываний
short _disableint();
#pragma aux _disableint value [ax] = "pushf" "pop ax" "cli"
// Включение прерываний
void _restoreint(short);
#pragma aux _restoreint parm [ax] = "push ax" "popf"
#include "sndfunc.cpp"
//#include "convert.cpp"
// Подсчитать sbTimerConst из частоты
void CalculateTimerConst(unsigned short rt)
{
 sbTimerConst = 256 - 1000000 / rt;
}
// Получить из BLASTER port,irq,dma,dma16,version
// из переменной среды.
// Возвращает:   0-Переменная BLASTER не установлена
//               1-OK
static unsigned char GetBlasterInfo()
{
 sbModel  = -1;
 sbPort   = -1;
 sbIRQ    = -1;
 sbDMA    = -1;
 sbDMA16  = -1;
 //дополнительная информация
 sbUseVer = -1;
 char *s = getenv("BLASTER");
 if (!s) return 0;
  while (1)
  {
   while (*s == ' ')
   s++;
   if (!*s) break;
    switch (*s++)
    {
     case 'a': case 'A':
      sbPort = strtoul(s, 0, 16);
      break;
     case 'i': case 'I':
      sbIRQ = strtoul(s, 0, 10);
      break;
     case 'd': case 'D':
      sbDMA = strtoul(s, 0, 10);
      break;
     case 'h': case 'H':
      sbDMA16 = strtoul(s, 0, 10);
      break;
     case 't': case 'T':
      sbUseVer = strtoul(s, 0, 10);
      switch (sbUseVer)
      {
       case 6:         sbUseVer = 4; break;
       case 2: case 4: sbUseVer = 3; break;
       case 3:         sbUseVer = 2; break;
       case 1:         sbUseVer = 1; break;
       default: sbUseVer = -1;
      }
      break;
    }
    while ((*s != ' ') && *s)
      s++;
  }
  return 1;
}

// Взять из SB порта
static unsigned char inpSB(unsigned char p)
{
 return inp( sbPort + p );
}

// Закинуть в SB порт
static void outpSB(unsigned char p, unsigned char v)
{
 outp( sbPort + p , v);
}

// Закинуть в DSP
static void WriteDSP(unsigned char v)
{
 while ( inpSB ( 0xC ) & 0x80 );
 outpSB( 0xC , v );
}

// Взять из DSP
static unsigned char ReadDSP()
{
 while (!( inpSB ( 0xE ) & 0x80 ));
 return inpSB( 0xA );
}
//подключаются функции распределения памяти и навеска на прерывания и т.д.
#include "system.cpp"
// Проинитить SB
static void InitSB()
{
outpSB(0x6,1);
inpSB(0x6);
inpSB(0x6);
inpSB(0x6);
inpSB(0x6);
inpSB(0x6);
inpSB(0x6);
inpSB(0x6);
inpSB(0x6);
outpSB(0x6,0);
}
static void InitSBp()
{
  outpSB(0x6,1);
  biosdelay(3);
  outpSB(0x6,0);
  biosdelay(3);
}
// Сбросить DSP
static void ResetDSP()
{
 outpSB(0x6,1);
 biosdelay(3);
 outpSB(0x6,0);
 biosdelay(3);
}

// Установить частоту
static setrateSB(unsigned char r)
{
 WriteDSP(0x40);
 WriteDSP(r);
}

// Подтвердить возврат из прерываний
static void IRQacknowl()
{
 inpSB(0xE);
}

// Подтвердить возврат из прерываний для 16 битных операций
static void IRQacknowl16()
{
 inpSB(0xF);
}

// Включить колонки
static void spkronSB()
{
 WriteDSP(0xD1);
}

// Выключить колонки
static void spkroffSB()
{
 WriteDSP(0xD3);
}
// Засунуть в MIXER
static void writeMixer(unsigned char p, unsigned char v)
{
 outpSB ( 0x4, p);
 outpSB ( 0x5, v);
}
// Взять из MIXER'a
static unsigned char readMixer(unsigned char p)
{
 outpSB       ( 0x4,p);
 return inpSB ( 0x5 );
}

// Получить DSP версию
static unsigned short getVersion()
{
WriteDSP(0xE1);
 while(1)
 {
  unsigned char verhi = ReadDSP();
  if (verhi == 0xAA) continue;
  return ((unsigned short)verhi * 100) + ReadDSP();
 }
}

// Играет на SB 1.x
static void sb1Player()
{
 IRQacknowl();
 WriteDSP(0x14);
 WriteDSP( LOBYTE( sblen - 1));
 WriteDSP( HIBYTE( sblen - 1));
}

// Играет на SB Pro и SB 2.x
// (Для Бена)
static void sb2proPlayerHS()
{
 IRQacknowl();
 WriteDSP(0x91);
}
// Играет на SB 16 в 8 битном режиме
static void sb16Player8()
{
 IRQacknowl();
 WriteDSP(0x45);
}
// Играет на SB 16 в 16 битном режиме (Класс)
static void sb16Player16()
{
 WriteDSP(0x47);
 IRQacknowl16();
}
// Програмиррует DMA
void dmaStart(int ch, void *buf, int buflen, int autoinit)
{
dmaCh                 = ch & 7;
dmaPort               = dmaPorts[dmaCh];
unsigned long realadr = (unsigned long)buf;
if (dmaCh & 4)
  {
   realadr >>= 1;
   buflen    = (buflen + 1) >> 1;
  }
dmaLen = buflen - 1;
int is = _disableint();
outp(dmaPort[2] , dmaCh|4);
outp(dmaPort[3] , (autoinit?0x18 : 0x08) | (ch&3));
outp(dmaPort[3] , autoinit|(ch&3));
outp(dmaPort[4] , 0);
outp(dmaPort[0] , realadr);
outp(dmaPort[0] , (realadr >> 8));
outp(dmaPort[5] , (( (unsigned long) buf) >> 16));
outp(dmaPort[5] , (( (unsigned long) buf) >> 16));
outp(dmaPort[1] , dmaLen);
outp(dmaPort[1] , (dmaLen >> 8));
outp(dmaPort[2] , dmaCh&3);
_restoreint(is);
}
void dmaStop()
{
 outp(dmaPort[4],0);
 outp(dmaPort[2],dmaCh | 4);
}

// Получить текущую позицию в DMA буфере
int dmaGetBufPos()
{
 unsigned int a,b;
 int is = _disableint();
 while(1)
  {
   outp( dmaPort[4] , 0xFF);
   a  = inp (dmaPort[1]);
   a += inp (dmaPort[1]) << 8;
   b  = inp (dmaPort[1]);
   b += inp (dmaPort[1]) << 8;
   if (abs(a - b) <= 64) break;
  }
 _restoreint(is);
 int p = dmaLen - b;
 if (p < 0) return 0;
 if (p >= dmaLen) return 0;
 if (dmaCh & 4) p <<= 1;
 return p;
}
// Висит на прерываниях SB из нее вызываеться моя функция PLAY
static void __interrupt irqInt()
{
 if (irqPreEOI)
  {
   outp(irqPort, inp(irqPort)|(1 << (irqNum&7)));
   if (irqNum & 8)
   outp(0xA0,0x20);
   outp(0x20,0x20);
   irqRoutine();
   outp(irqPort, inp(irqPort)&~(1<<(irqNum&7)));
  } else
       {
        irqRoutine();
        if (irqNum&8)
        outp(0xA0,0x20);
        outp(0x20,0x20);
       }
}

// Устанавливает мой обработчик прерываний на SB_IRQ
void irqInit(unsigned char inum, void (*routine)(), unsigned char pre)
{
 irqPreEOI  = pre;
 inum      &= 15;
 irqNum     = inum;
 irqIntNum  = (inum & 8) ? (inum + 0x68) : (inum + 8);
 irqPort    = (inum & 8) ? 0xA1:0x21;
 irqOldInt  = getvect(irqIntNum);
 irqOldMask = inp(irqPort) & (1 << (inum & 7));
 irqRoutine = routine;
 //unknown model or sb not present
 setvect(irqIntNum, irqInt);
 outp(irqPort, inp(irqPort)&~(1 << (inum&7)));
}

// Устанавливает старый обработчик прерываний на SB_IRQ
void irqClose()
{
 outp(irqPort, inp(irqPort)|irqOldMask);
 setvect(irqIntNum, irqOldInt);
}

// Начать вывод на SB
void StartDAC(void)
{
 unsigned short DSP = DSPver / 100;
 switch (DSP)
  {
  case 1:
   playproc = sb1Player;
   break;
  case 2: case 3:
   playproc = sb2proPlayerHS;
   break;
  case 4:
   if (!output16bit) playproc = sb16Player8;
     else
      playproc = sb16Player16;
   break;
  }
 irqInit(sbIRQ,playproc,0);
 if (!output16bit) dmaStart(sbDMA,  buf, dmalen, 0x58);
   else
     dmaStart(sbDMA16,buf, dmalen, 0x58);
 switch (DSP)
  {
  case 1:
   WriteDSP(0x14);
   WriteDSP( LOBYTE( sblen - 1) );
   WriteDSP( HIBYTE( sblen - 1) );
   break;
  case 2: case 3:
   if (!stereo) writeMixer( 0xE, readMixer(0xE) & ~2 | 0x20);
      else
        writeMixer( 0xE, readMixer(0xE) & ~2 | 0x22);
   WriteDSP(0x48);
   WriteDSP( LOBYTE( sblen - 1) );
   WriteDSP( HIBYTE( sblen - 1) );
   WriteDSP(0x1C);
   break;
  case 4:
   if (output16bit)
    {
     WriteDSP(0xB6);
     sblen /= 2;
     if (stereo == 1) WriteDSP((0x20)|(0x10));
     if (stereo != 1) WriteDSP((0x00)|(0x10));
     WriteDSP( LOBYTE( sblen - 1 ) );
     WriteDSP( HIBYTE( sblen - 1 ) );
    }
    else
    {
     WriteDSP(0xC6);
     if (stereo == 1) WriteDSP((0x20)|(0x00));
     if (stereo != 1) WriteDSP((0x00)|(0x00));
     WriteDSP( LOBYTE( sblen - 1) );
     WriteDSP( HIBYTE( sblen - 1) );
    }
   break;
   }
}
// Остановить SB
static void StopDAC()
{
 irqClose();
 dmaStop ();
 ResetDSP();
 IRQacknowl  ();
 IRQacknowl16();
 dmaFree(dmabufsel);
}
// Поверить есть SB на SB_PORT или нет
// Возвращает:        0-Ошибка
//                    1-OK
static unsigned char CheckSB()
{
 unsigned short rez = 0;
 _asm {
       mov   al, 1
       mov   dx, 0x06
       add   dx,sbPort
       out   dx, al
       in    al, dx
       in    al, dx
       in    al, dx
       in    al, dx
       mov   al, 0
       out   dx, al
       mov   cx, 0x64
      @@DataWait:
       mov   dx, 0x0E
       add   dx,sbPort
       in    al, dx
       test  al, 0x80
       jnz   @@YesData
       loop  @@DataWait
       jmp   @@Exit
      @@YesData:
       mov   dx, 0x0A
       add   dx,sbPort
       in    al, dx
       cmp   al, 0x0AA
       je    @@YepSB
       loop  @@DataWait
       jmp   @@Exit
      @@YepSB:
       mov   ax, 1
       jmp   @@continue
      @@Exit:
       mov   ax, 0
      @@continue:
       mov   rez, ax
      }
return rez;
}

// Собрать всю информацию о SB
// Возвращает:     -1 - Переменная BLASTER не установлена
//                 -2 - отсутствует SB, или занят другим приложением!
//                  0 - OK
short dwse_GetSBInfo()
{
 if (!GetBlasterInfo())
   if (GetSettingsFromDevice())
     return  -1;
 if (!CheckSB())
   {
    sbModel = -1;
    return -2;
   }
 DSPver  = getVersion();
 sbModel = DSPver / 100;
 return 0;
}
#include "mix.cpp"
// Создать из всех каналов Virtual DMA buffer 8 bit
static void PrepareVirtualBuffer8(void)
{
 memset (vbuf,0,dmasize << 1);
 ChannelInfo *Channel = Channels;
 for (char numb = 0;numb < MaxChannel;numb++)
  {
   if (Channel->busy && !Channel->pause)
    {
      MixVoices (vbuf,Channel->memory + Channel->curpos,dmasize,Channel->volume,Channel->k);
      Channel->curpos_step++;
      Channel->curpos += Channel->shiftlength;
      if (Channel->curpos_step >= Channel->size_step)
        {
         if (!Channel->looping)
           dwse_StopChannel(numb);
         if (Channel->looping)
           {
            Channel->curpos = 0;
            Channel->curpos_step = 0;
           }
         }
    }
     else MixVoidSound (vbuf,dmasize);
  Channel++;
  }
}
// Создать из всех каналов Virtual DMA buffer 16 bit
static void PrepareVirtualBuffer16(void)
{
 memset (vbuf,0,dmasize << 1);
 ChannelInfo *Channel = Channels;
 for (char numb = 0;numb < MaxChannel;numb++)
    {
     if (Channel->busy && !Channel->pause)
       {
        MixVoices (vbuf,Channel->memory + Channel->curpos
        ,halfdmasize,Channel->volume,Channel->k);
        Channel->curpos_step++;
        Channel->curpos += Channel->halfshiftlength;
        if (Channel->curpos_step >= (Channel->size_step << 1))
          {
           if (!Channel->looping)
             dwse_StopChannel(numb);
           if (Channel->looping)
             {
              Channel->curpos      = 0;
              Channel->curpos_step = 0;
             }
          }
       }
        else MixVoidSound (vbuf,dmasize);
        Channel++;
    }
}
//Для стерео режима
static void PrepareVirtualBufferStereo8(void)
{
 memset (vbuf,0,dmasize << 1);
 ChannelInfo *Channel = Channels;
 for (char numb = 0;numb < MaxChannel;numb++)
    {
     if (Channel->busy && !Channel->pause)
       {
        //Левый канал
        MixVoicesStereo (vbuf + rl,Channel->memory + Channel->curpos,
                         halfdmasize,Channel->volume,Channel->balright,Channel->k);

        //Правый канал
        MixVoicesStereo (vbuf + rr,Channel->memory + Channel->curpos,
                         halfdmasize,Channel->volume,Channel->balleft,Channel->k);
        Channel->curpos_step++;
        Channel->curpos += Channel->halfshiftlength;
        if (Channel->curpos_step >= (Channel->size_step << 1))
          {
           if (!Channel->looping)
             dwse_StopChannel(numb);
           if (Channel->looping)
             {
              Channel->curpos      = 0;
              Channel->curpos_step = 0;
             }
          }
       }
        else MixVoidSound (vbuf,dmasize);
        Channel++;
    }
}
//16 бит для сиерео режима
static void PrepareVirtualBufferStereo16(void)
{
 memset (vbuf,0,dmasize << 1);
 ChannelInfo *Channel = Channels;
 for (char numb = 0;numb < MaxChannel;numb++)
    {
     if (Channel->busy && !Channel->pause)
       {
        //Левый канал
        MixVoicesStereo (vbuf + rl,Channel->memory + Channel->curpos,
                         quarterdmasize,Channel->volume,Channel->balright,Channel->k);
        //Правый канал
        MixVoicesStereo (vbuf + rr,Channel->memory + Channel->curpos,
                         quarterdmasize,Channel->volume,Channel->balleft,Channel->k);
        Channel->curpos_step++;
        Channel->curpos += Channel->quartershiftlength;
        if (Channel->curpos_step >= (Channel->size_step << 2))
          {
           if (!Channel->looping)
             dwse_StopChannel(numb);
           if (Channel->looping)
             {
              Channel->curpos      = 0;
              Channel->curpos_step = 0;
             }
          }
       }
        else MixVoidSound (vbuf,dmasize);
        Channel++;
    }
}
// Засунуть новые данные в DMA буфер
// эта проца формирует буфер до того, как до него дошел
// указатель Blaster'а.
void UpDateDmaBuffer(void)
{
//Для первой части
if ((dmaoffset >= dmasize) && !part)
  {
    //указатель на выбранную процедуру
    PrepareVirtualBuffer();
   ++part &= 1;
    CopyData(buf,vbuf);
  }
//Для второй части
if ((dmaoffset < dmasize) && part)
  {
   PrepareVirtualBuffer();
   ++part &= 1;
   CopyData(buf + dmasize,vbuf);
  }
}
// Висит на прерываниях таймера, подсовывает SB новые данные для проигрывания
void PlayerTimer(void)
{
 dmaoffset = dmaGetBufPos();
 UpDateDmaBuffer();
}
//эмулирует пустой sb.
void NoneSB(void)
{
}
void VoidProcedure(void)
{
}
//Здесь стоит моя процедура и вызываеться старая
static void __interrupt tmTimerHandler()
{
 tmCount++;
 UserProcedure();
 if (tmCount == OldRate)
   {
    tmCount   = 1;
    tmOldTimer();
   }
outp(0x20,0x20);
tmTimerRoutine();
}
//Поставить мою процедуру на таймер
static char tmInit(void (*rout)())
{
  tmCount        = 1;
  tmTimerRoutine = rout;
  tmOldTimer     = (void (__far __interrupt *)()) getvect(0x08);
  setvect(0x08, tmTimerHandler);
  return 1;
}

// Убрать мою процедуру с таймера
static void tmClose()
{
  setvect(0x08, tmOldTimer);
  outp(0x43, 0x34);
  outp(0x40, 0x00);
  outp(0x40, 0x00);
}
// Подсчитывает LoTimer и HiTimer из частоты
static void CalculateTimerRate(unsigned short TimerHz)
{
 LoTimer = LOBYTE(1190000 / TimerHz);
 HiTimer = HIBYTE(1190000 / TimerHz);
}

// Начать играть на SB
void PlaySB(void)
{
 CalculateTimerRate(TimerRate);
 SetTimer();
 if (sbModel == -1)
   tmInit(NoneSB);
   else
    {
     tmInit(PlayerTimer);
     StartDAC();
    }
}
// Проинитить Mixer

static short SetParams(void)
{
   //Если sb не найден, то выйти с ошибкой.
   if (sbModel == -1)
     return -1;
   //Если частота проигрывания >= 11025
   if (rate >= 11025)
      dmasize       = (BEGSIZE * (rate / 11025)) * (stereo + 1)
                      *(output16bit + 1) ;
     else
      dmasize       = (BEGSIZE / 2 * (rate / 4000)) * (stereo + 1)
                      *(output16bit + 1) ;
   halfdmasize   = dmasize / 2;
   quarterdmasize= dmasize / 4;
   dmalen = sblen= dmasize * 2;
   len           = 4 * dmasize;
   VirtualBuffer = new unsigned short[dmasize]; //Виртуальный DMA буффер 16битный
   if (!VirtualBuffer)
      return -2;
   if (output16bit)
     memset(VirtualBuffer,0x00,dmasize);
     else
     memset(VirtualBuffer,0x80,dmasize);
   buf = (unsigned char*)dmaAlloc(len, dmabufsel);
   if (!buf)
      return -2;
   if (output16bit)
     memset(buf,0x00,len);
     else
     memset(buf,0x80,len);
   vbuf          = VirtualBuffer;
   SetChannels();
   memset(Channels,0,sizeof(ChannelInfo) * MaxChannel);
return 0;
}
//Первое (ручное) обновление DMA
//потом DMA обновляеться из прерываний
static void FirstDMAUpdate (void)
{
 PrepareVirtualBuffer();
}
// Делает все для Initialization SB
static short AutoInitSB (void)
{
 short error = SetParams();
 if (error)
   {
    sbModel = -1;
    return error;
   }
 //переобозначения функций в зависимости от установки
 if (!stereo)
   {
     if (output16bit)
       {
        CopyData = CopyData16;
        PrepareVirtualBuffer = PrepareVirtualBuffer16;
       }
      else
       {
        CopyData = CopyData8;
        PrepareVirtualBuffer = PrepareVirtualBuffer8;
       }
   }
    else
       {
         if (output16bit)
           {
            CopyData = CopyData16;
            PrepareVirtualBuffer = PrepareVirtualBufferStereo16;
           }
           else
              {
              //пока для 8 бит
               CopyData = CopyData8;
               PrepareVirtualBuffer = PrepareVirtualBufferStereo8;
              }
       }
  dwse_SetMainVolume(MainVolume);
  InitSB();
  ResetDSP();
  IRQacknowl();
  IRQacknowl16();
  CalculateTimerConst(rate);
  setrateSB(sbTimerConst);
  spkronSB();
  FirstDMAUpdate();
  return 0;
}
//возвращает :0 - успешно
short dwse_InitMixer(unsigned short rate,char bit,char mode)
    {
     if (initactive)
        return 0;
     dwse_SetPlayRate(rate);
     dwse_SetPlayQuality(bit);
     dwse_SetPlayMode(mode);
     short error = AutoInitSB();
     if (error)
       initactive = 0;
     else
       initactive = 1;
     PlaySB();
     dwse_SetMainVolume(127);
     return error;
    }
void dwse_CloseMixer(void)
{
 initactive = 0;
 if (sbModel != -1)
  {
   spkroffSB();
   StopDAC();
  }
 tmClose();
 RestoreTimer();
 if (VirtualBuffer)
    delete VirtualBuffer; //Виртуальный DMA буффер 16битный
 if (buf)
   free(buf);//освобождаем выделенную память
 if (Clip8)
    delete Clip8;
 if (clip8table)
    delete clip8table;
 if (Clip16)
    delete Clip16;
 if (Channels)
    delete Channels;
}
#include "test.cpp"