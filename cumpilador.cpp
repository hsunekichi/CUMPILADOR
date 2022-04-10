/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Hecho por: Hugo Mateo
 * Colaborador: Mario Ortega
 * Última revisión: 10/04/2022
 * 
 * Sintaxis de instrucciones: INST PARAM1 PARAM2 PARAM3
 * Ej: ADD r1 r2 r2        
 * Una palabra única, sin parámetros, es una etiqueta que apunta a la siguiente instrucción válida (no etiqueta)
 * Ej: estoEsUnaEtiqueta
 *     estoEs UnaInstrucción con parámetros
 * Por ello las instrucciones sin parámetros como la NOP deben tener un parámetro vacío
 * La última línea debe terminar con un fín de línea, si no no reconocerá la última instrucción
 * 
 * 
 * Para añadir instrucciones al repertorio o modificar las ya existentes, 
 *     crear las clases correspondientes (hijas de "instruccion") y añadirlas a factoría de instrucciones "crearInst"
 * 
 * 
 *    Mejoras a pendientes:
 * El compilador no admite un fichero de configuración que simplifique la definición del ASM
 * Los errores indican la línea en la que están, pero esas líneas obvian etiquetas y líneas en blanco por lo que no coincide del todo con el código
 * El compilador no admite caracteres poniendo '', ni hexadecimal poniendo 0x
 * El compilador no admite comentarios
 * El compilador no admite etiquetas para posiciones de memoria
 * El compilador comprueba algunos errores de sintaxis, pero es aún muy limitado al dar información sobre ellos
 * 
 * Por algún motivo si separas los registros con ", " o "," en vez de con " " funciona correctamente, 
 *      lo cual es preferible pero es preocupante que lo haga sin pretenderlo 
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */



#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <map>
#include <list>
#include <sstream>

using namespace std;



const bool HEX_OUT = 0;                 // Da la salida en hexadecimal en lugar de en binario
const bool LOGISIM_OUT = 0;             // Imprime la salida en un formato compatible con la rom de logisim
const int MAX_PARAMETROS = 4;           // Número máximo de tokens que puede tener una instrucción del repertorio (ADD r1, r2, r2 -> MAX_PARAMETROS = 4)
const int TAMANYO_INSTRUCCION = 32;     // Tamaño de una instrucción en bits

map <string, int> g_etiquetas;          // Diccionario global de dirección-etiqueta



// Excepciones

struct exception_wrong_instruction_syntax : exception
{
    string instruction;
    int linea;
};

struct exception_wrong_number_of_parameters : exception
{
    string instruction;
    int linea;
};



// Funciones auxiliares

// Convierte un bitset a un string hexadecimal

string binToHex (bitset<TAMANYO_INSTRUCCION> binario)
{
    stringstream hexadecimal;
    hexadecimal << hex << uppercase << binario.to_ulong();
    return hexadecimal.str();
}


// Convierte un string binario a un string hexadecimal

string binSToHex (string binarioString)
{
    bitset<TAMANYO_INSTRUCCION> binario {binarioString};
    stringstream hexadecimal;
    hexadecimal << hex << uppercase << binario.to_ulong();
    return hexadecimal.str();
}



// Clases

class instruccion
{
    public:

        virtual std::string to_string () = 0;                  // Devuelve la instrucción ensamblador, legible por humanos

        virtual std::string to_bin () = 0;                     // Ensambla la instrucción y la devuelve en binario, legible por la máquina

        std::string to_hex ()                                  // Ensambla la instrucción y la devuelve en hexadecimal, legible por la máquina
        {
            return binSToHex (this->to_bin());
        }
};

class ADD : public instruccion
{
    private:

        const std::string operacion = "ADD";           // Nombre de la instrucción

        // Formato de instrucción
        const bitset<6> operacionBin {"000001"};       // Código de la instrucción
        bitset<5> rdBin;                               // Registro destino, en binario
        bitset<5> raBin;                               // Registro origen a, en binario
        bitset<5> rbBin;                               // Registro origen b, en binario
        const bitset<11> rellenoBin {0};               // Relleno

    public:

