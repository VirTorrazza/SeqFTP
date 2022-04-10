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
#define RETR "RETR" //Código RETR a enviar
#define CODE229 "229"
#define CODE331 "331"
#define CODE530 "530"
#define CODE550 "550"
#define PASS "PASS"
#define GET  "get"
#define PORT "PORT"
//#define DEBUG

struct sockaddr_in clientAddr;// Estructura de mi socket cliente
struct sockaddr_in localAddr;// Estructura de mi cliente local.
struct sockaddr_in dataAddr;// Estructura de datos
char server_data[256]; //Tamaño de mi server buffer
char operation [256];
char client_buffer [256];
int state_flag=0;//Flag de estado de mi login. 0=login correcto; 1=login incorrecto. 

int main(int argc,char * argv[]){
	int  sd; //Creación del socket descriptor
	char *ip=argv[1];
	char *port= argv[2];
	char *quitr="221 Goodbye";
	char filename [30];
	int dataFlag=0; 
	int  sdd; //Creación del socket descriptor para los datos
	char port_l [10]; // Variable para mi puerto (string)
	char ipread[16]; // Guardo la ip de mi estructura
	int high=0;
	int low=0;
	
	check_args(argc);
	sd=create_socket();
	set_struct(ip,port);//Seteo la estructura del socket
	establish_connection(sd);//Realizo la conexión del socket
	set_localstruct(sd);//Seteo mi estructura local
	int local_port=ntohs(localAddr.sin_port);
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
		else if ((strncmp(operation,GET,strlen(GET))==0)) {
			get_parameter(operation,filename);
			clear_buffer(client_buffer);
			sprintf(client_buffer,"%s %s\r\n",RETR,filename);
			write_command(sd,0);
			read_command(sd);
			
			if (strncmp(server_data,CODE550,strlen(CODE550))==0){
				print_response(server_data);
			}
			else if (strncmp(server_data,CODE229,strlen(CODE229))==0){
				dataFlag=1; // Flag para iniciar canal de datos
				print_response(server_data);
			}
			

		}
		else{
			printf("[-]Comando erróneo\n");
		}
		
		if (dataFlag==1){
			sdd= create_socket();// Creo el socket para datos
			local_port=local_port +1;
			sprintf(port_l,"%d",local_port);
			set_datastruct(ip,port_l);
			socklen_t data_size= sizeof(dataAddr);
			if(bind (sdd,(struct sockaddr *)&dataAddr,data_size)<0){
				perror("[-] Falla en el bind\n");
				set_datastruct(ip,"0");
				if(bind (sdd,(struct sockaddr *)&dataAddr,data_size)<0){
					perror("[-] Falla en el bind\n");
					exit(SYSF);// Falla del sistema
				}
				else{
					getsockname(sdd, (struct sockaddr *)&dataAddr,&data_size);
					inet_ntop(AF_INET,&(dataAddr.sin_addr),ipread,INET_ADDRSTRLEN);
					#ifdef DEBUG
					printf("[+] Bind exitoso al puerto\n");
					printf("Mi ip es %s y mi puerto %d\n",ipread,ntohs(dataAddr.sin_port));
					#endif
				}
			}
			else{
				getsockname(sdd, (struct sockaddr *)&dataAddr,&data_size);
				inet_ntop(AF_INET,&(dataAddr.sin_addr),ipread,INET_ADDRSTRLEN);
				
				#ifdef DEBUG
				printf("[+] Bind exitoso al puerto\n");
				printf("Mi ip es %s y mi puerto %d\n",ipread,ntohs(dataAddr.sin_port));
				#endif
			}
			if (listen(sdd,1)<0){
				printf("[-]Falla en el listen \n");
				exit(-1);
			}
			local_port=ntohs(dataAddr.sin_port);
			convert_ip(ipread,'.',','); // Convierto mi ip para enviarla adecuadamente con PORT
			#if DEBUG
			printf("Mi ip formateada es: %s\n",ipread);
			#endif
			convert_port(local_port,&high,&low);
			sprintf(client_buffer,"%s %s,%d,%d\r\n",PORT,ipread,high,low);
			printf("soy severdata: %s\n",client_buffer);
			write_command(sd,0);

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
void set_datastruct(char* ip,char* port){
	dataAddr.sin_family=AF_INET; //Familia del cliente
	dataAddr.sin_port=htons(atoi(port));//Puerto para la conexión
	dataAddr.sin_addr.s_addr=inet_addr(ip);//Dirección del cliente
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
	fgets(operation,256,stdin);
	substitute_char(operation,'\n','\0');

}

void authenticate_data(int s){
	char username [60];
	char password [60];
	char *auxstring;
	printf("username: ");
	fgets(username,sizeof(username),stdin);
	substitute_char(operation,'\n','\0');
	sprintf(client_buffer,"%s %s\r\n",USER,username);
	write_command(s,0);
	read_command(s);
	auxstring=verify_datanumber(server_data);
	if (strncmp(auxstring,CODE331,strlen(CODE331))==0){
		printf("passwd: ");
		fgets(password,sizeof(password),stdin);
		substitute_char(operation,'\n','\0');
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

void get_parameter(char * buff, char *param){ 
	char *auxcommand= buff;
	char *aux2;
	strtok(auxcommand, " ");
	aux2=strtok (NULL,"\0");
	strcpy(param,aux2);

}
void substitute_char(char *operation, char o, char d){

    if ((strlen(operation) > 0) && (operation[strlen (operation) - 1] ==o)){
        operation[strlen (operation) - 1] = d;

	}
}
void convert_ip( char * txt, char replace, char new){
	int stlen= strlen(txt);
	for (int i=0; i<stlen;i++){
		if (txt[i]==replace){
			txt[i]=new;
		}
	}

}void convert_port (int port, int *high, int * low){
	*high=(port >>8) & 0b0000000011111111;
	*low=(port)& 0b0000000011111111;

}