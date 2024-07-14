#include <iostream>
#include <windows.h>
#include <regex>
#include <cmath>
#include <sstream>
#include <ctime>
#include <conio.h>
#include <vector>
#include <fstream>

#define PROCESOS_MINIMOS 0
#define DELIMITADOR_CAMPOS "|"

//Comprobar que sistema operativo se tiene
#ifdef _WIN32
    #define CLEAR "cls" //Se le otorga cls a la constante si es windows
#elif defined(unix)
    #define CLEAR "clear" //Se le otorga clear la constante si es linuxs
#else
    #error "S0 no soportado para limpiar pantalla"
#endif

using namespace std;

struct Proceso{
    string operacion;
    int ID;
    float resultado;
    short estado=0;
    //Tiempos calculados
    int tiempoLlegada=0; //Se le asigna el contador global cuando entra al sistema
    int tiempoFinalizacion=0; //Se le asigna el contador global cuando finaliza


    int tiempoRetorno=0; //Es el tiempo que estuvo en el programa(tiempo transcurrido-tiempo de espera)
//Es el tiempo que espera desde que empieza hasta que entra al sistema por primera vez(incrementar cada segundo hasta que entre en proceso)
    int tiempoRespuesta=0;
    int tiempoEspera=0; //Tiempo retorno - tiempo servicio
    int tiempoServicio=0; //El tiempo que trabajo(tiempo transcurrido o  tiempo transcurrido-tiempo de espera)

    int tiempoMaximoEstimado=0;
    int tiempoTranscurrido=0;
    int tiempoBloqueado=0;

    bool entradaPrimeraVez=false;
    bool llegada=false;
    int longitud;
    int longitudTotal;
};

struct TablaMemoria{
    Proceso procesosTabla;
    int pagina;
    int id;
    string estado;
    int posicionX;
    int posicionY;
    bool enUso=false;
    short cantidadPaginas;
};

void limpiar();
void pausa();
void pausaSinBuffer();
void gotoxy(int x, int y);
int solicitarCantidadProcesos();
void obtenerDatosProceso(int cantidadProcesos, int cantidadLotes);
string generarOperador();
bool validarOperacion(string operacion);
bool validarId(int id);
bool verificarMemoria(int cantidadPaginas, int &contadorPagina);
void guardarDatos(string operacionRealizar, int numeroPrograma, int tiempoMaximoEstimado, bool esNuevo);
float realizarOperacion(string operacion);

void llenarTablaMemoria();
void mostrarTablas();
void multiprogramacion(char tecla);
void interrupcionTecla();
void errorTecla();
void pausaConTecla();
void nuevoProceso();

void imprimirTablaEspera();
void imprimirTablaPendientes();
void imprimirTablaEjecucion();
void imprimirTablaBloqueados();
void imprimirTablaTerminados();
void imprimirTablaTiemposActuales();
void llamarTablas();
void calcularTiempos();
void imprimirTablaTiempos();
void ponerProcesosMemoria();
void calcularTiemposActuales();

void imprimirTablaMemoria();

void LimpiarBufferTeclado();

bool VerificarTeclado(char& tecla, int ms);

char ProcesarTeclado();

void limpiarPaginasMemoria(Proceso procesoTerminado);

void cambiarEstadoTabla(int id, string estado);

void agregarRegistro(Proceso &procesoSuspendido);
void recibirRegistro();
void guardarProcesoArchivo();
bool agregarNuevoProceso(Proceso procesoTemporal, bool aBloqueados);
void tablaProcesoSuspendido();

int procesosGuardados=0, tiempoGlobalTranscurrido=0, quantum, contadorQuantumProceso;
int contadorId=1, cantidadProcesosTablaMemoria=0;
bool vacio=false, enEjecucion=false;

vector <Proceso> procesos;
vector <Proceso> procesosSuspendidos;
vector <Proceso> procesosBloqueados;
vector <Proceso> procesosTerminados;
vector <Proceso> procesosMemoria;

vector <TablaMemoria> tablaMemoria;
Proceso procesoEjecucion;

void agregarRegistro(Proceso &procesoSuspendido) {
    ofstream archivoSalida("archivo.txt", ios::app); // Abre en modo append
    int cantidadProcesosSuspendidos;
    if (archivoSalida.is_open()) {
        // Verifica si el archivo ya tiene contenido
        ifstream archivoEntrada("archivo.txt");
        bool archivoVacio = archivoEntrada.peek() == ifstream::traits_type::eof();
        archivoEntrada.close();

        if (!archivoVacio) {
            archivoSalida << "\n"; // Agrega un salto de línea si el archivo no está vacío
        }

        archivoSalida << procesoSuspendido.ID                   << DELIMITADOR_CAMPOS;
        archivoSalida << procesoSuspendido.operacion            << DELIMITADOR_CAMPOS;
        archivoSalida << procesoSuspendido.resultado            << DELIMITADOR_CAMPOS;
        archivoSalida << procesoSuspendido.tiempoLlegada        << DELIMITADOR_CAMPOS;
        archivoSalida << procesoSuspendido.tiempoFinalizacion   << DELIMITADOR_CAMPOS;
        archivoSalida << procesoSuspendido.tiempoRetorno        << DELIMITADOR_CAMPOS;
        archivoSalida << procesoSuspendido.tiempoRespuesta      << DELIMITADOR_CAMPOS;
        archivoSalida << procesoSuspendido.tiempoEspera         << DELIMITADOR_CAMPOS;
        archivoSalida << procesoSuspendido.tiempoServicio       << DELIMITADOR_CAMPOS;
        archivoSalida << procesoSuspendido.tiempoMaximoEstimado << DELIMITADOR_CAMPOS;
        archivoSalida << procesoSuspendido.tiempoTranscurrido   << DELIMITADOR_CAMPOS;
        //archivoSalida << 0     << DELIMITADOR_CAMPOS;
        archivoSalida << procesoSuspendido.tiempoBloqueado      << DELIMITADOR_CAMPOS;
        archivoSalida << procesoSuspendido.entradaPrimeraVez    << DELIMITADOR_CAMPOS;
        archivoSalida << procesoSuspendido.llegada              << DELIMITADOR_CAMPOS;
        archivoSalida << procesoSuspendido.longitudTotal;
        archivoSalida.close();
        cout << "Registro agregado exitosamente" << endl;
        //pausaSinBuffer();
        Sleep(1000);
        procesosSuspendidos.push_back(procesoSuspendido);
        cantidadProcesosTablaMemoria--;
        procesosGuardados--;
    }
}

