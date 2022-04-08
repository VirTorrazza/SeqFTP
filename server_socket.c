#include <stdio.h>//perror,printf
#include <stdlib.h>//exit
#include <arpa/inet.h>//socket
#include <stdbool.h>//bool types
#include <string.h>//memset
#include <unistd.h>//file management
#include <netinet/in.h>
#include "server_socket.h"//Librería de funciones de mi servidor
#include "error.h"//Definición de errores
#define VERSION "1.0"//Versión de mi programa
#define TYPES "srvFtp" //Nombre/tipo de mi servidor
#define TYPEC "cltFtp" //Nombre/tipo de mi cliente
#define LOCALHOST "127.0.0.1"
#define QUEUE   5      //Número de solicitudes que se pueden encolar
#define GOODBYE "Goodbye"
#define DEBUG
char * cquit="QUIT";
char * cuser="USER";
char * cpass="PASS";
char * code530="530";
char * code230="230";
char * code221="221";
char *logged_in="logged in";
char * elogin= "Login incorrect";
char * xversion="220 cltFtp 1.0";
char * code331="331";
char * rq= "Password required for";
char server_buffer[256];
char client_buffer[256];
char *operations [20];
char passwd[50];
char userpiece [50];
struct sockaddr_in serverAddr;
struct sockaddr_in clientAddr;
bool cconnect= true;
FILE *fpointer;

int main(int argc, char* argv[]){

    check_args(argc);
    int sd;
    char * port=argv[1];
    int csd;//Creación del socket del cliente al que se va a escuchar
	operations[0]="220";
    socklen_t size_addr;
    size_addr=sizeof(serverAddr);
    sd=create_socket();
    set_struct(port);
    binding(sd); 
    listening(sd);
    struct userdata ud;
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
                close(csd);
                break;
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
                free(aux);//Libero el espacio del heap
            }
            else{
                clear_buffer(server_buffer);
                sprintf(server_buffer, "%s %s\r\n",code530,elogin);
                write_command(csd);
                printf("[+]Cerrando conexión del cliente...\n");
                close(csd);
                break; //Finalizo mi bucle
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
	int sd=socket(AF_INET,SOCK_STREAM,0);//Especificaciones de mi socket TCP
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
    serverAddr.sin_family=AF_INET;//Familia de protocolo Ipv4
    serverAddr.sin_port= htons(atoi(port));//Puerto para la conexión
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
        //#ifdef DEBUG
        printf("{+]Accept exitoso\n");
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET,&(serverAddr.sin_addr),ip,INET_ADDRSTRLEN);
        printf("[+] Conexión establecida con la ip: %s y el puerto:%d\n",ip,ntohs(serverAddr.sin_port));
        //#endif
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
        else if(strncmp(buffpiece,cpass,strlen(cpass))==0){//Analizo si recibí PASS
            return 4;
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
    int flagclose=0; //Flag que me indica si cerré o no el archivo. 0=no;!=0=sí
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
    strncpy(buffpiece,f,strlen(f)-1);//Elimino caracter nulo
    return buffpiece;
}
