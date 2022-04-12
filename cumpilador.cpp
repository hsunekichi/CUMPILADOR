/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Hecho por: Hugo Mateo
 * Última revisión: 13/04/2022
 * 
 * Sintaxis de la configuración ASM: 
 * En la primera línea deberá poner BIN o HEX, dependiendo de la salida deseada.
 * Si la segunda línea es LOGISIM_OUT (puede no serlo), la salida tendrá sintaxis logisim
 * 
 * Primero va el nombre de la instrucción, con su codificación entre < >.
 * Tras ello van los parámetros. Las XXXX tras una palabra son bits indeterminados que 
 *      serán sustituidos por el valor que haya tras la palabra.
 * 
 * Un parámetro que comience con # será o bien un valor (que comience por #) o bien una etiqueta.
 * Un parámetro que comience con & será relleno fijo, y deberá especificarse su valor en la configuración
 *     con 1s y 0s. En la programación ASM el parámetro no aparecerá.
 * Al programar, una línea con un solo parámetro será una etiqueta.
 * Si una etiqueta tiene un =, se le asignará el valor que la siga.
 * Si no tiene un = se le asignará la posición de la siguiente instrucción válida (se obvian todas las etiquetas).
 * Por tanto, toda instrucción tiene que tener al menos un parámetro además del nombre.
 * 
 * Algunos ejemplos de configuración y su uso:
 *     MOV<000001> rXXXXX &00000 #XXXXXXXXXXXXXXXX
 *     MOV r1 #3
 *     salto=0x100
 *     MOV r3 salto
 * 
 *     BEQ<000011> raXXXXX rbXXXXX #XXXXXXXXXXXXXXXX 
 *     fin
 *     BEQ ra0 rb0 fin
 * 
 * NOTA: Los bits de tamaño de instrucción totales deben definirse antes de la compilación
 * --------------------NOTA: La última línea de todos los ficheros debe terminar en \n, si no se perderá--------------------
 * 
 * 
 *    Mejoras pendientes:
 * El compilador no admite comentarios
 * 
 * 
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



bool HEX_OUT = 0;                       // Da la salida en hexadecimal en lugar de en binario
bool LOGISIM_OUT = 0;                   // Imprime la salida en un formato compatible con la rom de logisim
const int TAMANYO_INSTRUCCION = 32;     // Tamaño de una instrucción en bits

map <string, int> gl_etiquetas;                      // Diccionario global de dirección-etiqueta
map <string, vector<string>> gl_instrucciones;       // Diccionario global de instrucciones


// Excepciones

class exception_wrong_config_syntax : public exception
{
    public:

    string msg;

    exception_wrong_config_syntax(string _msg)
    {
        msg = _msg; 
    }

    const char * what() const throw() override
    {
        return msg.c_str();
    }
};

class exception_unknown_instruction : public exception
{
    public:
    
    string msg;

    exception_unknown_instruction (string instruction, int linea)
    {
        stringstream ss;
        ss << "Instruccion desconocida: \"" << instruction << "\" en la linea " << linea;
        msg = ss.str();
    }

    const char * what() const throw() override
    {
        
        return msg.c_str();
    }
};

class exception_wrong_instruction_syntax : public exception
{
    public:

    string msg;

    exception_wrong_instruction_syntax (string instruction, int linea, string encontrado, string esperado)
    {
        stringstream ss;
        ss << "Error en la sintaxis de la instruccion \"" << instruction << "\" en la linea " << linea << ": Se ha encontrado \"" << 
            encontrado << "\", se esperaba \"" << esperado << "\"";
        msg = ss.str();
    }

    const char * what() const throw() override
    {
        return msg.c_str();
    }
};

class exception_wrong_label : public exception
{
    public:

    string msg;

    exception_wrong_label (string etiqueta, int linea)
    {
        stringstream ss;
        ss << "Etiqueta desconocida \"" << etiqueta << "\" en la linea " << linea;
        msg = ss.str();
    }

    const char * what() const throw() override
    {
        return msg.c_str();
    }
};

class exception_wrong_number_of_parameters : public exception
{
    public:

    string msg;

    exception_wrong_number_of_parameters (string instruction, int linea)
    {
        stringstream ss;
        ss << "Numero de parametros incorrecto en la instruccion \"" << instruction << "\" en la linea " << linea;
        msg = ss.str();
    }

    const char * what() const throw() override
    {
        return msg.c_str();
    }
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



// Convierte un string a decimal
// El string puede ser un decimal, hexadecimal comenzado con 0x o un caracter entre ''

int to_decimal(string numero)
{
    if (numero[0] == '0' && numero[1] == 'x')                               // Hexadecimal
    {
        int decimal;
        stringstream ss;
        ss << numero;
        ss >> hex >> decimal;
        return decimal;
    }
    else if  (numero[0] == '\'' && numero[numero.length()-1] == '\'')       // Caracter
    {
        return numero[1];
    }
    else                                                                    // Decimal
    {
        return stoi(numero);
    }
}



// Clases

class instruccion
{
    private:
        
