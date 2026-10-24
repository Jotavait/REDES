// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>  // Llibreria per realitzar funció erno (la veurem mes endevant en el codi)
#include <math.h>  // Llibreria per realitzar operacions

#define DEFAULT_PORT 8080   
#define BUFFER_SIZE 1024    // -----------> Defineix la quantitat de memoria del buffer (2^10 bytes)

// ___________________________________________FUNCIONS/OPERACIONS PRINCIPALS______________________________________________________
void procesar_comando(char *peticion, char *respuesta) {
    char cmd[30]; // ---------> 10 caracters per almecenar els nombres
    double a, b, res;

    if (sscanf(peticion, "%s %lf %lf", cmd, &a, &b) == 3) {  //lf variable tipus double
        if (strcmp(cmd, "SUMA") == 0) {
            sprintf(respuesta, "Resultat: %.2f\n", a + b);
        } else if (strcmp(cmd, "RESTA") == 0) {
            sprintf(respuesta, "Resultat: %.2f\n", a - b);
        } else if (strcmp(cmd, "MULT") == 0) {
            sprintf(respuesta, "Resultat: %.2f\n", a * b);
        } else if (strcmp(cmd, "POTE") == 0) {
            sprintf(respuesta, "Resultat: %.2f\n", pow(a, b));   // Pow funcio de la llibreria math.h per representar una potencia
        } else {
            strcpy(respuesta, "ERROR: Comanda no reconeguda.\n");
        }
    } else {
        strcpy(respuesta, "ERROR: Format de instruccio invalid.\n");
    }
}
  // ______________________________________________ESTRUCTURA DEL SERVIDOR_____________________________________________________________