        ADD (string ra, string rb, string rd) : rdBin {stoi(rd.substr(1))},                                     // Elimina la r inicial y lo convierte a int (r1 -> 1)
                                                    raBin {stoi(ra.substr(1))}, 
                                                    rbBin {stoi(rb.substr(1))} {}

        std::string to_string ()                                                                                // Devuelve la instrucción ensamblador, legible por humanos
        {
            return operacion + " r" + std::to_string(raBin.to_ulong()) + " r" + std::to_string(rbBin.to_ulong()) +  + " r" + std::to_string(rdBin.to_ulong());
        }

        std::string to_bin ()                                                                                   // Ensambla la instrucción y la devuelve en binario, legible por la máquina
        {
            return operacionBin.to_string() + raBin.to_string() + rbBin.to_string() + rdBin.to_string() + rellenoBin.to_string();
        }
};

class MOV : public instruccion
{
    private:

        const std::string operacion = "MOV";           // Nombre de la instrucción

        // Formato de instrucción
        const bitset<6> operacionBin {"000000"};       // Código de la instrucción
        bitset<5> rbBin;                               // Registro destino, en binario
        bitset<16> kBin;                               // Constante de la instrucción
        const bitset<5> rellenoBin {0};                // Relleno

    public:

        MOV (string rd, string k) : rbBin {stoi(rd.substr(1))},                             // Elimina la r inicial y lo convierte a int (r1 -> 1)
                                                        kBin {stoi(k.substr(1))}            // Elimina el # inicial y lo convierte a int
                                                        {}
        std::string to_string ()                                                            // Devuelve la instrucción ensamblador, legible por humanos
        {
            return operacion + " r" + std::to_string(rbBin.to_ulong()) + " #" + std::to_string(kBin.to_ulong());
        }

        std::string to_bin ()                                                               // Ensambla la instrucción y la devuelve en binario, legible por la máquina
        {
            return operacionBin.to_string() + rellenoBin.to_string() + rbBin.to_string() + kBin.to_string();
        }
};

class LW : public instruccion
{
    private:

        const std::string operacion = "LW";             // Nombre de la instrucción

        //Formato de la instrucción
        const bitset<6> operacionBin {"000010"};
        bitset<5> rbBin;                                // Registro destino
        bitset<5> raBin;                                // Registro con la dirección de memoria de la que se va a leer
        const bitset<16> rellenoBin {0};                // Relleno

    public:

        LW (string ra, string rb) : raBin {stoi(ra.substr(1))}, rbBin {stoi(rb.substr(1))} {}

        std::string to_string ()                                                                             // Devuelve la instrucción ensamblador, legible por humanos
        {
            return operacion + " r" + std::to_string(raBin.to_ulong()) + " r" + std::to_string(rbBin.to_ulong());
        }

        std::string to_bin ()                                                                                // Ensambla la instrucción y la devuelve en binario, legible por la máquina
        {
            return operacionBin.to_string() + raBin.to_string() + rbBin.to_string() + rellenoBin.to_string();
        }
};

class BEQ : public instruccion
{
    private:

        const std::string operacion = "BEQ";           // Nombre de la instrucción

        // Formato de instrucción
        const bitset<6> operacionBin {"000100"};       // Código de la instrucción
        bitset<5> raBin;                               // Registro de origen a en binario
        bitset<5> rbBin;                               // Registro de origen b en binario

        string etiqueta;                               // Etiqueta de la instrucción
        bitset<16> kBin;                               // Dirección de salto de la instrucción

    public:

        BEQ (string ra, string rb, string _etiqueta) : raBin {stoi(ra.substr(1))},             // Elimina la r inicial y lo convierte a int (r1 -> 1)
                                                        rbBin {stoi(rb.substr(1))},            // Elimina el # inicial y lo convierte a int
                                                        etiqueta {_etiqueta}                   // Crea la etiqueta
                                                        {}

        std::string to_string ()                                                               // Devuelve la instrucción ensamblador, legible por humanos
        {
            return operacion + " r" + std::to_string(raBin.to_ulong()) + " r" + std::to_string(rbBin.to_ulong()) + etiqueta;
        }