        string nombre;                                         // Nombre de la instrucción
        bitset<TAMANYO_INSTRUCCION> instruccionBinario;        // Instrucción final en binario
        vector<string> tokens;                                 // Vector con los tokens de entrada de la instrucción
        int i_linea;                                           // Número de línea de la instrucción

    public:
        
        // Constructor
        instruccion (vector<string> _tokens, int _i_linea) : tokens (_tokens)
        {   
            if (gl_instrucciones.find(_tokens[0]) == gl_instrucciones.end())
            {
                throw exception_unknown_instruction(_tokens[0], _i_linea);
            }

            nombre = _tokens[0];
            i_linea = _i_linea;

            int nParametros = 0;
            for (string elemento : gl_instrucciones[nombre])                  // Cuenta el número de parámetros que no sean relleno
            {   
                if (elemento[0] != '&')
                {
                    nParametros++;
                }
            }

            if (tokens.size() != nParametros)                                 // Número de parámetros incorrecto
            {
                exception_wrong_number_of_parameters exc (tokens[0], i_linea);
                throw exc;
            }
        }
        
        // Devuelve la instrucción ensamblador, legible por humanos 
        std::string to_string ()                               
        {
            string salida = "";
            for (string token : tokens)
            {
                salida += token + " ";   
            }
            return salida;
        }                  

        // Ensambla la instrucción y la devuelve en binario, legible por la máquina
        std::string to_bin ()                                  
        {   
            string salida = gl_instrucciones[nombre][0];

            for (int str = 1, tok = 1; str < gl_instrucciones[nombre].size(); str++)
            {   
                if (gl_instrucciones[nombre][str][0] == '#')                        // Es una constante
                {
                    int direccion;
                    if (tokens[tok][0] != '#')                                                             // Es una etiqueta
                    {
                        if (gl_etiquetas.find(tokens[tok]) == gl_etiquetas.end())                          // La etiqueta no existe
                        {
                            exception_wrong_label exc (tokens[tok], i_linea);
                            throw exc;
                        }

                        direccion = gl_etiquetas[tokens[tok]];                                             // Obtiene la dirección de la etiqueta        
                    }
                    else direccion = to_decimal(tokens[tok].erase(0, 1));                                  // Obtiene la dirección del número

                    int nBits = count (gl_instrucciones[nombre][str].begin(),                              // Número de bits de la dirección
                                            gl_instrucciones[nombre][str].end(), 'X');     
                    bitset<TAMANYO_INSTRUCCION> direccionBinario {(long long unsigned int)direccion};      // Convierte el número a binario

                    for (int i = nBits - 1; i >= 0; i--)
                    {
                        salida += direccionBinario[i] ? "1" : "0";                                         // Añade los bits correspondientes a la instrucción
                    }

                    tok++;                                                                                 // Avanza el contador de los tokens
                }

                else if (gl_instrucciones[nombre][str][0] == '&')                                // Es una constante (relleno)
                {
                    salida += gl_instrucciones[nombre][str].substr(1);
                }

                else                                                                             // Es un parámetro normal
                {
                    int inicioNumero = gl_instrucciones[nombre][str].find("X");                            // Posición del primer caracter del número
                    int nBits = count (gl_instrucciones[nombre][str].begin(),
                                        gl_instrucciones[nombre][str].end(), 'X');                         // Número de bits del número

                    string encontrado = tokens[tok].substr(0, inicioNumero);
                    string esperado = gl_instrucciones[nombre][str].substr(0, inicioNumero);

                    if (encontrado != esperado)                                                            // Nombre del parámetro incorrecto
                    {
                        exception_wrong_instruction_syntax exc (tokens[0], i_linea, encontrado, esperado);
                        throw exc;
                    }

                    string numero = tokens[tok].substr(inicioNumero);                                      // Quita los caracteres no numéricos
                    bitset<TAMANYO_INSTRUCCION> numeroBinario {(long long unsigned int)to_decimal(numero)};// Convierte el número a binario

                    for (int i = nBits - 1; i >= 0; i--)
                    {
                        salida += numeroBinario[i] ? "1" : "0";                                            // Añade los bits correspondientes a la instrucción
                    }

                    tok++;                                                                                 // Avanza el contador de los tokens
                }
            }

            return salida;
        }

        // Ensambla la instrucción y la devuelve en hexadecimal, legible por la máquina
        std::string to_hex ()                                  
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

