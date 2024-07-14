## Este programa se usa cmd y durante la ejecucion de la simulacion en memoria se pueden utilizar las siguientes teclas.

**Tecla I: Interrupción por entrada/salida (pasa a estado bloqueado)**

El proceso que está en uso del procesador (ejecución) debe salir de este y esperar a que se lleve a cabo la solicitud realizada para luego poder continuar con su ejecución (planificador a corto plazo). Para este programa, si se presiona “I”, el proceso en ejecución saldrá del procesador y se irá a la cola de Bloqueados, permaneciendo allí un tiempo de 8. Al terminar este tiempo, el proceso pasará a la cola de listos a esperar su turno para usar el procesador.

**Tecla E: Error**

El proceso que se esté ejecutando en ese momento terminará por error, es decir, saldrá del procesador y se mostrará en terminados. Para este caso, como el proceso no terminó normalmente, se desplegará "error" en lugar de un resultado. (Recuerda que al terminar un proceso queda un espacio en memoria que puede ser ocupado al admitir un proceso nuevo).

**Tecla P: Pausa**

Detiene la ejecución de su programa momentáneamente. La simulación se reanuda cuando se presione la tecla “C”.

**Tecla C: Continuar**

Al presionar esta tecla, se reanudará el programa pausado previamente con “P”.

**Tecla N: Nuevo**

Al presionar esta tecla, se generará un nuevo proceso, creando con ello los datos necesarios de forma aleatoria. El planificador a largo plazo es el que definirá su ingreso al sistema (recuerda el máximo de procesos en memoria).

**Tecla B: Tabla de procesos (BCP de cada proceso)**

Al presionar esta tecla, el programa se pausará y se deberá visualizar la tabla de procesos, es decir, los BCP de cada uno de los procesos. Con la tecla “C” continúa la simulación de su programa en el punto donde quedó.

**Tecla T: Tabla de Páginas**

Al presionar esta tecla, se detendrá la ejecución y se mostrará la tabla de páginas de cada proceso, además de los marcos libres. Continuará al presionar la tecla “C”.

**Tecla S: Suspendido**

Al presionar esta tecla, el primero en la cola de bloqueados saldrá de memoria principal e irá a estado suspendido, es decir, a disco. Debe generar un archivo con los datos de los procesos suspendidos. Si no hay procesos bloqueados, no aplica.

**Tecla R: Regresa**

Si presiona esta tecla, el primero en la cola de suspendidos regresará a memoria principal siempre y cuando haya espacio. Si no hay procesos suspendidos, no aplica.
