#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

struct BMPHeader{
  int file_size;
  int width;
  int height;
  int image_size;
};

//struct stat file_st;

void gray_conversion(const char *dir_name, const char *dir_out){
  char aux[100];
  int fint=open(dir_name,O_RDONLY);
  if(fint==-1){
    printf("Error at file opening\n");
    exit(-1);
  }
  strcpy(aux,dir_name);
  char *p=strtok(aux,".");
  p=strtok(NULL,".");
  if(strcmp(p,"bmp")==0){
    struct BMPHeader header;
    int bmp_file;
    bmp_file=open(dir_name,O_RDWR);
    if(bmp_file==-1){
      printf("Error at BMP file opening\n");
      exit(-1);
    }
    char type[3];
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
    int xpixels;
    int ypixels;
    int color_used;
    int colors_imp;
    read(bmp_file,&xpixels,sizeof(int));
    read(bmp_file,&ypixels,sizeof(int));
    read(bmp_file,&color_used,sizeof(int));
    read(bmp_file,&colors_imp,sizeof(int));
    
    uint8_t colors[4];
    uint8_t gray;
    //printf("%d %d\n",width,height);
    for(int i=0;i<header.width;i++)
      for(int j=0;j<header.height;j++){
	read(bmp_file,colors,4);
	gray=0.299*colors[0]+0.587*colors[1]+0.114*colors[2];
	colors[0]=gray;
	colors[1]=gray;
	colors[2]=gray; //parcurgere poza pixel cu pixel, citire pixel, setare nunta gri
	lseek(bmp_file,SEEK_CUR,-4); //mutare inapoi inaintea pixelului curent pentru rescrierea culorii din poza
	write(bmp_file,colors,4);
      }
  }
  //printf("%d %d %d %d %d",colors[0],colors[1],colors[2],colors[3],gray);
}//functie de conversie culoare gri

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

void statistics_file(const char *dir_name,const char *dir_out, struct stat file_st, int file_out){
  //char statistics[1500];
  if(S_ISDIR(file_st.st_mode)){
    char statistics[1500];
    snprintf(statistics,sizeof(statistics),"Nume director: %s\nIdentificator: %d\n",dir_name,file_st.st_uid);
    int w=write(file_out,statistics,strlen(statistics));
    if(w==-1){
      printf("Error at writing information about director\n");
      exit(-1);
    }
    const char *rez=decod_permissions(file_st.st_mode);
    int w2=write(file_out,rez,strlen(rez));
    if(w2==-1){
      printf("Error at writing in <statistica.txt> file\n");
      exit(-1);
    }
    //count_lines=7;
  }//if dir
  else if(S_ISLNK(file_st.st_mode)){
    char statistics[1500];
    char *buf=malloc(file_st.st_size+1);
    readlink(dir_name,buf,file_st.st_size);
    struct stat buf_st;
    if(stat(buf,&buf_st)==0 && S_ISREG(buf_st.st_mode)){
      snprintf(statistics,sizeof(statistics),"Nume legatura: %s\nDimensiune: %ld\nDimensiune fisier: %ld\n",dir_name,file_st.st_size,buf_st.st_size);
      int w=write(file_out,statistics,strlen(statistics));
      if(w==-1){
	printf("Error at writing information about symbolic link\n");
	exit(-1);
      }
      const char *rez=decod_permissions(file_st.st_size);
      int w2=write(file_out,rez,strlen(rez));
      if(w2==-1){
	printf("Error at writing in <statistica.txt> file\n");
	exit(-1);
      }
      //count_lines=8;
    }
  }//if link
  else if(S_ISREG(file_st.st_mode)){
    char aux[100];
    int fint=open(dir_name,O_RDONLY);
    if(fint==-1){
      printf("Error at file opening\n");
      exit(-1);
    }
    strcpy(aux,dir_name);
    char *p=strtok(aux,".");
    p=strtok(NULL,".");
    if(strcmp(p,"bmp")==0){
      struct BMPHeader header;
      int bmp_file;
      bmp_file=open(dir_name,O_RDWR);
      if(bmp_file==-1){
	printf("Error at BMP file opening\n");
	exit(-1);
      }
      char type[3];
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
      int xpixels;
      int ypixels;
      int color_used;
      int colors_imp;
      read(bmp_file,&xpixels,sizeof(int));
      read(bmp_file,&ypixels,sizeof(int));
      read(bmp_file,&color_used,sizeof(int));
      read(bmp_file,&colors_imp,sizeof(int));
      char statistics[1500];
      snprintf(statistics,sizeof(statistics),"Nume fisier: %s\nInaltime: %d\nLungime: %d\nDimensiune: %ld\nIdentificatorul utilizatorului: %d\nTimpul ultimei modificari: %s\nContorul de legaturi: %ld\n",dir_name,header.height,header.width,file_st.st_size,file_st.st_uid,ctime(&file_st.st_mtime),file_st.st_nlink);
      int w=write(file_out,statistics,strlen(statistics));
      if(w==-1){
	printf("Error at writing information about BMP file\n");
	exit(-1);
      }
      const char *rez=decod_permissions(file_st.st_mode);
      int w2=write(file_out,rez,strlen(rez));
      if(w2==-1){
	printf("Error at writing permissions in <statistica.txt> file\n");
	exit(-1);
      }
      //count_lines=12;
      close(bmp_file);
    }//bmp
    if(strcmp(p,"bmp")!=0){
      int file;
      file=open(dir_name,O_RDONLY);
      if(file==-1){
	printf("Error at file opening\n");
	exit(-1);
      }
      char statistics[1500];
      snprintf(statistics,sizeof(statistics),"Nume fisier: %s\nDimensiune: %ld\nIdentificatorul utilizatorului: %d\nTimpul ultimei modificari: %s\nContorul de legaturi: %ld\n",dir_name,file_st.st_size,file_st.st_uid,ctime(&file_st.st_mtime),file_st.st_nlink);
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
      //count_lines=10;
      close(file);
    }
  }//if reg
  //close(file_out);
}

