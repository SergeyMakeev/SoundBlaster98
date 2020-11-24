struct ChannelInfo
{
unsigned char busy;           //0-��������, 1-�����          1
char          *memory;        //�����⥫� �� ������� �����  1
unsigned long size;           //������ � �����              4
unsigned long size_step;      //������ � DMA ������        4
unsigned long curpos;         //����饥 ᬥ饭�� � �����    4
unsigned long curpos_step;    //����饥 ᬥ饭�� � DMA ������ 4
unsigned char volume;            //�஬�����, 祬 ����� ⥬ �� 1
unsigned char looping;        //Looping 1-�� 0-���                1
unsigned char balright;       //�஬����� ��㪠 �� ����� �����   1
unsigned char balleft;        //⮫쪮 ��� ��८ ०���          1
short         priority;       //�ਮ��⭮��� ��� ��㪠          2
unsigned char pause;          //䫠� ����
short         *handle;        //堭�� ��㪠, �࠭�� ����� ᠬ���
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
       void *buffer;//��������� �� �����
       unsigned long length;//����� ࠢ�
       short priority;//��� �ਮ��⭮��� ���砭��
       unsigned char volume;//�஬����� �ᯮ������
       char looping;//䫠� ��横�������
       short *handle;//㪠��⥫� �� handler ��㪠, �� �����⨨ ������
                     //�����頥� -1.
       char balance;//��砫쭠� ��⠭���� ������ ��� �ந��뢠���
       char mode;//mono or stereo
       char bits;//nuber bits of sample
       unsigned rate;//rate of sample
      };