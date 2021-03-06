#include <stdio.h>// perror,printf
#include <stdlib.h>// exit
#include <ctype.h> //inclusión de función is_digit()
#include <arpa/inet.h>// socket
#include <stdbool.h>// bool types
#include <string.h>// memset
#include <unistd.h>// file management
#include <netinet/in.h>
#include <sys/stat.h> // Utilizada para conocer tamaño de archivos 
#include "server_socket.h"// Librería de funciones de mi servidor
#include "error.h"// Definición de errores
#define VERSION "1.0"// Versión de mi programa
#define TYPES "srvFtp" // Nombre/tipo de mi servidor
#define TYPEC "cltFtp" // Nombre/tipo de mi cliente
#define LOCALHOST "127.0.0.1"
#define QUEUE   5      // Número de solicitudes que se pueden encolar
#define GOODBYE "Goodbye"
//#define DEBUG
char * cquit="QUIT";
char * cuser="USER";
char * cport="PORT";
char * cpass="PASS";
char * cretr="RETR";
char * cnlst="NLST";
char * ccwd="CWD";
char * cmkd="MKD";
char * crmd="RMD";
char * portrcv="Port received";
char code550[4]="550";
char * code530="530";
char * code521="521";
char * code431="431";
char * code257="257";
char * code250="250";
char * code230="230";
char * code229="229";
char * code226="226";
char * code221="221";
char * code200="200";
char * twopoint="..";
char *logged_in="logged in";
char * elogin= "Login incorrect";
char * xversion="220 cltFtp 1.0";
char * code331="331";
char * rq= "Password required for";
char  notfound [28]=": no such file or directory";
char * transfer= "Transfer complete";
char * cdok= "directory changed";
char * mkok= "directory created";
char * rmok= "remove directory";
char * cderror= "no such directory";
char * mkerror= "taking no action";
char * bytes= "bytes";
char * nfile= "File";
char * size ="size";
char server_buffer[256];
char client_buffer[256];
char *operations [20];
char passwd[50];
char userpiece [50];
char ip_client[16];
char filename [200];

int  port_client=0;
struct sockaddr_in serverAddr;
struct sockaddr_in clientAddr;
struct sockaddr_in dataAddr;
bool cconnect= true;
FILE *fpointer;


