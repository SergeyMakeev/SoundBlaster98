#define LEVELS  33  //количество уровней громкости каждого сампла
//таблицы соотвествий
static unsigned char  MainVolume = 127;//
char                 *Clip8      = 0,
                     *clip8table = 0;
unsigned short       *Clip16     = 0;
//имена все известных программе устройств
static char *devicename[] =
            {
             "Unknown",
             "Sound Blaster 1 or 1.5",
             "Sound Blaster 2.0",
             "Sound Blaster Pro",
             "SoundBlaster Pro 2",
             "Sound Blaster 16",
             "Sound Blaster 16 SCSI - 2",
             "Sound Blaster 16 or AWE32",
             "Sound Blaster AWE64"
            };
//---------------------------------------------------------------------------
static ChannelInfo *Channels;
//устанавливает таблицу соответствия для 8 битного режима
static void setnclip8range( char *buff,unsigned char n)
    {
     _asm cli
      short i;
      if (n < 0) n = 0;
      if (!n)
        {
         for (i = 0;i < MaxChannel * 256;i++)
             buff[i] = 0x80;
          return;
        }
      float c = 256 * 127 / n;
      float a = MaxChannel * 128 - c / 2;
      float b = MaxChannel * 128  + c / 2;
      for (i = 0;i < MaxChannel * 256;i++)
         {
          if (i < a) buff[i] = 0;
             else
                 if (i > b) buff[i] = 255;
                    else
                        buff[i] = ((i - a) * n) / 128;
         }
     _asm sti
    }
//устанавливает все таблицы соответствия для 8 битного режима
static void fillclip8table(void)
     {
       char k = 128 / (LEVELS - 1);
      for (char i = 0; i < LEVELS;i++)
           setnclip8range(clip8table + (i * shiftarray),(i  *  k) - 1);
     }
//устанавливает таблицу соответствия для 8 битного режима
static void setnclip16range(unsigned short *buff,unsigned char n)
    {
     _asm cli
      if (n < 0) n = 0;
      short i;
      if (!n)
        {
         for (i = 0;i < MaxChannel * 256;i++)
             buff[i] = 0x8000;
          return;
        }
      float c = 512 * 127 / n;
      float a = MaxChannel * 128 - c / 2;
      float b = MaxChannel * 128  + c / 2;
      for (i = 0;i < MaxChannel * 256;i++)
         {
          if (i < a) buff[i] = 0;
             else
                 if (i > b) buff[i] = 65535;
                    else
                       buff[i] = (i - a) * n;
         }
     _asm sti
    }
//устанавливает все таблицы соответствия для 16 битного режима
static void fillclip16table(void)
   {
    setnclip16range(Clip16,127);
   }
//----------------------------------------------------------------------------
//возвращает номер канала у которого самая малая приоритетность.
//Пустой канал также учитывается
static short GetMinPriority(void)
    {
     int number = 0;
     if (!ActiveChannel) return number;
     ChannelInfo *Channel = Channels;
     short priority = Channel->priority;
     Channel++;
     for (int i = 1;i < MaxChannel;i++)
       {
        if (priority > Channel->priority)
          {
           number   = i;
           priority = Channel->priority;
          }
        Channel++;
       }
  return number;
    }
