#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>

struct BMPHeader{
  int file_size;
  int width;
  int height;
  int image_size;
}; //definire structura cu campurile necesare preluarii de informatii

const char *decod_permissions(mode_t mode){
  static char string[1000];
  snprintf(string,sizeof(string),"Drepturi de acces user:%c%c%c\nDrepturi de acces grup:%c%c%c\nDrepturi de acces altii:%c%c%c",
	   (mode & S_IRUSR) ? 'R' : '-',
	   (mode & S_IWUSR) ? 'W' : '-',
	   (mode & S_IXUSR) ? 'X' : '-',
	   (mode & S_IRGRP) ? 'R' : '-',
	   (mode & S_IWGRP) ? 'W' : '-',
	   (mode & S_IXGRP) ? 'X' : '-',
	   (mode & S_IROTH) ? 'R' : '-',
	   (mode & S_IWOTH) ? 'W' : '-',
	   (mode & S_IXOTH) ? 'X' : '-');
  return string;
}//O functie care converteste permisiunile fisierului in sir de caractere pentru afisarea drepturilor

int main(int argc, char **argv){
  if(argc!=2){
    printf("Usage: %s\n",argv[0]);
    exit(-1);
  }
  char *filename=argv[1];
  struct BMPHeader header;
  int bmp_file;
  bmp_file=open(filename,O_RDONLY);//deschidere fisier BMP
  if(bmp_file==-1){
    printf("Error at BMP file opening\n");
    exit(-1);
  }//verificare daca a fost deschis cu succes
  char type[3];
  //cu read se realizeaza citirea informatiilor din antetul BMP
  read(bmp_file,type,sizeof(char)*2);
  if(strcmp(type,"BM")!=0){
    printf("File %s is not BMP File\n",argv[1]);
    exit(-1);
  }
  //se citeste fiecare camp din structura unui fisier si se stocheaza in structura declarata mai sus doar informatiile necesare
  read(bmp_file,&header.file_size,sizeof(int));
  int reserved,data_offset,size;
  read(bmp_file,&reserved,sizeof(int));
  read(bmp_file,&data_offset,sizeof(int));
  read(bmp_file,&size,sizeof(int));
  read(bmp_file,&header.width,sizeof(int));
  read(bmp_file,&header.height,sizeof(int));
  short planes,bit_count;
  int compression;
  read(bmp_file,&planes,sizeof(short));
  read(bmp_file,&bit_count,sizeof(short));
  read(bmp_file,&compression,sizeof(int));
  read(bmp_file,&header.image_size,sizeof(int));
  //printf("%d %d %d %d\n",header.file_size,header.width,header.height,header.image_size);
  struct stat file_st;//fisier pentru generarea statisticilor si a permisiunilor
  int file_out;
  file_out=open("statistica.txt",O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if(file_out==-1){
    printf("Error at <statistica.txt> file opening\n");
    exit(-1);
  }
  if(fstat(bmp_file,&file_st)==-1){
    printf("Cannot access file information\n");
    close(bmp_file);
    exit(-1);
  }
  char statistics[1500];
  snprintf(statistics,sizeof(statistics),"Nume fisier: %s\nInaltime: %d\nLungime: %d\nDimensiune: %ld\nIdentificatorul utilizatorului: %d\nTimpul ultimei modificari: %sContorul de legaturi: %ld\n",filename,header.height,header.width,file_st.st_size,file_st.st_uid,ctime(&file_st.st_mtime),file_st.st_nlink);
  //scrierea informatiilor despre fisier
  const char *rez=decod_permissions(file_st.st_mode);//apelul functiei de decodificare permisiuni
  int w=write(file_out,statistics,strlen(statistics));//scrierea stringului rezultat intr-un fisier de iesire
  if(w==-1){
    printf("Error at writing information in <statistica.txt> file\n");
    exit(-1);
  }
  int w2=write(file_out,rez,strlen(rez));//scrierea permisiunilor decodificate in fisierul de iesire
  if(w2==-1){
    printf("Error at writing permissions in <statistica.txt> file\n");
    exit(-1);
  }
  close(file_out);
  close(bmp_file);
  return 0;
}