int main(int argc, char *argv[]) {
    int sock, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char respuesta[BUFFER_SIZE] = {0};

    int port;
    char *endptr;

    if (argc == 1) {
        port = DEFAULT_PORT;
        printf("Cap port esta indicat. Utilitzant valor per defecte: %d\n", port);
    } else if (argc == 2) {
        errno = 0;   // Neteja variable global d'errors del sistema (Aixó ho vam tenir que buscar a Internet)
        port = strtol(argv[1], &endptr, 10);  // strtol agafa un text i el converteix a numero (Aixó ho vam tenir que buscar a Internet)
        if (errno != 0 || *endptr != '\0' || port < 1 || port > 65535) {
            fprintf(stderr, "Error: el port ha de ser un enter entre 1 i 65535.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Ús: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

  //_____________________________________________________CREACIO DEL SOCKET___________________________________________________________________
   
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error en crear el socket");  // perror es similar a un print pero per mostrar missatges d'error (Aixó ho vam tenir que buscar en internet)
    }
  /*
     * EXPLICACIÓN socket():
     * - AF_INET: Familia de direccions IPv4.
     * - SOCK_STREAM: Per que el socket sea fiable i segur (Model TCP), s'encarrega de asegurar que els paquets de dades arribin complets i en ordre sense perdre's pel camí.
     * - 0: Selecciona automaticament el protocol per defecte asociat (IPPROTO_TCP).
     */
   
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);   // htonl: Converteix num de 32 bits(4Bytes) de l'ordre de l'equip a l'ordre de la red.
    address.sin_port = htons(port);     // htons: Converteix num de 16 bits(2Bytes) de l'ordre de l'equip a l'ordre de la red.
 /*
     * EXPLICACIÓN PARÁMETRES I FUNCIONS DE LA RED:
     * - sin_family: Especifica la familia del protocol (AF_INET = IPv4). ---------------> IPv4: 4a versió del protocol d'Internet i principal sistema per identificar y connectar dispositius
     * - sin_addr.s_addr: Direcció IP del servidor que nosaltres hem creat. --------------> htons/htonl son funcions que modifiquen l'ordre (Ho expliquem mes endevant)
     *   Modifiquen l'ordre de bytes del host (Little Endian = (Byte menys significatiu / Direcció de memoria mes baixa) en x86) a l'ordre de red (Big Endian =(Byte mes significatiu / Direcció de memoria mes baixa)).
     * - INADDR_ANY: Permet al servidor acceptar qualsevol conexió provenientes de cualquier interfaz de red local.
     * - sin_port: Puerto de escucha convertido a Network Byte Order mediante htons().
     */
  
    if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {   // (Vam informarnos abans d'utilitzar el bind)
        perror("Error en fer el bind");
        close(sock);
        exit(EXIT_FAILURE);
    }
  /*
     * EXPLICACIÓN bind():
     * Li demana al Sistema Operatiu si pot reservar oficialment un número de port (p.ex. 8080-------------> s'utilitza habitualment com un port HTTP alternatiu per a servidors web.)
     */
  
    if (listen(sock, 3) < 0) {
        perror("Error en fer el listen");
        close(sock);
        exit(EXIT_FAILURE);   // -------------->(EXIT_FAILURE)  avisa que el programa ha finalitzar a causa d'un error (Ho vam tenir que buscar)
    }

    printf("Servidor en funcionament, esperant connexions de client...\n");  // Missatge que surt cuan inicialitzes el servidor abans i esperes a que es conectin els clients 

    while (1) {
        /*
         * EXPLICACIÓN accept() Y new_socket:
         * accept() extrae la primera petició de la cua de conexions pendients del socket 'sock' (RECORDEM QUE MAX. 3 connexions).
         * Crea y retorna un NUEVO socket ('new_socket') dedicado exclusivamente para la comunicació d'aquesta nova petició
         * bidireccional con este cliente en particular. El socket 'sock' original continúa escuchando
         * nuevas conexiones en el puerto especificado.
         */
        if ((new_socket = accept(sock, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Error en acceptar la connexió");
            continue;  // Ho vam tenir que buscar Continue ens informa que si la connexió amb un client falla el servidor no s'apaga ni es bloqueja
        }
          /*
     * EXPLICACIÓN listen():
     * listen es una funció que avisa al sistema operatiu "Ja tinc el port reservat (bind) ara posa al socket a escoltar per que els clients es puguin conectar al servidor"
     * - 3: El número máxim de conexions que pot tenir a l'espera.
     */

        // CONCURRENCIA MEDIANTE FORK:
        // Es crea un proces fill per gestionar la sessió del client de forma independient.
        pid_t pid = fork();
        if (pid == 0) {   // -------------------> jo soc el fill
            // Proces Fill
            close(sock); // El fill no necesita el socket passiu principal

            while (1) {
                memset(buffer, 0, BUFFER_SIZE);
                memset(respuesta, 0, BUFFER_SIZE);   

                if (recv(new_socket, buffer, BUFFER_SIZE, 0) <= 0) {
                    printf("Client desconnectat (PID: %d)\n", getpid());
                    break;
                }

                // Treu el salt de línea del final de la cadena recibida
                buffer[strcspn(buffer, "\r\n")] = 0;
                printf("Missatge rebut del client (PID: %d): %s\n", getpid(), buffer);

                // Comprobar orden EXIT
                if (strcmp(buffer, "EXIT") == 0) {
                    printf("Tancant connexió amb el client (PID: %d)...\n", getpid());
                    send(new_socket, "Connexió tancada\n", strlen("Connexió tancada\n"), 0);
                    break;
                } 
                // Opción 1: Envia missatge normal
                else if (strncmp(buffer, "MSG ", 4) == 0) {
                    snprintf(respuesta, sizeof(respuesta), "%s. Missatge rebut\n", buffer + 4);  // --------> buffer+4 agafa missatge ignorant "MSG"
                    send(new_socket, respuesta, strlen(respuesta), 0);
                } 
                // Opciones extra (Calculadora)
                else {
                    procesar_comando(buffer, respuesta);
                    send(new_socket, respuesta, strlen(respuesta), 0);
                }
            }

            close(new_socket);
            exit(0); // El fill finaliza el proces despres de desconectar-se del client
        } else {
            // Proces Pare: Tanca la copia local del socket client i segueix escoltant
            close(new_socket);
        }
    }

    close(sock);
    return 0;
}