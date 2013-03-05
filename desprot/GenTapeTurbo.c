#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __DMC__
  #define strcasecmp stricmp
#endif
unsigned char *mem, *precalc;
char *ext, *command;
unsigned char rem= 0, inibit= 0, tzx= 0, wav= 0, channel_type= 1,
              checksum, turbo, mod;
FILE *fi, *fo;
int i, j, k, l, ind= 0, nextsilence= 0;
float silence;
unsigned short length, param, frequency= 44100;

char char2hex(char value, char * name){
  if( value<'0' || value>'f' || value<'A' && value>'9' || value<'a' && value>'F' )
    printf("\nInvalid character %c or '%s' not exists\n", value, name),
    exit(-1);
  return value>'9' ? 9+(value&7) : value-'0';
}

int parseHex(char * name, int index){
  int flen= strlen(name);
  if( name[0]=='\'' )
    for ( i= 1; i<11 && name[i]!='\''; ++i )
      mem[i+6]= name[i];
  else if( ~flen & 1 ){
    flen>>= 1;
    flen>10 && index==7 && (flen= 10);
    for ( i= 0; i < flen; i++ )
      mem[i+index]= char2hex(name[i<<1|1], name) | char2hex(name[i<<1], name) << 4;
    ++i;
  }
  while( ++i<12 )
    mem[i+5]= ' ';
  return flen;
}