void recibirRegistro(){
    ifstream archivoEntrada("archivo.txt");
    ofstream archivoTemporal("temporal.txt");
    string cadena, subCadena;
    int posInicio, posFinal;
    bool aceptadoEnMemoria=false;
    bool archivoVacio;
    bool archivoEntradaVacio;
    if (archivoEntrada.is_open()) {
        Proceso procesoSuspendido;
        archivoEntradaVacio = archivoEntrada.peek() == ifstream::traits_type::eof();
        if(!archivoEntradaVacio){
            getline(archivoEntrada, cadena, '\n');
            posInicio=0;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal);
            procesoSuspendido.ID=stoi(subCadena);

            posInicio=posFinal+1;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal-posInicio);
            procesoSuspendido.operacion=subCadena;

            posInicio=posFinal+1;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal-posInicio);
            procesoSuspendido.resultado=stof(subCadena);

            posInicio=posFinal+1;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal-posInicio);
            procesoSuspendido.tiempoLlegada=stoi(subCadena);

            posInicio=posFinal+1;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal-posInicio);
            procesoSuspendido.tiempoFinalizacion=stoi(subCadena);

            posInicio=posFinal+1;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal-posInicio);
            procesoSuspendido.tiempoRetorno=stoi(subCadena);

            posInicio=posFinal+1;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal-posInicio);
            procesoSuspendido.tiempoRespuesta=stoi(subCadena);

            posInicio=posFinal+1;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal-posInicio);
            procesoSuspendido.tiempoEspera=stoi(subCadena);

            posInicio=posFinal+1;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal-posInicio);
            procesoSuspendido.tiempoServicio=stoi(subCadena);

            posInicio=posFinal+1;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal-posInicio);
            procesoSuspendido.tiempoMaximoEstimado=stoi(subCadena);

            posInicio=posFinal+1;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal-posInicio);
            procesoSuspendido.tiempoTranscurrido=stoi(subCadena);

            posInicio=posFinal+1;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal-posInicio);
            procesoSuspendido.tiempoBloqueado=stoi(subCadena);

            posInicio=posFinal+1;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal-posInicio);
            procesoSuspendido.entradaPrimeraVez=stoi(subCadena);

            posInicio=posFinal+1;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal-posInicio);
            procesoSuspendido.llegada=stoi(subCadena);

            posInicio=posFinal+1;
            posFinal=cadena.find_first_of(DELIMITADOR_CAMPOS, posInicio);
            subCadena=cadena.substr(posInicio, posFinal-posInicio);
            procesoSuspendido.longitud=stoi(subCadena);

            if(!agregarNuevoProceso(procesoSuspendido, true)){
                cout << "No hay espacio en memoria" << endl;
                //pausaSinBuffer();
                Sleep(1000);
            }else{
                cambiarEstadoTabla(procesoSuspendido.ID, "Bloq.");
                aceptadoEnMemoria=true;
                procesosSuspendidos.erase(procesosSuspendidos.begin());
                cantidadProcesosTablaMemoria++;
                procesosGuardados++;
            }

            // Verifica si el archivo ya tiene contenido
            while(!archivoEntrada.eof()){
                getline(archivoEntrada, cadena, '\n');
                archivoVacio = archivoTemporal.tellp() == 0;
                if (!archivoVacio) {
                    archivoTemporal << "\n"; // Agrega un salto de línea si el archivo no está vacío
                }
                archivoTemporal << cadena;
            }
        }
        archivoEntrada.close();
        archivoTemporal.close();
    }
    if(aceptadoEnMemoria){
        // Elimina el archivo original
        remove("archivo.txt");
        // Renombra el archivo temporal al nombre del archivo original
        rename("temporal.txt", "archivo.txt");
    } else {
        // No se realizó ninguna eliminación, elimina el archivo temporal
        remove("temporal.txt");
    }
}

void guardarProcesoArchivo(){
    if(procesosBloqueados.size()!=0){
        limpiarPaginasMemoria(procesosBloqueados[0]);
        agregarRegistro(procesosBloqueados[0]);
        procesosBloqueados.erase(procesosBloqueados.begin());
        if(procesos.size()!=0){
            agregarNuevoProceso(procesos[0], false);
        }
    }else{
        cout << "No hay procesos disponibles para suspender" << endl;
        //pausaSinBuffer();
        Sleep(1000);
    }
}

int main(){
    int cantidadProcesos,  cantidadLotes;
    //recibirRegistro();
    remove("archivo.txt");
    remove("temporal.txt");
    //pausaSinBuffer();
    llenarTablaMemoria();
    cantidadProcesos=solicitarCantidadProcesos();
    limpiar();
    obtenerDatosProceso(cantidadProcesos, cantidadLotes);
    mostrarTablas();
    pausaSinBuffer();
    return 0;
}

void llenarTablaMemoria(){
    TablaMemoria tablaMemoriaTemporal;
    int posicionX=2, posicionY=24;
    for(int i=0; i<40; i++){
        if(i%11 == 0 && i!=0){
            posicionX=2;
            posicionY+=5;
        }
        posicionX+=9;
        tablaMemoriaTemporal.id=-1;
        tablaMemoriaTemporal.pagina=-1;
        tablaMemoriaTemporal.procesosTabla.longitud=0;
        tablaMemoriaTemporal.estado="";
        tablaMemoriaTemporal.posicionX=posicionX;
        tablaMemoriaTemporal.posicionY=posicionY;
        tablaMemoria.push_back(tablaMemoriaTemporal);
    }
}

void limpiar(){
    system(CLEAR);
}

void pausa(){ //Funcion para pausar si se tiene buffer guardado
    cin.get(); //Si tiene buffer lo limpia
    cout << "Presione enter para continuar...";
    cin.get(); //Detiene un programa a la espera de un enter
}

void pausaSinBuffer(){ //Funcion para pausar si no tiene buffer guardado
    cout << "Presione enter para continuar . . .";
    cin.get(); //Detiene un programa a la espera de un enter
}

void gotoxy(int x, int y){ //Función para colocar el cursor en ciertas coordenadas
    HANDLE hcon;
    hcon= GetStdHandle(STD_OUTPUT_HANDLE);
    COORD dwPos;
    dwPos.X = x;
    dwPos.Y = y;
    SetConsoleCursorPosition(hcon, dwPos);
}

int solicitarCantidadProcesos(){ //Funcion para recibir la cantidad de procesos
    int cantidadProcesos;
    do{//Se repite mientras el valor sea menor igual que cero
        cout << "Cuantos procesos quieres ejecutar?-> ";
        if(cin >> cantidadProcesos){
            //Entrada con numero entero
            if(cantidadProcesos <= PROCESOS_MINIMOS){
                cout << "Cantidad de procesos debe ser mayor que 0" << endl;
                pausa();
                limpiar();
            }
        }else{
            //Entrada con otros caracteres
            cout << "La entrada debe ser un numero entero" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            pausaSinBuffer();
            limpiar();
        }
    }while(cantidadProcesos <= PROCESOS_MINIMOS); //si es menor o igual que 0 se repite
    cin.get();
    do{//Se repite mientras el valor sea menor igual que cero
        cout << "Ingresa el tiempo del quantum?-> ";
        if(cin >> quantum){
            //Entrada con numero entero
            if(quantum <= 0){
                cout << "El tiempo del quantum debe ser mayor a 0" << endl;
                pausa();
                limpiar();
            }
        }else{
            //Entrada con otros caracteres
            cout << "La entrada debe ser un numero entero" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            pausaSinBuffer();
            limpiar();
        }
    }while(quantum <= 0); //si es menor o igual que 0 se repite
    cin.get();
    return cantidadProcesos;
}

void obtenerDatosProceso(int cantidadProcesos, int cantidadLotes){
    string operacionRealizar;
    int numeroPrograma, tiempoMaximoEstimado;
    srand(time(NULL));
    do{
        //Generar Operación a realizar
        do{
            operacionRealizar=to_string(rand()%101) + generarOperador() + to_string(1 + rand()% (101-1));
        }while(!validarOperacion(operacionRealizar)); //Se repite si la operacion no es valida

        //Generar tiempo estimado
        do{
            tiempoMaximoEstimado=6+rand()%(18-6);
            //tiempoMaximoEstimado=3+rand()%(6-3);
        }while(tiempoMaximoEstimado<=0);

        //Generar id automatico
        do{
            numeroPrograma=contadorId;
            contadorId++;
        }while(!validarId(numeroPrograma)); //Validar que el id no esté repetido
        guardarDatos(operacionRealizar, numeroPrograma, tiempoMaximoEstimado, false);
        cantidadProcesos--;
    }while(cantidadProcesos!=PROCESOS_MINIMOS);
    //limpiar();
}

