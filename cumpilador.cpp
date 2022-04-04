/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Hecho por: Hugo Mateo
 * Colavorador: Mario Ortega
 * Última revisión: 30/03/2022
 * 
 * Sintaxis de instrucciones: INST PARAM1 PARAM2 PARAM3
 * Ej: ADD r1 r2 r2        
 * Una palabra única, sin parámetros, es una etiqueta que apunta a la siguiente instrucción
 * Ej: estoEsUnaEtiqueta
 *     estoEs UnaInstrucción 
 * 
 * El compilador admite etiquetas de salto, aunque actualmente no hacen nada
 * El compilador no admite etiquetas para posiciones de memoria
 * El compilador ni comprueba ni notifica errores de sintaxis en el código ASM
 * 
 * Por algún motivo si separas los registros con ", " o "," en vez de con " " funciona correctamente, 
 *      lo cual es mejor pero es preocupante que lo haga sin pretenderlo 
 * Además, por algún motivo igual de misterioso admite instrucciones con solo 2 parámetros como el MOV sin un tercer parámetro basura
 * 
 * Para añadir instrucciones al repertorio o modificar las ya existentes, hacerlo en la función tradBinario
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */



#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <map>

using namespace std;



const bool HEX_OUT = 1;                 // Da la salida en hexadecimal en lugar de en binario
const bool LOGISIM_OUT = 1;             // Imprime la salida en un formato compatible con la rom de logisim
const int MAX_PARAMETROS = 4;           // Número máximo de tokens que puede tener una instrucción del repertorio (ADD r1, r2, r2 -> MAX_PARAMETROS = 4)
const int TAMANYO_INSTRUCCION = 32;     // Tamaño de una instrucción en bits

map <string, int> etiquetas;            // Diccionario global de dirección-etiqueta



// Excepciones

struct exception_wrong_instruction_syntax
{
    string instruction;
};

struct exception_wrong_number_of_parameters
{
    string instruction;
};



// Clases

class instruccion
{
    public:

        virtual std::string to_string () = 0;                  // Devuelve la instrucción ensamblador, legible por humanos

        virtual std::string to_bin () = 0;                     // Ensambla la instrucción y la devuelve en binario, legible por la máquina

        std::string to_hex ()                                    // Ensambla la instrucción y la devuelve en hexadecimal, legible por la máquina
        {
            return binToHex (this->to_bin());
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

        ADD (string rd, string ra, string rb) : rdBin {stoi(rd.substr(1))},                                       // Elimina la r inicial y lo convierte a int (r1 -> 1)
                                                    raBin {stoi(ra.substr(1))}, 
                                                    rbBin {stoi(rb.substr(1))} {}

        std::string to_string ()                                                                                // Devuelve la instrucción ensamblador, legible por humanos
        {
            return operacion + " r" + std::to_string(rdBin.to_ulong()) + " r" + std::to_string(raBin.to_ulong()) +  + " r" + std::to_string(rbBin.to_ulong());
        }

        std::string to_bin ()                                                                                   // Ensambla la instrucción y la devuelve en binario, legible por la máquina
        {
            return operacionBin.to_string() + rdBin.to_string() + raBin.to_string() + rbBin.to_string() + rellenoBin.to_string();
        }
};

class MOV : public instruccion
{
    private:

        const std::string operacion = "MOV";           // Nombre de la instrucción

        // Formato de instrucción
        const bitset<6> operacionBin {"000000"};       // Código de la instrucción
        bitset<5> rdBin;                               // Registro destino, en binario
        bitset<16> kBin;                               // Constante de la instrucción
        const bitset<5> rellenoBin {0};                // Relleno

    public:

        MOV (string rd, string k) : rdBin {stoi(rd.substr(1))},                             // Elimina la r inicial y lo convierte a int (r1 -> 1)
                                                        kBin {stoi(k.substr(1))}            // Elimina el # inicial y lo convierte a int
                                                        {}
        std::string to_string ()                                                          // Devuelve la instrucción ensamblador, legible por humanos
        {
            return operacion + " r" + std::to_string(rdBin.to_ulong()) + " #" + std::to_string(kBin.to_ulong());
        }

