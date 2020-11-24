struct fmt
      {
       unsigned short Compress;
       unsigned short Channels;
       long Rate1;
       long Rate2;
       unsigned short Block;
       unsigned short BitsPerNum;
      };
struct WAVHeader
      {
       char IdRIFF[4];
       long FileSize;
       char IdWAVE[4];
       char Idfmt[4];
       long fmtSize;
       fmt  Block;
       char IdData[4];
       long DataSize;
      };