        std::string to_bin ()                                                                  // Ensambla la instrucción y la devuelve en binario, legible por la máquina
        {   
            kBin = g_etiquetas[etiqueta];                                                      // Obtiene la dirección de salto de la etiqueta                       
            return operacionBin.to_string() + raBin.to_string() + rbBin.to_string() + kBin.to_string();
        }
};

class SW : public instruccion
{
    private:

        const std::string operacion = "SW";             // Nombre de la instrucción

        //Formato de la instrucción
        const bitset<6> operacionBin {"000011"};
        bitset<5> raBin;                                // Registro destino
        bitset<5> rbBin;                                // Registro con la dirección de memoria de la que se va a leer
        const bitset<16> rellenoBin {0};                // Relleno

    public:

        SW (string ra, string rb) : raBin {stoi(ra.substr(1))}, rbBin {stoi(rb.substr(1))} {}

        std::string to_string ()                                                                             // Devuelve la instrucción ensamblador, legible por humanos
        {
            return operacion + " r" + std::to_string(raBin.to_ulong()) + " r" + std::to_string(rbBin.to_ulong());
        }

        std::string to_bin ()                                                                                // Ensambla la instrucción y la devuelve en binario, legible por la máquina
        {
            return operacionBin.to_string() + raBin.to_string() + rbBin.to_string() + rellenoBin.to_string();
        }
};

class NOP : public instruccion
{
    private:

        const std::string operacion = "NOP";            // Nombre de la instrucción

        //Formato de la instrucción
        const bitset<6> operacionBin {"000101"};
        const bitset<26> rellenoBin {0};                // Relleno

    public:

        NOP (string ra) {}

        std::string to_string ()                                                                             // Devuelve la instrucción ensamblador, legible por humanos
        {
            return operacion;
        }

        std::string to_bin ()                                                                                // Ensambla la instrucción y la devuelve en binario, legible por la máquina
        {
            return operacionBin.to_string() + rellenoBin.to_string();
        }
};


// Factoría de instrucciones
// nParametros es el número de parámetros que contiene el vector (incluido el nombre de la instrucción)

instruccion* crearInst (string parametros[MAX_PARAMETROS], int nParametros, int i_linea)
{
    if (parametros[0] == "ADD")                                                 // ADD
    {
        if (nParametros == 4)
        {
            return new ADD (parametros[1], parametros[2], parametros[3]);
        }
        else
        {
            exception_wrong_number_of_parameters exc;
            exc.instruction = parametros [0];
            exc.linea = i_linea;
            throw exc;
        }
    }
    else if (parametros[0] == "MOV")                                            // MOV
    {
        if (nParametros == 3)
        {
            return new MOV (parametros[1], parametros[2]);
        }
        else
        {
            exception_wrong_number_of_parameters exc;
            exc.instruction = parametros [0];
            exc.linea = i_linea;
            throw exc;
        }
    }
    else if (parametros[0] == "LW")                                             // LW
    {
        if (nParametros == 3)
        {
            return new LW (parametros[1], parametros[2]);
        }
        else
        {
            exception_wrong_number_of_parameters exc;
            exc.instruction = parametros [0];
            exc.linea = i_linea;
            throw exc;
        }
    }
    else if (parametros[0] == "BEQ")                                            // BEQ
    {
        if (nParametros == 4)
        {
            return new BEQ (parametros[1], parametros[2], parametros[3]);
        }
        else
        {
            exception_wrong_number_of_parameters exc;
            exc.instruction = parametros [0];
            exc.linea = i_linea;
            throw exc;
        }
    }
    else if (parametros[0] == "SW")                                            // BEQ
    {
        if (nParametros == 3)
        {
            return new SW (parametros[1], parametros[2]);
        }
        else
        {
            exception_wrong_number_of_parameters exc;
            exc.instruction = parametros [0];
            exc.linea = i_linea;
            throw exc;
        }
    }
    else if (parametros[0] == "NOP")                                            // BEQ
    {
        if (nParametros >= 1)
        {
            return new NOP (parametros[1]);
        }
        else
        {
            exception_wrong_number_of_parameters exc;
            exc.instruction = parametros [0];
            exc.linea = i_linea;
            throw exc;
        }
    }
    else                                                                        // Error
    {
        exception_wrong_instruction_syntax exc;
        throw exc;
    }
}