int main(int argc, char* argv[]){
  mem= (unsigned char *) malloc (0x20000);
  if( argc==1 )
    printf("\nGenTapeTurbo v0.01, converts basic, hdata and data to turbo, 5 Mar 2013\n\n"),
    printf("  GenTapeTurbo [<frequency>] [<channel_type>] <output_file>\n"),
    printf("          [ basic <name> <startline> <input_file>\n"),
    printf("          | hdata <name> <address>   <input_file>\n"),
    printf("          |  data                    <input_file>\n"),
    printf("          | pilot <pilot_ts> <pilot_ms>\n"),
    printf("          | pulse <M> <pulse1_ts> <pulse2_ts> .. <pulseM_ts>\n"),
    printf("          | pause <pause_ms>\n"),
    printf("          |  pure <zero_ts> <one_ts> <pause_ms> <input_file>\n"),
    printf("          | turbo <pilot_ts> <syn1_ts> <syn2_ts> <zero_ts> <one_ts>\n"),
    printf("                                 <pilot_ms> <pause_ms> <input_file>\n"),
    printf("          | plug-xxx-N <param1> <param2> .. <paramN> ]\n\n"),
    printf("  <output_file>  Target file, between TAP, TZX or WAV file\n"),
    printf("  <name>         Up to 10 chars name between single quotes or in hexadecimal\n"),
    printf("  <startline>    In decimal, first BASIC line to execute\n"),
    printf("  <address>      In hexadecimal, address of the binary block\n"),
    printf("  <input_file>   Hexadecimal string or filename as data origin of that block\n"),
    printf("  <zero_ts> <one_ts> <syn1_ts> <syn2_ts> <pilot_ts>\n"),
    printf("                 Length of zero/one/syncs/pilot pulses at 3.528MHz clock\n"),
    printf("  <pilot_ms> <pause_ms>\n"),
    printf("                 Duration of pilot/pause after block in milliseconds\n"),
    printf("  <M>            Number of pulses in the sequence of pulses\n"),
    printf("  <pulseX_ts>    Length of X-th pulse in the sequence at 3.528MHz clock\n"),
    printf("  <plug-xxx-N>   External generator, must exists xxx.exe and accept N params\n\n"),
    printf("  WAV options:\n"),
    printf("      <frequency>    Sample frequency, 44100 or 48000. Default is 44100\n"),
    printf("      <channel_type> Possible values are: mono (default), stereo or stereoinv\n\n"),
    exit(0);
  while( 1 )
    if( !strcasecmp(argv[1], "mono") || !strcasecmp(argv[1], "44100") )
      ++argv, --argc;
    else if( !strcasecmp(argv[1], "stereo") )
      channel_type= 2, ++argv, --argc;
    else if( !strcasecmp(argv[1], "stereoinv") )
      channel_type= 6, ++argv, --argc;
    else if( !strcasecmp(argv[1], "48000") )
      frequency= 48000, ++argv, --argc;
    else
      break;
  if( !(ext= strchr(argv[1], '.')) )
    printf("\nInvalid argument name: %s\n", argv[1]),
    exit(-1);
  while ( argc-- > 2 ){
    if( !strcasecmp(argv++[2], "basic")){
 printf("basic\n");
      param= atoi(argv[3]);
      fi= fopen(argv[4], "rb");
      if( fi )
        length= fread(mem+27, 1, 0x20000-27, fi);
      else
        length= parseHex(argv[4], 27);
      *(int*)(mem+3)= 19;
      parseHex(argv[2], 7);
      *(short*)(mem+17)= *(short*)(mem+21)= length;
      *(short*)(mem+19)= param;
      length+= 2;
      *(short*)(mem+24)= length;
      mem[26]= 255;
      for ( checksum= 0, i= 5; i<23; ++i )
        checksum^= mem[i];
      mem[23]= checksum;
      for ( checksum= 0, i= 26; i<26+length; ++i )
        checksum^= mem[i];
      mem[length+25]= checksum;
      fclose(fi);
      argc-= 3;
      argv+= 3;
    }
    else if( !strcasecmp(argv[1], "hdata")){
 printf("hdata\n");
      param= strtol(argv[3], NULL, 16);
      fi= fopen(argv[4], "rb");
      if( fi )
        length= fread(mem+27, 1, 0x20000-27, fi);
      else
        length= parseHex(argv[4], 27);
      *(short*)(mem+3)= 19;
      *(short*)(mem+5)= 0x300;
      parseHex(argv[2], 7);
      *(short*)(mem+17)= length;
      *(short*)(mem+19)= param;
      *(short*)(mem+21)= 0x8000;
      length+= 2;
      *(short*)(mem+24)= length;
      mem[26]= 255;
      for ( checksum= 0, i= 5; i<23; ++i )
        checksum^= mem[i];
      mem[23]= checksum;
      for ( checksum= 0, i= 26; i<26+length-1; ++i )
        checksum^= mem[i];
      mem[length+25]= checksum;
      fclose(fi);
      argc-= 3;
      argv+= 3;
    }
    else if( !strcasecmp(argv[1], "data")){
      fi= fopen(argv[2], "rb");
      if( fi )
        length= fread(mem+6, 1, 0x20000-6, fi);
      else
        length= parseHex(argv[2], 6);
      *(short*)(mem+3)= length+= 2;
      mem[5]= 255;
      for ( checksum= 0, i= 5; i<5+length-1; ++i )
        checksum^= mem[i];
      mem[length+4]= checksum;
      fclose(fi);
      --argc;
      ++argv;
    }
    else if( !strcasecmp(argv[1], "pause")){
      nextsilence= silence= atof(argv[2]);
      --argc;
      ++argv;
    }
    else if( !strcasecmp(argv[1], "pilot")){
      k= atoi(argv[2]);
      argc-= 2;
      argv+= 2;
    }
    else if( !strcasecmp(argv[1], "pulse")){
      k= atoi(argv++[2]);
      *(unsigned char*)(mem+2)= k;
      for ( j= 0; j<k; j++ )
        *(unsigned short*)(mem+3+j*2)= atoi(argv++[2]);
      argc-= k+1;
    }
    else if( (turbo= !strcasecmp(argv[1], "turbo")) || !strcasecmp(argv[1], "pure") ){
      fi= fopen(argv[5+turbo*4], "rb");
      if( tzx ){
        if( turbo ){
          mem[1]= 0x11;
          *(short*)(mem+2)= k= atoi(argv++[2]);
          *(short*)(mem+4)= atoi(argv++[2]);
          *(short*)(mem+6)= atoi(argv++[2]);
          *(short*)(mem+8)= atoi(argv++[2]);
          *(short*)(mem+10)= atoi(argv++[2]);
          *(short*)(mem+12)= atof(argv++[2])*3500/k+0.5;
          *(char*)(mem+14)= 8;
          *(unsigned short*)(mem+15)= atoi(argv++[2]);
          if( fi )
            length= fread(mem+20, 1, 0x20000-20, fi);
          else
            length= parseHex(argv[2], 20);
          *(unsigned short*)(mem+17)= length;
          *(unsigned char*)(mem+19)= length>>16;
          fwrite(mem+1, 1, length+19, fo);
        }
        else{
          mem[1]= 0x14;
          *(short*)(mem+2)= atoi(argv++[2]);
          *(short*)(mem+4)= atoi(argv++[2]);
          *(char*)(mem+6)= 8;
          *(unsigned short*)(mem+7)= atoi(argv++[2]);
          if( fi )
            length= fread(mem+20, 1, 0x20000-20, fi);
          else
            length= parseHex(argv[2], 20);
          *(unsigned short*)(mem+9)= length;
          *(unsigned char*)(mem+11)= length>>16;
          fwrite(mem+1, 1, length+11, fo);
        }
        ++argv;
      }
      else if( wav ){
        if( turbo ){
          k= atoi(argv[2]) << 1;
          j= atof(argv[7])*7056/k+0.5;
        }
        if( fi )
          length= fread(mem, 1, 0x20000, fi);
        else
          length= parseHex(argv[5+turbo*4], 0);
        j= 0;
        param= atoi(argv[2+turbo*3]) << 1;
        k= atoi(argv[3+turbo*3]) << 1;
        fclose(fi);
        argv+= turbo+1<<2;
        nextsilence= silence= atof(argv[0]);
      }
      else
        printf("\nError: pure or turbo command not allowed in TAP files\n"),
        exit(-1);
      argc-= turbo+1<<2;
    }
    else if( strchr(argv[1], '-')
          && (*strchr(argv[1], '-')= 0, !strcasecmp(argv[1], "plug")) ){
      argv[1]+= 5;
      k= atoi(strstr(argv[1], "-")+1);
      *strchr(argv[1], '-')= 0;
      command= (char *) malloc (0x100);
      sprintf(command, "%s %d %s tmp.%s", argv[1], frequency,
              channel_type-1 ? (channel_type-2?"stereoinv":"stereo") : "mono", ++ext);
      argc-= k;
      while( k-- )
        strcat(command, " "),
        strcat(command, argv++[2]);
      if( system(command) )
        printf("\nError: plug error with command:\n", command),
        exit(-1);
      else{
        fwrite( precalc, 1, ind, fo );
        rem= ind= 0;
        sprintf(command, "tmp.%s", ext);
        fi= fopen(command, "rb");
        if( fi ){
          if( tzx )
            fseek(fi, 0, SEEK_END),
            i= ftell(fi)-10,
            fseek(fi, 10, SEEK_SET);
          else
            fread(mem, 1, 44, fi),
            i= *(int*)(mem+40);
          j= i>>20;
          k= i&0xfffff;
          for ( int i= 0; i<j; i++ )
            fread(precalc, 1, 0x100000, fi),
            fwrite(precalc, 1, 0x100000, fo);
          fread(precalc, 1, k, fi);
          fwrite(precalc, 1, k, fo);
          fclose(fi);
          if( remove(command) )
            printf("\nError: deleting \n", command),
            exit(-1);
        }
        else
          printf("\nError: plug doesn't generate valid file\n"),
          exit(-1);
      }
    }
    else
      printf("\nInvalid argument name: %s\n", argv[1]),
      exit(-1);
  }
  if( tzx ){
    fseek(fo, 0, SEEK_END);
    i= ftell(fo);
    fseek(fo, 10, SEEK_SET);
    while( ftell(fo)<i ){
      fread(&turbo, 1, 1, fo);
      switch(turbo){
        case 0x10:
          j= ftell(fo);
          fseek(fo, 2, SEEK_CUR);
          fread(&length, 2, 1, fo);
          fseek(fo, length, SEEK_CUR);
          break;
        case 0x11:
          fseek(fo, 13, SEEK_CUR);
          j= ftell(fo);
          fseek(fo, 2, SEEK_CUR);
          fread(&length, 2, 1, fo);
          fseek(fo, length+1, SEEK_CUR);
          break;
        case 0x12:
          j= 0;
          fseek(fo, 4, SEEK_CUR);
          break;
        case 0x13:
          j= 0;
          fread(&turbo, 1, 1, fo);
          fseek(fo, turbo<<1, SEEK_CUR);
          break;
        case 0x14:
          fseek(fo, 5, SEEK_CUR);
          j= ftell(fo);
          fseek(fo, 2, SEEK_CUR);
          fread(&length, 2, 1, fo);
          fseek(fo, length+1, SEEK_CUR);
          break;
        case 0x15:
          fseek(fo, 2, SEEK_CUR);
          j= ftell(fo);
          fseek(fo, 3, SEEK_CUR);
          fread(&length, 2, 1, fo);
          fseek(fo, length+1, SEEK_CUR);
          break;
        case 0x20:
          j= ftell(fo);
          fseek(fo, 2, SEEK_CUR);
          break;
        default: 
          printf("Invalid TZX ID: %X\n", turbo);
      }
    }
    fseek(fo, j, SEEK_SET);
    length= 100;
    j && fwrite(&length, 2, 1, fo);
  }
  fclose(fo);
  printf("\nFile generated successfully\n");
}