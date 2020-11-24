#define MAX_FILES 2048L
#define MAX_FILE  23
char id[] = "DWWAD";
char copyright[] = "Digital World Co.";
struct wadheader
      {
       char ID[5];
       char info[32];
       char majorver;
       char minorver;
       //boot pointer
       unsigned long BOOT;
       unsigned long numfiles;
       char reserved[32];
      };
struct wadfile
      {
       char name[24];
       unsigned long length;
       unsigned long position;
       unsigned char type;
       unsigned char reserved[7];
      };
