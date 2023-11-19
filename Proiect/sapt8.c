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
#include <dirent.h>

struct BMPHeader{
  int file_size;
  int width;
  int height;
  int image_size;
};

const char *decod_permissions(mode_t mode){
  static char string[1000];
  snprintf(string,sizeof(string),"Drepturi de acces user:%c%c%c\nDrepturi de acces grup:%c%c%c\nDrepturi de acces altii:%c%c%c\n\n",
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
}

int main(int argc, char **argv){
  if(argc!=3){
    printf("Usage: %s %s\n",argv[0], argv[1]);
    exit(-1);
  }

  struct dirent *pDirent; //declarare structura pentru director
  DIR *pDir, *pDir2;
  pDir=opendir(argv[1]);//deschiderea directorului
  if(pDir==NULL){
    printf("Error at opening director\n");
    exit(-1);
  }
  pDir2=opendir(argv[2]);
  if(pDir2==NULL){
    printf("Error at opening director\n");
    exit(-1);
  }//verificare director de iesire
  while((pDirent=readdir(pDir))!=NULL){//parcurgerea elementelor aflate in directorul dat ca argument
    int pid;
    int status;
    if ((pid = fork()) < 0){
      exit(-1);
    }
    if (pid == 0){
      struct stat file_st;
      lstat(pDirent->d_name,&file_st);
      int file_out;
      char nume_fis[300]; 
      snprintf(nume_fis, sizeof(nume_fis), "%s/%s_statistica.txt",argv[2],pDirent->d_name);
      file_out=open(nume_fis,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
      if(file_out==-1){
	printf("Error at <statistica.txt> file opening\n");
	exit(-1);
      }
      if(S_ISLNK(file_st.st_mode)){//verificam daca este legatura simbolica
	char statistics[1500];
	char *buf=malloc(file_st.st_size+1);//in symlink se salveaza numele fisierului legat/legaturii, aloc suficient spatiu pentru nume si terminatorul de sir
	readlink(pDirent->d_name,buf,file_st.st_size); //citire continut legatura simbolica
	struct stat buf_st;
	if(stat(buf,&buf_st)==0 && S_ISREG(buf_st.st_mode)){
	  snprintf(statistics,sizeof(statistics),"Nume legatura: %s\nDimensiune: %ld\nDimensiune fisier: %ld\n",pDirent->d_name,file_st.st_size,buf_st.st_size);
	  const char *rez=decod_permissions(file_st.st_mode);
	  int w=write(file_out,statistics,strlen(statistics));
	  if(w==-1){
	    printf("Error at writing information in <statistica.txt> file\n");
	    exit(-1);
	  }
	  int w2=write(file_out,rez,strlen(rez));
	  if(w2==-1){
	    printf("Error at writing permissions in <statistica.txt> file\n");
	    exit(-1);
	  }
	}
      }
      else{     
	if(stat(pDirent->d_name,&file_st)==-1){
	  exit(-1);
	}
	if(S_ISREG(file_st.st_mode)){//verificare daca este fisier obisnuit
	  //printf("%s\n",pDirent->d_name);
	  char aux[100];
	  int fint=open(pDirent->d_name,O_RDONLY);
	  if(fint==-1){
	    printf("Eroare la %s",pDirent->d_name);
	    exit(-1);
	  }
	  strcpy(aux,pDirent->d_name);
	  char *p=strtok(aux,".");//verificare extensie fisier prin separarea in doua stringuri, avand ca despartitor '.'
	  p=strtok(NULL,".");
	  if(strcmp(p,"bmp")==0){//daca extensia fisierului este 'bmp'
	  //se foloseste codul scris pentru Saptamana 6, nemodificat
	    struct BMPHeader header;
	    int bmp_file;
	    bmp_file=open(pDirent->d_name,O_RDONLY);
	    if(bmp_file==-1){
	      printf("Error at BMP file opening\n");
	      exit(-1);
	    }
	    char type[3];
	    read(bmp_file,type,sizeof(char)*2);
	    if(strcmp(type,"BM")!=0){
	      printf("File %s is not BMP File\n",argv[1]);
	      exit(-1);
	    }
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
	    char statistics[1500];
	    snprintf(statistics,sizeof(statistics),"Nume fisier: %s\nInaltime: %d\nLungime: %d\nDimensiune: %ld\nIdentificatorul utilizatorului: %d\nTimpul ultimei modificari: %sContorul de legaturi: %ld\n",pDirent->d_name,header.height,header.width,file_st.st_size,file_st.st_uid,ctime(&file_st.st_mtime),file_st.st_nlink);
	    const char *rez=decod_permissions(file_st.st_mode);
	    int w=write(file_out,statistics,strlen(statistics));
	    if(w==-1){
	      printf("Error at writing information in <statistica.txt> file\n");
	      exit(-1);
	    }
	    int w2=write(file_out,rez,strlen(rez));
	    if(w2==-1){
	      printf("Error at writing permissions in <statistica.txt> file\n");
	      exit(-1);
	    }
	    close(bmp_file);
	  }
	  printf("\n");
	}
	if(S_ISREG(file_st.st_mode)){//verificare daca este fisier obisnuit
	  char aux[100];
	  int fint=open(pDirent->d_name,O_RDONLY);
	  if(fint==-1){
	    printf("Eroare la %s",pDirent->d_name);
	    exit(-1);
	  }
	  strcpy(aux,pDirent->d_name);
	  char *p=strtok(aux,".");
	  p=strtok(NULL,".");
	  if(strcmp(p,"bmp")!=0){//verificam sa fie fisier obisnuit fara extensia 'bmp'
	    //struct BMPHeader header;
	    int file;
	    file=open(pDirent->d_name,O_RDONLY);
	    if(file==-1){
	      printf("Error at file opening\n");
	      exit(-1);
	    }
	    char statistics[1500];
	    snprintf(statistics,sizeof(statistics),"Nume fisier: %s\nDimensiune: %ld\nIdentificatorul utilizatorului: %d\nTimpul ultimei modificari: %sContorul de legaturi: %ld\n",pDirent->d_name,file_st.st_size,file_st.st_uid,ctime(&file_st.st_mtime),file_st.st_nlink);
	    const char *rez=decod_permissions(file_st.st_mode);
	    int w=write(file_out,statistics,strlen(statistics));
	    if(w==-1){
	      printf("Error at writing information in <statistica.txt> file\n");
	      exit(-1);
	    }
	    int w2=write(file_out,rez,strlen(rez));
	    if(w2==-1){
	      printf("Error at writing permissions in <statistica.txt> file\n");
	      exit(-1);
	    }
	    close(file);
	  }
	  printf("\n");
	}
	if(S_ISDIR(file_st.st_mode)){//verificam daca este un director
	  char statistics[1500];
	  snprintf(statistics,sizeof(statistics),"Nume director: %s\nIdentificator: %d\n",pDirent->d_name,file_st.st_uid);
	  const char *rez=decod_permissions(file_st.st_mode);
	  int w=write(file_out,statistics,strlen(statistics));
	  if(w==-1){
	    printf("Error at writing information in <statistica.txt> file\n");
	    exit(-1);
	  }
	  int w2=write(file_out,rez,strlen(rez));
	  if(w2==-1){
	    printf("Error at writing permissions in <statistica.txt> file\n");
	    exit(-1);
	  }
	  printf("\n");
	}      
	if(!S_ISREG(file_st.st_mode)  && !S_ISDIR(file_st.st_mode)){
	  printf("No information written in <statistica.txt> about this type of file\n");
	  //close(file);
	  exit(-1);
	}
      }
      close(file_out);
      printf("%d%s---\n",getpid(),pDirent->d_name); //pt verificare - creare procese
      exit(0);
    }
  }
  //close(file_out);
  return 0;
}

//Modificari: adaugare nou director ca argument
//mutare deschidere si inchidere fisier in while, pentru a crea cate un fisier statistica.txt  pentru fiecare fisier aflat in primul director
//adaugat fork in tot while-ul: parintele parcurge fisierele si procesele copii le proceseaza