string generarOperador(){
    int random;
    string operador;
    //srand(time(NULL));
    random=1+rand()%(7-1);
    switch(random){
        case 1:     operador="+";                       break;
        case 2:     operador="-";                       break;
        case 3:     operador="*";                       break;
        case 4:     operador="/";                       break;
        case 5:     operador="%";                       break;
        case 6:     operador="p";                       break;
        default:    cout << "Numero no valido" << endl; break;
    }
    return operador;
}

bool validarOperacion(string operacion){
    float primerNumero, segundoNumero, resultado;
    char operador;
    float tolerancia = 1e-9;
    istringstream iss(operacion);
    if ( !(iss >> primerNumero) || !(iss >> operador) || !(iss >> segundoNumero)) {
        cout << "La operacion no es valida" << endl;
    }else if ((fabs(segundoNumero) < tolerancia) && (operador == '/' || operador == '%')) {
        cout << "En esta operacion no es valido el 0 como segundo numero" << endl;
    }else if(operador=='P' || operador=='p'){
        return true;
    }else{
        // Patrón de expresión regular para validar una operación matemática simple
        regex patron("-?[0-9]+\\s*[-+*/%]\\s*-?[0-9]+");
        if(regex_match(operacion, patron)){
            return true;
        }else{
            cout << "La operacion no es valida" << endl;

        }
    }
    pausaSinBuffer();
    limpiar();
    return false;
}

bool validarId(int id){
    if(procesosGuardados!=0){
        for(int i=0; i<procesosGuardados; i++){
            if(id==procesos[i].ID){
                cout << "ID repetido, ingreso otro" << endl;
                pausa();
                limpiar();
                return false;
            }
        }
    }

    if(id<=0){
        return false;
    }
    return true;
}

bool verificarMemoria(int cantidadPaginas, int &contadorPagina){
    int contadorPaginasLibres=0;
    for(int i=0; i<tablaMemoria.size(); i++){
        if(!tablaMemoria[i].enUso){
            contadorPaginasLibres++;
        }
        if(contadorPaginasLibres==cantidadPaginas){
            contadorPagina=0;
            return true;
        }
    }
    return false;
}

//Funciona para guardar los datos en el arreglo de lotes
void guardarDatos(string operacionRealizar, int numeroPrograma, int tiempoMaximoEstimado, bool esNuevo){
    Proceso procesoTemporal;
    TablaMemoria tablaMemoriaTemporal;
    int longitudProceso, contadorPagina=0, cantidadPaginasNecesarias=0, longitudTemporal=0;

    longitudProceso=6+rand()%(26-6);
    procesoTemporal.operacion=operacionRealizar;
    procesoTemporal.ID=numeroPrograma;
    procesoTemporal.tiempoMaximoEstimado=tiempoMaximoEstimado;
    procesoTemporal.resultado=realizarOperacion(operacionRealizar);
    procesoTemporal.longitud=longitudProceso;
    procesoTemporal.longitudTotal=longitudProceso;
    procesos.push_back(procesoTemporal);
    procesosGuardados++;
    if(esNuevo){
        procesoTemporal=procesos[0];
        longitudProceso=procesoTemporal.longitud;
    }
    longitudTemporal=longitudProceso;
    while(longitudTemporal>0){
        cantidadPaginasNecesarias++;
        longitudTemporal-=5;
    }
    /*cout << longitudProceso << ", " << cantidadPaginasNecesarias << endl;
    pausa();*/
    if(verificarMemoria(cantidadPaginasNecesarias, contadorPagina)){
        do{
            if(!tablaMemoria[contadorPagina].enUso){
                tablaMemoria[contadorPagina].id=numeroPrograma;
                tablaMemoria[contadorPagina].pagina=contadorPagina+1;
                tablaMemoria[contadorPagina].estado="Listo";
                tablaMemoria[contadorPagina].enUso=true;
                tablaMemoria[contadorPagina].procesosTabla=procesoTemporal;
                tablaMemoria[contadorPagina].cantidadPaginas=cantidadPaginasNecesarias;
                if(longitudProceso>5){
                    tablaMemoria[contadorPagina].procesosTabla.longitud=5;
                    longitudProceso-=5;
                    contadorPagina++;
                }else{
                    tablaMemoria[contadorPagina].procesosTabla.longitud=longitudProceso;
                    contadorPagina++;
                    cantidadProcesosTablaMemoria++;
                    break;
                }
            }else{
                contadorPagina++;
            }
        }while(true);
    }else{

    }
}

//Funcion para realiazar la operacion recibida
float realizarOperacion(string operacion){
    float primerNumero, segundoNumero, resultado;
    char operador;
    istringstream iss(operacion);
    iss >> primerNumero >> operador >> segundoNumero; //Separar los numeros y el operador
    if(operador == '+'){
        resultado = primerNumero + segundoNumero;
    }else if(operador == '-'){
        resultado = primerNumero - segundoNumero;
    }else if(operador == '*'){
        resultado = primerNumero * segundoNumero;
    }else if(operador == '/'){
        resultado = primerNumero / segundoNumero;
    }else if(operador == '%'){
        resultado = int(primerNumero) % int(segundoNumero);
    }else if(operador == 'p' || operador == 'P'){
        resultado = (primerNumero * segundoNumero) / 100;
    }else{
        cout << "Operador no valido" << endl;
    }
    return resultado;
}

void LimpiarBufferTeclado() {
    while (_kbhit()) {
        _getch();
    }
}

bool VerificarTeclado(char& tecla, int ms) {
    if (_kbhit()) {
        tecla = _getch();
        return true;
    }

    // Verificar si ha pasado el tiempo especificado
    static clock_t lastCheck = clock();
    clock_t currentTime = clock();

    if (static_cast<double>(currentTime - lastCheck) / CLOCKS_PER_SEC * 1000 >= ms) {
        lastCheck = currentTime;
        return false;
    }

    return false;
}

char ProcesarTeclado() {
    char tecla;
    if (VerificarTeclado(tecla, 100)) {
        multiprogramacion(tecla);
        LimpiarBufferTeclado();
    }
    return tecla;
}

void ponerProcesosMemoria(){
    // Establece el tiempo de llegada y marca como llegados
    tablaMemoria[0].procesosTabla.tiempoLlegada = tiempoGlobalTranscurrido;
    tablaMemoria[0].procesosTabla.llegada = true;
    //procesos[0].tiempoLlegada = tiempoGlobalTranscurrido;
    //procesos[0].llegada = true;

    for(int i=0; tablaMemoria[i].enUso && i<tablaMemoria.size(); i++){
        procesosMemoria.push_back(tablaMemoria[i].procesosTabla);
        i=i+tablaMemoria[i].cantidadPaginas-1;
        procesos.erase(procesos.begin(), procesos.begin() + 1);
    }
}

void limpiarPaginasMemoria(Proceso procesoTerminado){
    for(int i=0; i<tablaMemoria.size(); i++){
        if(tablaMemoria[i].id==procesoTerminado.ID){
            tablaMemoria[i].procesosTabla.longitud=0;
            tablaMemoria[i].estado="";
            tablaMemoria[i].id=-1;
            tablaMemoria[i].enUso=false;

        }
    }
}