    if (argc == 4)                            // Se ha introducido un parámetro
    {
        f_entrada.open(argv[2]);              // Fichero de entrada
        f_salida.open(argv[3]);               // Fichero de salida    
        f_config.open(argv[1]);               // Fichero de configuración  

        if (f_entrada.is_open() && f_salida.is_open() && f_config.is_open())                  // El fichero existía
        {
            string linea;                                                       // Variable de lectura

            getline (f_config, linea);                                          // Lee la primera línea del fichero de configuración

            try
            {
                if (linea == "HEX") HEX_OUT = true;                             // Salida en hexadecimal
                else if (linea == "BIN") HEX_OUT = false;                       // Salida en binario
                else                                                            // Sintaxis incorrecta
                {
                    exception_wrong_config_syntax exc ("La primera linea debe ser HEX o BIN");
                    throw exc;
                }

                getline (f_config, linea);                                      // Lee la segunda línea del fichero de configuración

                if (linea == "LOGISIM_OUT")                                     // Activa la salida para logisim
                {
                    LOGISIM_OUT = true;
                    getline (f_config, linea);                                  // Lee la siguiente línea del fichero de configuración
                }
                else LOGISIM_OUT = false;

                while (!f_config.eof())
                {
                    if (linea != "")
                    {
                        int pos1 = linea.find("<");                                               // Posición del inicio de los bits de instrucción
                        int pos2 = linea.find(">");                                               // Posición del final de los bits de instrucción
                        string nombre = linea.substr(0, pos1);                                    // Nombre de la instrucción
                        string bits = linea.substr(pos1 + 1, pos2 - (pos1 + 1));                  // Bits de la instrucción

                        vector<string> estructura;                                                // Vector de tokens de la instrucción
                        bits = bits + linea.substr(pos2 + 1);                                     // Añade los bits de la instrucción

                        stringToVector(bits, estructura);                                         // Tokeniza la estructura de la instrucción
                        gl_instrucciones [nombre] = estructura;                                   // Añade la estructura a la tabla de instrucciones
                    }

                    getline (f_config, linea);                                                // Lee la siguiente línea del fichero de configuración
                }



                // Comienza la lectura y tokenizado del código

                vector<string> param;                                       // Variable para tokenizar la instrucción
                string salida;                                              // Variable para almacenar la instrucción ya traducida a binario
                bool vacio;                                                 // Finaliza el bucle de tokenizado de parámetros, cuando la instrucción tiene menos de MAX_PARAMETROS
                int i_PC = 0;                                               // Lleva la cuenta del número de línea para almacenar etiquetas de salto
                int posEspacio;                                             // Variable auxiliar para tokenizar la instrucción
                int i_numLinea = 0;                                         // Lleva la cuenta del número de línea para mostrar errores
                list <instruccion*> codigo;                                 // Lista de instrucciones del repertorio

                getline (f_entrada, linea);                                 // Lee la primera línea

                while (!f_entrada.eof())
                {
                    i_numLinea++;                                           // Incrementa el número de línea

                    if (linea != "" && linea.find(" ") != -1)                          // Es una instrucción 
                    {
                        vacio = false;

                        int nParametros = 0;                                           // Número de parámetros de la instrucción
                        for ( ; !vacio; nParametros++)                                 // Mientras queden parámetros
                        {
                            posEspacio = linea.find(" ");
                            param.push_back(linea.substr(0, posEspacio));              // Almacena el parámetro hasta el primer espacio
                            
                            if (posEspacio != -1)                                      // Quedan parámetros
                            {
                                linea.erase(0, posEspacio + 1);                        // Elimina todo hasta el primer espacio, incluido
                            }
                            else vacio = true;
                        }    

                        instruccion* inst = new instruccion (param, i_numLinea);       // Crea la instrucción
                        codigo.push_back(inst);                                        // Añade la instrucción al código
                        param.clear();                                                 // Limpia el vector de parámetros

                        i_PC++;                                                        // Incrementa el contador de instrucción (Para etiquetas de salto)
                    }   
                    else if (linea != "")                                   // Es una etiqueta de salto
                    {
                        if (linea.find('=') != -1)                                     // Almacena el valor de la etiqueta
                        {
                            gl_etiquetas[linea.substr(0, linea.find('='))] = to_decimal(linea.substr(linea.find('=') + 1));
                        }
                        else
                        {
                            gl_etiquetas[linea] = i_PC;                                // Almacena la posición de la etiqueta
                        }                        
                    }

                    getline (f_entrada, linea);                                        // Lee la siguiente línea
                }



                // Comienza el ensamblado

                if (LOGISIM_OUT)                                            // Imprime la cabecera de memoria de logisim
                {
                    f_salida << "v2.0 raw\n";
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
            catch (const exception& e)
            {
                cout << e.what() << endl;
            }
        }
        else            // Fichero incorrecto 
        {
            cerr << "No se ha encontrado el archivo " << argv[1] << endl;
        }
    }
    else                // Parámetros incorrectos
    {
        cerr << "Invocar como: ./cumpilador.exe fichero_config fichero_entrada fichero_salida" << endl;
    }
}