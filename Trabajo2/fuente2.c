// Este archivo es el fichero fuente que al compilarse produce el ejecutable Ej2
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define WHITE "\x1b[37m"
#define GREEN "\x1b[32m"
#define MAGENTA "\x1b[35m"
#define RESET "\x1b[0m"

const int MENSAJESIZE = 500;

// Imprime la salida de los mensajes de las distintas acciones de cada proceso
void imprime(char *color, char *proceso, int pid, char *mensaje)
{
  printf("%sEl proceso %s (PID=%d, Ej2)" RESET " %s\n", color, proceso, pid, mensaje);
}

// Imprime el mensaje introducido por el usuario
void imprime_mensaje(char *mensaje)
{
  printf("# %s%s" RESET "\n", WHITE, mensaje);
}

void main()
{

  printf("Iniciando Ej2...\n");

  // Pid del proceso
  int pid = getpid();

  // Mensaje
  char mensaje[MENSAJESIZE];

  // Datos del semáforo
  union {
    // Valor del semáforo
    int valor;
  } semaforo;

  // Operación V
  struct sembuf operacion_v = {0, -1, 0};
  // Operación P
  struct sembuf operacion_p = {0, +1, 0};

  // nombre del fichero fifo
  char *fifo = "fichero1";

  // Abrimos el fichero FIFO "fichero1"
  int resultado_fichero_open = open(fifo, O_RDONLY | O_NONBLOCK);
  if (resultado_fichero_open == -1)
  {
    perror("open");
    exit(-1);
  }
  else
  {
    imprime(GREEN, "P2", pid, "ha abierto correctamente el fichero FIFO \"fichero1\"");
  }

  // Leemos el mensaje escrito en el fichero FIFO "fichero1"
  int resultado_fichero_read = read(resultado_fichero_open, &mensaje, MENSAJESIZE);
  if (resultado_fichero_read == -1)
  {
    perror("read");
    exit(-1);
  }
  else
  {
    imprime(GREEN, "P2", pid, "ha leído el mensaje a través del fichero FIFO \"fichero1\":");
    imprime_mensaje(mensaje);
  }
  
  // Cerramos el fichero FIFO "fichero1"
  int resultado_fichero_close = close(resultado_fichero_open);
  if (resultado_fichero_close == -1)
  {
    perror("close");
    exit(-1);
  }
  else
  {
    imprime(GREEN, "P2", pid, "ha cerrado correctamente el fichero FIFO \"fichero1\"");
  }

  // Creamos la llave
  key_t llave = ftok("Ej2", 'S');
  // Creamos el semáforo
  int sem1 = semget(llave, 1, 0777 | IPC_CREAT);
  if (sem1 == -1)
  {
    perror("semget");
    exit(-1);
  }
  else {
    imprime(GREEN, "P2", pid, "ha creado correctamente un semáforo");
  }
  // Creamos la zona de memoria compartida
  int shmid = shmget(llave, MENSAJESIZE * sizeof (int), IPC_CREAT | 0777);
  if (shmid == -1)
  {
    perror("shmget");
    exit(-1);
  }
  else {
    imprime(GREEN, "P2", pid, "ha creado correctamente una zona de memoria compartida");
  }
  // Unimos  la región de memoria compartida de la tabla de regiones con el
  // espacio de direcciones del proceso
  int *vc1 = shmat(shmid, 0, 0);

  // Cerramos el semáforo para que solo P2 pueda escribir en él
  semaforo.valor = 0;
  semctl(sem1, 0, SETVAL, semaforo);
  semop(sem1, &operacion_p, 1);

  // Creamos el proceso hijo P3 y si produce error terminamos la ejecución
  int resultado_fork = fork();
  if (resultado_fork == -1)
  {
    perror("fork");
    exit(-1);
  }
  // Proceso P3
  else if (resultado_fork == 0)
  {
    pid = getpid();
    imprime(MAGENTA, "P3", pid, "ha sido creado correctamente");

    // Creamos el path al ejecutable Ej3
    // Sacamos el directorio de trabajo actual
    char directorio_actual[PATH_MAX];
    getcwd(directorio_actual, sizeof(directorio_actual));
    // Concatenamos la ruta relativa al directorio de trabajo actual
    char path[PATH_MAX];
    strcat(path, directorio_actual);
    strcat(path, "/Trabajo2/Ej3");
    // Creamos los argumentos a pasar a execv
    char *const argumentos[] = {};
    // Ejecutamos Ej3
    imprime(MAGENTA, "P3", pid, "ejecuta el ejecutable \"Ej3\"");
    int resultado_exec_ej3 = execv(path, argumentos);
    if (resultado_exec_ej3 == -1)
    {
      perror("execv");
      exit(-1);
    }

    pause();

    imprime(MAGENTA, "P3", pid, "ha terminado su ejecución");
  }
  // Proceso P2
  else {
    // Esperamos 1 segundo
    imprime(GREEN, "P2", pid, "espera 1 segundo");
    sleep(1);
    imprime(GREEN, "P2", pid, "ha esperado 1 segundo");

    // Escribimos el mensaje en la memoria compartida
    for (int i = 0; i < MENSAJESIZE; ++i) {
      vc1[i] = mensaje[i];
    }
    imprime(GREEN, "P2", pid, "ha escrito el mensaje en la memoria compartida");

    // Abrimos el semáforo
    semop(sem1, &operacion_v, 1);

    imprime(GREEN, "P2", pid, "ha terminado su ejecución");

    wait(0);
  }

}  
