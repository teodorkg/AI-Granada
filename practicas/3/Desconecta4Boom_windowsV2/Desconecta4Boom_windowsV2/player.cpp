#include <iostream>
#include <cstdlib>
#include <vector>
#include <queue>
#include <stdlib.h>
#include <time.h>
#include <string>
#include "player.h"
#include "environment.h"

using namespace std;

const double masinf=9999999999.0, menosinf=-9999999999.0;

double alpha_beta_pruning(const Environment& actual_, int player, int currDepth, const int& PROFUNDIDAD_ALFABETA,
                       Environment::ActionType& action, double alpha, double beta);
double evaluatePosition(int player, const Environment& state);
void printMap(const Environment& state);

// Constructor
Player::Player(int jug){
    jugador_=jug;
}

// Actualiza el estado del juego para el jugador
void Player::Perceive(const Environment & env){
    actual_=env;
}

double Puntuacion(int jugador, const Environment &estado){
    double suma=0;

    for (int i=0; i<7; i++)
      for (int j=0; j<7; j++){
         if (estado.See_Casilla(i,j)==jugador){
            if (j<3)
               suma += j;
            else
               suma += (6-j);
         }
      }

    return suma;
}


// Funcion de valoracion para testear Poda Alfabeta
double ValoracionTest(const Environment &estado, int jugador){
    int ganador = estado.RevisarTablero();

    if (ganador==jugador)
       return 99999999.0; // Gana el jugador que pide la valoracion
    else if (ganador!=0)
            return -99999999.0; // Pierde el jugador que pide la valoracion
    else if (estado.Get_Casillas_Libres()==0)
            return 0;  // Hay un empate global y se ha rellenado completamente el tablero
    else
          return Puntuacion(jugador,estado);
}

// ------------------- Los tres metodos anteriores no se pueden modificar



// Funcion heuristica (ESTA ES LA QUE TENEIS QUE MODIFICAR)
double Valoracion(const Environment &estado, int jugador){
    if (estado.Get_Casillas_Libres()==0)
            return 0;  // Hay un empate global y se ha rellenado completamente el tablero
    else {
        double eval(0);
        switch (jugador) {
        case 1:
            eval = evaluatePosition(1, estado);
            if (eval != menosinf) {
                double evalEnemy = evaluatePosition(2, estado);
                eval -= double(evalEnemy);
            }
            break;
        case 2:
            eval = evaluatePosition(2, estado);
            if (eval != menosinf) {
                double evalEnemy = evaluatePosition(1, estado);
                eval -= double(evalEnemy);
            }
            break;
        }
        return eval;
    }
}





// Esta funcion no se puede usar en la version entregable
// Aparece aqui solo para ILUSTRAR el comportamiento del juego
// ESTO NO IMPLEMENTA NI MINIMAX, NI PODA ALFABETA
void JuegoAleatorio(bool aplicables[], int opciones[], int &j){
    j=0;
    for (int i=0; i<8; i++){
        if (aplicables[i]){
           opciones[j]=i;
           j++;
        }
    }
}






