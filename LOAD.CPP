#include "wav.h"
#include "wad.h"
struct DBW
      {
       wadheader h;
       void      *boot;
       wadfile   *first;
       int       handler;
      };
long FileLength(FILE *in)
    {
     long curpos = ftell(in);
     fseek(in,0L,SEEK_END);
     long newpos = ftell(in);
     fseek(in,curpos,SEEK_SET);
     return newpos;
    }
int LoadSoundToBuffer(char *path,snd &p)
   {
    WAVHeader w;
    memset(&p,0,sizeof(snd));
    FILE *in = fopen(path,"rb");
    if (!in) return -1;
    fread(&w,1,sizeof(WAVHeader),in);
    if (memcmp(w.IdRIFF,"RIFF",4) ||
        memcmp(w.IdWAVE,"WAVE",4))
      {
       fclose(in);
       return -2;
      }
    if (w.Block.Compress != 1)
     {
      fclose(in);
      return -3;
     }
    if (w.Block.Channels > 1)
       w.Block.Rate1 *= 2;
    p.length = w.DataSize;
    p.buffer = malloc(p.length);
    if (!p.buffer)
      {
       fclose(in);
       return -5;
      }
    fread(p.buffer,1,p.length,in);
    if (w.Block.BitsPerNum == 16)
      {
       unsigned char *p8  = (unsigned char *)p.buffer;
       unsigned char *p16 = p8 + 1;
       for (long i = 0;i < p.length;i++)
          {
           *p8 = *p16 - 128;
           p8++;
           p16++;
          }
       p.length /= 2;
       p.buffer = realloc(p.buffer,p.length);
      }
    fclose(in);
    p.priority = 1;
    p.volume   = 31;
    p.balance  = 0;
    p.looping  = UNLOOP;
    p.rate     = w.Block.Rate1;
    p.bits     = _8BIT;
    p.mode     = MONO;
    p.handle   = 0;
    return 0;
   }
void UnLoadSoundFromBuffer(snd &p)
    {
    free(p.buffer);
    memset(&p,0,sizeof(snd));
    }
DBW *DBW_OpenRD(char *filename)
   {
    DBW *p = new DBW;
    if (!p) return 0;
    p->handler = open(filename,O_BINARY | O_RDONLY);
    if (p->handler < 0)
      {
       delete p;
       return 0;
      }
     read(p->handler,&p->h,sizeof(wadheader));
     if (memcmp(p->h.ID,id,4))
       {
        delete p;
        close(p->handler);
        return 0;
       }
     p->boot = new wadfile[MAX_FILES];
     if (!p->boot)
       {
       close(p->handler);
       delete p;
       return 0;
       }
    lseek(p->handler,p->h.BOOT,SEEK_SET);
    read(p->handler,p->boot,sizeof(wadfile) * MAX_FILES);
    p->first = (wadfile *)p->boot;
    return p;
   }
void DBW_Close(DBW *p)
    {
     close(p->handler);
     delete p->boot;
     delete p;
    }
int DBW_fileread(DBW *p,char *fname,void *buffer,long length,long begpos = 0)
    {
     wadfile *file = p->first;
     char ok = 0;
     for (int i = 0;i < MAX_FILES;i++)
        {
         if (!strcmp(file->name,fname))
           {
            ok = 1;
            break;
           }
         file++;
        }
     //file not found
     if (!ok) return -1;
     lseek(p->handler,file->position + begpos,SEEK_SET);
     if (length > file->length) length = file->length;
     if (read(p->handler,buffer,length) != length)
       return -2;
     return 0;
    }
long DBW_filelength(DBW *p,char *fname)
    {
     wadfile *file = p->first;
     char ok = 0;
     for (int i = 0;i < MAX_FILES;i++)
        {
         if (!strcmp(file->name,fname))
           {
            ok = 1;
            break;
           }
         file++;
        }
     if (!ok) return -1;
     return file->length;
    }
int DBW_LoadSoundToBuffer(DBW *h,char *fname,snd &p)
   {
    WAVHeader w;
    memset(&p,0,sizeof(snd));
    long length = DBW_filelength(h,fname);
    if (length < 0) return -1;
    DBW_fileread(h,fname,&w,sizeof(WAVHeader));
    if (memcmp(w.IdRIFF,"RIFF",4) ||
        memcmp(w.IdWAVE,"WAVE",4))
       return -2;
    if (w.Block.Compress != 1)
      return -3;
    if (w.Block.Channels > 1)
       w.Block.Rate1 *= 2;
    p.length = w.DataSize;
    p.buffer = malloc(p.length);
    if (!p.buffer)
       return -5;
    DBW_fileread(h,fname,p.buffer,p.length,sizeof(WAVHeader));
    if (w.Block.BitsPerNum == 16)
      {
       unsigned char *p8  = (unsigned char *)p.buffer;
       unsigned char *p16 = p8 + 1;
       for (long i = 0;i < p.length;i++)
          {
           *p8 = *p16 - 128;
           p8++;
           p16++;
          }
       p.length /= 2;
       p.buffer = realloc(p.buffer,p.length);
      }
    p.priority = 1;
    p.volume   = 31;
    p.balance  = 0;
    p.looping  = UNLOOP;
    p.rate     = w.Block.Rate1;
    p.bits     = _8BIT;
    p.mode     = MONO;
    p.handle   = 0;
    return 0;
   }
void DBW_UnLoadSoundFromBuffer(snd &p)
    {
     free(p.buffer);
     memset(&p,0,sizeof(snd));
    }