        std::string to_bin ()                                                             // Ensambla la instrucción y la devuelve en binario, legible por la máquina
        {
            return operacionBin.to_string() + rdBin.to_string() + kBin.to_string() + rellenoBin.to_string();
        }
};

class LW : public instruccion
{
    private:
        const std::string operacion = "LW";             //Nombre de la instrucción

        //Formato de la instrucción
        const bitset<6> operacionBin {"000010"};
        bitset<5> rdBin;                                //Registro destino
        bitset<16> rsBin;                               //Registro con la dirección de memoria de la que se va a leer
        const bitset<5> rellenoBin {0};

    public:
<<<<<<< HEAD

        LW (string rd, string rs) : rdBin {stoi(rd.substr(1))}, rsBin {stoi(rs.substr(1))} {}

        std::string&& to_string ()                                                                             // Devuelve la instrucción ensamblador, legible por humanos
=======
        LW(string rd, string rs) : rdBin {stoi(rd.substr(1))}, rsBin {stoi(rs.substr(1))} {}
        LW(LW&& oldInst) : rdBin {oldInst.rdBin}, rsBin {oldInst.rsBin} {}      //Constructor de transferencia

        std::string&& to_string ()                                                                                // Devuelve la instrucción ensamblador, legible por humanos
>>>>>>> 1667d6590c7dd77911524c984d3ae41e09aaea2d
        {
            return operacion + " r" + std::to_string(rdBin.to_ulong()) + " #" + std::to_string(rsBin.to_ulong());
        }

        std::string&& to_bin ()                                                                                //Ensambla la instrucción y la devuelve en binario, legible por la máquina
        {
            return operacionBin.to_string() + rdBin.to_string() + rsBin.to_string() + rellenoBin.to_string();
        }
};
<<<<<<< HEAD

class BEQ : public instruccion
{
    private:

        const std::string operacion = "MOV";           // Nombre de la instrucción

        // Formato de instrucción
        const bitset<6> operacionBin {"000011"};       // Código de la instrucción
        bitset<5> raBin;                               // Registro de origen a en binario
        bitset<5> rbBin;                               // Registro de origen b en binario

        string etiqueta;                               // Etiqueta de la instrucción
        bitset<16> kBin;                               // Dirección de salto de la instrucción

    public:

        MOV (string ra, string rb, string _etiqueta) : raBin {stoi(ra.substr(1))},             // Elimina la r inicial y lo convierte a int (r1 -> 1)
                                                        rbBin {stoi(rb.substr(1))},            // Elimina el # inicial y lo convierte a int
                                                        etiqueta {_etiqueta}                   // Crea la etiqueta
                                                        {}

        std::string to_string ()                                                               // Devuelve la instrucción ensamblador, legible por humanos
        {
            return operacion + " r" + std::to_string(raBin.to_ulong()) + " r" + std::to_string(rbBin.to_ulong()) + etiqueta;
        }

        std::string to_bin ()                                                                  // Ensambla la instrucción y la devuelve en binario, legible por la máquina
        {
            kBin {etiquetas[etiqueta]};                                                        // Obtiene la dirección de salto de la etiqueta
            return operacionBin.to_string() + raBin.to_string() + rbBin.to_string() + kBin.to_string();
        }
};

=======
>>>>>>> 1667d6590c7dd77911524c984d3ae41e09aaea2d


// Factoría de instrucciones
// nParametros es el número de parámetros inc

instruccion* crearInst (string parametros[MAX_PARAMETROS], int nParametros)
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
            throw exc;
        }
    }
    else                                                                        // Error
    {
        exception_wrong_instruction_syntax exc;
        throw exc;
    }
}



// Convierte un bitset a un string hexadecimal