bool agregarNuevoProceso(Proceso procesoTemporal, bool aBloqueados){
    int longitudTemporal, cantidadPaginasNecesarias=0, contadorPagina=0;
    int longitudProceso=procesoTemporal.longitud;
    procesoTemporal.longitudTotal=procesoTemporal.longitud;
    longitudTemporal=longitudProceso;
    while(longitudTemporal>0){
        cantidadPaginasNecesarias++;
        longitudTemporal-=5;
    }
    if(verificarMemoria(cantidadPaginasNecesarias, contadorPagina)){
        do{
            if(!tablaMemoria[contadorPagina].enUso){
                tablaMemoria[contadorPagina].id=procesoTemporal.ID;
                tablaMemoria[contadorPagina].pagina=contadorPagina+1;
                tablaMemoria[contadorPagina].estado="Listo";
                tablaMemoria[contadorPagina].enUso=true;
                tablaMemoria[contadorPagina].procesosTabla=procesoTemporal;
                tablaMemoria[contadorPagina].cantidadPaginas=cantidadPaginasNecesarias;
                if(longitudProceso>5){
                    tablaMemoria[contadorPagina].procesosTabla.longitud=5;
                    longitudProceso-=5;
                    contadorPagina++;
                }else{
                    tablaMemoria[contadorPagina].procesosTabla.longitud=longitudProceso;
                    contadorPagina++;
                    cantidadProcesosTablaMemoria++;
                    if(aBloqueados){
                        procesosBloqueados.push_back(procesoTemporal);
                    }else{
                        procesosMemoria.push_back(procesoTemporal);
                        procesos.erase(procesos.begin());
                    }
                    return true;
                }
            }else{
                contadorPagina++;
            }
        }while(true);
    }else{
        return false;
    }
}

void cambiarEstadoTabla(int id, string estado){
    for(int i=0; i<tablaMemoria.size(); i++){
        if(tablaMemoria[i].id==id){
            tablaMemoria[i].estado=estado;
        }
    }
}

void mostrarTablas(){
    int posicionProceso=0;
    bool interrupcion, error, interrupcionQuantum;
    int tiempoTranscurrido;
    char tecla;
    ponerProcesosMemoria();
    do{
        interrupcion=false;
        error=false;
        interrupcionQuantum=false;
        contadorQuantumProceso=0;
        if(procesosMemoria.size()!=0){
            vacio=false;
            procesoEjecucion=procesosMemoria[posicionProceso];
            cambiarEstadoTabla(procesoEjecucion.ID, "Ejec.");
            procesosMemoria.erase(procesosMemoria.begin());
            tiempoTranscurrido=procesoEjecucion.tiempoTranscurrido;
            for(int i=tiempoTranscurrido; i<procesoEjecucion.tiempoMaximoEstimado && !vacio; i++){
                if(contadorQuantumProceso==quantum){
                    cambiarEstadoTabla(procesoEjecucion.ID, "Listo");
                    procesosMemoria.push_back(procesoEjecucion);
                    interrupcionQuantum=true;
                    break;
                }
                if(kbhit()){
                    tecla=ProcesarTeclado();
                    if(tecla=='i'){
                        interrupcion=true;
                        break;
                    }else if(tecla=='e'){
                        error=true;
                        break;
                    }
                }
                llamarTablas();
                contadorQuantumProceso++;
                if(procesoEjecucion.tiempoMaximoEstimado==procesoEjecucion.tiempoTranscurrido){
                    break;
                }
            }
            if(!interrupcion && !vacio && !error && !interrupcionQuantum){
                if(procesoEjecucion.estado!=3 && procesoEjecucion.estado!=4){
                    procesoEjecucion.estado=2;
                    procesoEjecucion.tiempoFinalizacion=tiempoGlobalTranscurrido;
                    procesosTerminados.push_back(procesoEjecucion);
                    limpiarPaginasMemoria(procesoEjecucion);
                    cantidadProcesosTablaMemoria--;
                    while(procesos.size()!=0){
                        if(!agregarNuevoProceso(procesos[0], false)){
                            break;
                        }
                    }
                }
            }
            LimpiarBufferTeclado();
            if(vacio){
                llamarTablas();
            }
        }else{
            vacio=true;
            if(kbhit()){
                if (VerificarTeclado(tecla, 100)) {
                    switch(tecla){
                        case 'p':   pausaConTecla();                break;
                        case 't':   pausaConTecla();                break;
                        case 'n':   nuevoProceso();                 break;
                        case 'b':   imprimirTablaTiemposActuales(); break;
                        case 'r':   recibirRegistro();              break;
                        case 's':   guardarProcesoArchivo();        break;
                        default:                                    break;
                    }
                    LimpiarBufferTeclado();
                }
            }
            llamarTablas();
        }
        if(procesosTerminados.size()==procesosGuardados){
            vacio=true;
            llamarTablas();
            if(procesosSuspendidos.size()==0){
                break;
            }
        }
    }while(true);
    pausaSinBuffer();
    limpiar();
    calcularTiempos();
    imprimirTablaTiempos();
}

void calcularTiempos(){
    //Tiempo llegada Listo
    //Tiempo Finalizacion Listo
    for(int i=0; i<procesosTerminados.size(); i++){
        //Tiempo retorno
        procesosTerminados[i].tiempoRetorno=procesosTerminados[i].tiempoFinalizacion-procesosTerminados[i].tiempoLlegada;

        //Tiempo tiempo servicio
        procesosTerminados[i].tiempoServicio=procesosTerminados[i].tiempoTranscurrido;

        //Tiempo espera
        procesosTerminados[i].tiempoEspera=procesosTerminados[i].tiempoRetorno-procesosTerminados[i].tiempoServicio;
    }
}

void calcularTiemposActuales(){
    //Tiempo llegada Listo
    //Tiempo Finalizacion Listo
    calcularTiempos();
    for(int i=0; i<procesosMemoria.size(); i++){
        //Tiempo tiempo servicio
        procesosMemoria[i].tiempoServicio=procesosMemoria[i].tiempoTranscurrido;

        //Tiempo espera
        procesosMemoria[i].tiempoEspera=tiempoGlobalTranscurrido-procesosMemoria[i].tiempoLlegada-procesosMemoria[i].tiempoTranscurrido;

        //Tiempo retorno
        procesosMemoria[i].tiempoRetorno=procesosMemoria[i].tiempoEspera+procesosMemoria[i].tiempoServicio;
    }
    for(int i=0; i<procesosBloqueados.size(); i++){
        //Tiempo tiempo servicio
        procesosBloqueados[i].tiempoServicio=procesosBloqueados[i].tiempoTranscurrido;

        //Tiempo espera
        procesosBloqueados[i].tiempoEspera=tiempoGlobalTranscurrido-procesosBloqueados[i].tiempoLlegada-procesosBloqueados[i].tiempoTranscurrido;

        //Tiempo retorno
        procesosBloqueados[i].tiempoRetorno=procesosBloqueados[i].tiempoEspera+procesosBloqueados[i].tiempoServicio;
    }
    for(int i=0; i<procesosSuspendidos.size(); i++){
        //Tiempo tiempo servicio
        procesosSuspendidos[i].tiempoServicio=procesosSuspendidos[i].tiempoTranscurrido;

        //Tiempo espera
        procesosSuspendidos[i].tiempoEspera=tiempoGlobalTranscurrido-procesosSuspendidos[i].tiempoLlegada-procesosSuspendidos[i].tiempoTranscurrido;

        //Tiempo retorno
        procesosSuspendidos[i].tiempoRetorno=procesosSuspendidos[i].tiempoEspera+procesosSuspendidos[i].tiempoServicio;
    }
    if(enEjecucion){
        //Tiempo tiempo servicio
        procesoEjecucion.tiempoServicio=procesoEjecucion.tiempoTranscurrido;

        //Tiempo espera
        procesoEjecucion.tiempoEspera=tiempoGlobalTranscurrido-procesoEjecucion.tiempoLlegada-procesoEjecucion.tiempoTranscurrido;

        //Tiempo retorno
        procesoEjecucion.tiempoRetorno=procesoEjecucion.tiempoEspera+procesoEjecucion.tiempoServicio;
    }
}

