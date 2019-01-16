// Este archivo es el fichero fuente que al compilarse produce el ejecutable Ej1
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define WHITE "\x1b[37m"
#define RED "\x1b[31m"
#define BLUE "\x1b[34m"
#define RESET "\x1b[0m"

const int MENSAJESIZE = 500;

// Imprime la salida de los mensajes de las distintas acciones de cada proceso
void imprime(char *color, char *proceso, int pid, char *mensaje)
{
  printf("%sEl proceso %s (PID=%d, Ej1)" RESET " %s\n", color, proceso, pid, mensaje);
}

// Imprime el mensaje introducido por el usuario
void imprime_mensaje(char *mensaje)
{
  printf("# %s%s" RESET "\n", WHITE, mensaje);
}

void main()
{

  // Datos de uso de CPU
  struct tms datos_uso_cpu_inicial, datos_uso_cpu_final;
  clock_t tiempo_inicial, tiempo_final;
  tiempo_inicial = times(&datos_uso_cpu_inicial);
  int CLK_TCK = sysconf(_SC_CLK_TCK);

  printf("Iniciando Ej1...\n");

  // Tubería sin nombre a compartir entre el proceso P1 y el proceso P2
  int tuberia[2];

  // Pid del proceso actual
  int pid = getpid();

  // Mensaje
  char mensaje[MENSAJESIZE];

  // Nombre del fichero FIFO
  char *fifo = "fichero1";

  // Mensaje enviado/recibido en la cola
  typedef struct {
    int pid;
    char mensaje[MENSAJESIZE];
  } datos_mensaje_cola;
  struct {
    long tipo;
    datos_mensaje_cola datos;
  } mensaje_cola;

  // Creamos la tubería y si produce error terminamos la ejecución
  if (pipe(tuberia) == -1)
  {
    perror("pipe");
    exit(-1);
  }

  imprime(RED, "P1", pid, "ha sido creado correctamente");

  // Creamos el proceso hijo P2 y si produce error terminamos la ejecución
  int resultado_fork = fork();
  if (resultado_fork == -1)
  {
    perror("fork");
    exit(-1);
  }
  // Proceso hijo P2
  else if (resultado_fork == 0)
  {
    // Obtenemos el pid del proceso P2
    pid = getpid();

    imprime(BLUE, "P2", pid, "ha sido creado correctamente");

    // Leemos el mensaje escrito por el usuario en P1 a través de la tubería sin
    // nombre
    read(tuberia[0], mensaje, MENSAJESIZE);
    imprime(BLUE, "P2", pid, "recibe el mensaje a través de una tubería sin nombre:");
    imprime_mensaje(mensaje);

    // Cerramos la tubería
    close(tuberia[0]);
    close(tuberia[1]);

    // Eliminamos el fichero si ha sido creado anteriormente
    unlink(fifo);
    // Creamos el fichero FIFO "fichero1"
    int resultado_mknod = mknod(fifo, S_IFIFO | 0666, 0);
    if (resultado_mknod == -1)
    {
      perror("mknod");
      exit(-1);
    }
    else
    {
      imprime(BLUE, "P2", pid, "crea correctamente el fichero FIFO \"fichero1\"");
    }

    // Abrimos el fichero FIFO "fichero1"
    int resultado_fichero_open = open(fifo, O_RDWR | O_NONBLOCK);
    if (resultado_fichero_open == -1)
    {
      perror("open");
      exit(-1);
    }
    else
    {
      imprime(BLUE, "P2", pid, "ha abierto correctamente el fichero FIFO \"fichero1\"");
    }
    
    // Escribimos en el fichero FIFO "fichero1" el mensaje
    int resultado_fichero_write = write(resultado_fichero_open, mensaje, MENSAJESIZE);
    if (resultado_fichero_write == -1)
    {
      perror("write");
      exit(-1);
    }
    else
    {
      imprime(BLUE, "P2", pid, "ha copiado correctamente el mensaje en el fichero FIFO \"fichero1\"");
      imprime_mensaje(mensaje);
    }

    // Creamos el path al ejecutable Ej2
    // Sacamos el directorio de trabajo actual
    char directorio_actual[PATH_MAX];
    getcwd(directorio_actual, sizeof(directorio_actual));
    // Concatenamos la ruta relativa al directorio de trabajo actual
    char path[PATH_MAX];
    strcat(path, directorio_actual);
    strcat(path, "/Trabajo2/Ej2");
    // Creamos los argumentos a pasar a execv
    char *const argumentos[] = {};
    // Ejecutamos Ej2
    imprime(BLUE, "P2", pid, "ejecuta el ejecutable \"Ej2\"");
    int resultado_exec_ej2 = execv(path, argumentos);
    if (resultado_exec_ej2 == -1)
    {
      perror("execv");
      exit(-1);
    }

    imprime(BLUE, "P2", pid, "ha terminado su ejecución");

    pause();
  }
  // Proceso padre P1
  else
  {

    // Pedimos el mensaje al usuario
    imprime(RED, "P1", pid, "Escriba el mensaje deseado [Máximo 500 caracteres]:");

    // Leemos el mensaje
    scanf("%[^\n]", mensaje);

    // Guardamos el mensaje en la tubería
    imprime(RED, "P1", pid, "ha guardado el mensaje en la tubería sin nombre:");
    imprime_mensaje(mensaje);
    write(tuberia[1], mensaje, strlen(mensaje) + 1);
    
    // Cerramos la tubería
    close(tuberia[0]);
    close(tuberia[1]);

    // Creamos la llave
    key_t llave = ftok("Ej1", 'P');
    // Creamos la cola de mensajes
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
      imprime(RED, "P1", pid, "ha creado correctamente la cola de mensajes");
    }

    imprime(RED, "P1", pid, "está esperando el mensaje a través de la cola de mensajes");
    mensaje_cola.tipo = 2;
    int resultado_msgrcv = msgrcv(msqid, &mensaje_cola, longitud, 2, 0);
    if (resultado_msgrcv == -1)
    {
      perror("msgrcv");
      exit(-1);
    }
    else 
    {
      imprime(RED, "P1", pid, "ha recibido el mensaje desde P3 a través de la cola de mensajes");
      imprime_mensaje(mensaje_cola.datos.mensaje);
      char pid_mensaje[10];
      sprintf(pid_mensaje, "PID = %d", mensaje_cola.datos.pid);
      imprime_mensaje(pid_mensaje);
    }

    // Matamos el proceso P3
    imprime(RED, "P1", pid, "quiere matar la ejecución de P3");
    int resultado_kill_p3 = kill(mensaje_cola.datos.pid, SIGTERM);
    if (resultado_kill_p3 == -1)
    {
      perror("kill");
      exit(-1);
    }
    else
    {
      imprime(RED, "P1", pid, "ha matado la ejecución de P3");
    }

    // Borramos el fichero "fichero1"
    int resultado_unlink = unlink(fifo);
    if (resultado_unlink == -1)
    {
      perror("unlink");
      exit(-1);
    }
    else
    {
      imprime(RED, "P1", pid, "ha eliminado correctamente el fichero \"fichero1\"");
    }

    // Esperamos a que P2 termine
    wait(0);

    imprime(RED, "P1", pid, "ha terminado su ejecución");

    // Mostramos las estadísticas de uso de CPU
    tiempo_final = times(&datos_uso_cpu_final);
    imprime_mensaje("Estadísticas de uso de CPU");
    printf("- Tiempo real = %.2f segundos\n", (float)(tiempo_final - tiempo_inicial) / CLK_TCK);
    printf("- Tiempo de uso de la CPU:\n");
    printf("  - P1 en modo usuario    = %.2f segundos\n", (float)(datos_uso_cpu_final.tms_utime - datos_uso_cpu_inicial.tms_utime) / CLK_TCK);
    printf("  - P1 en modo núcleo     = %.2f segundos\n", (float)(datos_uso_cpu_final.tms_stime - datos_uso_cpu_inicial.tms_stime) / CLK_TCK);
    printf("  - Hijos en modo usuario = %.2f segundos\n", (float)(datos_uso_cpu_final.tms_cutime - datos_uso_cpu_inicial.tms_cutime) / CLK_TCK);
    printf("  - Hijos en modo núcleo  = %.2f segundos\n", (float)(datos_uso_cpu_final.tms_cstime - datos_uso_cpu_inicial.tms_cstime) / CLK_TCK);

    exit(0);
  }

}
