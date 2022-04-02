#include <stdio.h>//perror,printf
#include <stdlib.h>//exit
#include <arpa/inet.h>//socket
#include <stdbool.h>//bool types
#include <string.h>//memset
#include <unistd.h>//file management
#define VERSION "1.0"//Versión de mi programa
#define TYPEC "cltFtp" //Nombre/tipo de mi cliente
#define CODE "220" //Código a enviar
#define USER "USER" //
#define CODE331 "331"
#define PASS "PASS"

struct sockaddr_in clientAddr;//estructura de mi socket cliente
char server_data[256]; //Tamaño de mi server buffer
char operation [256];
char client_buffer [256];
void check_args(int nargs);//Función para chequear el número de argumentos ingresados
int create_socket(void);//Función para crear el socket
void set_struct(char*ip,char* port);//Función para setear sockaddr_in 
void establish_connection(int sockd);//Función para establecer la conexión
void write_command(int s,int n);// Función para escritura del socket;n es un número que representa a un array. operation=1;client_buffer=0
void read_command(int s);//Función para lectura del socket
void clear_buffer(char *buff);//Función para blanquear el buffer pasado como parámetro en el valor 0
void print_response(char *buff);//Función para imprimir buffer del servidor(respuesta)
void get_input(char *operation);//Almaceno operación ingresada por teclado
void authenticate_data( int s);
char* verify_datanumber(char * buffer);

int main(int argc,char * argv[]){
	int  sd; //Creación del socket descriptor
	char *ip=argv[1];
	char *port= argv[2];
	char *quitr="Quit recibido";
	
	check_args(argc);
	sd=create_socket();
	set_struct(ip,port);//Seteo la estructura del socket
	establish_connection(sd);//Realizo la conexión del socket
	clear_buffer (client_buffer);
	sprintf(client_buffer, "%s %s %s\r\n",CODE,TYPEC,VERSION);
	write_command(sd,0);
	read_command(sd);
	print_response(server_data);
	clear_buffer(client_buffer);
	authenticate_data(sd);
	
	while(true){
		get_input(operation);
		if (strncmp(operation,"QUIT",strlen(operation))==0){
			write_command(sd,1);
			read_command(sd);
			if (strncmp(server_data,quitr,strlen(quitr))==0){
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
		exit(-2);
	}
}

int create_socket(void){
	int sd=socket(AF_INET,SOCK_STREAM,0);//Especificaciones de mi socket TCP
	if(sd<0){
		perror("[-]No se pudo crear el socket\n");
		exit(-1);// Código de error -1 que me indica que la imposibilidad de crear el socket
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
		exit(1);

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
			exit(2);
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
			exit(2);
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
		exit(2);
	}
	else{
		#ifdef DEBUG
		printf("[+]Operación de lectura exitosa\n");
		#endif
	}
}

void print_response(char *buff){
	printf("El servidor envió: %s\n",buff);

}

void get_input(char *operation){
	printf("< ");
	scanf("%s",operation); //Almaceno lo escrito por teclado
}

void authenticate_data(int s){
	char *username [60];
	char *password [60];
	char *auxstring;
	printf("username: ");
	scanf("%s",username);
	sprintf(client_buffer,"%s %s\r\n",USER,username);
	write_command(s,0);
	read_command(s);
	//printf("lei %s\n",server_data);
	auxstring=verify_datanumber(server_data);
	if (strncmp(auxstring,CODE331,strlen(CODE331))==0){
		printf("passwd: ");
		scanf("%s",password);
		clear_buffer(client_buffer);
		sprintf(client_buffer,"%s %s\r\n",PASS,password);
		write_command(s,0);
		read_command(s);
		//printf("scribi en el client %s\n",client_buffer);
		printf("%s\n",server_data);
		
	}
	
}
char* verify_datanumber(char * buffer){ //Retorno mi código en string
	return strtok(buffer, " ");
}