void multiprogramacion(char tecla){
    switch(tecla){
        case 'i':   interrupcionTecla();            break;
        case 'e':   errorTecla();                   break;
        case 'p':   pausaConTecla();                break;
        case 't':   pausaConTecla();                break;
        case 'n':   nuevoProceso();                 break;
        case 'b':   imprimirTablaTiemposActuales(); break;
        case 's':   guardarProcesoArchivo();        break;
        case 'r':   recibirRegistro();              break;
        default:                                    break;
    }
}

void interrupcionTecla(){
    cambiarEstadoTabla(procesoEjecucion.ID, "Bloc.");
    procesosBloqueados.push_back(procesoEjecucion);
    Sleep(500);
}

void errorTecla(){
    procesoEjecucion.estado=3;
    procesoEjecucion.tiempoFinalizacion=tiempoGlobalTranscurrido;
    procesosTerminados.push_back(procesoEjecucion);
    limpiarPaginasMemoria(procesoEjecucion);
    cantidadProcesosTablaMemoria--;
    while(procesos.size()!=0){
        if(!agregarNuevoProceso(procesos[0], false)){
            break;
        }
    }
}

void pausaConTecla(){
    char tecla;
    //Bucle que se repite mientras no sea c
    while(tecla!='c'){
        if(kbhit()){
            tecla= getch();
        }else {
            tecla = ' ';
        }
    }
}

void nuevoProceso(){
    string operacionRealizar;
    int numeroPrograma, tiempoMaximoEstimado, totalProcesosMemoria;
    srand(time(NULL));
    //Generar Operación a realizar
    do{
        operacionRealizar=to_string(rand()%101) + generarOperador() + to_string(1 + rand()% (101-1));
    }while(!validarOperacion(operacionRealizar)); //Se repite si la operacion no es valida

    //Generar tiempo estimado
    do{
        tiempoMaximoEstimado=6+rand()%(18-6);
        //tiempoMaximoEstimado=3+rand()%(6-3);
    }while(tiempoMaximoEstimado<=0);

    //Generar id automatico
    do{
        numeroPrograma=contadorId;
        contadorId++;
    }while(!validarId(numeroPrograma)); //Validar que el id no esté repetido
    guardarDatos(operacionRealizar, numeroPrograma, tiempoMaximoEstimado, true);
    if(enEjecucion){
        totalProcesosMemoria=procesosMemoria.size()+procesosBloqueados.size()+1;
    }else{
        totalProcesosMemoria=procesosMemoria.size()+procesosBloqueados.size();
    }
    if(totalProcesosMemoria < cantidadProcesosTablaMemoria){
    //if (totalProcesosMemoria < 5) {
        // Mover el nuevo registro a procesosMemoria
        procesosMemoria.push_back(procesos.back());  // Asumiendo que el nuevo registro está al final del vector procesos
        procesos.pop_back();  // Eliminar el nuevo registro del vector procesos
        totalProcesosMemoria++;  // Incrementar el contador de procesos en memoria
        /*cout << "a" << endl;
        pausaSinBuffer();*/
    }
}

void llamarTablas(){
    int posicionY, tiempoMaximoEstimado;
    int terminados=procesosTerminados.size();
    limpiar();
    if((procesosGuardados-terminados-cantidadProcesosTablaMemoria) > 0){
        cout << "Procesos nuevos: " << procesosGuardados-terminados-cantidadProcesosTablaMemoria << endl;
    }else{
        cout << "Procesos nuevos: " << 0 << endl;
    }
    cout << "Quantum: " << quantum << endl;
    procesoEjecucion.tiempoTranscurrido++;
    imprimirTablaEspera();
    //imprimirTablaPendientes();
    imprimirTablaEjecucion();
    imprimirTablaBloqueados();
    imprimirTablaTerminados();
    imprimirTablaMemoria();
    tablaProcesoSuspendido();
    posicionY = 45;
    gotoxy(0, posicionY);
    cout << "Contador global de tiempo transcurrido: " << tiempoGlobalTranscurrido;
    posicionY++;
    gotoxy(0, posicionY);
    Sleep(1100);
    tiempoGlobalTranscurrido++;
}

void imprimirTablaEspera(){
    int posicionY=3, posicionX=0;
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "Total procesos: " << procesos.size();
    gotoxy(posicionX,posicionY);
    posicionY++;
    cout << "|-----------------|";
    gotoxy(posicionX,posicionY);
    posicionY++;
    cout << "|      Espera     |";
    gotoxy(posicionX,posicionY);
    posicionY++;
    cout << "|-----------------|";
    gotoxy(posicionX,posicionY);
    posicionY++;
    cout << "|  ID  | Longitud |";
    if(procesos.size()!=0){
        gotoxy(posicionX, posicionY);
        posicionY++;
        cout << "|------|----------|";
        gotoxy(posicionX, posicionY); cout << "|"; gotoxy(posicionX+1, posicionY); cout << procesos[0].ID; gotoxy(posicionX+7, posicionY); cout << "|";
        gotoxy(posicionX+8, posicionY); cout << procesos[0].longitud; gotoxy(posicionX+18, posicionY); cout << "|";
        posicionY++;
    }
    gotoxy(posicionX,posicionY);
    cout << "|------|----------|";
    posicionY++;
    gotoxy(posicionX,posicionY);
}

void imprimirTablaPendientes(){
    int posicionY=3, posicionX=0;
    int j=0;
    gotoxy(posicionX,posicionY);
    posicionY++;
    cout << "|--------------------|";
    gotoxy(posicionX,posicionY);
    posicionY++;
    cout << "|     Pendientes     |";
    gotoxy(posicionX,posicionY);
    posicionY++;
    cout << "|--------------------|";
    gotoxy(posicionX,posicionY);
    posicionY++;
    cout << "|  ID  |  TM  |  TT  |";
    for(int i=0; i<procesosMemoria.size(); i++){
        gotoxy(posicionX,posicionY);
        cout << "|------|------|------|";
        posicionY++;
        gotoxy(posicionX, posicionY); cout << "|"; gotoxy(posicionX+1, posicionY); cout << procesosMemoria[i].ID; gotoxy(posicionX+7, posicionY); cout << "|";
        gotoxy(posicionX+8, posicionY); cout << procesosMemoria[i].tiempoMaximoEstimado; gotoxy(posicionX+14, posicionY); cout << "|";
        gotoxy(posicionX+15, posicionY); cout << procesosMemoria[i].tiempoTranscurrido; gotoxy(posicionX+21, posicionY); cout << "|";
        posicionY++;

        if(!procesosMemoria[i].llegada){
            procesosMemoria[i].tiempoLlegada=tiempoGlobalTranscurrido;
            procesosMemoria[i].llegada=true;
        }
    }
    gotoxy(posicionX,posicionY);
    cout << "|------|------|------|";
    posicionY++;
    gotoxy(posicionX,posicionY);
}