int main(int argc, char* argv[]){

    check_args(argc);
    int sd;
    int sddc;
    char p[250];
    char * port=argv[1];
    int csd;// Creación del socket del cliente al que se va a escuchar
	long int sizef;
    char bf [10];
    char auxstr[150];
    char command[200];
    int cantc;
    operations[0]="220";
    socklen_t size_addr;
    size_addr=sizeof(serverAddr);
    sd=create_socket();
    set_struct(port);
    binding(sd); 
    listening(sd);
    struct userdata ud;
    char path[150];
    memset(path, 0 , sizeof(path));
    set_int_path(path);
    csd=connection_accepted(sd, size_addr);
    clear_buffer(client_buffer);
    read_command(csd);
    clear_buffer(server_buffer);
    sprintf(server_buffer, "%s %s %s\r\n", operations[0], TYPES, VERSION);
    write_command(csd);
    #ifdef DEBUG
    printf("[+]Se escribe:%s",server_buffer);
    #endif
    set_structUser("","",&ud);
    while(1){
        clear_buffer(client_buffer);
        read_command(csd);
        int nvar=compare_input(&ud);
        clear_buffer(server_buffer);
        if(nvar==0){
            again: //Etiqueta para mi cláusula goto
            clear_buffer(server_buffer);
            printf("[+]Desconectando cliente...\n");
            sprintf(server_buffer, "%s %s\r\n",code221,GOODBYE);
            write_command(csd);
            sleep(1);
            shutdown(csd,SHUT_RDWR);
            close(csd);
            csd=connection_accepted(sd, size_addr);
    
        }

        else if (nvar==1){
            char *error="Código de Error";
            clear_buffer(server_buffer);
            sprintf(server_buffer, "%s\r\n",error);
            write_command(csd);
            
        } 
        else if (nvar==2){
            clear_buffer(server_buffer);
            sprintf(server_buffer, "%s %s %s\r\n", operations[0], TYPES, VERSION);
            write_command(csd);
        }
        else if (nvar==3){
            clear_buffer(server_buffer);
            open_file("ftpusers");
            authenticate_data(&ud);
            if(ud.user!=NULL){
                clear_buffer(server_buffer);
                sprintf(server_buffer, "%s %s %s\r\n",code331,rq,ud.user);
                write_command(csd);
                
            }
            else{
                clear_buffer(server_buffer);
                sprintf(server_buffer, "%s %s\r\n",code530,elogin);
                write_command(csd);
                printf("[+]Cerrando conexión del cliente...\n");
                goto again;
                
            }
        
        }
        else if (nvar==4){
                
            if(ud.password!=NULL){
                char * aux=get_passw();
                if(strncmp(aux,ud.password,strlen(ud.password)-1)==0){
                    clear_buffer(server_buffer);
                    sprintf(server_buffer,"%s %s %s %s\r\n",code230,"User",ud.user,logged_in);
                    write_command(csd);
                }
                else{
                    clear_buffer(server_buffer);
                    sprintf(server_buffer, "%s %s\r\n",code530,elogin);
                    write_command(csd);
                    printf("[+]Cerrando conexión del cliente...\n");
                    goto again;

                }
                free(aux);// Libero el espacio del heap
            }
            else{
                clear_buffer(server_buffer);
                sprintf(server_buffer, "%s %s\r\n",code530,elogin);
                write_command(csd);
                printf("[+]Cerrando conexión del cliente...\n");
                goto again;
            }
        }
        else if (nvar==5){
            
            clear_buffer(p);
            get_parameter(client_buffer,p);
            file_exists(p);
            if (file_exists(p)==0) {
                clear_buffer(server_buffer);
                sizef=get_filesize(p);
                
                
                memset(filename,0,strlen(filename));
                sprintf(filename,"%s",p);
                sprintf(bf,"%ld",sizef);
                sprintf(server_buffer,"%s %s %s %s %s %s\r\n",code229,nfile,p,size,bf,bytes);
                write_command(csd);

            }
            else{
                clear_buffer(server_buffer);
                sprintf(server_buffer, "%s %s %s\r\n",code550,p, notfound);
                write_command(csd);
            }
            

        }
        else if (nvar==6){
            clear_buffer(server_buffer);
            sprintf(server_buffer, "%s %s\r\n",code200,portrcv);
            write_command(csd);
            convert_iport(ip_client,&port_client,client_buffer);
            set_datastruct(ip_client,port_client);
            sddc=create_socket();
            if(connect(sddc,(struct sockaddr *)&dataAddr,sizeof(dataAddr))<0){
                printf("[-]Conexión fallida\n");
                exit(CONN);
            }
            #ifdef DEBUG
            printf("[+]Conexión exitosa para Data transfer\n");
            #endif
            fpointer=fopen(p,"rb");
            long int sizef=get_filesize(p);
            write_file(sddc,fpointer,sizef);
            memset(server_buffer,0, sizeof(server_buffer));
            sprintf(server_buffer, "%s %s\r\n",code226,transfer);
            write_command(csd);
    
        }
        else if (nvar==7){
            
            memset(command, 0 , sizeof(command));
            sprintf(command,"ls -l %s > cmd.tmp", path);
            system(command);
            memset(filename,0,strlen(filename));
            sprintf(filename, "%s", "cmd.tmp");
            sprintf(p, "%s", "cmd.tmp");
            sizef=get_filesize("cmd.tmp");
            memset(bf, 0, sizeof(bf));
            sprintf(bf,"%ld",sizef);
            sprintf(server_buffer,"%s %s %s %s %s %s\r\n",code229,nfile,p,size,bf,bytes);
            write_command(csd);
        }
        else if (nvar==8){
            clear_buffer(p);
            get_parameter(client_buffer,p); //extrae el parametro
                                        
            if(strncmp(p, twopoint, 2) == 0){
                char * auxch = strrchr(path, '/');
                cantc=auxch - path;
                memset(auxstr, 0 , sizeof(auxstr));
                memset(path, 0 , sizeof(path));
                strncpy(auxstr, path, cantc);
                sprintf(path, "%s", auxstr);
            }else{
                sprintf(path, "%s/%s", path, p);
            }

            memset(command, 0 , sizeof(command));
            sprintf(command,"cd %s", path);

            if(system(command) == 0){//200 directory changed
                memset(server_buffer, 0, sizeof(server_buffer));
                sprintf(server_buffer,"%s %s\r\n",code200,cdok);
                write_command(csd);
            }else{//431 No such directory
                memset(server_buffer, 0, sizeof(server_buffer));
                sprintf(server_buffer,"%s %s\r\n",code431,cderror);
                write_command(csd);                                
            }
                
        }
        else if (nvar==9){
            clear_buffer(p);
            get_parameter(client_buffer,p); //extrae el parametro
            
            memset(command, 0 , sizeof(command));
            sprintf(command,"mkdir %s/%s", path, p);
            
            if(system(command) == 0){//257 "/usr/dm/pathname" directory created
                memset(server_buffer, 0, sizeof(server_buffer));
                sprintf(server_buffer,"%s %s %s\r\n",code257,p,mkok);
                write_command(csd);
            }else{//521 taking no action.
                memset(server_buffer, 0, sizeof(server_buffer));
                sprintf(server_buffer,"%s %s\r\n",code521,mkerror);
                write_command(csd);                                
            }
        }
        else if (nvar==10){
            clear_buffer(p);
            get_parameter(client_buffer,p); //extrae el parametro
            
            memset(command, 0 , sizeof(command));
            sprintf(command,"rmdir %s/%s", path, p);
            
            if(system(command) == 0){//250 remove directory
                memset(server_buffer, 0, sizeof(server_buffer));
                sprintf(server_buffer,"%s %s %s\r\n",code250,p,rmok);
                write_command(csd);
            }else{//521 taking no action.
                memset(server_buffer, 0, sizeof(server_buffer));
                sprintf(server_buffer,"%s %s\r\n",code521,mkerror);
                write_command(csd);                                
            }
        }
        
    }


    close(sd);
    return 0;
}