// Закрывает канал
void dwse_StopChannel(unsigned short chan)
{
 ChannelInfo *Channel = (Channels + chan);
 if (!initactive || !Channel->busy || chan >= MaxChannel)
      return;
 ActiveChannel--;
 Channel->busy        = 0;
 Channel->priority    = 0;
 if (Channel->handle)
    *(Channel->handle) = -1;
}
// Играть канал: указатель,длина,канал,громкость,looping
void dwse_PlayChannel (unsigned short chan,snd &c)
{
 if (!c.buffer || !c.length || chan >= MaxChannel) return;
 ChannelInfo *Channel = (Channels + chan);
 if (c.volume >= LEVELS) c.volume = LEVELS - 1;
 if (Channel->busy)
    dwse_StopChannel(chan);
 Channel->memory      = (unsigned char *)c.buffer;
 Channel->volume      = c.volume;
 Channel->looping     = c.looping;
 Channel->priority    = c.priority;
 Channel->handle      = c.handle;
 Channel->rate        = c.rate;
 Channel->bits        = c.bits;
 Channel->channels    = c.mode;
 Channel->k           = (c.rate << 8) / rate;
 if (c.bits == _8BIT)
   Channel->shiftlength = (dmasize * Channel->k) >> 8;
 else
   return;
 Channel->halfshiftlength = Channel->shiftlength >> 1;
 Channel->quartershiftlength = Channel->shiftlength >> 2;
 Channel->size_step   = c.length / Channel->shiftlength;
 Channel->size        = c.length;
 Channel->curpos      = 0;
 Channel->curpos_step = 0;
 if (Channel->handle)
   *(Channel->handle) = chan;
 Channel->pause       = 0;
 char level                = c.balance;
 if (level < -15) level    = -15;
 if (level >  15) level    =  15;
 if (level <= 0)
   {
    Channel->balright = LEVELS + (level * 2) - 1;
    Channel->balleft  = LEVELS               - 1;
   }
  else
   {
    Channel->balleft  = LEVELS - (level * 2) - 1;
    Channel->balright = LEVELS               - 1;
   }
 Channel->busy        = 1;
 ActiveChannel++;
}
// Играть звук: структура спараметрами
// Возвращает:        handle-OK,
//                    -1 - Все каналы заняты (не установился).
short dwse_PlaySound (snd &c)
{
 if (!initactive) return -1;
 short number = GetMinPriority();
 if (c.priority > Channels[number].priority)
   {
     dwse_PlayChannel(number,c);
     return number;
   }
return -1;
}
void dwse_CloseAllChannels(void)
    {
     for (short i = 0;i < MaxChannel;i++)
        dwse_StopChannel(i);
    }
// Изменяет громкость каналa
void dwse_SetVolumeChannel (unsigned short chan,char volume)
{
 if (!initactive || !Channels[chan].busy || chan >= MaxChannel)
      return;
 if (volume < 0)   volume = 0;
 if (volume > (LEVELS - 1))  volume = LEVELS - 1;
 Channels[chan].volume     = volume;
}
//Устанавливает баланс при стерео режиме
void dwse_SetBalanceChannel(unsigned short chan,short level)
 {
 /*                             Balance
                           ------|||------
                  left     -16    0     16     right
 */
 if (!initactive || !Channels[chan].busy || chan >= MaxChannel)
      return;
 if (level < -15) level   = -15;
 if (level >  15) level   = 15;
 if (level <= 0)
   {
    Channels[chan].balright = LEVELS + (level * 2) - 1;
    Channels[chan].balleft  = LEVELS               - 1;
    return;
   }
  else
   {
    Channels[chan].balleft  = LEVELS - (level * 2) - 1;
    Channels[chan].balright = LEVELS               - 1;
   }
 }
void dwse_SetNewPriority(unsigned short chan,short p)
    {
     if (!initactive || !Channels[chan].busy || chan >= MaxChannel)
      return;
     Channels[chan].priority = p;
    }
//ставит паузу для конкретного канала
void dwse_PauseChannel (unsigned short chan)
{
 if (!initactive || !Channels[chan].busy || chan >= MaxChannel)
   return;
 Channels[chan].pause = 1;
}
//Продолжить играть канал
void dwse_ContinueChannel (unsigned short chan)
{
 if (!initactive || !Channels[chan].busy || chan >= MaxChannel)
   return;
 Channels[chan].pause = 0;
}
//ставит паузу для всех каналов
void dwse_PauseAllChannels (void)
{
 for (short i = 0;i <  MaxChannel;i++)
    dwse_PauseChannel (i);
}
//продолжает проигрывание для всех каналов
void dwse_ContinueAllChannels(void)
{
 for (short i = 0;i <  MaxChannel;i++)
    dwse_ContinueChannel(i);
}

//устанавливает глобальную громкость.
void dwse_SetMainVolume(short vol)
    {
     if (!initactive) return;
     if (vol < 0)   vol = 0;
     if (vol > 127) vol = 127;
     MainVolume = vol;
     if (output16bit)
        setnclip16range(Clip16,MainVolume);
       else
        setnclip8range(Clip8,MainVolume);
    }