void imprimirTablaEjecucion(){
    int posicionX=23, posicionY=0, tiempoTranscurrido;
    tiempoTranscurrido=procesoEjecucion.tiempoTranscurrido;
    if(vacio){
        enEjecucion=false;
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|---------------------------|";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|   Procesos en ejecucion   |";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|---------------------------|";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|ID           |             |";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|-------------|-------------|";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|Operacion    |             |";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|-------------|-------------|";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|TM estimado  |             |";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|-------------|-------------|";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|TT           |             |";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|-------------|-------------|";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|TR           |             |";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|-------------|-------------|";
    }else{
        enEjecucion=true;
        if(!procesoEjecucion.entradaPrimeraVez){
            procesoEjecucion.tiempoRespuesta=tiempoGlobalTranscurrido-procesoEjecucion.tiempoLlegada;
            procesoEjecucion.entradaPrimeraVez=true;
        }
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|---------------------------|";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|   Procesos en ejecucion   |";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|---------------------------|";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|ID           |             |"; gotoxy(posicionX+15, posicionY-1); cout << procesoEjecucion.ID;
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|-------------|-------------|";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|Operacion    |             |"; gotoxy(posicionX+15, posicionY-1); cout << procesoEjecucion.operacion;
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|-------------|-------------|";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|TM estimado  |             |"; gotoxy(posicionX+15, posicionY-1); cout << procesoEjecucion.tiempoMaximoEstimado;
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|-------------|-------------|";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|TT           |             |"; gotoxy(posicionX+15, posicionY-1); cout << tiempoTranscurrido;
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|-------------|-------------|";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|TR           |             |"; gotoxy(posicionX+15, posicionY-1); cout << procesoEjecucion.tiempoMaximoEstimado-tiempoTranscurrido;
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|-------------|-------------|";
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|TT Quantum   |             |"; gotoxy(posicionX+15, posicionY-1); cout << quantum-contadorQuantumProceso;
        gotoxy(posicionX,posicionY);
        posicionY++;
        cout << "|-------------|-------------|";
    }
}

void imprimirTablaBloqueados(){
    int posicionX=53, posicionY=0;
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|---------------------|";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "| Procesos Bloqueados |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|---------------------|";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|    ID    | TT Bloq. |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|----------|----------|";
    for(int i=0; i<procesosBloqueados.size(); i++){
        gotoxy(posicionX, posicionY); cout << "|" << procesosBloqueados[i].ID;
        gotoxy(posicionX+11, posicionY); cout << "|" << procesosBloqueados[i].tiempoBloqueado+1;
        gotoxy(posicionX+22, posicionY); cout << "|";
        posicionY++;
        gotoxy(posicionX, posicionY); cout << "|----------|----------|";
        posicionY++;
        if(procesosBloqueados[i].tiempoBloqueado==7){
            procesosBloqueados[i].estado=0;
            procesosBloqueados[i].tiempoBloqueado=0;
            cambiarEstadoTabla(procesosBloqueados[0].ID, "Listo");
            procesosMemoria.push_back(procesosBloqueados[0]);
            procesosBloqueados.erase(procesosBloqueados.begin());
            i--;
            procesosBloqueados[i].tiempoBloqueado--;
        }else{
            procesosBloqueados[i].tiempoBloqueado++;
        }
    }
    posicionY++;
}

void imprimirTablaTerminados(){
    int posisionX=120, posicionY=3;
    gotoxy(posisionX,posicionY);
    posicionY++;
    cout << "|----------------------------------|";
    gotoxy(posisionX,posicionY);
    posicionY++;
    cout << "|        Procesos terminados       |";
    gotoxy(posisionX,posicionY);
    posicionY++;
    cout << "|----------------------------------|";
    gotoxy(posisionX,posicionY);
    posicionY++;
    cout << "|    ID    | Operacion | Resultado |";
    for(int j=0; j<procesosTerminados.size(); j++){
        gotoxy(posisionX,posicionY);
        cout << "|----------|-----------|-----------|";
        posicionY++;
        gotoxy(posisionX, posicionY); cout << "|"; gotoxy(posisionX+1, posicionY); cout << procesosTerminados[j].ID; gotoxy(posisionX+11, posicionY); cout << "|";
        gotoxy(posisionX+12, posicionY); cout << procesosTerminados[j].operacion; gotoxy(posisionX+23, posicionY); cout << "|";
        if(procesosTerminados[j].estado == 3){
            gotoxy(posisionX+24, posicionY); cout << "Error"; gotoxy(posisionX+35, posicionY); cout << "|";
        }else{
            gotoxy(posisionX+24, posicionY); cout << procesosTerminados[j].resultado; gotoxy(posisionX+35, posicionY); cout << "|";
        }
        posicionY++;
    }
    gotoxy(posisionX,posicionY);
    cout << "|----------|-----------|-----------|";
    posicionY++;
    gotoxy(posisionX,posicionY);
}

void imprimirTablaTiempos(){
    int posicionX=2, posicionY=1;
    gotoxy(posicionX, posicionY);
    cout << "Tabla de Tiempos de los Procesos";
    posicionY+=2;
    gotoxy(posicionX, posicionY);
    cout << "|-------------------------------------------------------------------------------------------------|";
    posicionY++;
    gotoxy(posicionX, posicionY);
    cout << "| ID | TME |Operacion|Resultado| TT |TLlegada|TFin|TServicio|TEspera|TRetorno|TRespuesta|TRestante|";
    posicionY++;
    gotoxy(posicionX, posicionY);
    cout << "|-------------------------------------------------------------------------------------------------|";
    for(int i=0; i<procesosTerminados.size(); i++){
        posicionY++;
        gotoxy(posicionX, posicionY); cout << "|" << procesosTerminados[i].ID; gotoxy(posicionX+5, posicionY); cout << "|";
        gotoxy(posicionX+6, posicionY); cout << procesosTerminados[i].tiempoMaximoEstimado; gotoxy(posicionX+11, posicionY); cout << "|";
        gotoxy(posicionX+12, posicionY); cout << procesosTerminados[i].operacion; gotoxy(posicionX+21, posicionY); cout << "|";
        if(procesosTerminados[i].estado==3){
            gotoxy(posicionX+22, posicionY); cout << "error"; gotoxy(posicionX+31, posicionY); cout << "|";
        }else{
            gotoxy(posicionX+22, posicionY); cout << procesosTerminados[i].resultado; gotoxy(posicionX+31, posicionY); cout << "|";
        }
        gotoxy(posicionX+32, posicionY); cout << procesosTerminados[i].tiempoTranscurrido; gotoxy(posicionX+36, posicionY); cout << "|";
        gotoxy(posicionX+37, posicionY); cout << procesosTerminados[i].tiempoLlegada; gotoxy(posicionX+45, posicionY); cout << "|";
        gotoxy(posicionX+46, posicionY); cout << procesosTerminados[i].tiempoFinalizacion; gotoxy(posicionX+50, posicionY); cout << "|";
        gotoxy(posicionX+51, posicionY); cout << procesosTerminados[i].tiempoServicio; gotoxy(posicionX+60, posicionY); cout << "|";
        gotoxy(posicionX+61, posicionY); cout << procesosTerminados[i].tiempoEspera; gotoxy(posicionX+68, posicionY); cout << "|";
        gotoxy(posicionX+69, posicionY); cout << procesosTerminados[i].tiempoRetorno; gotoxy(posicionX+77, posicionY); cout << "|";
        gotoxy(posicionX+78, posicionY); cout << procesosTerminados[i].tiempoRespuesta; gotoxy(posicionX+88, posicionY); cout << "|";
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|-------------------------------------------------------------------------------------------------|";
    }
    posicionY++;
    gotoxy(posicionX, posicionY);
}