string binToHex (bitset<TAMANYO_INSTRUCCION> binario)
{
    stringstream hexadecimal;
    hexadecimal << hex << uppercase << binario.to_ulong();
    return hexadecimal.str();
}



// Traduce los parámetros a una instrucción en binario completa.
// Modificar aquí la sintaxis del ensamblador

string ensamblar (string inst, string param1, string param2, string param3)        //Necesitará tener acceso al diccionario de etiquetas de salto
{   
    string instruccion = "";

    if (inst == "ADD")
    {

        string reg1 = param1.substr(1);             // Elimina la r del número de registro (r1 -> 1)
        string reg2 = param2.substr(1);             // Elimina la r del número de registro (r1 -> 1)
        string reg3 = param3.substr(1);             // Elimina la r del número de registro (r1 -> 1)

        bitset<6> instBin {"000001"};               // Código de la instrucción
        bitset<5> param1Bin {stoi(reg1)};           // Parámetro 1
        bitset<5> param2Bin {stoi(reg2)};           // Parámetro 2
        bitset<5> param3Bin {stoi(reg3)};           // Parámetro 3

        string relleno = "";
        for (int i = 0; i < 11; i++) relleno += "0";            // Crea el relleno para completar la instrucción

        // Genera la instrucción completa
        instruccion = instBin.to_string() + param1Bin.to_string() + param2Bin.to_string() + param3Bin.to_string() + relleno;     
    }
    else if (inst == "MOV")
    {
        string reg1 = param1.substr(1);                     // Elimina la r del número de registro (r1 -> 1)

        bitset<6> instBin {"000000"};
        bitset<5> param1Bin {stoi(reg1)};                   // Parámetro 1
        bitset<16> param2Bin {stoi(param2)};                // Parámetro 2

        string relleno = "";
        for (int i = 0; i < 5; i++) relleno += "0";         // Crea el relleno para completar la instrucción

        // Genera la instrucción completa
        instruccion = instBin.to_string() + param1Bin.to_string() + param2Bin.to_string() + relleno ;        
    }

    return instruccion;
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
            string linea, inst[MAX_PARAMETROS];                         // Variables para leer y tokenizar la instrucción
            string etiqueta;                                            // Variable para leer etiquetas de salto
            bool vacio;                                                 // Finaliza el bucle de tokenizado de parámetros, cuando la instrucción tiene menos de MAX_PARAMETROS
            int i_PC = 1;                                               // Lleva la cuenta del número de línea para almacenar etiquetas de salto
            int posEspacio;                                             // Lleva la cuenta del número de línea para almacenar etiquetas de salto

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
                        inst [nParametros] = linea.substr(0, posEspacio);          // Almacena el parámetro hasta el primer espacio
                        
                        if (posEspacio != -1)                                      // Quedan parámetros
                        {
                            linea.erase(0, posEspacio + 1);                        // Elimina todo hasta el primer espacio, incluido
                        }
                        else
                        {
                            vacio = true;
                        }
                    }      
                    nParametros--;                                                 // Decrementa el número de parámetros, para corregir nParametros++ final del for

                    instruccion* inst = crearInst (inst, nParametros);             // Crea la instrucción

                    string salida = inst->to_hex();                                // Traduce la instrucción a hexadecimal

                    // Imprime la instrucción en la salida
                    if (LOGISIM_OUT)                                               // Código para rom de logisim
                    {
                        f_salida << salida << " ";
                    }
                    else                                                           // Código estándar
                    {
                        f_salida << salida << endl;
                    }

                    i_PC++;                                                        // Incrementa el contador de instrucción (Para etiquetas de salto)
                }   
                else if (linea != "")                                   // Es una etiqueta de salto
                {
                    etiqueta = linea; 
                    etiquetas[etiqueta] = i_PC;                                    // almacenar par (etiqueta, i_PC)                                    
                }

                getline (f_entrada, linea);                                        // Lee la siguiente línea
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