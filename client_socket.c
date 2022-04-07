#include <stdio.h>//perror,printf
#include <stdlib.h>//exit
#include <arpa/inet.h>//socket
#include <stdbool.h>//bool types
#include <string.h>//memset
#include <unistd.h>//file management
#include "client_socket.h"//Librería de funciones de mi cliente
#include "error.h"//Definición de errores
#define VERSION "1.0"//Versión de mi programa
#define TYPEC "cltFtp" //Nombre/tipo de mi cliente
#define CODE "220" //Código a enviar
#define USER "USER" //
#define CODE331 "331"
#define CODE530 "530"
#define PASS "PASS"

struct sockaddr_in clientAddr;//Estructura de mi socket cliente
struct sockaddr_in localAddr;//Estructura de mi cliente local.
char server_data[256]; //Tamaño de mi server buffer
char operation [256];
char client_buffer [256];
int state_flag=0;//Flag de estado de mi login. 0=login correcto; 1=login incorrecto. 

int main(int argc,char * argv[]){
	int  sd; //Creación del socket descriptor
	char *ip=argv[1];
	char *port= argv[2];
	char *quitr="221 Goodbye";
	
	check_args(argc);
	sd=create_socket();
	set_struct(ip,port);//Seteo la estructura del socket
	establish_connection(sd);//Realizo la conexión del socket
	set_localstruct(sd);//Seteo mi estructura local
	clear_buffer(client_buffer);
	sprintf(client_buffer,"%s %s %s\r\n",CODE,TYPEC,VERSION);
	write_command(sd,0);
	read_command(sd);
	print_response(server_data);
	clear_buffer(client_buffer);
	authenticate_data(sd);
	
	while(state_flag==0){
		get_input(operation);
		if (strncmp(operation,"QUIT",strlen(operation))==0){
			write_command(sd,1);
			read_command(sd);
			if (strncmp(server_data,quitr,strlen(quitr))==0){
				print_response(server_data);
				printf("[+]Desconectando...\n");
				close(sd);
				break;
			}

		}
		else{
			printf("[-]Comando erróneo\n");
		}

	}
	
	return 0;
}

void check_args(int nargs){

    if (nargs <3 || nargs>3){
		perror("[-]Número incorrecto de argumentos. 3 argumentos requeridos\n");
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

void set_struct(char* ip,char* port){
	clientAddr.sin_family=AF_INET; //Familia del cliente
	clientAddr.sin_port=htons(atoi(port));//Puerto para la conexión
	clientAddr.sin_addr.s_addr=inet_addr(ip);//Dirección del cliente
}

void establish_connection(int sockd){
	int connect_stat;
	connect_stat=connect(sockd,(struct sockaddr *)&clientAddr,sizeof(clientAddr));
	if(connect_stat<0){
		perror("[-]No se puede establecer conexión con el servidor\n");
		exit(CONN);

	}
	else{
		printf("[+]Conexión establecida con el servidor\n");		
	}
}

void clear_buffer(char * buff){
	memset(buff,0,sizeof(*buff));

}

void write_command(int s, int n){
	if(n==0){
		int write_stat=write(s,client_buffer,sizeof(client_buffer));
		if(write_stat==-1){
			perror("[-]No se puede escribir en el socket\n");
			exit(WRTE);
		}
		else{
			#ifdef DEBUG
			printf("[+]Operación de escritura exitosa\n");
			printf("Se escribió %s",client_buffer);
			#endif
		}
	}
	else if(n==1){
		int write_stat=write(s,operation,sizeof(operation));
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
}

void read_command(int s){
	int reads=read(s,server_data,sizeof(server_data));
	if(reads==-1){
		perror("[-]No se pudo recibir el mensaje\n");
		exit(READ);
	}
	else{
		#ifdef DEBUG
		printf("[+]Operación de lectura exitosa\n");
		#endif
	}
}

void print_response(char *buff){
	printf("%s\n",buff);

}

void get_input(char *operation){
	printf("< ");
	scanf("%s",operation); //Almaceno lo escrito por teclado
}

void authenticate_data(int s){
	char username [60];
	char password [60];
	char *auxstring;
	printf("username: ");
	scanf("%s",username);
	sprintf(client_buffer,"%s %s\r\n",USER,username);
	write_command(s,0);
	read_command(s);
	auxstring=verify_datanumber(server_data);
	if (strncmp(auxstring,CODE331,strlen(CODE331))==0){
		printf("passwd: ");
		scanf("%s",password);
		clear_buffer(client_buffer);
		sprintf(client_buffer,"%s %s\r\n",PASS,password);
		write_command(s,0);
		read_command(s);
		printf("%s\n",server_data);
		
	}
    else if(strncmp(auxstring,CODE530,strlen(CODE530))==0){
        print_response(server_data);
        state_flag=1;
        close(s);

    }
    auxstring=NULL;
    free(auxstring);
	
}
char* verify_datanumber(char * buffer){ //Retorno mi código en string
    char *aux=malloc(4);
    sprintf(aux, "%s",buffer);
	return strtok(aux, " ");
}
void set_localstruct(int sd){
    localAddr.sin_family=AF_INET;
    localAddr.sin_port=htons(0);
    localAddr.sin_addr.s_addr=INADDR_ANY;
	//#ifdef DEBUG
	socklen_t localsz=sizeof(localAddr);
	getsockname(sd,(struct sockaddr*)&localAddr,&localsz);
	printf("[+] Mi puerto es %d\n",ntohs(localAddr.sin_port));
	//#endif
}