int main(int argc, char **argv){
  if(argc!=4){
    printf("Usage: %s %s %s\n",argv[0],argv[1], argv[2]);
    exit(-1);
  }
  struct dirent *pDirent;
  DIR *pDir,*pDir2;
  pDir=opendir(argv[1]);
  if(pDir==NULL){
    printf("Error at opening director\n");
    exit(-1);
  }
  pDir2=opendir(argv[2]);
  if(pDir2==NULL){
    printf("Error at opening director\n");
    exit(-1);
  }

  char character=argv[3][0];
  
  chdir(argv[1]);

  while((pDirent=readdir(pDir))!=NULL){
    if(strcmp(pDirent->d_name,".")==0 || strcmp(pDirent->d_name,"..")==0){
      continue;
    }
    int pfd[2];
    if(pipe(pfd)<0){
      printf("Error at pipe\n");
      exit(-1);
    }
    
	int pipe_fd[2];
	if(pipe(pipe_fd)==-1){
	  printf("Error at pipe for script\n");
	  exit(-1);
	}
    //printf("%s\n",pDirent->d_name);
    struct stat file_st;
    lstat(pDirent->d_name,&file_st);
    int file_out;
    char nume_fis[300];
    snprintf(nume_fis,sizeof(nume_fis),"../%s/%s_statistica.txt",argv[2],pDirent->d_name);
    file_out=open(nume_fis,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(file_out==-1){
      printf("Error at <statistica.txt> file opening\n");
      exit(-1);
    }
    int count_lines=0;
    //int pid;
    //int status;
    if(S_ISREG(file_st.st_mode)){
      if(strstr(pDirent->d_name,".bmp")!=NULL){//verificare fisier bmp
	int pid_reg;
	int status_reg;
	if((pid_reg=fork())<0){
	  exit(-1);
	}
	if(pid_reg==0){//proces fiu
	  statistics_file(pDirent->d_name,argv[2],file_st,file_out);
	  count_lines=12;
	  //printf("%d\n",count_lines);
	  close(file_out);
	  char print_pipe[10];
	  snprintf(print_pipe,sizeof(print_pipe),"%2d\n",count_lines);
	  write(pfd[1],print_pipe,strlen(print_pipe));
	  close(pfd[1]);
	  printf("%d%s----\n",getpid(),pDirent->d_name);
	  exit(0);
	}
	waitpid(pid_reg,&status_reg,0);
	printf("S-a incheiat procesul cu pid-ul %d si codul %d\n",pid_reg,status_reg);
	int pid_bmp;
	int status_bmp;
	if((pid_bmp=fork())<0){
	  exit(-1);
	}
	if((pid_bmp=fork())<0){
	  exit(-1);
	}
	if(pid_bmp==0){//al doilea proces fiu
	  gray_conversion(pDirent->d_name,argv[2]);
	  exit(0);
	}
	waitpid(pid_bmp,&status_bmp,1);
	printf("S-a incheiat procesul cu pid-ul %d si codul %d\n",pid_bmp,status_bmp);
      }//bmp
      else{
	int pid_fis;
	int status_fis;
	if((pid_fis=fork())<0){
	  exit(-1);
	}
	if(pid_fis==0){
	  statistics_file(pDirent->d_name,argv[2],file_st,file_out);
	  count_lines=10;
	  close(file_out);
	  char print_pipe[10];
	  snprintf(print_pipe,sizeof(print_pipe),"%2d",count_lines);
	  write(pfd[1],print_pipe,strlen(print_pipe));
	  close(pfd[1]);
	  printf("%d%s----\n",getpid(),pDirent->d_name);
	  //close(pipe_fd[0]);//inchide capatul de citire al pipe-ului
	  //dup2(pipe_fd[1],STDOUT_FILENO);//inlocuire stdin cu capatul de scriere al pipe-ului
	  //close(pipe_fd[1]);//inchidere pipe de scriere
	  //0 input, 1 output
	  int exec=execlp("cat","cat",pDirent->d_name,NULL);
	  //executare afisare continut fisier pe "ecran"
	  if(exec==-1){
	    exit(-1);
	  }
	}
	waitpid(pid_fis,&status_fis,0);
	printf("S-a incheiat procesul cu pid-ul %d si codul %d\n",pid_fis,status_fis);
	int pid_script,status_script;
	if((pid_script=fork())<0){
	  exit(-1);
	}
	if(pid_script==0){//al doilea proces fiu
	  //close(pipe_fd[1]);//inchidere capat de scriere al pipe-ului
	  //prelucrarea datelor venite din bash
	  //dup2(pipe_fd[0],STDIN_FILENO);
	  //close(pipe_fd[0]);
	  int exec=execlp("bash", "bash","../script_s9.sh",&character, NULL);
	  if(exec==-1){
	    printf("Script execution error\n");
	    exit(-1);
	  }
	}
	waitpid(pid_script,&status_script,0);
	printf("S-a incheiat procesul cu pid-ul %d si codul %d\n",pid_script,status_script);
      }
    }//if reg
    if(S_ISLNK(file_st.st_mode)){
      int pid_lnk;
      int status_lnk;
      if((pid_lnk=fork())<0){
	exit(-1);
      }
      if(pid_lnk==0){
	statistics_file(pDirent->d_name,argv[2],file_st,file_out);
	count_lines=7;
	    close(file_out);
	    char print_pipe[10];
	    snprintf(print_pipe,sizeof(print_pipe),"%2d",count_lines);
	    write(pfd[1],print_pipe,strlen(print_pipe));
	    close(pfd[1]);
	    printf("%d%s----\n",getpid(),pDirent->d_name);
	    exit(0);
      }
      waitpid(pid_lnk,&status_lnk,0);
      printf("S-a incheiat procesul cu pid-ul %d si codul %d\n",pid_lnk,status_lnk);
    }//link
    if(S_ISDIR(file_st.st_mode)){
      int pid_dir;
      int status_dir;
      if((pid_dir=fork())<0){
	exit(-1);
      }
      if(pid_dir==0){
	statistics_file(pDirent->d_name,argv[2],file_st,file_out);
	count_lines=6;
	close(file_out);
	char print_pipe[10];
	snprintf(print_pipe,sizeof(print_pipe),"%2d",count_lines);
	write(pfd[1],print_pipe,strlen(print_pipe));
	close(pfd[1]);
	printf("%d%s----\n",getpid(),pDirent->d_name);
	exit(0);
      }
      waitpid(pid_dir,&status_dir,0);
      printf("S-a incheiat procesul cu pid-ul %d si codul %d\n",pid_dir,status_dir);
    }
    close(pfd[1]);
    char read_pipe[10];
    read(pfd[0],read_pipe,sizeof(read_pipe));
    close(pfd[0]);
    printf("Lines number in %s is %s\n",pDirent->d_name,read_pipe);
  }//while
  return 0;
}
