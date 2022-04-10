void check_args(int nargs);//Función para chequear el número de argumentos ingresados
int create_socket(void);//Función para crear el socket
void set_struct(char*ip,char* port);//Función para setear sockaddr_in 
void set_localstruct(int sd);
void establish_connection(int sockd);//Función para establecer la conexión
void write_command(int s,int n);// Función para escritura del socket;n es un número que representa a un array. operation=1;client_buffer=0
void read_command(int s);//Función para lectura del socket
void clear_buffer(char *buff);//Función para blanquear el buffer pasado como parámetro en el valor 0
void print_response(char *buff);//Función para imprimir buffer del servidor(respuesta)
void get_input(char *operation);//Almaceno operación ingresada por teclado
void authenticate_data( int s);
char* verify_datanumber(char * buffer);
void get_parameter(char * buff, char *param); //Función para obtener el parámetro de un comando
void substitute_char( char *operation, char o, char d);//Función para substituir un caracter
void convert_ip( char * txt, char replace, char new);
void convert_port (int port, int *high, int * low);
void set_datastruct(char* ip,char* port);