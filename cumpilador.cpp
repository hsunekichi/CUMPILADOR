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

using namespace std;



const bool HEX_OUT = 1;                 // Da la salida en hexadecimal en lugar de en binario
const bool LOGISIM_OUT = 1;             // Imprime la salida en un formato compatible con la rom de logisim
const int MAX_PARAMETROS = 4;           // Número máximo de tokens que puede tener una instrucción del repertorio (ADD r1, r2, r2 -> MAX_PARAMETROS = 4)



struct exception_wrong_instruction_syntax
{
    string instruction;
};



struct exception_wrong_number_of_parameters
{
    string instruction;
};



class instruccion
{
    public:

        virtual std::string&& to_string () = 0;                  // Devuelve la instrucción ensamblador, legible por humanos

        virtual std::string&& to_bin () = 0;                     // Ensambla la instrucción y la devuelve en binario, legible por la máquina

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

        ADD (ADD&& oldInst) : rdBin {oldInst.rdBin}, raBin {oldInst.raBin},  rbBin {oldInst.rbBin}{}              //Constructor de transferencia


        std::string&& to_string ()                                                                                // Devuelve la instrucción ensamblador, legible por humanos
        {
            return operacion + " r" + std::to_string(rdBin.to_ulong()) + " r" + std::to_string(raBin.to_ulong()) +  + " r" + std::to_string(rbBin.to_ulong());
        }

        std::string&& to_bin ()                                                                                   // Ensambla la instrucción y la devuelve en binario, legible por la máquina
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

        MOV (MOV&& oldInst) : rdBin {oldInst.rdBin}, kBin {oldInst.kBin}{}                  // Constructor de transferencia

        std::string&& to_string ()                                                          // Devuelve la instrucción ensamblador, legible por humanos
        {
            return operacion + " r" + std::to_string(rdBin.to_ulong()) + " #" + std::to_string(kBin.to_ulong());
        }

        std::string&& to_bin ()                                                             // Ensambla la instrucción y la devuelve en binario, legible por la máquina
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
        LW(string rd, string rs) : rdBin {stoi(rd.substr(1))}, rsBin {stoi(rs.substr(1))} {}
        LW(LW&& oldInst) : rdBin {oldInst.rdBin}, rsBin {oldInst.rsBin} {}      //Constructor de transferencia

        std::string&& to_string ()                                                                                // Devuelve la instrucción ensamblador, legible por humanos
        {
            return operacion + " r" + std::to_string(rdBin.to_ulong()) + " #" + std::to_string(rsBin.to_ulong());
        }

        std::string&& to_bin ()                                                                                //Ensambla la instrucción y la devuelve en binario, legible por la máquina
        {
            return operacionBin.to_string() + rdBin.to_string() + rsBin.to_string() + rellenoBin.to_string();
        }
};


// Factoría de instrucciones
// nParametros es el número de parámetros inc

instruccion&& crearInst (string parametros[MAX_PARAMETROS], int nParametros)
{
    if (parametros[0] == "ADD")                                                 // ADD
    {
        if (nParametros == 4)
        {
            return ADD (parametros[1], parametros[2], parametros[3]);
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
        return MOV (parametros[1], parametros[2]);
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

// Convierte un string binario en uno hexadecimal

string&& binToHex (string binario)
{
	string final = "";                              // String para rellenar con el hexadecimal
    string aux, relleno = "0000";                   // Strings auxiliares
    int bitsTot = binario.length();                 // Tamaño del binario
	int resto = bitsTot % 4;                        // Calcula el relleno que hace falta

    if (resto != 0)                                 // Se necesita relleno
    {
        relleno.erase(0, resto);                    // Crea el relleno necesario hasta que sea múltiplo de 4
	    binario = relleno + binario;                // Completa el binario con relleno
    }   

	for (int i = 0; i < bitsTot; i += 4)            // Convierte el string completo, de 4 en 4 bits
	{
		aux = binario.substr(i,4);

		if (aux == "0000")
		{
			final += "0";
		}
		else if (aux == "0001")
		{
			final += "1";
		}
		else if (aux == "0010")
		{
			final += "2";
		}
		else if (aux == "0011")
		{
			final += "3";
		}
		else if (aux == "0100")
		{
			final += "4";
		}
		else if (aux == "0101")
		{
			final += "5";
		}
		else if (aux == "0110")
		{
			final += "6";
		}
		else if (aux == "0111")
		{
			final += "7";
		}
		else if (aux == "1000")
		{
			final += "8";
		}
		else if (aux == "1001")
		{
			final += "9";
		}
		else if (aux == "1010")
		{
			final += "A";
		}
		else if (aux == "1011")
		{
			final += "B";
		}
		else if (aux == "1100")
		{
			final += "C";
		}
		else if (aux == "1101")
		{
			final += "D";
		}
		else if (aux == "1110")
		{
			final += "E";
		}
		else if (aux == "1111")
		{
			final += "F";
		}
	}

	return final;
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

        if (f_entrada.is_open() && f_salida.is_open())         // El fichero existía
        {
            string linea, inst[MAX_PARAMETROS];                         // Variables para leer y tokenizar la instrucción
            string etiqueta;                                            // Variable para leer etiquetas de salto
            bool vacio;                                                 // Finaliza el bucle de tokenizado de parámetros, cuando la instrucción tiene menos de MAX_PARAMETROS
            int i_PC = 0;                                               // Lleva la cuenta del número de línea para almacenar etiquetas de salto
            int posEspacio;                                             // Lleva la cuenta del número de línea para almacenar etiquetas de salto

            if (LOGISIM_OUT)                                            // Imprime la cabecera de memoria de logisim
            {
                f_salida << "v2.0 raw\n";
            }

            getline (f_entrada, linea);                                   // Lee la primera línea
            i_PC++;                                                     // Incrementa el contador de instrucción (Para etiquetas de salto)

            while (!f_entrada.eof())
            {
                if (linea != "" && linea.find(" ") != -1)                              // Es una instrucción 
                {
                    vacio = false;

                    for (int i = 0; !vacio; i++)                                   // Mientras queden parámetros
                    {
                        posEspacio = linea.find(" ");
                        inst [i] = linea.substr(0, posEspacio);                    // Almacena el parámetro hasta el primer espacio
                        
                        if (posEspacio != -1)                                      // Quedan parámetros
                        {
                            linea.erase(0, posEspacio + 1);                        // Elimina todo hasta el primer espacio, incluido
                        }
                        else
                        {
                            vacio = true;
                        }
                    }      

                    string salida = ensamblar (inst[0], inst[1], inst[2], inst[3]);           // Traduce la instrucción a binario

                    if (HEX_OUT)                                                // Transforma la instrucción en hexadecimal
                    {
                        salida = binToHex (salida);
                    }

                    if (LOGISIM_OUT)                                            // Código para rom de logisim
                    {
                        f_salida << salida << " ";
                    }
                    else                                                        // Código estándar
                    {
                        f_salida << salida << endl;
                    }
                    
                }   
                else if (linea != "")                                                   // Es una etiqueta de salto
                {
                    etiqueta = linea; 
                    // almacenar par (etiqueta, i_PC)                                    
                }

                getline (f_entrada, linea);                           // Lee la siguiente línea
                i_PC++;                                             // Incrementa el contador de instrucción (Para etiquetas de salto)
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