// Invoca el siguiente movimiento del jugador
Environment::ActionType Player::Think(){
    const int PROFUNDIDAD_MINIMAX = 6;  // Umbral maximo de profundidad para el metodo MiniMax
    const int PROFUNDIDAD_ALFABETA = 8; // Umbral maximo de profundidad para la poda Alfa_Beta

    Environment::ActionType accion; // acción que se va a devolver
    bool aplicables[8]; // Vector bool usado para obtener las acciones que son aplicables en el estado actual. La interpretacion es
                        // aplicables[0]==true si PUT1 es aplicable
                        // aplicables[1]==true si PUT2 es aplicable
                        // aplicables[2]==true si PUT3 es aplicable
                        // aplicables[3]==true si PUT4 es aplicable
                        // aplicables[4]==true si PUT5 es aplicable
                        // aplicables[5]==true si PUT6 es aplicable
                        // aplicables[6]==true si PUT7 es aplicable
                        // aplicables[7]==true si BOOM es aplicable



    double valor; // Almacena el valor con el que se etiqueta el estado tras el proceso de busqueda.
    double alpha, beta; // Cotas de la poda AlfaBeta

    int n_act; //Acciones posibles en el estado actual


    n_act = actual_.possible_actions(aplicables); // Obtengo las acciones aplicables al estado actual en "aplicables"
    int opciones[10];

    // Muestra por la consola las acciones aplicable para el jugador activo
    //actual_.PintaTablero();
    cout << " Acciones aplicables ";
    (jugador_==1) ? cout << "Verde: " : cout << "Azul: ";
    for (int t=0; t<8; t++)
      if (aplicables[t])
         cout << " " << actual_.ActionStr( static_cast< Environment::ActionType > (t)  );
    cout << endl;


    //--------------------- COMENTAR Desde aqui
    /*
    cout << "\n\t";
    int n_opciones=0;
    JuegoAleatorio(aplicables, opciones, n_opciones);

    if (n_act==0){
      (jugador_==1) ? cout << "Verde: " : cout << "Azul: ";
      cout << " No puede realizar ninguna accion!!!\n";
      //accion = Environment::actIDLE;
    }
    else if (n_act==1){
           (jugador_==1) ? cout << "Verde: " : cout << "Azul: ";
            cout << " Solo se puede realizar la accion "
                 << actual_.ActionStr( static_cast< Environment::ActionType > (opciones[0])  ) << endl;
            accion = static_cast< Environment::ActionType > (opciones[0]);

         }
         else { // Hay que elegir entre varias posibles acciones
            int aleatorio = rand()%n_opciones;
            cout << " -> " << actual_.ActionStr( static_cast< Environment::ActionType > (opciones[aleatorio])  ) << endl;
            accion = static_cast< Environment::ActionType > (opciones[aleatorio]);
         }
    */
    //--------------------- COMENTAR Hasta aqui


    //--------------------- AQUI EMPIEZA LA PARTE A REALIZAR POR EL ALUMNO ------------------------------------------------


    // Opcion: Poda AlfaBeta
    // NOTA: La parametrizacion es solo orientativa

    valor = alpha_beta_pruning(actual_, jugador_, 0, PROFUNDIDAD_ALFABETA, accion, menosinf, masinf);
    cout << "Valor MiniMax: " << valor << "  Accion: " << actual_.ActionStr(accion) << endl;

    return accion;
}

double alpha_beta_pruning(const Environment& actual_, int player, int currDepth, const int& maxDepth,
                       Environment::ActionType& action, double alpha, double beta) {

    if (currDepth == maxDepth || actual_.JuegoTerminado()) {
        return Valoracion(actual_, player);
    }

    bool* actions = new bool[8];
    int n = actual_.possible_actions(actions);
    Environment* V = new Environment[n];
    actual_.GenerateAllMoves(V);
    int bestAction(-1);
    int Vindex(n);
    if (currDepth % 2 == 0) { ///maximizing player
        double maxEval = menosinf;
        for (int i = 7; i >= 0; i--) {
            if (!actions[i]) {
                continue;
            }
            if (bestAction == -1) {
                bestAction = i;
            }
            Vindex--;
            double eval = alpha_beta_pruning(V[Vindex], player, currDepth + 1, maxDepth, action, alpha, beta);

            if (maxEval < eval) {
                maxEval = eval;
                bestAction = i;
            }
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                break;
            }
        }
        if (currDepth == 0) {
            action = Environment::ActionType(bestAction);
        }
        delete[] actions;
        delete[] V;
        return maxEval;
    }
    else { ///minimizing player
        double minEval = masinf;
        for (int i = 7; i >= 0; i--) {
            if (!actions[i]) {
                continue;
            }
            Vindex--;
            double eval = alpha_beta_pruning(V[Vindex], player, currDepth + 1, maxDepth, action, alpha, beta);

            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
        }
        delete[] actions;
        delete[] V;
        return minEval;
    }

}

