#!/bin/bash
# Este archivo es un script que compila el trabajo 2 y luego ejecuta Ej1

# Limpiamos la pantalla
clear

# Comprobamos si existen ejecutables anteriores y en caso afirmativo lo eliminamos
if [ -e "./Trabajo2/Ej1" ] && [ -e "./Trabajo2/Ej2" ] && [ -e "./Trabajo2/Ej3" ]; then
  rm ./Trabajo2/Ej1
  rm ./Trabajo2/Ej2
  rm ./Trabajo2/Ej3
fi

# Compilamos los tres archivos
gcc ./Trabajo2/fuente1.c -o ./Trabajo2/Ej1
gcc ./Trabajo2/fuente2.c -o ./Trabajo2/Ej2
gcc ./Trabajo2/fuente3.c -o ./Trabajo2/Ej3

# Damos permisos de ejecución a los ejecutables
chmod +x ./Trabajo2/Ej1
chmod +x ./Trabajo2/Ej2
chmod +x ./Trabajo2/Ej3

# Ejecutamos Ej1
./Trabajo2/Ej1

# Eliminamos los ejecutables una vez terminada la ejecución
rm ./Trabajo2/Ej1
rm ./Trabajo2/Ej2
rm ./Trabajo2/Ej3
