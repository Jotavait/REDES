// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define DEFAULT_PORT 8080
#define DEFAULT_DOMAIN "localhost"
#define BUFFER_SIZE 1024
// ______________________________________________________LO QUE ES MOSTRA PER PANTALLA (FUNCIONS)___________________________________________
void mostrar_menu() {
    printf("\n================ MENU CLIENT ================\n");
    printf("1. Enviar missatge simple\n");
    printf("2. Suma de dos nombres (SUMA)\n");
    printf("3. Resta de dos nombres (RESTA)\n");
    printf("4. Multiplicacio de dos nombres (MULTIPLICAR)\n");
    printf("5. Potencia de dos nombres (POTENCIA)\n");
    printf("6. Sortir Programa (EXIT)\n");
    printf("Selecciona una opció: ");
}
 
int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char peticion[BUFFER_SIZE] = {0};
    int port = DEFAULT_PORT;
    char *ip_servidor = "127.0.0.1";

    if (argc >= 2) {
        ip_servidor = argv[1];
    }
    if (argc == 3) {
        port = atoi(argv[2]);
    }

    // Crear el socket TCP para el cliente
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error en crear el socket \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convertir direcció IP a binari
    if (inet_pton(AF_INET, ip_servidor, &serv_addr.sin_addr) <= 0) {
        printf("\nDirecció IP no vàlida o no suportada \n");
        return -1;
    }

    // _____________________________________________ESTABLIR CONNEXIÓ AMB EL SERVIDOR___________________________________________________________
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n Error en connectar amb el servidor \n");
        return -1;
    }

    printf("Connectat al servidor %s:%d\n", ip_servidor, port);

    int opcio = 0;
    double num1, num2;
    char msg[512];

    while (1) {
        mostrar_menu();
        if (scanf("%d", &opcio) != 1) {
            while (getchar() != '\n'); // Limpiar buffer de entrada
            continue;
        }
        while (getchar() != '\n'); // Limpiar salto de línea residual

        memset(peticion, 0, BUFFER_SIZE);

        if (opcio == 1) {
            printf("Introdueix el missatge: ");
            fgets(msg, sizeof(msg), stdin);
            msg[strcspn(msg, "\n")] = 0;
            snprintf(peticion, BUFFER_SIZE, "MSG %s", msg);
        } else if (opcio >= 2 && opcio <= 5) {
            printf("Introdueix el primer número: ");
            scanf("%lf", &num1);
            printf("Introdueix el segon número: ");
            scanf("%lf", &num2);

            if (opcio == 2) sprintf(peticion, "SUMA %.2f %.2f", num1, num2);
            else if (opcio == 3) sprintf(peticion, "RESTA %.2f %.2f", num1, num2);
            else if (opcio == 4) sprintf(peticion, "MULT %.2f %.2f", num1, num2);
            else if (opcio == 5) sprintf(peticion, "POTE %.2f %.2f", num1, num2);
        } else if (opcio == 6) {
            strcpy(peticion, "EXIT");
        } else {
            printf("Opció invàlida.\n");
            continue;
        }

        // Enviar petición al servidor
        send(sock, peticion, strlen(peticion), 0);

        // Recibir respuesta
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        printf("Resposta del servidor: %s", buffer);

        if (opcio == 6) {
            break; // Finalizar aplicación cliente
        }
    }

    close(sock);
    return 0;
}