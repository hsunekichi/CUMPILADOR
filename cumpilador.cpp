/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Hecho por: Hugo Mateo
 * Última revisión: 26/04/2022
 * 
 * Sintaxis de la configuración ASM: 
 * En la primera línea deberá poner BIN o HEX, dependiendo de la salida deseada.
 * Si la segunda línea es LOGISIM_OUT (puede no serlo), la salida tendrá sintaxis logisim
 * Si la segunda línea es VHDL_OUT (puede no serlo), la salida tendrá sintaxis VHDL
 * Si la segunda línea (si no hay ni VHDL_OUT ni LOGISIM_OUT) o la tercera (si aparece uno de ellos) línea es SALTO_RELATIVO
 *      los parámetros de configuración que comiencen por ## serán saltos relativos a PC
 *      las direcciones de salto se calcularán relativas a PC 
 * 
 * Primero va el nombre de la instrucción, con su codificación entre < >.
 * Tras ello van los parámetros. Las **** tras una palabra son bits indeterminados que 
 *      serán sustituidos por el valor que haya tras la palabra.
 * 
 * En configuración:
 * Un parámetro que comience con # será o bien un valor (que comience por #) o bien una etiqueta.
 * Un parámetro que comience con & será relleno fijo, y deberá especificarse su valor en la configuración
 *     con 1s y 0s. En la programación ASM el parámetro no aparecerá.
 * Si la línea comienza con & se considerará se definirá como una palabra especial que se sustituirá automáticamente por aquello que haya en tras el igual.
 * Al programar:
 * Una línea sin espacios será una etiqueta.
 * Cuando aparezca un ';' se descartará el resto de la línea (comentarios).
 * Si una etiqueta tiene un =, se le asignará el valor que la siga.
 * Si no tiene un = se le asignará la posición de la siguiente instrucción válida (se obvian todas las etiquetas).
 * Por tanto, toda instrucción tiene que tener al menos un parámetro además del nombre.
 * 
 * Algunos ejemplos de configuración y su uso:
 *  fichero configuración:
 * 
 *     HEX
 *     LOGISIM_OUT
 *     SALTO_RELATIVO
 *     
 *     &ADD_CODE=&00001 
 *    
 *     ADDFP<000000> rs*****fp rt*****fp rd*****fp &000000 &ADD_CODE 
 *     MOV<000001> rXXXXX &00000 #XXXXXXXXXXXXXXXX
 *     BEQ<000011> rXXXXX rXXXXX ##XXXXXXXXXXXXXXXX  
 *     ST<000100> rDirXXXXX rDatXXXXX &0000000000000000  
 * 
 * fichero programa:
 *
 *     MOV r1 #3
 *     dir_pantalla=0x10f
 *     MOV r1 dir_pantalla
 * 
 *     ADDFP rs1fp rt2fp rd3fp
 *     MOV r0 #'a'
 *     ST rDir1 rDat0
 *     fin
 *     BEQ r0 r0 fin
 * 
 * ------------------ ADVERTENCIA: La última línea de todos los ficheros debe terminar en \n, si no se perderá la línea -------------------- 
 *                                 En las etiquetas se deben separar los comentarios del código con '\t', o se confundirá con una instrucción
 *                                 Funciona en linux y en windows, pero cuidado con usar un txt de windows en linux y viceversa
 * 
 * NOTA: Los bits de tamaño de instrucción totales deben definirse antes de la compilación
 * 
 *    Mejoras pendientes:
 * 
 *    Errores conocidos: 
 *      Los comentarios dan problemas a veces, de momento es mejor compilar código sin ellos
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



bool gl_hex_out;                                     // Da la salida en hexadecimal en lugar de en binario
bool gl_logisim_out;                                 // Imprime la salida en un formato compatible con la rom de logisim
bool gl_vhdl_out;                                    // Imprime la salida en un formato compatible con las memorias VHDL
bool gl_salto_relativo;                              // Si se habilita, los saltos se calcularán relativos a PC
const int TAMANYO_INSTRUCCION = 32;                  // Tamaño de una instrucción en bits

const char CH_VARIABLE_CONFIG = '*';                 // Caracter que indica un dato variable en configuración
const char CH_CONSTANTE_CONFIG = '&';                // Caracter que indica un dato fijo en configuración
const char CH_ETIQUETA_CONFIG = '#';                 // Caracter que indica una etiqueta en configuración

