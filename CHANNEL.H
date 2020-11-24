struct ChannelInfo
{
unsigned char busy;           //0-Свободен, 1-Занят          1
char          *memory;        //Указатель на область памяти  1
unsigned long size;           //Размер в байтах              4
unsigned long size_step;      //Размер в DMA буфферах        4
unsigned long curpos;         //Текущее смещение в байтах    4
unsigned long curpos_step;    //Текущее смещение в DMA буфферах 4
unsigned char volume;            //Громкость, чем больше тем тише 1
unsigned char looping;        //Looping 1-да 0-нет                1
unsigned char balright;       //громкость звука на каждый канал   1
unsigned char balleft;        //только для стерео режима          1
short         priority;       //приоритетность для звука          2
unsigned char pause;          //флаг паузы
short         *handle;        //хандл звука, хранит номер сампла
//-----------------------------------------------------------------------------
unsigned rate;
char channels;
char bits;
unsigned k;
unsigned shiftlength;
unsigned halfshiftlength;
unsigned quartershiftlength;
};
struct snd
      {
       void *buffer;//УКАЗАТЕЛЬ НА БУФЕР
       unsigned long length;//длина рава
       short priority;//его приоритетность звучания
       unsigned char volume;//громкость исполнения
       char looping;//флаг зацикливания
       short *handle;//указатель на handler звука, при закрытии канала
                     //возвращает -1.
       char balance;//начальная установка баланса для проигрывания
       char mode;//mono or stereo
       char bits;//nuber bits of sample
       unsigned rate;//rate of sample
      };