void imprimirTablaTiemposActuales(){
    int posicionX=2, posicionY=1;
    int tiempoRestante;
    calcularTiemposActuales();
    limpiar();
    gotoxy(posicionX, posicionY);
    cout << "Tabla de Tiempos de los Procesos actuales";
    posicionY++;
    gotoxy(posicionX, posicionY);
    cout << "Contador Global: " << tiempoGlobalTranscurrido;
    posicionY++;
    gotoxy(posicionX, posicionY);
    cout << "|----------------------------------------------------------------------------------------------------------|";
    posicionY++;
    gotoxy(posicionX, posicionY);
    cout << "| ID | TME |Operacion|Resultado| TT |TLlegada|TFin|TServicio|TEspera|TRetorno|TRespuesta|TRestante|TBloqueo|";
    posicionY++;
    gotoxy(posicionX, posicionY);
    cout << "|----------------------------------------------------------------------------------------------------------|";
    if(procesosTerminados.size()!=0){
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|                                              TERMINADOS                                                  |";
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|----------------------------------------------------------------------------------------------------------|";
    }
    for(int i=0; i<procesosTerminados.size(); i++){
        posicionY++;
        tiempoRestante=procesosTerminados[i].tiempoMaximoEstimado-procesosTerminados[i].tiempoTranscurrido;
        gotoxy(posicionX, posicionY); cout << "|" << procesosTerminados[i].ID; gotoxy(posicionX+5, posicionY); cout << "|";
        gotoxy(posicionX+6, posicionY); cout << procesosTerminados[i].tiempoMaximoEstimado; gotoxy(posicionX+11, posicionY); cout << "|";
        gotoxy(posicionX+12, posicionY); cout << procesosTerminados[i].operacion; gotoxy(posicionX+21, posicionY); cout << "|";
        if(procesosTerminados[i].estado==3){
            gotoxy(posicionX+22, posicionY); cout << "error"; gotoxy(posicionX+31, posicionY); cout << "|";
        }else{
            gotoxy(posicionX+22, posicionY); cout << procesosTerminados[i].resultado; gotoxy(posicionX+31, posicionY); cout << "|";
        }
        gotoxy(posicionX+32, posicionY); cout << procesosTerminados[i].tiempoTranscurrido; gotoxy(posicionX+36, posicionY); cout << "|";
        gotoxy(posicionX+37, posicionY); cout << procesosTerminados[i].tiempoLlegada; gotoxy(posicionX+45, posicionY); cout << "|";
        gotoxy(posicionX+46, posicionY); cout << procesosTerminados[i].tiempoFinalizacion; gotoxy(posicionX+50, posicionY); cout << "|";
        gotoxy(posicionX+51, posicionY); cout << procesosTerminados[i].tiempoServicio; gotoxy(posicionX+60, posicionY); cout << "|";
        gotoxy(posicionX+61, posicionY); cout << procesosTerminados[i].tiempoEspera; gotoxy(posicionX+68, posicionY); cout << "|";
        gotoxy(posicionX+69, posicionY); cout << procesosTerminados[i].tiempoRetorno; gotoxy(posicionX+77, posicionY); cout << "|";
        gotoxy(posicionX+78, posicionY); cout << procesosTerminados[i].tiempoRespuesta; gotoxy(posicionX+88, posicionY); cout << "|";
        gotoxy(posicionX+89, posicionY); cout << tiempoRestante; gotoxy(posicionX+98, posicionY); cout << "|";
        gotoxy(posicionX+99, posicionY); cout << "0"; gotoxy(posicionX+107, posicionY); cout << "|";
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|----------------------------------------------------------------------------------------------------------|";
    }
    if(enEjecucion){
        tiempoRestante=procesoEjecucion.tiempoMaximoEstimado-procesoEjecucion.tiempoTranscurrido;
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|                                             EN EJECUCION                                                 |";
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|----------------------------------------------------------------------------------------------------------|";
        posicionY++;
        gotoxy(posicionX, posicionY); cout << "|" << procesoEjecucion.ID; gotoxy(posicionX+5, posicionY); cout << "|";
        gotoxy(posicionX+6, posicionY); cout << procesoEjecucion.tiempoMaximoEstimado; gotoxy(posicionX+11, posicionY); cout << "|";
        gotoxy(posicionX+12, posicionY); cout << procesoEjecucion.operacion; gotoxy(posicionX+21, posicionY); cout << "|";
        gotoxy(posicionX+22, posicionY); cout << "NA"; gotoxy(posicionX+31, posicionY); cout << "|";
        gotoxy(posicionX+32, posicionY); cout << procesoEjecucion.tiempoTranscurrido; gotoxy(posicionX+36, posicionY); cout << "|";
        gotoxy(posicionX+37, posicionY); cout << procesoEjecucion.tiempoLlegada; gotoxy(posicionX+45, posicionY); cout << "|";
        gotoxy(posicionX+46, posicionY); cout << "NA"; gotoxy(posicionX+50, posicionY); cout << "|";
        gotoxy(posicionX+51, posicionY); cout << procesoEjecucion.tiempoServicio; gotoxy(posicionX+60, posicionY); cout << "|";
        gotoxy(posicionX+61, posicionY); cout << procesoEjecucion.tiempoEspera; gotoxy(posicionX+68, posicionY); cout << "|";
        gotoxy(posicionX+69, posicionY); cout << "NA"; gotoxy(posicionX+77, posicionY); cout << "|";
        gotoxy(posicionX+78, posicionY); cout << procesoEjecucion.tiempoRespuesta; gotoxy(posicionX+88, posicionY); cout << "|";
        gotoxy(posicionX+89, posicionY); cout << tiempoRestante; gotoxy(posicionX+98, posicionY); cout << "|";
        gotoxy(posicionX+99, posicionY); cout << "0"; gotoxy(posicionX+107, posicionY); cout << "|";
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|----------------------------------------------------------------------------------------------------------|";
    }
    if(procesosMemoria.size()!=0){
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|                                              PENDIENTES                                                  |";
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|----------------------------------------------------------------------------------------------------------|";
    }
    for(int i=0; i<procesosMemoria.size(); i++){
        tiempoRestante=procesosMemoria[i].tiempoMaximoEstimado-procesosMemoria[i].tiempoTranscurrido;
        posicionY++;
        gotoxy(posicionX, posicionY); cout << "|" << procesosMemoria[i].ID; gotoxy(posicionX+5, posicionY); cout << "|";
        gotoxy(posicionX+6, posicionY); cout << procesosMemoria[i].tiempoMaximoEstimado; gotoxy(posicionX+11, posicionY); cout << "|";
        gotoxy(posicionX+12, posicionY); cout << procesosMemoria[i].operacion; gotoxy(posicionX+21, posicionY); cout << "|";
        gotoxy(posicionX+22, posicionY); cout << "NA"; gotoxy(posicionX+31, posicionY); cout << "|";
        gotoxy(posicionX+32, posicionY); cout << procesosMemoria[i].tiempoTranscurrido; gotoxy(posicionX+36, posicionY); cout << "|";
        gotoxy(posicionX+37, posicionY); cout << procesosMemoria[i].tiempoLlegada; gotoxy(posicionX+45, posicionY); cout << "|";
        gotoxy(posicionX+46, posicionY); cout << "NA"; gotoxy(posicionX+50, posicionY); cout << "|";
        gotoxy(posicionX+51, posicionY); cout << procesosMemoria[i].tiempoServicio; gotoxy(posicionX+60, posicionY); cout << "|";
        gotoxy(posicionX+61, posicionY); cout << procesosMemoria[i].tiempoEspera; gotoxy(posicionX+68, posicionY); cout << "|";
        gotoxy(posicionX+69, posicionY); cout << "NA"; gotoxy(posicionX+77, posicionY); cout << "|";
        gotoxy(posicionX+78, posicionY); cout << procesosMemoria[i].tiempoRespuesta; gotoxy(posicionX+88, posicionY); cout << "|";
        gotoxy(posicionX+89, posicionY); cout << tiempoRestante; gotoxy(posicionX+98, posicionY); cout << "|";
        gotoxy(posicionX+99, posicionY); cout << "0"; gotoxy(posicionX+107, posicionY); cout << "|";
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|----------------------------------------------------------------------------------------------------------|";
    }
    if(procesosBloqueados.size()!=0){
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|                                              BLOQUEADOS                                                  |";
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|----------------------------------------------------------------------------------------------------------|";
    }
    for(int i=0; i<procesosBloqueados.size(); i++){
        tiempoRestante=procesosBloqueados[i].tiempoMaximoEstimado-procesosBloqueados[i].tiempoTranscurrido;
        posicionY++;
        gotoxy(posicionX, posicionY); cout << "|" << procesosBloqueados[i].ID; gotoxy(posicionX+5, posicionY); cout << "|";
        gotoxy(posicionX+6, posicionY); cout << procesosBloqueados[i].tiempoMaximoEstimado; gotoxy(posicionX+11, posicionY); cout << "|";
        gotoxy(posicionX+12, posicionY); cout << procesosBloqueados[i].operacion; gotoxy(posicionX+21, posicionY); cout << "|";
        gotoxy(posicionX+22, posicionY); cout << "NA"; gotoxy(posicionX+31, posicionY); cout << "|";
        gotoxy(posicionX+32, posicionY); cout << procesosBloqueados[i].tiempoTranscurrido; gotoxy(posicionX+36, posicionY); cout << "|";
        gotoxy(posicionX+37, posicionY); cout << procesosBloqueados[i].tiempoLlegada; gotoxy(posicionX+45, posicionY); cout << "|";
        gotoxy(posicionX+46, posicionY); cout << "NA"; gotoxy(posicionX+50, posicionY); cout << "|";
        gotoxy(posicionX+51, posicionY); cout << procesosBloqueados[i].tiempoServicio; gotoxy(posicionX+60, posicionY); cout << "|";
        gotoxy(posicionX+61, posicionY); cout << procesosBloqueados[i].tiempoEspera; gotoxy(posicionX+68, posicionY); cout << "|";
        gotoxy(posicionX+69, posicionY); cout << "NA"; gotoxy(posicionX+77, posicionY); cout << "|";
        gotoxy(posicionX+78, posicionY); cout << procesosBloqueados[i].tiempoRespuesta; gotoxy(posicionX+88, posicionY); cout << "|";
        gotoxy(posicionX+89, posicionY); cout << tiempoRestante; gotoxy(posicionX+98, posicionY); cout << "|";
        gotoxy(posicionX+99, posicionY); cout << procesosBloqueados[i].tiempoBloqueado; gotoxy(posicionX+107, posicionY); cout << "|";
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|----------------------------------------------------------------------------------------------------------|";
    }
    if(procesosMemoria.size()!=0){
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|                                             Suspendidos                                                  |";
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|----------------------------------------------------------------------------------------------------------|";
    }
    for(int i=0; i<procesosSuspendidos.size(); i++){
        tiempoRestante=procesosSuspendidos[i].tiempoMaximoEstimado-procesosSuspendidos[i].tiempoTranscurrido;
        posicionY++;
        gotoxy(posicionX, posicionY); cout << "|" << procesosSuspendidos[i].ID; gotoxy(posicionX+5, posicionY); cout << "|";
        gotoxy(posicionX+6, posicionY); cout << procesosSuspendidos[i].tiempoMaximoEstimado; gotoxy(posicionX+11, posicionY); cout << "|";
        gotoxy(posicionX+12, posicionY); cout << procesosSuspendidos[i].operacion; gotoxy(posicionX+21, posicionY); cout << "|";
        gotoxy(posicionX+22, posicionY); cout << "NA"; gotoxy(posicionX+31, posicionY); cout << "|";
        gotoxy(posicionX+32, posicionY); cout << procesosSuspendidos[i].tiempoTranscurrido; gotoxy(posicionX+36, posicionY); cout << "|";
        gotoxy(posicionX+37, posicionY); cout << procesosSuspendidos[i].tiempoLlegada; gotoxy(posicionX+45, posicionY); cout << "|";
        gotoxy(posicionX+46, posicionY); cout << "NA"; gotoxy(posicionX+50, posicionY); cout << "|";
        gotoxy(posicionX+51, posicionY); cout << procesosSuspendidos[i].tiempoServicio; gotoxy(posicionX+60, posicionY); cout << "|";
        gotoxy(posicionX+61, posicionY); cout << procesosSuspendidos[i].tiempoEspera; gotoxy(posicionX+68, posicionY); cout << "|";
        gotoxy(posicionX+69, posicionY); cout << "NA"; gotoxy(posicionX+77, posicionY); cout << "|";
        gotoxy(posicionX+78, posicionY); cout << procesosSuspendidos[i].tiempoRespuesta; gotoxy(posicionX+88, posicionY); cout << "|";
        gotoxy(posicionX+89, posicionY); cout << tiempoRestante; gotoxy(posicionX+98, posicionY); cout << "|";
        gotoxy(posicionX+99, posicionY); cout << "0"; gotoxy(posicionX+107, posicionY); cout << "|";
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|----------------------------------------------------------------------------------------------------------|";
    }
    posicionY++;
    gotoxy(posicionX, posicionY);
    pausaConTecla();
}

