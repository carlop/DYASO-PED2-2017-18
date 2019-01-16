// Este archivo es el fichero fuente que al compilarse produce el ejecutable Ej3
#include <limits.h>
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
#include <time.h>
#include <unistd.h>

#define WHITE "\x1b[37m"
#define CYAN "\x1b[36m"
#define RESET "\x1b[0m"

const int MENSAJESIZE = 500;

// Imprime la salida de los mensajes de las distintas acciones de cada proceso
void imprime(char *color, char *proceso, int pid, char *mensaje)
{
  printf("%sEl proceso %s (PID=%d, Ej3)" RESET " %s\n", color, proceso, pid, mensaje);
}

// Imprime el mensaje introducido por el usuario
void imprime_mensaje(char *mensaje)
{
  printf("# %s%s" RESET "\n", WHITE, mensaje);
}

void main()
{

  printf("Iniciando Ej3...\n");

  // Pid del proceso actual
  int pid = getpid();

  // Mensaje
  char mensaje[MENSAJESIZE];

  // Mensaje enviado/recibido en la cola
  typedef struct {
    int pid;
    char mensaje[MENSAJESIZE];
  } datos_mensaje_cola;
  struct {
    long tipo;
    datos_mensaje_cola datos;
  } mensaje_cola;

  // Datos del semáforo
  union {
    // Valor del semáforo
    int valor;
  } semaforo;

  // Operación V
  struct sembuf operacion_v = {0, -1, 0};
  // Operación P
  struct sembuf operacion_p = {0, +1, 0};

  // Creamos la llave
  key_t llave = ftok("Ej3", 'S');
  // Cargamos el semáforo
  int sem1= semget(llave, 1, 0666);
  if (sem1 == -1)
  {
    perror("semget");
    exit(-1);
  }
  else {
    imprime(CYAN, "P3", pid, "ha cargado correctamente el semáforo");
  }
  // Cargamos la zona de memoria compartida
  int shmid = shmget(llave, MENSAJESIZE * sizeof (int), SHM_R | SHM_W);
  if (shmid == -1)
  {
    perror("shmget");
    exit(-1);
  }
  else {
    imprime(CYAN, "P3", pid, "ha cargado correctamente una zona de memoria compartida");
  }

  // Unimos  la región de memoria compartida de la tabla de regiones con el
  // espacio de direcciones del proceso
  int *vc1 = shmat(shmid, 0, 0);

  // Esperamos que el semáforo nos permita leer de la zona de memoria compartida
  imprime(CYAN, "P3", pid, "está esperando que el semáforo permita el acceso a la zona de memoria compartida");
  semaforo.valor = 0;
  while(semctl(sem1, 0, GETVAL, semaforo) > 0);
  imprime(CYAN, "P3", pid, "tiene acceso a la zona de memoria compartida");

  // Cerramos el semáforo para que solo P3 pueda escribir en él
  semop(sem1, &operacion_p, 1);
  
  // Leemos el mensaje de la memoria compartida
  for (int i = 0; i < MENSAJESIZE; ++i) {
    mensaje[i] = vc1[i];
  }
  imprime(CYAN, "P3", pid, "ha leído el mensaje de la zona de memoria compartida");
  imprime_mensaje(mensaje);

  // Separamos la zona de memoria compartida del espacio de direcciones
  // virtuales del proceso actual
  shmdt(vc1);
  // Liberamos el semáforo
  semctl(sem1, 0, IPC_RMID);
  semop(sem1, &operacion_v, 1);
  imprime(CYAN, "P3", pid, "ha liberado la zona de memoria compartida y el semáforo");

  // Realizamos la espera activa
  imprime(CYAN, "P3", pid, "inicia la espera activa");
  int espera_activa;
  for (int i = 0; i < 2000000; ++i) {
    espera_activa = open("README.txt", O_RDONLY);
    close(espera_activa);
  }
  imprime(CYAN, "P3", pid, "termina la espera activa");

  mensaje_cola.datos.pid = pid;
  strcpy(mensaje_cola.datos.mensaje, mensaje);

  // Creamos la llave
  llave = ftok("Ej1", 'P');
  // Accedemos la cola de mensajes
  int msqid = msgget(llave, IPC_CREAT | 0600);
  // Calculamos la longitud
  int longitud = sizeof(mensaje_cola) - sizeof(mensaje_cola.tipo);

  if (msqid == -1)
  {
    perror("msgget");
    exit(-1);
  }
  else
  {
    imprime(CYAN, "P3", pid, "ha accedido correctamente la cola de mensajes");
    // Enviamos el mensaje a la cola de mensajes creada por el proceso P1
    mensaje_cola.tipo = 2;
    int resultado_msgsnd = msgsnd(msqid, &mensaje_cola, longitud, 0);
    if (resultado_msgsnd == -1)
    {
      perror("msgsnd");
      exit(-1);
    }
    else 
    {
      imprime(CYAN, "P3", pid, "ha enviado el mensaje a P1 a través de la cola de mensajes");
    }

  }

  imprime(CYAN, "P3", pid, "ha terminado su ejecución");
  
  pause();

  exit(0);
}