int main(int argc, char * argv[])
{
    ifstream f_entrada;
    ofstream f_salida;

    if (argc == 3)                            // Se ha introducido un parámetro
    {
        f_entrada.open(argv[1]);              // Fichero de entrada
        f_salida.open(argv[2]);               // Fichero de salida      

        if (f_entrada.is_open() && f_salida.is_open())                  // El fichero existía
        {
            try
            {
                string linea, param[MAX_PARAMETROS];                        // Variables para leer y tokenizar la instrucción
                string etiqueta;                                            // Variable para leer etiquetas de salto
                string salida;                                              // Variable para almacenar la instrucción ya traducida a binario
                bool vacio;                                                 // Finaliza el bucle de tokenizado de parámetros, cuando la instrucción tiene menos de MAX_PARAMETROS
                int i_PC = 0;                                               // Lleva la cuenta del número de línea para almacenar etiquetas de salto
                int posEspacio;                                             // Lleva la cuenta del número de línea para almacenar etiquetas de salto
                list <instruccion*> codigo;                                 // Lista de instrucciones del repertorio


                if (LOGISIM_OUT)                                            // Imprime la cabecera de memoria de logisim
                {
                    f_salida << "v2.0 raw\n";
                }

                getline (f_entrada, linea);                                 // Lee la primera línea

                while (!f_entrada.eof())
                {
                    if (linea != "" && linea.find(" ") != -1)                          // Es una instrucción 
                    {
                        vacio = false;

                        int nParametros = 0;                                           // Número de parámetros de la instrucción
                        for ( ; !vacio; nParametros++)                                 // Mientras queden parámetros
                        {
                            posEspacio = linea.find(" ");
                            param [nParametros] = linea.substr(0, posEspacio);         // Almacena el parámetro hasta el primer espacio
                            
                            if (posEspacio != -1)                                      // Quedan parámetros
                            {
                                linea.erase(0, posEspacio + 1);                        // Elimina todo hasta el primer espacio, incluido
                            }
                            else
                            {
                                vacio = true;
                            }
                        }      

                        instruccion* inst = crearInst (param, nParametros, i_PC);      // Crea la instrucción
                        codigo.push_back(inst);                                        // Añade la instrucción a la lista de instrucciones

                        i_PC++;                                                        // Incrementa el contador de instrucción (Para etiquetas de salto)
                    }   
                    else if (linea != "")                                   // Es una etiqueta de salto
                    {
                        etiqueta = linea; 
                        g_etiquetas[etiqueta] = i_PC;                                  // Almacenar par (etiqueta, i_PC)                                    
                    }

                    getline (f_entrada, linea);                                        // Lee la siguiente línea
                }

                for (instruccion *inst : codigo)                            // Recorre la lista de instrucciones
                {
                    if (HEX_OUT)
                    {
                        salida = inst->to_hex();                            // Traduce la instrucción a hexadecimal
                    }
                    else
                    {
                        salida = inst->to_bin();                            // Traduce la instrucción a binario
                    }

                    // Imprime la instrucción en la salida
                    if (LOGISIM_OUT)                                        // Código para rom de logisim
                    {
                        f_salida << salida << " ";
                    }
                    else                                                    // Código estándar
                    {
                        f_salida << salida << endl;
                    }
                }
            }
            catch (exception_wrong_instruction_syntax& e)
            {
                cout << "Sintaxis incorrecta en la línea: " << e.linea << endl << "Se encontró: " << e.instruction << endl;
            }
            catch (exception_wrong_number_of_parameters& e)
            {
                cout << "Número de parámetros incorrecto en la instrucción: " << e.instruction << ", línea: " << e.linea << endl;
            }
        }
        else            // Fichero incorrecto 
        {
            cerr << "No se ha encontrado el archivo " << argv[1] << endl;
        }
    }
    else                // Parámetros incorrectos
    {
        cerr << "Invocar como: ./cumpilador.exe fichero_entrada fichero_salida" << endl;
    }
}