void imprimirTablaMemoria(){
    int posicionX=2, posicionY=21;
    gotoxy(posicionX,posicionY);
    posicionY++;
    cout << "|-------------------------------------------------------------------------------------------------------|";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|                                               Tabla Memoria                                           |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|-------------------------------------------------------------------------------------------------------|";
    gotoxy(posicionX, posicionY);
    posicionY++;

    cout << "|MAR.|        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|ID  |        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|LONG|        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|EST.|        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|-------------------------------------------------------------------------------------------------------|";
    gotoxy(posicionX, posicionY);
    posicionY++;

    cout << "|MAR.|        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|ID  |        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|LONG|        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|EST.|        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|-------------------------------------------------------------------------------------------------------|";
    gotoxy(posicionX, posicionY);
    posicionY++;


    cout << "|MAR.|        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|ID  |        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|LONG|        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|EST.|        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|-------------------------------------------------------------------------------------------------------|";
    gotoxy(posicionX, posicionY);
    posicionY++;


    cout << "|MAR.|        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|ID  |        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|LONG|        |        |        |        |        |        |        |   SO   |   SO   |   SO   |   SO   |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|EST.|        |        |        |        |        |        |        |        |        |        |        |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|-------------------------------------------------------------------------------------------------------|";
    gotoxy(posicionX, posicionY);
    posicionY++;

    for(int i=0; i<tablaMemoria.size(); i++){
        gotoxy(tablaMemoria[i].posicionX, tablaMemoria[i].posicionY);
        cout << tablaMemoria[i].pagina;
        gotoxy(tablaMemoria[i].posicionX, tablaMemoria[i].posicionY+1);
        cout << tablaMemoria[i].id;
        gotoxy(tablaMemoria[i].posicionX-1, tablaMemoria[i].posicionY+2);
        cout << tablaMemoria[i].procesosTabla.longitud << "/5";
        gotoxy(tablaMemoria[i].posicionX-2, tablaMemoria[i].posicionY+3);
        cout << tablaMemoria[i].estado;
    }
}

void tablaProcesoSuspendido(){
    int posicionX=80, posicionY=0;
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "Total procesos suspendidos: " << procesosSuspendidos.size();
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|------------------|";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|Proceso Suspendido|";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|------------------|";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|   ID   |longitud |";
    gotoxy(posicionX, posicionY);
    posicionY++;
    cout << "|--------|---------|";
    if(procesosSuspendidos.size()!=0){
        gotoxy(posicionX, posicionY); cout << "|" << procesosSuspendidos[0].ID;
        gotoxy(posicionX+9, posicionY); cout << "|" << procesosSuspendidos[0].longitudTotal;
        gotoxy(posicionX+19, posicionY); cout << "|";
        posicionY++;
        gotoxy(posicionX, posicionY);
        cout << "|--------|---------|";
    }
}