double evaluatePosition(int player, const Environment& state) {
    int size = 7;
    int num1(0), num2(0), num3(0);
    int count2v(0), count3v(0), count4v(0); //vertically
    int count2h(0), count3h(0), count4h(0); //horizontally
    int count2dru(0), count3dru(0), count4dru(0); // diagonal rise up
    int count2drd(0), count3drd(0), count4drd(0); // diagonal rise down
    int count2ddu(0), count3ddu(0), count4ddu(0); // diagonal descend up
    int count2ddd(0), count3ddd(0), count4ddd(0); // diagonal descend down
    int cell;
    //printMap(state);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {

            ///horizontally
            cell = state.See_Casilla(i,j);
            if (cell == player || cell == player + 3) {
                num1++;
                count2h++;
                count3h++;
                count4h++;
                if (count2h == 2) {
                    num2++;
                    count2h = 1;
                    //cout<<"2 horizontal\n";
                }
                if (count3h == 3) {
                    num3++;
                    count3h = 2;
                    //cout<<"3 horizontal\n";
                }
                if (count4h == 4) {
                    //cout<<"4 horizontal\n";
                    return menosinf;
                }
            } else {
                count2h = 0;
                count3h = 0;
                count4h = 0;
            }

            ///vertically
            cell = state.See_Casilla(j,i);
            if (cell == player || cell == player + 3) {
                count2v++;
                count3v++;
                count4v++;
                if (count2v == 2) {
                    num2++;
                    count2v = 1;
                    //cout<<"2 vertical\n";
                }
                if (count3v == 3) {
                    num3++;
                    count3v = 2;
                    //cout<<"3 vertical\n";
                }
                if (count4v == 4) {
                    //cout<<"4 vertical\n";
                    return menosinf;
                }
            } else {
                count2v = 0;
                count3v = 0;
                count4v = 0;
            }

            ///Diagonal rise up
            if (6 - i - j >= 0) {
                cell = state.See_Casilla(6 - i - j, j);
                if (cell == player || cell == player + 3) {
                    count2dru++;
                    count3dru++;
                    count4dru++;
                    if (count2dru == 2) {
                        num2++;
                        count2dru = 1;
                        //cout<<"2 d-ru\n";
                    }
                    if (count3dru == 3) {
                        num3++;
                        count3dru = 2;
                        //cout<<"3 d-ru\n";
                    }
                    if (count4dru == 4) {
                        //cout<<"4 d-ru\n";
                        return menosinf;
                    }
                } else {
                    count2dru = 0;
                    count3dru = 0;
                    count4dru = 0;
                }
            }

            ///Diagonal rise down
            if (i + j < 7 && i != 0) {  /// we must skip the main diagonal we checked in Diagonal rise up
                cell = state.See_Casilla(i + j, 6 - j);
                if (cell == player || cell == player + 3) {
                    count2drd++;
                    count3drd++;
                    count4drd++;
                    if (count2drd == 2) {
                        num2++;
                        count2drd = 1;
                        //cout<<"2 d-rd\n";
                    }
                    if (count3drd == 3) {
                        num3++;
                        count3drd = 2;
                        //cout<<"3 d-rd\n";
                    }
                    if (count4drd == 4) {
                        //cout<<"4 d-rd\n";
                        return menosinf;
                    }
                } else {
                    count2drd = 0;
                    count3drd = 0;
                    count4drd = 0;
                }
            }

            ///Diagonal descend up
            if (i + j < 7) {
                cell = state.See_Casilla(j, i + j);
                if (cell == player || cell == player + 3) {
                    count2ddu++;
                    count3ddu++;
                    count4ddu++;
                    if (count2ddu == 2) {
                        num2++;
                        count2ddu = 1;
                        //cout<<"2 d-du\n";
                    }
                    if (count3ddu == 3) {
                        num3++;
                        count3ddu = 2;
                        //cout<<"3 d-du\n";
                    }
                    if (count4ddu == 4) {
                        //cout<<"4 d-du\n";
                        return menosinf;
                    }
                } else {
                    count2ddu = 0;
                    count3ddu = 0;
                    count4ddu = 0;
                }
            }

            ///Diagonal descend down
            if (i + j < 7 && i != 0) { /// we must skip the main diagonal we checked in Diagonal descend up
                cell = state.See_Casilla(i + j, j);
                if (cell == player || cell == player + 3) {
                    count2ddd++;
                    count3ddd++;
                    count4ddd++;
                    if (count2ddd == 2) {
                        num2++;
                        count2ddd = 1;
                        //cout<<"2 d-dd\n";
                    }
                    if (count3ddd == 3) {
                        num3++;
                        count3ddd = 2;
                        //cout<<"3 d-dd\n";
                    }
                    if (count4ddd == 4) {
                        //cout<<"4 d-dd\n";
                        return menosinf;
                    }
                } else {
                    count2ddd = 0;
                    count3ddd = 0;
                    count4ddd = 0;
                }
            }
        }
        count2h = 0; count3h = 0; count4h = 0;
        count2v = 0; count3v = 0; count4v = 0;
        count2dru = 0, count3dru = 0; count4dru = 0;
        count2drd = 0, count3drd = 0; count4drd = 0;
        count2ddu = 0, count3ddu = 0; count4ddu = 0;
        count2ddd = 0, count3ddd = 0; count4ddd = 0;
    }


    return -(num1 + num2*2 + num3*5);
}

void printMap(const Environment& state) {
    for (int i = 0; i < 7;cout<<std::endl && i++)
        for (int j = 0; j < 7; j++)
            cout<<(int)state.See_Casilla(i,j)<<' ';
}