void check_args(int nargs){

    if(nargs != 2){
        perror("[-] Numero incorrecto de argumentos.1 argumento requerido\n");
        exit(WARG);
    }

}
int create_socket(void){
	int sd=socket(AF_INET,SOCK_STREAM,0);// Especificaciones de mi socket TCP
	if(sd<0){
		perror("[-]No se pudo crear el socket\n");
		exit(SCRN);
	}
	else{
        #ifdef DEBUG
		printf("[+]Socket creado exitosamente\n");
        #endif
	}

	return sd;
}

void set_struct(char* port){
    serverAddr.sin_family=AF_INET;// Familia de protocolo Ipv4
    serverAddr.sin_port= htons(atoi(port));// Puerto para la conexión
    serverAddr.sin_addr.s_addr = inet_addr(LOCALHOST);
}

void binding(int sd){
    int bind_stat=bind(sd,(struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if(bind_stat <0){
        perror("[-]Falla en el bind\n");
        exit(BIND);
    }
    #ifdef DEBUG
    printf("[+] Bind exitoso al puerto\n");
    #endif
}

void listening(int sd){
    int listen_stat=listen(sd,QUEUE);
    if(listen_stat<0){
		perror("[-]Error en la escucha\n");
		exit(LSTN);
	}
    else{
        #ifdef DEBUG
        printf("[+]Listening exitoso\n");
        #endif
    }
}

int connection_accepted(int sd,socklen_t sockt){
    printf("[+]Esperando conexión...\n");
    int csd=accept(sd,(struct sockaddr *)&clientAddr, &sockt);
    if(csd < 0){
        perror("[-]No se puedo realizar el accept\n");
        exit(ACCT);
    }
    else{
        #ifdef DEBUG
        printf("{+]Accept exitoso\n");
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET,&(serverAddr.sin_addr),ip,INET_ADDRSTRLEN);
        printf("[+] Conexión establecida con la ip: %s y el puerto:%d\n",ip,ntohs(serverAddr.sin_port));
        #endif
    }
    return csd;
}

void clear_buffer(char * buff){
	memset(buff,0,sizeof(*buff));

}

void read_command(int s){
	int reads=read(s,client_buffer,sizeof(client_buffer));
	if(reads==-1){
		perror("[-]No se pudo recibir el mensaje\n");
		exit(READ);
	}
	else{
        #ifdef DEBUG
		printf("[+]Operación de lectura exitosa\n");
        printf("[+]Recibido < %s\n",client_buffer);
        #endif 
        
	}
}

void write_command(int s){
	int write_stat=write(s,server_buffer,sizeof(server_buffer));
	if(write_stat==-1){
		perror("[-]No se puede escribir en el socket\n");
		exit(WRTE);
	}
	else{
        #ifdef DEBUG
		printf("[+]Operación de escritura exitosa\n");
        #endif
	}
}
int compare_input(struct userdata *ud){
    char auxbuffclient[256];
    sprintf(auxbuffclient,"%s",client_buffer);
    char * buffpiece=strtok(auxbuffclient, " ");
    int compares=strncmp(buffpiece, cquit,strlen(cquit)); //Analizo si se trata del comando quit
		if (compares==0){
            return 0;
        }
        else if(strncmp(buffpiece, xversion,3)==0){
            
            return 2;
        }
        else if(strncmp(buffpiece,cuser,strlen(cuser))==0){
            return 3;
            #ifdef DEBUG
            printf("Comando %s recibido\n",cuser);
            #endif
           
        }
        else if(strncmp(buffpiece,cpass,strlen(cpass))==0){// Analizo si recibí PASS
            return 4;
        }
        else if(strncmp(buffpiece,cretr,strlen(cretr))==0){ // Analizo si recibí RETR
            
            return 5;

        }
        else if(strncmp(buffpiece,cport,strlen(cport))==0){ // Analizo si recibí PORT
            
            return 6;
        }
        else if(strncmp(buffpiece,cnlst,strlen(cnlst))==0){ // Analizo si recibí NLST
            
            return 7;
        }
        else if(strncmp(buffpiece,ccwd,strlen(ccwd))==0){ // Analizo si recibí CWD
            
            return 8;
        }
        else if(strncmp(buffpiece,cmkd,strlen(cmkd))==0){ // Analizo si recibí MKD
            
            return 9;
        }
        else if(strncmp(buffpiece,crmd,strlen(crmd))==0){ // Analizo si recibí RMD
            
            return 10;
        }
        else{
            printf("Comando  erróneo %s\n",client_buffer);
            return 1;
        }
}

FILE * open_file( char* path){
    fpointer=fopen(path,"r");
    return fpointer;
}

void close_file(FILE* file){
    fclose(file);
}

void authenticate_data(struct userdata * ud){
    char singleLine[200];
    char * upiece;
    char * passpiece;
    char * pointeraux;
    int x=100;
    int flagclose=0; // Flag que me indica si cerré o no el archivo. 0=no;!=0=sí
    int var=0;
    while(!feof(fpointer)){
        fgets(singleLine,200,fpointer);
        upiece=strtok(singleLine,":");
        passpiece=strtok(NULL,"");
        if(var==0){
            pointeraux=strtok(client_buffer," ");
            pointeraux=strtok(NULL,"");
            var++;
        }
        strncpy(userpiece,pointeraux,strlen(upiece));
        x=strncmp(upiece,userpiece,strlen(upiece)-2);
        if(x==0){
            set_structUser(userpiece,passwd,ud);
            sprintf(passwd,"%s",passpiece);
            close_file(fpointer);
            flagclose++;
            break;
        }
        else{
            set_structUser(NULL,NULL,ud); 
        }
    }
    if (flagclose==0){
        close_file(fpointer);
    }
}

void set_structUser(char *usr, char *pass, struct userdata *udata){
    (*udata).user=usr;
    (*udata).password=pass;
}

char * get_passw(void){
    char auxbuffclient[256];
    sprintf(auxbuffclient,"%s",client_buffer);
    strtok(auxbuffclient, " ");
    char * f =strtok(NULL, " ");
    char * buffpiece = malloc(strlen(f));
    strncpy(buffpiece,f,strlen(f)-1);// Elimino caracter nulo
    return buffpiece;
}

void get_parameter(char * buff, char *param){ 
	char *auxcommand= buff;
	char *aux2;
    memset(param,0,256);
	strtok(auxcommand, " ");
	aux2=strtok (NULL,"\0");
	strncpy(param,aux2,strlen(aux2)-2);// Quito /r/n

}

int file_exists(char *p){
    return (access(p,F_OK));

}

long int get_filesize(char *fname){// Función para obtener el tamaño de mi archivo
    struct stat fstatus;
    if(stat(fname,&fstatus)<0){
        return WSZE;
    }
    return fstatus.st_size;

}
void convert_iport(char *ip, int *port, char* buffer){
    buffer=buffer+5;
    char porthigh[5];
    char portlow [5];
    int high;
    int low;
    int i=0;
    memset(porthigh,0,sizeof(porthigh));
    memset(portlow,0,sizeof(portlow));
    int comnumb=0;// Número de comas
    while(comnumb<4){
        if(*buffer != ','/*isdigit(*buffer)==0*/){
            *ip=*buffer;
            
        }
        else{
            comnumb++;
            *ip='.';
        }
        ip++;
        buffer++;
    }
    *(ip -1)='\0';
    comnumb=0;
    while(*buffer!= '\r'){
        if(*buffer!=','){
            if (comnumb==0) {
                porthigh[i]=*buffer;
                i++;
                buffer++;
            }
            else{
                portlow[i]=*buffer;
                i++;
                buffer++;
            }
        } else{
            buffer++;
            comnumb++;
            i=0;
        }

    }
    high=atoi(porthigh);
    low=atoi(portlow);
    high= high<<8;
    (*port)=high + low;
    

}
void set_datastruct(char* ip, int port){
    dataAddr.sin_family=AF_INET;// Familia de protocolo Ipv4
    dataAddr.sin_port= htons(port);// Puerto para la conexión
    dataAddr.sin_addr.s_addr = inet_addr(ip);
}

void write_file(int sddc , FILE *fpointer, long int sz){
    char data[512]={0};
    while(sz>=0){
        if(sz <=512){
            while(fread(data,1,sz,fpointer)!=(long int)NULL){
                if (write(sddc,data,sz)<0){
                    perror("[-] Error al escribir el archivo\n");
                    exit(FWRT);
                }
                memset(data,0,sizeof(data));
            }
            sz=-1;
        }
        else{
            
            while(fread(data,1,512,fpointer)!=(long int)NULL){
                if (write(sddc,data,sizeof(data))<0){
                    perror("[-] Error al escribir el archivo\n");
                    exit(FWRT);
                }
                memset(data,0,sizeof(data));
            }
            sz=sz -512;
            if(sz<0){
                sz=512+sz;
            }
            
        }
    }
}

void set_int_path(char * s){ // Función para identificar mi path inicial
    system("pwd > cmd.tmp");
    FILE * auxfp = fopen("cmd.tmp", "r");

    struct stat statf;
    if(stat("cmd.tmp", &statf) == -1) {
        perror("Error: stat");
    }
    
    char* auxfile = malloc(statf.st_size);
    fread(auxfile, statf.st_size, 1, auxfp);

    if ((strlen(auxfile) != 0) && (auxfile[strlen (auxfile) - 1] == '\n')){
        auxfile[strlen (auxfile) - 1] = '\0';
    }
    
    sprintf(s, "%s", auxfile);
    fclose(auxfp);
    free(auxfile);
}