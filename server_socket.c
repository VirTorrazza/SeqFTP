#include <stdio.h>//perror,printf
#include <stdlib.h>//exit
#include <arpa/inet.h>//socket
#include <stdbool.h>//bool types
#include <string.h>//memset
#include <unistd.h>//file management
#define VERSION "1.0"//Versión de mi programa
#define TYPES "srvFtp" //Nombre/tipo de mi servidor
#define TYPEC "cltFtp" //Nombre/tipo de mi cliente
#define LOCALHOST "127.0.0.1"
#define QUEUE   5      //Número de solicitudes que se pueden encolarF
#define GOODBYE "Quit recibido"
char * cquit="QUIT";
char * xversion="220 cltFtp 1.0";
char server_buffer[256];
char client_buffer[256];
char *operations [20];
struct sockaddr_in serverAddr;
struct sockaddr_in clientAddr;
void check_args(int nargs);//Función para chequear el número de argumentos ingresados
int create_socket(void);//Función para crear el socket
void set_struct(char* port);//Función para setear sockaddr_in 
void binding(int sd);// Función que liga mi socket descriptor al puerto y dirección IP especificada
void listening(int sd);//Función de listening propia del servidor
int connection_accepted(int sd, socklen_t sockt);// Función para aceptar una conexión entrante
void clear_buffer(char *buff);//Función para limpiar mi buffer y setear sus valores en 0
void write_command(int s);//Función de escritura en el socket(Envío)
void read_command(int s);//Función de lectura en el socket (Recepción)
int compare_input();//Función para interpretar comandos recibidos por consola
bool cconnect= true;

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
  
    
    csd=connection_accepted(sd, size_addr);
    clear_buffer(client_buffer);
    read_command(csd);
    clear_buffer(server_buffer);
    sprintf(server_buffer, "%s %s %s\r\n", operations[0], TYPES, VERSION);
    write_command(csd);
    #ifdef DEBUG
    printf("[+]Se escribe:%s",server_buffer);
    #endif
    
    while(1){
        clear_buffer(client_buffer);
        read_command(csd);
        compare_input();
        clear_buffer(server_buffer);

        if(compare_input()==0){
            printf("Desconectando cliente...\n");
            sprintf(server_buffer, "%s\r\n", GOODBYE);
            write_command(csd);
            sleep(1);
            shutdown(csd,SHUT_RDWR);
            close(csd);
            csd=connection_accepted(sd, size_addr);
    
        }

        else if (compare_input()==1){
            char *error="Código de Error";
            clear_buffer(server_buffer);
            sprintf(server_buffer, "%s\r\n",error);
            write_command(csd);
            
        } 
        else if (compare_input()==2){
            clear_buffer(server_buffer);
            sprintf(server_buffer, "%s %s %s\r\n", operations[0], TYPES, VERSION);
            write_command(csd);
        }
        
    }

    
    close(sd);

    return 0;
    
}
void check_args(int nargs){

    if(nargs != 2){
        perror("[-] Numero incorrecto de argumentos.1 argumento requerido\n");
        exit(-1);
    }

}
int create_socket(void){
	int sd=socket(AF_INET,SOCK_STREAM,0);//Especificaciones de mi socket TCP
	if(sd<0){
		perror("[-]No se pudo crear el socket\n");
		exit(-1);// código de error -1 que me indica que la imposibilidad de crear el socket
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
        exit(-1);
    }
    #ifdef DEBUG
    printf("[+] Bind exitoso al puerto\n");
    #endif
}

void listening(int sd){
    int listen_stat=listen(sd,QUEUE);
    if(listen_stat<0){
		perror("[-]Error en la escucha\n");
		exit(0);
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
        exit(1);
    }
    else{
        #ifdef DEBUG
        printf("{+]Accept exitoso\n");
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
		exit(2);
	}
	else{
        #ifdef DEBUG
		printf("[+]Operación de lectura exitosa\n");
        #endif
        printf("Recibido <%s\n",client_buffer);
	}
}

void write_command(int s){
	int write_stat=write(s,server_buffer,sizeof(server_buffer));
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
int compare_input(){
    int compares=strncasecmp(client_buffer, cquit,strlen(cquit)); //Analizo si se trata del comando quit
		if (compares==0){
        
            return 0;
        }
        else if(strncasecmp(client_buffer, xversion,strlen(xversion))==0){
            
            return 2;
        }
        else{
            printf("Comando  erróneo %s\n",client_buffer);
            return 1;
        }
}
