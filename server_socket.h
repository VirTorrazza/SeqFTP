struct userdata{ //Defino una struct para usuario con nombre y contraseña
    char *user;
    char *password;
};

void check_args(int nargs);//Función para chequear el número de argumentos ingresados
int create_socket(void);//Función para crear el socket
void set_struct(char* port);//Función para setear sockaddr_in 
void binding(int sd);// Función que liga mi socket descriptor al puerto y dirección IP especificada
void listening(int sd);//Función de listening propia del servidor
int connection_accepted(int sd, socklen_t sockt);// Función para aceptar una conexión entrante
void clear_buffer(char *buff);//Función para limpiar mi buffer y setear sus valores en 0
void write_command(int s);//Función de escritura en el socket(Envío)
void read_command(int s);//Función de lectura en el socket (Recepción)
void set_structUser(char *usr, char *pass, struct userdata *udata);
int compare_input(struct userdata *ud);//Función para interpretar comandos recibidos por consola
void authenticate_data( struct userdata *ud);//Función para corroborar usuario correcto. Setea mi estructura de tipo userdata
FILE *open_file(char *p);//Función para abrir un archivo
void close_file(FILE* file);// Función para cerrar un archivo
char * get_passw(void);//Función para obtener el password