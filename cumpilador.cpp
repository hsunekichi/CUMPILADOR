#include <iostream>
#include <fstream>
#include <string>
#include <bitset>

using namespace std;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Sintaxis de instrucciones: INST PARAM1 PARAM2 PARAM3
 * Ej: ADD r1 r2 r2        
 * Actualmente el compilador solo admite instrucciones de 3 parámetros (el MOV debe tener un parámetro basura). 
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






class instruccion
{
    private:


    public:
        std::string to_string ()
        {
            
        }
};





// Traduce los parámetros a una instrucción en binario completa.
// Modificar aquí la sintaxis del ensamblador

string tradBinario (string inst, string param1, string param2, string param3)        //Necesitará tener acceso al diccionario de etiquetas de salto
{   
    string instruccion = "";

    if (inst == "ADD")
    {

        string reg1 = param1.substr(1);             // Elimina la r del número de registro (r1 -> 1)
        string reg2 = param2.substr(1);             // Elimina la r del número de registro (r1 -> 1)
        string reg3 = param3.substr(1);             // Elimina la r del número de registro (r1 -> 1)

        bitset<6> instBin {"000001"};                // Código de la instrucción
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
        string reg1 = param1.substr(1);             // Elimina la r del número de registro (r1 -> 1)

        bitset<6> instBin {"000000"};
        bitset<5> param1Bin {stoi(reg1)};         // Parámetro 1
        bitset<16> param2Bin {stoi(param2)};         // Parámetro 2

        string relleno = "";
        for (int i = 0; i < 5; i++) relleno += "0";            // Crea el relleno para completar la instrucción

        // Genera la instrucción completa
        instruccion = instBin.to_string() + param1Bin.to_string() + param2Bin.to_string() + relleno ;        
    }

    return instruccion;
}





int main(int argc, char * argv[])
{
    ifstream entrada;
    
    if (argc == 2)                          // Se ha introducido un parámetro
    {
        entrada.open(argv[1]);

        if (entrada.is_open())              // El fichero existía
        {
            string linea, inst, param1, param2, param3;                 // Variables para leer y tokenizar la instrucción
            string etiqueta;                                            // Variable para leer etiquetas de salto
            int i_PC = 0;                                               // Lleva la cuenta del número de línea para almacenar etiquetas de salto

            getline (entrada, linea);                                   // Lee la primera línea
            i_PC++;                                                     // Incrementa el contador de instrucción (Para etiquetas de salto)

            while (!entrada.eof())
            {
                if (linea.find(" ") != -1)                              // Es una instrucción 
                {
                    inst = linea.substr(0, linea.find(" "));                    // Almacena la operación
                    linea = linea.substr(linea.find(" ") + 1);

                    param1 = linea.substr(0, linea.find(" "));                  // Almacena el primer parámetro
                    linea = linea.substr(linea.find(" ") + 1);

                    param2 = linea.substr(0, linea.find(" "));                  // Almacena el segundo parámetro
                    linea = linea.substr(linea.find(" ") + 1);

                    param3 = linea;                                             // Almacena el tercer parámetro

                    string salida = tradBinario (inst, param1, param2, param3);

                    cout << salida << endl;
                }   
                else                                                    // Es una etiqueta de salto
                {
                    etiqueta = linea; 
                    // almacenar par (etiqueta, i_PC)                                    
                }

                getline (entrada, linea);                           // Lee la siguiente línea
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
        cerr << "Invocar como: ./cumpilador.exe nombre_fichero" << endl;
    }
}