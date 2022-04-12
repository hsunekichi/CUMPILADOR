/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Hecho por: Hugo Mateo
 * Colaborador: Mario Ortega
 * Última revisión: 10/04/2022
 * 
 * Sintaxis del generador ASM: 
 * NomInst<000> raXXXXXX rbXXXXXX &000 etiqueta_saltoXXXXX
 * Generará una instrucción de nombre NomInst que tendrá como binario 000.
 * El número de X mayúsculas o 1 y 0 son el número de bits del parámetro, y un parámetro que comience con & son bits fijos de relleno.
 * Solo un parámetro que comience por & puede tener bits fijos, y no deberá tener ninguna letra (se usan solo para especificar relleno)
 * 
 * Así, un ejemplo de uso usando el registro 1 y 2 y la dirección de salto "salto1" (siendo salto1 = 4) quedará:
 * NomInst ra1 rb2 salto1
 * binario:
 * 000 00001 00010 000 00100
 * (Se ha separado el binario con espacios por motivos ilustrativos)
 * 
 * Una palabra única, sin parámetros, es una etiqueta que apunta a la siguiente instrucción válida (no etiqueta)
 * Ej: estoEsUnaEtiqueta
 *     estoEs UnaInstrucción con parámetros
 * Por ello las instrucciones sin parámetros como la NOP deben tener un parámetro vacío
 * La última línea debe terminar con un fín de línea, si no no reconocerá la última instrucción
 * 
 * 
 *  
 * 
 *    Mejoras a pendientes:
 * Los errores indican la línea en la que están, pero esas líneas obvian etiquetas y líneas en blanco por lo que no coincide del todo con el código
 * El compilador no admite caracteres poniendo '', ni hexadecimal poniendo 0x
 * El compilador no admite comentarios
 * El compilador no admite etiquetas para posiciones de memoria
 * El compilador comprueba algunos errores de sintaxis, pero es aún muy limitado al dar información sobre ellos
 *     No se comprueba si una instrucción es válida, ni si una etiqueta es válida
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
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;



bool HEX_OUT = 0;                 // Da la salida en hexadecimal en lugar de en binario
bool LOGISIM_OUT = 0;             // Imprime la salida en un formato compatible con la rom de logisim
const int TAMANYO_INSTRUCCION = 32;     // Tamaño de una instrucción en bits

map <string, int> gl_etiquetas;                      // Diccionario global de dirección-etiqueta
map <string, vector<string>> gl_instrucciones;       // Diccionario global de instrucciones


// Excepciones

struct exception_wrong_config_syntax : exception
{
    string msg;
};

struct exception_unknown_instruction : exception
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
    private:
        
        bitset<TAMANYO_INSTRUCCION> instruccionBinario;        // Instrucción final en binario
        vector<string> estructura;                             // Vector con la estructura de la instrucción
        vector<string> tokens;                                 // Vector con los tokens de entrada de la instrucción

    public:
        
        // Constructor
        instruccion (vector<string> _tokens, int i_linea) : estructura (gl_instrucciones[_tokens[0]]), tokens (_tokens)
        {   
            if (tokens.size() != estructura.size())             // Número de parámetros incorrecto
            {
                exception_wrong_number_of_parameters exc;
                exc.instruction = tokens[0];
                exc.linea = i_linea;
                throw exc;
            }
        }
        
        std::string to_string ()                               // Devuelve la instrucción ensamblador, legible por humanos 
        {
            string salida = "";
            for (string token : tokens)
            {
                salida += token + " ";   
            }
            return salida;
        }                  

        std::string to_bin ()                                  // Ensambla la instrucción y la devuelve en binario, legible por la máquina
        {
            string salida = tokens[0];

            for (int inst = 1; inst < tokens.size(); inst++)
            {
                if (estructura[inst].substr(0, 14) == "etiqueta_salto")                               // Es una etiqueta de salto
                {
                    int direccion = gl_etiquetas[tokens[inst]];                                             // Obtiene la dirección de la etiqueta        
                    int nBits = count (estructura[inst].begin(), estructura[inst].end(), 'X');                  // Número de bits de la dirección

                    bitset<TAMANYO_INSTRUCCION> direccionBinario {direccion};                                   // Convierte el número a binario

                    for (int i = 0; i < nBits; i++)
                    {
                        salida += direccionBinario[i];                                                          // Añade los bits correspondientes a la instrucción
                    }

                }
                else if (estructura[inst][0] == '&')                                                  // Es una constante (relleno)
                {
                    salida += estructura[inst].substr(1);
                }
                else                                                                                  // Es un parámetro normal
                {
                    int inicioNumero = estructura[inst].find("X");                                              // Posición del primer caracter del número
                    int nBits = count (estructura[inst].begin(), estructura[inst].end(), 'X');                  // Número de bits del número
                    
                    string numero = tokens[inst].substr(inicioNumero);                                     // Quita los caracteres no numéricos
                    bitset<TAMANYO_INSTRUCCION> numeroBinario {numero};                                         // Convierte el número a binario
                    
                    for (int i = 0; i < nBits; i++)
                    {
                        salida += numeroBinario[i];                                                             // Añade los bits correspondientes a la instrucción
                    }
                }
            }
        }
        std::string to_hex ()                                  // Ensambla la instrucción y la devuelve en hexadecimal, legible por la máquina
        {
            return binSToHex (this->to_bin());
        }
};



