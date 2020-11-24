//shiftarray
//clip8table
//summa7
//Clip8
//dmasize
//Clip16
//halfdmasize
inline void MixVoices(unsigned short *buf1,char *buf2,unsigned long len,
                      unsigned char curvol,unsigned k)
    {
     _asm
          {
           xor ebx,ebx
           mov bl,curvol
           imul ebx,shiftarray
           add ebx,summa7
           mov ecx,len
           mov esi,buf1
           mov edi,buf2
           mov eax,k
           xor edx,edx
     repeat:
           push eax
           xor eax,eax
           push edx
           shr edx,8
           mov al,[edi + edx] //get number
           pop edx
           add eax,ebx
           push edx
           mov edx,clip8table
           mov al,[edx + eax]
           pop edx
           xor ah,ah
           add [esi],ax
           pop eax
           add edx,eax
           add esi,2
     loop repeat
          }
    }
void MixVoidSound(unsigned short *buf1,unsigned long len)
    {
     _asm
        {
         mov esi,buf1
         mov ecx,len
         mov ax,128
       l1:
         add [esi],ax
         add esi,2
         loop l1
        }
    }
//микширование для стерео режима 8 бит (запись байта через один)
inline void MixVoicesStereo(unsigned short *buf1,char *buf2,unsigned long len,
                            unsigned char curvol,unsigned char balance,unsigned k)
    {
     _asm
          {
           xor ecx,ecx
           mov cl,balance
           mov eax,shiftarray
           imul ecx,eax
           add ecx,summa7

           xor ebx,ebx
           mov bl,curvol
           imul ebx,eax
           add ebx,summa7
           mov eax,len
           mov esi,buf1
           mov edi,buf2
           xor edx,edx
     repeat:
           push eax
           xor eax,eax
           push edx
           shr edx,8
           mov al,[edi + edx] //get number
           pop edx
           push edx
           add eax,ecx
           mov edx,clip8table
           mov al,[edx + eax]
           shl eax,24
           shr eax,24
           add eax,ebx
           //mov edx,clip8table
           mov al,[edx + eax]
           pop edx
           add edx,k
           xor ah,ah
           add [esi],ax
           add esi,4
           pop eax
           dec eax
           jnz repeat
          }
    }
//Создать из Virtual Buffer 8 bit нормальный смикшенный DMA buffer 8 bit
inline void CopyData8(char *buf1,unsigned short *buf2)
    {
     _asm {
           mov edi,buf1
           mov esi,buf2
           mov edx,Clip8
           mov ecx,dmasize
           xor eax,eax
       repeat:
           mov ax,[esi]
           mov bl,[edx+eax]
           mov [edi],bl
           add esi,2
           inc edi
       loop repeat
          }
    }
//Создать из Virtual Buffer 16 bit нормальный смикшенный DMA buffer 16 bit
inline void CopyData16(char *buf1,unsigned short *buf2)
    {
     _asm {
           mov edi,buf1
           mov esi,buf2
           mov ecx,halfdmasize
           mov edx,Clip16
       repeat:
           xor eax,eax
           mov ax,[esi]
           add ax,ax
           mov bx,[edx + eax]
           sub bx,32768
           mov [edi],bx
           add esi,2
           add edi,2
       loop repeat
          }
    }
