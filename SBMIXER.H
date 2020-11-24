#if !defined (_SBMIXER)
   #define _SBMIXER
#define NORMAL  0x00
#define REVERSE 0xFF
#define LOOP    0xFF
#define UNLOOP  0x00
#define STEREO  0xFF// режимы проигрывания
#define MONO    0x00//
#define _16BIT  0xFF//битность звука
#define _8BIT   0x00//
#include "channel.h"
short dwse_GetMajorVer(void);
short dwse_GetMinorVer(void);
unsigned short dwse_GetTimerRate(void);
void dwse_InstallUserProcedure(void (*userproc)());
void dwse_UninstallUserProcedure(void);
char *dwse_GetDeviceName(void);
char dwse_SetPlayQuality(unsigned char mode);
void dwse_SetReverse(unsigned char mode);
char dwse_SetNumberChannels(char number);
void dwse_StopChannel(unsigned short chan);
void dwse_PlayChannel (unsigned short chan,snd &c);
short dwse_PlaySound (snd &c);
void dwse_CloseAllChannels(void);
void dwse_SetVolumeChannel (unsigned short chan,char volume);
void dwse_SetBalanceChannel(unsigned short chan,short level);
void dwse_SetNewPriority(unsigned short num,short p);
void dwse_PauseChannel (unsigned short chan);
void dwse_ContinueChannel (unsigned short chan);
void dwse_PauseAllChannels (void);
void dwse_ContinueAllChannels(void);
void dwse_SetMainVolume(short vol);
char dwse_SetPlayMode(unsigned char mode);
unsigned short dwse_SetPlayRate(unsigned short num);
#include "sbmixer.cpp"
#endif
