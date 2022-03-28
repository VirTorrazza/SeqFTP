# SeqFTP
Comunicación cliente-servidor utilizando protocolo TCP
El servidor funcionará de forma iterativa, es decir, hasta que no termine de atender por
completo a un cliente, no podrá aceptar nuevas peticiones de conexión de otros clientes. En cuanto al funcionamiento de este programa, se prevee que el servidor reciba como argumento el puerto donde estará escuchando peticiones del cliente.Asimismo,todas las operaciones que el cliente solicite al servidor son cadenas de caracteres.El código de respuesta será un número de 3 cifras y la descripción es una breve explicación de dicho código de respuesta.
Al arrancar el cliente se conectara al servidor de ficheros a través de un socket TCP. El
servidor aceptará dicha conexión y le enviará un mensaje de saludo que consistirá en una
cadena de caracteres formada por un código de respuesta y un mensaje que informe de la
versión del servidor de ficheros que se está usando.El cliente imprimirá el mensaje recibido. A partir de ese momento el cliente estará preparado para enviar operaciones al servidor.