// Tokeniza un string separado por espacios en un vector de strings
void stringToVector (string s, vector<string> &vect)
{
    stringstream ss (s);
    string token;
    while (getline (ss, token, ' '))
    {
        vect.push_back (token);
    }
}	




int main(int argc, char * argv[])
{
    ifstream f_entrada, f_config;
    ofstream f_salida;

    if (argc == 3)                            // Se ha introducido un parámetro
    {
        f_entrada.open(argv[2]);              // Fichero de entrada
        f_salida.open(argv[3]);               // Fichero de salida    
        f_config.open(argv[1]);               // Fichero de configuración  

        if (f_entrada.is_open() && f_salida.is_open() && f_config.is_open())                  // El fichero existía
        {
            string linea;                                                   // Variable de lectura

            getline (f_config, linea);                                      // Lee la primera línea del fichero de configuración

            try
            {
                if (linea == "HEX")                                             // Salida en hexadecimal
                {
                    HEX_OUT = true;
                }
                else if (linea == "BIN")                                        // Salida en binario
                {
                    HEX_OUT = false;
                }
                else                                                            // Sintaxis incorrecta
                {
                    exception_wrong_config_syntax exc;
                    exc.msg = "La primera lína debe ser HEX o BIN";
                    throw exc;
                }

                getline (f_config, linea);                                      // Lee la segunda línea del fichero de configuración

                if (linea == "LOGISIM_OUT")                                     // Salida logisim
                {
                    LOGISIM_OUT = true;
                }
                else
                {
                    LOGISIM_OUT = false;
                }

                getline (f_config, linea);                                      // Lee la siguiente línea del fichero de configuración

                while (!f_config.eof())
                {
                    string nombre = linea.substr(0, linea.find("<"));                                                       // Nombre de la instrucción
                    string bits = linea.substr(linea.find("<") + 1, linea.find(">"));                                       // Bits de la instrucción
                    vector<string> estructura;                                                                              // Vector de tokens de la instrucción
                    bits = bits + linea.substr(linea.find(">") + 1);                                                        // Añade los bits de la instrucción

                    stringToVector(bits, estructura);                                                                       // Tokeniza la estructura de la instrucción
                    gl_instrucciones [nombre] = estructura;                                                                 // Añade la estructura a la tabla de instrucciones
                    getline (f_config, linea);                                                                              // Lee la siguiente línea del fichero de configuración
                }
            }
            catch(const exception_wrong_config_syntax& e)
            {
                cerr << e.msg << '\n';
            }
            


            // Comienza la lectura y tokenizado del código

            try
            {
                vector<string> param;                                       // Variables tokenizar la instrucción
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

                        instruccion* inst = new instruccion (param, i_PC);             // Crea la instrucción
                        codigo.push_back(inst);                                        // Añade la instrucción a la lista de 
                        
                        i_PC++;                                                        // Incrementa el contador de instrucción (Para etiquetas de salto)
                    }   
                    else if (linea != "")                                   // Es una etiqueta de salto
                    {
                        etiqueta = linea; 
                        gl_etiquetas[etiqueta] = i_PC;                                  // Almacenar par (etiqueta, i_PC)                                    
                    }

                    getline (f_entrada, linea);                                        // Lee la siguiente línea
                }



                // Comienza el ensamblado

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
            catch (exception_unknown_instruction& e)
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