//утанавливает стерео или моно режим
char dwse_SetPlayMode(unsigned char mode)
    {
     if (initactive) return 0;
     if (mode)
      {
       //for sb.pro
       if ((DSPver >= 300) && (rate <= 22727) && !output16bit
            && (DSPver < 400))
          {
           rate  *= 2;
           stereo = 1;
           return STEREO;
          }
       //for sb16,AWE32,AWE64 and others.
       if (DSPver >= 400)
         {
          stereo = 1;
          return STEREO;
         }
      }
       else
          stereo = 0;
     return MONO;
    }
//Устанавливает в системную переменную частоту проигрывания
unsigned short dwse_SetPlayRate(unsigned short num)
    {
      if (initactive) return 0;
      if (num < 4000) num = 4000;
      //sb 1.x
      if ((DSPver >= 100) && (DSPver < 200))
        {
         if (num > 22222) num = 22222;
         rate = num;
         return rate;
        }
      //pro,pro2
      if ((DSPver >= 300) && (DSPver < 400))
         if (stereo)
           {
            if (num > 22727) num = 22727;
            rate = num;
            return rate;
           }
       //sb16,AWE32,AWE64
      if (num > 45454) num = 45454;
      rate = num;
    return rate;
    }
//устанавливает 16 или 8 битное проигрывание
char dwse_SetPlayQuality(unsigned char mode)
    {
     if (initactive) return 0;
     if (mode)
       {
       //SB16,AWE32,AWE64
       if (DSPver >= 400)
         {
          output16bit = 1;
          return _16BIT;
         }
       } else
           output16bit = 0;
     return _8BIT;
    }
void dwse_SetReverse(unsigned char mode)
    {
      if (mode)
        {
        //reverse
         rl = 1;
         rr = 0;
        }
        else
        {
         //normal
         rl = 0;
         rr = 1;
        }
    }
//перед проигрыванием установить количество каналов 1..64
char dwse_SetNumberChannels(char number)
    {
     if (initactive) return -1;
     if (number > 64) number = 64;
     if (number < 1)  number = 1;
     MaxChannel              = number;
     return number;
    }
static short SetChannels(void)
    {
     if (initactive) return -1;
     summa7        = (MaxChannel - 1) * 128;
     shiftarray    = 256 * MaxChannel;
     Clip8 = new char[shiftarray];
     if (!Clip8)
       return -1;
     clip8table = new char [LEVELS * shiftarray];
     if (!clip8table)
       return -1;
     Clip16     = new unsigned short [shiftarray];
     if (!Clip16)
       return -1;
     Channels   = new ChannelInfo [MaxChannel];
     if (!Channels)
       return -1;
     fillclip8table();
     fillclip16table();
     return 0;
    }
//вешает свою процедуру на прерывание
void dwse_InstallUserProcedure(void (*userproc)())
    {
     UserProcedure = userproc;
    }
//убирает процедуру юзера
void dwse_UninstallUserProcedure(void)
    {
     UserProcedure = VoidProcedure;
    }
//определяет текущее устройство
char *dwse_GetDeviceName(void)

    {
     if ((DSPver == 413) || (DSPver == 412))
       return devicename[7];
     if (DSPver == 411)
       return devicename[6];
     if ((DSPver >= 400) && (DSPver < 411))
       return devicename[5];
     if ((DSPver >= 301) && (DSPver < 400))
       return devicename[4];
     if (DSPver == 300)
       return devicename[3];
     if ((DSPver >= 200) && (DSPver <= 300))
       return devicename[2];
     if ((DSPver >= 100) && (DSPver < 200))
       return devicename[1];
     if (DSPver > 413)
       return devicename[8];
    return devicename[0];
    }
short dwse_GetMajorVer(void)
     {
     return DSPver / 100;
     }
short dwse_GetMinorVer(void)
     {
     return DSPver - (DSPver / 100) * 100;
     }
unsigned short dwse_GetTimerRate(void)
     {
      return TimerRate;
     }
unsigned short dwse_GetActiveChannels(void)
     {
     return ActiveChannel;
     }