map <string, int> gl_etiquetas;                      // Diccionario global de dirección-etiqueta
map <string, vector<string>> gl_instrucciones;       // Diccionario global de instrucciones
map <string, string> gl_constantes_config;           // Lista de constantes en configuración


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
    string salida = hexadecimal.str();

    if (salida.size() < TAMANYO_INSTRUCCION / 4)
        salida = string(TAMANYO_INSTRUCCION / 4 - salida.size(), '0') + salida;
    
    if (TAMANYO_INSTRUCCION % 4 != 0)
        salida = "0" + salida;

    return salida;
}


// Convierte un string binario a un string hexadecimal

string binSToHex (string binarioString)
{
    bitset<TAMANYO_INSTRUCCION> binario {binarioString};
    stringstream hexadecimal;
    hexadecimal << hex << uppercase << binario.to_ulong();
    
    string salida = hexadecimal.str();

    if (salida.size() < TAMANYO_INSTRUCCION / 4)
        salida = string(TAMANYO_INSTRUCCION / 4 - salida.size(), '0') + salida;

    if (TAMANYO_INSTRUCCION % 4 != 0)
        salida = "0" + salida;

    return salida;
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
        int i_PC;                                              // Número de PC de la instrucción
    
    public:
        
        // Constructor
        instruccion (vector<string> _tokens, int _i_linea, int _i_PC) : tokens (_tokens)
        {   
            if (gl_instrucciones.find(_tokens[0]) == gl_instrucciones.end())
            {
                throw exception_unknown_instruction(_tokens[0], _i_linea);
            }

            nombre = _tokens[0];
            i_linea = _i_linea;
            i_PC = _i_PC;

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
                if (gl_instrucciones[nombre][str][0] == CH_ETIQUETA_CONFIG)                        // Es una constante
                {
                    int direccion;
                    if (tokens[tok][0] != CH_ETIQUETA_CONFIG)                                                 // Es una etiqueta
                    {
                        if (gl_etiquetas.find(tokens[tok]) == gl_etiquetas.end())                             // La etiqueta no existe
                        {
                            exception_wrong_label exc (tokens[tok], i_linea);
                            throw exc;
                        }

                        if (gl_salto_relativo && gl_instrucciones[nombre][str][1] == CH_ETIQUETA_CONFIG)
                        {
                            direccion = gl_etiquetas[tokens[tok]] - (i_PC + 1);                               // Dirección relativa a la instrucción
                        }
                        else
                        {
                            direccion = gl_etiquetas[tokens[tok]];                                            // Obtiene la dirección de la etiqueta 
                        }                                               
                    }
                    else direccion = to_decimal(tokens[tok].erase(0, 1));                                     // Obtiene la dirección del número

                    int nBits = count (gl_instrucciones[nombre][str].begin(),                                 // Número de bits de la dirección
                                            gl_instrucciones[nombre][str].end(), CH_VARIABLE_CONFIG);     
                    bitset<TAMANYO_INSTRUCCION> direccionBinario {(long long unsigned int)direccion};         // Convierte el número a binario

                    for (int i = nBits - 1; i >= 0; i--)
                    {
                        salida += direccionBinario[i] ? "1" : "0";                                            // Añade los bits correspondientes a la instrucción
                    }

                    tok++;                                                                                    // Avanza el contador de los tokens
                }

                else if (gl_instrucciones[nombre][str][0] == CH_CONSTANTE_CONFIG)                             // Es una constante (relleno)
                {
                    salida += gl_instrucciones[nombre][str].substr(1);
                }

                else                                                                             // Es un parámetro normal
                {
                    int inicioNumero = gl_instrucciones[nombre][str].find(CH_VARIABLE_CONFIG);                // Posición del primer caracter del número
                    int finNumero = gl_instrucciones[nombre][str].find_last_of(CH_VARIABLE_CONFIG);           // Posición del último caracter del número
                    int nBits = finNumero - inicioNumero + 1;                                                 // Número de bits del número
                    
                    string encontrado = tokens[tok].substr(0, inicioNumero);                                  // Parte inicial del token (previa al número)
                    string resto = tokens[tok].substr(inicioNumero);                                          // Parte final del token (después del número)

                    int inicioCaracFin = resto.find_first_not_of("1234567890");                               // Posición del primer caracter no numérico
                    string encontradoFinal = "";
                    if (inicioCaracFin != -1)
                        encontradoFinal = resto.substr(inicioCaracFin);                                       // Parte inicial del token (previa al número)

                    string esperado = gl_instrucciones[nombre][str].substr(0, inicioNumero);                  // Parte inicial esperada
                    string esperadoFinal = "";
                    if (gl_instrucciones[nombre][str].size() >= finNumero + 1)                                // Si hay caracteres al final del parámetro
                        esperadoFinal = gl_instrucciones[nombre][str].substr(finNumero + 1);                  // Parte final esperada

                    if (encontrado != esperado || encontradoFinal != esperadoFinal)                           // Nombre del parámetro incorrecto
                    {
                        if (esperadoFinal != "" || encontradoFinal != "")
                        {
                            exception_wrong_instruction_syntax exc (tokens[0], i_linea, tokens[tok], esperado + "*" + esperadoFinal);
                            throw exc;
                        }
                        exception_wrong_instruction_syntax exc (tokens[0], i_linea, encontrado, esperado);
                        throw exc;
                    }

                    string numero = tokens[tok].substr(inicioNumero);                                         // Quita los caracteres no numéricos
                    bitset<TAMANYO_INSTRUCCION> numeroBinario {(long long unsigned int)to_decimal(numero)};   // Convierte el número a binario

                    for (int i = nBits - 1; i >= 0; i--)
                    {
                        salida += numeroBinario[i] ? "1" : "0";                                               // Añade los bits correspondientes a la instrucción
                    }

                    tok++;                                                                                    // Avanza el contador de los tokens
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
                if (linea == "HEX") gl_hex_out = true;                          // Salida en hexadecimal
                else if (linea == "BIN") gl_hex_out = false;                    // Salida en binario
                else                                                            // Sintaxis incorrecta
                {
                    exception_wrong_config_syntax exc ("La primera linea debe ser HEX o BIN");
                    throw exc;
                }

                getline (f_config, linea);                                      // Lee la segunda línea del fichero de configuración

                if (linea == "LOGISIM_OUT")                                     // Activa la salida para logisim
                {
                    gl_logisim_out = true;
                    getline (f_config, linea);                                  // Lee la siguiente línea del fichero de configuración
                }
                else
                    gl_logisim_out = false;

                if (linea == "VHDL_OUT")                                        // Activa la salida para VHDL
                {
                    gl_vhdl_out = true;
                    getline (f_config, linea);                                  // Lee la siguiente línea del fichero de configuración
                }
                else 
                    gl_vhdl_out = false;

                if (linea == "SALTO_RELATIVO")                                  // Activa los saltos relativos a PC
                {
                    gl_salto_relativo = true;
                    getline (f_config, linea);                                  // Lee la siguiente línea del fichero de configuración
                }
                else
                    gl_salto_relativo = false;

                while (!f_config.eof())
                {
                    if (linea != "")
                    {
                        if (linea[0] == CH_CONSTANTE_CONFIG)                                            // Es una constante de configuración
                            gl_constantes_config[linea.substr(0, linea.find('='))] =                    // Añade el valor tras el '=' a la tabla de constantes
                                linea.substr(linea.find('=') + 1);
                        else
                        {
                            int pos1 = linea.find("<");                                                     // Posición del inicio de los bits de instrucción
                            int pos2 = linea.find(">");                                                     // Posición del final de los bits de instrucción
                            string nombre = linea.substr(0, pos1);                                          // Nombre de la instrucción
                            string bits = linea.substr(pos1 + 1, pos2 - (pos1 + 1));                        // Bits de operación

                            vector<string> estructura;                                                      // Vector de tokens de la instrucción
                            bits = bits + linea.substr(pos2 + 1);                                           // Añade los bits de la instrucción

                            stringToVector(bits, estructura);                                               // Tokeniza la estructura de la instrucción

                            for (string &str : estructura)                                                  // Transforma las etiquetas de configuración en su valor 
                            {
                                if (str[0] == CH_CONSTANTE_CONFIG                                           // Es una etiqueta interna de configuración
                                    && 
                                    count(str.begin(), str.end(), '0') 
                                    + 
                                    count(str.begin(), str.end(), '1') 
                                    + 
                                    1
                                    !=  
                                    str.size())                                 
                                {
                                    if (gl_constantes_config.find(str) == gl_constantes_config.end())       // No existe la etiqueta
                                    {
                                        exception_wrong_config_syntax exc ("La constante " + str + " no existe");
                                        throw exc;
                                    }

                                    str = gl_constantes_config[str];                                        // Cambia el nombre de la etiqueta por su valor
                                }
                            }

                            gl_instrucciones [nombre] = estructura;                                         // Añade la estructura a la tabla de instrucciones
                        }
                    }

                    getline (f_config, linea);                                                // Lee la siguiente línea del fichero de configuración
                }



                // Comienza la lectura y tokenizado del código

                vector<string> param;                                       // Variable para tokenizar la instrucción
                string salida;                                              // Variable para almacenar la instrucción ya traducida a binario
                int i_PC = 0;                                               // Lleva la cuenta del número de línea para almacenar etiquetas de salto
                int posEspacio;                                             // Variable auxiliar para tokenizar la instrucción
                int i_numLinea = 0;                                         // Lleva la cuenta del número de línea para mostrar errores
                list <instruccion*> codigo;                                 // Lista de instrucciones del repertorio

                getline (f_entrada, linea);                                 // Lee la primera línea

                while (!f_entrada.eof())
                {
                    i_numLinea++;                                           // Incrementa el número de línea

                    linea.find(";") != string::npos ?                       // Busca un comentario
                        linea = linea.substr(0, linea.find(";"))            // Elimina el comentario
                        :   
                        linea;                                              // No hay un comentario

                    if (linea != "" && linea.find(" ") != -1)                   // Es una instrucción 
                    {
                        std::replace( linea.begin(), linea.end(), '\t', ' ');               // Elimina los tabuladores
                        stringstream ss (linea);                                            // Convierte la línea en un flujo de datos
                        string token;                                                       // Variable para almacenar el token

                        while (getline(ss, token, ' '))
                            if (token != "")
                                param.push_back(token);

                        instruccion* inst = new instruccion (param, i_numLinea, i_PC);      // Crea la instrucción
                        codigo.push_back(inst);                                             // Añade la instrucción al código
                        param.clear();                                                      // Limpia el vector de parámetros

                        i_PC++;                                                             // Incrementa el contador de instrucción (Para etiquetas de salto)
                    }   
                    else if (linea != "")                                       // Es una etiqueta de salto
                    {
                        if (linea.find('=') != -1)                                     // Almacena el valor de la etiqueta
                            gl_etiquetas[linea.substr(0, linea.find('='))] 
                            = 
                            to_decimal(linea.substr(linea.find('=') + 1));

                        else
                            gl_etiquetas[linea] = i_PC;                                // Almacena la posición de la etiqueta                     
                    }

                    getline (f_entrada, linea);                                        // Lee la siguiente línea
                }



                // Comienza el ensamblado

                if (gl_logisim_out)                                                    // Imprime la cabecera de memoria de logisim
                {
                    f_salida << "v2.0 raw\n";
                }
                
                int contador = 0;                                                      // Contador de instrucciones
                for (instruccion *inst : codigo)                                       // Recorre la lista de instrucciones
                {   
                    if (gl_hex_out)
                    {
                        salida = inst->to_hex();                                       // Traduce la instrucción a hexadecimal
                    }
                    else
                    {
                        salida = inst->to_bin();                                       // Traduce la instrucción a binario
                    }

                    // Imprime la instrucción en la salida
                    if (gl_logisim_out)                                                // Código para rom de logisim
                        f_salida << salida << " ";

                    else if (gl_vhdl_out)
                        f_salida << "X\"" << salida << "\", ";

                    else                                                               // Código estándar
                        f_salida << salida << endl;

                    if (contador == 7 && (gl_logisim_out || gl_vhdl_out))              // Si se ha alcanzado el número máximo de instrucciones por línea
                    {
                        f_salida << endl;
                        contador = 0;
                    }
                    else
                        contador++;                                                    // Incrementa el contador de instrucciones
                }
            }
            catch (const exception& e)
            {
                cout << e.what() << endl;
            }
        }
        else if (!f_config.is_open())           // Fichero de configuración incorrecto 
            cerr << "No se ha encontrado el archivo " << argv[1] << endl;

        else if (!f_entrada.is_open())           // Fichero a ensamblar incorrecto 
            cerr << "No se ha encontrado el archivo " << argv[2] << endl;
        
        else if (!f_config.is_open())           // No se ha podido crear o escribir el en fichero 
            cerr << "No se ha podido escribir el fichero " << argv[3] << endl;
            
    }
    else                // Parámetros incorrectos
    {
        cerr << "Invocar como: ./cumpilador.exe fichero_config fichero_entrada fichero_salida" << endl;
    }
}
