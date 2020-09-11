#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <fstream>
#include <ctime>

void refillMyMap(const Sensores& sensores, char myMap[200][200], const estado& myMapState);
bool level2_Dijkstra(const estado &origen, const estado &destino, list<Action> &plan, char myMap[][200]);
bool level2_Dijkstra_toFirstUnknown(const estado &origen, list<Action> &plan, char myMap[][200]);
bool level2_DFS(const estado &origen, const estado &destino, list<Action> &plan, char myMap[][200]);
bool level2_BFS(const estado &origen, const estado &destino, list<Action> &plan, char myMap[][200]);
bool level2_BFS_toFirstUnknown(const estado &origen, list<Action> &plan, char myMap[][200]);
void translateIndex(int indP, estado& myMapState, estado& pState);


bool isForwardOccupied(estado &st, char myMap[][200]);
bool EsObstaculo(unsigned char casilla);



void printMyMap(char myMap[][200]) {
    ofstream file("myMap.txt", std::ofstream::trunc);
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 200; j++) {
            if (myMap[i][j] != 0)
                file<<myMap[i][j]<<" ";
            else
                file<<' '<<" ";
        }
        file << std::endl;
    }
}

// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar.
Action ComportamientoJugador::think(Sensores sensores) {
	Action accion = actIDLE;
	// Estoy en el nivel 1

	if (sensores.nivel != 4){
		if (sensores.mensajeF != -1){
			fil = sensores.mensajeF;
			col = sensores.mensajeC;

			actual.fila = fil;
			actual.columna = col;
			if (actual.orientacion == -1)
                actual.orientacion = 0;
		}

        if (!plan.empty() && sensores.destinoC == destino.columna && sensores.destinoF == destino.fila) {
            accion = plan.front();
            switch (accion) {
            case actFORWARD:
                switch (actual.orientacion) {
                    case 0: actual.fila--; break;
                    case 1: actual.columna++; break;
                    case 2: actual.fila++; break;
                    case 3: actual.columna--; break;
                    cout<< "BAD ORIENTATION : " << actual.orientacion << endl;
                }
                break;
            case actTURN_R:
                actual.orientacion = (actual.orientacion + 1) % 4;
                break;
            case actTURN_L:
                actual.orientacion = (actual.orientacion + 3) % 4;
                break;
            }
            plan.pop_front();
        }
		else {
            if (sensores.destinoC != destino.columna || sensores.destinoF != destino.fila) {
                destino.fila = sensores.destinoF;
                destino.columna = sensores.destinoC;
                pathFinding (sensores.nivel, actual, destino, plan);
            }

		}
	}
	else {
		// Estoy en el nivel 4
        if (offsetCol == 0)
            refillMyMap(sensores, myMap, myMapState);
        else
            refillResultMap(sensores);

        if (offsetCol == 0 && sensores.mensajeC != -1) {
            offsetRow = myMapState.fila - sensores.mensajeF;
            offsetCol = myMapState.columna - sensores.mensajeC;
            actual.fila = sensores.mensajeF;
            actual.columna = sensores.mensajeC;
            destino.fila = sensores.destinoF;
            destino.columna = sensores.destinoC;
            actual.orientacion = myMapState.orientacion;
            updateResultMap();
        }
        else if (pState.fila == -1) {
            int indP;
            for (indP = 0; indP < 16 && sensores.terreno[indP] != 'K'; indP++);

            if (indP != 16) {
                std::cout<<"see point of reference\n";
                //see Point of reference
                translateIndex(indP, myMapState, pState);
                //update Point of reference
                plan.clear();
            }
        }

        if (offsetRow != 0 &&
            (sensores.destinoC != destino.columna || sensores.destinoF != destino.fila)) {
            destino.fila = sensores.destinoF;
            destino.columna = sensores.destinoC;
            plan.clear();
        }

		if (plan.empty()) {
            makePlan(sensores);
		}

		accion = plan.front();
		plan.pop_front();

		if (offsetCol == 0) {
            //working with myMap
            switch (accion) {
            case actFORWARD:
                if (EsObstaculo(sensores.terreno[2])) {
                    accion = actTURN_R;
                    myMapState.orientacion = (myMapState.orientacion + 1) % 4;
                    plan.clear();
                    break;
                }
                if (sensores.superficie[2] == 'a') {
                    plan.push_front(accion);
                    accion = actIDLE;
                    break;
                }
                switch (myMapState.orientacion) {
                    case 0: myMapState.fila--; break;
                    case 1: myMapState.columna++; break;
                    case 2: myMapState.fila++; break;
                    case 3: myMapState.columna--; break;
                }
                break;
            case actTURN_R:
                myMapState.orientacion = (myMapState.orientacion + 1) % 4;
                break;
            case actTURN_L:
                myMapState.orientacion = (myMapState.orientacion + 3) % 4;
                break;
            }
		}
		else {
            //working with mapaResultado
            switch (accion) {
            case actFORWARD:
                if (EsObstaculo(sensores.terreno[2])) {
                    accion = actIDLE;
                    plan.clear();
                    break;
                }
                if (sensores.superficie[2] == 'a') {
                    plan.push_front(accion);
                    accion = actIDLE;
                    break;
                }
                switch (actual.orientacion) {
                    case 0: actual.fila--; break;
                    case 1: actual.columna++; break;
                    case 2: actual.fila++; break;
                    case 3: actual.columna--; break;
                }
                break;
            case actTURN_R:
                actual.orientacion = (actual.orientacion + 1) % 4;
                break;
            case actTURN_L:
                actual.orientacion = (actual.orientacion + 3) % 4;
                break;
            }

		}
	}

    return accion;
}


// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding (int level, const estado &origen, const estado &destino, list<Action> &plan){
	switch (level){
		case 1: cout << "Busqueda en profundad\n";
            return pathFinding_Profundidad(origen,destino,plan);
            break;
		case 2: cout << "Busqueda en Anchura\n";
            return pathFinding_BFS(origen,destino,plan);
            break;
		case 3: cout << "Busqueda Costo Uniforme\n";
            return pathFinding_Dijkstra(origen, destino, plan);
            break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}


//---------------------- Implementación de la busqueda en profundidad ---------------------------

// Dado el código en carácter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M' or casilla =='D')
		return true;
	else
	  return false;
}


// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
        default: cout<< "BAD ORIENTATION : " << actual.orientacion << endl;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
        st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}



struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};


// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	stack<nodo> pila;											// Lista de Abiertos

    nodo current;
	current.st = origen;
	current.secuencia.empty();

	pila.push(current);

    while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

bool ComportamientoJugador::pathFinding_BFS(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	queue<nodo> cola;					// Lista de Abiertos

    nodo current;
	current.st = origen;
	current.secuencia.empty();

	cola.push(current);

    while (!cola.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		cola.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			cola.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			cola.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				cola.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la cola
		if (!cola.empty()){
			current = cola.front();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

bool ComportamientoJugador::pathFinding_Dijkstra(const estado &origen, const estado &destino, list<Action> &plan) {
    clock_t begin = clock();
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	unordered_map<estado, int, StateHasher> closed;             // map de Cerrados
	unordered_map<estado, int, StateHasher> discovered;         // map de Abiertos
	NodeHeap discoveredHeap;			                        // pirámide de Abiertos

    nodo currentNode;
	currentNode.st = origen;
	currentNode.secuencia.empty();
    int inWhile = 0;
	int currentPrice = 0;
	discoveredHeap.insert(currentNode, currentPrice);
	discovered.insert({currentNode.st, currentPrice});

    while (!discoveredHeap.empty() and (currentNode.st.fila!=destino.fila or currentNode.st.columna != destino.columna)){
        discoveredHeap.pop();
        discovered.erase(currentNode.st);
		closed.insert({currentNode.st, currentPrice});

		// Generar descendiente de girar a la derecha
		int hijoPrice;
		nodo hijoTurnR = currentNode;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (discovered.count(hijoTurnR.st) > 0)
            hijoPrice = discovered[hijoTurnR.st];
        else
            hijoPrice = -1;
		if (hijoPrice == -1){
            if (closed.find(hijoTurnR.st) == closed.end()) {
                hijoTurnR.secuencia.push_back(actTURN_R);
                discoveredHeap.insert(hijoTurnR, currentPrice + 1);
                discovered.insert({hijoTurnR.st, currentPrice + 1});
            }
		} else if (currentPrice + 1 < hijoPrice) {
            hijoTurnR.secuencia.push_back(actTURN_R);
			discoveredHeap.decreasePriority(hijoTurnR, currentPrice + 1);
			discovered[hijoTurnR.st] = currentPrice + 1;
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = currentNode;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (discovered.count(hijoTurnL.st) > 0)
            hijoPrice = discovered[hijoTurnL.st];
        else
            hijoPrice = -1;
		if (hijoPrice == -1){
            if (closed.find(hijoTurnL.st) == closed.end()) {
                hijoTurnL.secuencia.push_back(actTURN_L);
                discoveredHeap.insert(hijoTurnL, currentPrice + 1);
                discovered.insert({hijoTurnL.st, currentPrice + 1});
            }
		} else if (currentPrice + 1 < hijoPrice) {
            hijoTurnL.secuencia.push_back(actTURN_L);
			discoveredHeap.decreasePriority(hijoTurnL, currentPrice + 1);
			discovered[hijoTurnL.st] = currentPrice + 1;
		}

		// Generar descendiente de avanzar
		nodo hijoForward = currentNode;
		int newPrice = currentPrice;
		switch(mapaResultado[hijoForward.st.fila][hijoForward.st.columna]) {
            case 'T': newPrice += 2; break;
            case 'B': newPrice += 5; break;
            case 'A': newPrice += 10; break;
            case '?': newPrice += 4; break;
            default: newPrice += 1;
		}
		if (!HayObstaculoDelante(hijoForward.st)) {
            if (discovered.count(hijoForward.st) > 0)
                hijoPrice = discovered[hijoForward.st];
            else
                hijoPrice = -1;
			if (hijoPrice == -1){
                if (closed.find(hijoForward.st) == closed.end()) {
                    hijoForward.secuencia.push_back(actFORWARD);
                    discoveredHeap.insert(hijoForward, newPrice);
                    discovered.insert({hijoForward.st, newPrice});
                }
            } else if (newPrice < hijoPrice) {
                hijoForward.secuencia.push_back(actFORWARD);
                discoveredHeap.decreasePriority(hijoForward, newPrice);
                discovered.insert({hijoForward.st, newPrice});
            }
		}

		// Tomo el siguiente valor de la cola
		if (!discoveredHeap.empty()){
			currentNode = discoveredHeap.top().first;
			currentPrice = discoveredHeap.top().second;
		}
	}
    clock_t end = clock();
    cout << "Terminada la busqueda en " << double(end - begin) / CLOCKS_PER_SEC << endl;

	if (currentNode.st.fila == destino.fila and currentNode.st.columna == destino.columna){
		plan = currentNode.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
        cout << "Precio del plan: " << currentPrice << endl;

		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

// Sacar por la términal la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}

void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}

// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
    AnularMatriz(mapaConPlan);
    estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}

int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}

//level 2

void translateIndex(int indP, estado& myMapState, estado& pState) {
    int row = myMapState.fila;
    int col = myMapState.columna;
    int i;
    if (indP == 0)
        pState = myMapState;
    switch (myMapState.orientacion) {
    case 0:
        if (indP - 1 < 3) {
            row = row - 1;
            col = col - 1 + (indP - 1);
        }
        else if (indP - 4 < 5) {
            row = row - 2;
            col = col - 2 + (indP - 4);;
        }
        else if (indP - 9 < 7)
            row = row - 3;
            col = col - 3 + (indP - 9);
        break;
    case 1:
        if (indP - 1 < 3) {
            row = row - 1 + (indP - 1);
            col = col + 1;
        }
        else if (indP - 4 < 5) {
           row = row - 2 + (indP - 4);
           col = col + 2;
        }
        else if (indP - 9 < 7) {
            row = row - 3 + (indP - 9);
            col = col + 3;
        }
        break;
    case 2:
        if (indP - 1 < 3) {
           row = row + 1;
           col = col + 1 - (indP - 1);
        }
        else if (indP - 4 < 5) {
            row = row + 2;
            col = col + 2 - (indP - 4);
        }
        else if (indP - 9 < 7) {
            row = row + 3;
            col = col + 3 - (indP - 9);
        }
        break;
    case 3:
        if (indP - 1 < 3) {
           row = row + 1 - (indP - 1);
           col = col - 1;
        }
        else if (indP - 4 < 5) {
            row = row + 2 - (indP - 4);
            col = col - 2;
        }
        else if (indP - 9 < 7) {
            row = row + 3 - (indP - 9);
            col = col - 3;
        }
        break;
    }
    pState.fila = row;
    pState.columna = col;
    pState.orientacion = 0;
}


void refillMyMap(const Sensores& sensores, char myMap[200][200], const estado& myMapState) {
    int row = myMapState.fila;
    int col = myMapState.columna;
    switch (myMapState.orientacion) {
    case 0:
        myMap[row][col] = sensores.terreno[0];
        for(int i = 0; i < 3; i++)
            myMap[row - 1][col - 1 + i] = sensores.terreno[1 + i];
        for(int i = 0; i < 5; i++)
            myMap[row - 2][col - 2 + i] = sensores.terreno[4 + i];
        for(int i = 0; i < 7; i++)
            myMap[row - 3][col - 3 + i] = sensores.terreno[9 + i];
        break;
    case 1:
        myMap[row][col] = sensores.terreno[0];
        for(int i = 0; i < 3; i++)
            myMap[row - 1 + i][col + 1] = sensores.terreno[1 + i];
        for(int i = 0; i < 5; i++)
            myMap[row - 2 + i][col + 2] = sensores.terreno[4 + i];
        for(int i = 0; i < 7; i++)
            myMap[row - 3 + i][col + 3] = sensores.terreno[9 + i];
        break;
    case 2:
        myMap[row][col] = sensores.terreno[0];
        for(int i = 0; i < 3; i++)
            myMap[row + 1][col + 1 - i] = sensores.terreno[1 + i];
        for(int i = 0; i < 5; i++)
            myMap[row + 2][col + 2 - i] = sensores.terreno[4 + i];
        for(int i = 0; i < 7; i++)
            myMap[row + 3][col + 3 - i] = sensores.terreno[9 + i];
        break;
    case 3:
        myMap[row][col] = sensores.terreno[0];
        for(int i = 0; i < 3; i++)
            myMap[row + 1 - i][col - 1] = sensores.terreno[1 + i];
        for(int i = 0; i < 5; i++)
            myMap[row + 2 - i][col - 2] = sensores.terreno[4 + i];
        for(int i = 0; i < 7; i++)
            myMap[row + 3 - i][col - 3] = sensores.terreno[9 + i];
        break;
    }
}


void ComportamientoJugador::makePlan(Sensores sensores) {
    if (offsetRow != 0) {
        //have The offsets that differs myMap from the real map
        if (actual.fila == sensores.destinoF && actual.columna == sensores.destinoC)
            plan.push_back(actIDLE);
        else {
            destino.fila = sensores.destinoF;
            destino.columna = sensores.destinoC;
            pathFinding_Dijkstra(actual, destino, plan);
        }
        return;
    }
    if (pState.fila == -1) {
        //have not seen Point of reference
        level2_Dijkstra_toFirstUnknown(myMapState, plan, myMap);
        /*
        estado exploreDFS;
        exploreDFS.columna=199;
        exploreDFS.fila=199;
        exploreDFS.orientacion = 0;
        std::cout<<"Exploring\n";
        level2_DFS(myMapState, exploreDFS, plan, myMap);
        */
    }
    else {
        //have seen point of reference
        std::cout<<"have seen point of reference\n";
        if(!level2_BFS(myMapState, pState, plan, myMap)) {
            std::cout<<"BFS failed to find a way to K\n";
            pState.fila = -1;
            pState.columna = -1;
            plan.clear();
            plan.push_back(actTURN_R);
        }
    }
}

bool isForwardOccupied(estado &st, char myMap[][200]){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=200) return true;
	if (col<0 or col>=200) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(myMap[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
        st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}

bool level2_Dijkstra(const estado &origen, const estado &destino, list<Action> &plan, char myMap[][200]) {
	//Borro la lista
	cout << "Calculando plan Dijkstra\n";
	plan.clear();
	unordered_map<estado, int, StateHasher> closed;             // Lista de Cerrados
	NodeHeap discoveredHeap;			                        // Piramide de Abiertos

    nodo currentNode;
	currentNode.st = origen;
	currentNode.secuencia.empty();
    int inWhile = 0;
	int currentPrice = 0;
	discoveredHeap.insert(currentNode, currentPrice);

    while (!discoveredHeap.empty() and (currentNode.st.fila!=destino.fila or currentNode.st.columna != destino.columna)){
        discoveredHeap.pop();
		closed.insert({currentNode.st, currentPrice});
		// Generar descendiente de girar a la derecha
		int hijoPrice;
		nodo hijoTurnR = currentNode;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		hijoPrice = discoveredHeap.getPrice(hijoTurnR);
		if (hijoPrice == -1){
            if (closed.find(hijoTurnR.st) == closed.end()) {
                hijoTurnR.secuencia.push_back(actTURN_R);
                discoveredHeap.insert(hijoTurnR, currentPrice + 1);
            }
		} else if (currentPrice + 1 < hijoPrice) {
            hijoTurnR.secuencia.push_back(actTURN_R);
			discoveredHeap.decreasePriority(hijoTurnR, currentPrice + 1);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = currentNode;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		hijoPrice = discoveredHeap.getPrice(hijoTurnL);
		if (hijoPrice == -1){
            if (closed.find(hijoTurnL.st) == closed.end()) {
                hijoTurnL.secuencia.push_back(actTURN_L);
                discoveredHeap.insert(hijoTurnL, currentPrice + 1);
            }
		} else if (currentPrice + 1 < hijoPrice) {
            hijoTurnL.secuencia.push_back(actTURN_L);
			discoveredHeap.decreasePriority(hijoTurnL, currentPrice + 1);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = currentNode;
		int newPrice = currentPrice;
		switch(myMap[hijoForward.st.fila][hijoForward.st.columna]) {
            case 'T': newPrice += 2; break;
            case 'B': newPrice += 5; break;
            case 'A': newPrice += 10; break;
            case 0: newPrice += 5; break;
            default: newPrice += 1;
		}
		if (!isForwardOccupied(hijoForward.st, myMap)) {
            hijoPrice = discoveredHeap.getPrice(hijoForward);
			if (hijoPrice == -1){
                if (closed.find(hijoForward.st) == closed.end()) {
                    hijoForward.secuencia.push_back(actFORWARD);
                    discoveredHeap.insert(hijoForward, newPrice);
                }
            } else if (newPrice < hijoPrice) {
                hijoForward.secuencia.push_back(actFORWARD);
                discoveredHeap.decreasePriority(hijoForward, newPrice);
            }
		}

		// Tomo el siguiente valor de la cola
		if (!discoveredHeap.empty()){
			currentNode = discoveredHeap.top().first;
			currentPrice = discoveredHeap.top().second;
		}
	}

	if (currentNode.st.fila == destino.fila and currentNode.st.columna == destino.columna){
		plan = currentNode.secuencia;
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

bool level2_DFS(const estado &origen, const estado &destino, list<Action> &plan, char myMap[][200]) {
	//Borro la lista
	cout << "Calculando plan DFS\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	stack<nodo> pila;											// Lista de Abiertos

  nodo current;
	current.st = origen;
	current.secuencia.empty();

	pila.push(current);

    while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!isForwardOccupied(hijoForward.st, myMap)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}
	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		plan = current.secuencia;
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}

	return false;
}

bool level2_BFS(const estado &origen, const estado &destino, list<Action> &plan, char myMap[][200]) {

	//Borro la lista
	cout << "Calculando plan BFS\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	queue<nodo> cola;					// Lista de Abiertos

    nodo current;
	current.st = origen;
	current.secuencia.empty();

	cola.push(current);

    while (!cola.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		cola.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			cola.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			cola.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!isForwardOccupied(hijoForward.st, myMap)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				cola.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la cola
		if (!cola.empty()){
			current = cola.front();
		}
	}

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

bool level2_BFS_toFirstUnknown(const estado &origen, list<Action> &plan, char myMap[][200]) {
    //Borro la lista
	cout << "Calculando plan BFS Explore\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	queue<nodo> cola;					// Lista de Abiertos

    nodo current;
	current.st = origen;
	current.secuencia.empty();

	cola.push(current);

    while (!cola.empty() and (myMap[current.st.fila][current.st.columna] != 0)){

		cola.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			cola.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			cola.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!isForwardOccupied(hijoForward.st, myMap)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				cola.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la cola
		if (!cola.empty()){
			current = cola.front();
		}
	}

	if (myMap[current.st.fila][current.st.columna] == 0){
		plan = current.secuencia;
		return true;
	}
	else {
		cout << "No encontrado cajilla no observada\n";
	}


	return false;
}

void ComportamientoJugador::updateResultMap() {
    int rows = mapaResultado.size();
    int cols = mapaResultado[0].size();
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (myMap[i+offsetRow][j+offsetCol] != 0)
                mapaResultado[i][j] = myMap[i+offsetRow][j+offsetCol];
}

void ComportamientoJugador::refillResultMap(Sensores sensores) {
    int row = actual.fila;
    int col = actual.columna;
    switch (actual.orientacion) {
    case 0:
        mapaResultado[row][col] = sensores.terreno[0];
        for(int i = 0; i < 3; i++)
            mapaResultado[row - 1][col - 1 + i] = sensores.terreno[1 + i];
        for(int i = 0; i < 5; i++)
            mapaResultado[row - 2][col - 2 + i] = sensores.terreno[4 + i];
        for(int i = 0; i < 7; i++)
            mapaResultado[row - 3][col - 3 + i] = sensores.terreno[9 + i];
        break;
    case 1:
        mapaResultado[row][col] = sensores.terreno[0];
        for(int i = 0; i < 3; i++)
            mapaResultado[row - 1 + i][col + 1] = sensores.terreno[1 + i];
        for(int i = 0; i < 5; i++)
            mapaResultado[row - 2 + i][col + 2] = sensores.terreno[4 + i];
        for(int i = 0; i < 7; i++)
            mapaResultado[row - 3 + i][col + 3] = sensores.terreno[9 + i];
        break;
    case 2:
        mapaResultado[row][col] = sensores.terreno[0];
        for(int i = 0; i < 3; i++)
            mapaResultado[row + 1][col + 1 - i] = sensores.terreno[1 + i];
        for(int i = 0; i < 5; i++)
            mapaResultado[row + 2][col + 2 - i] = sensores.terreno[4 + i];
        for(int i = 0; i < 7; i++)
            mapaResultado[row + 3][col + 3 - i] = sensores.terreno[9 + i];
        break;
    case 3:
        mapaResultado[row][col] = sensores.terreno[0];
        for(int i = 0; i < 3; i++)
            mapaResultado[row + 1 - i][col - 1] = sensores.terreno[1 + i];
        for(int i = 0; i < 5; i++)
            mapaResultado[row + 2 - i][col - 2] = sensores.terreno[4 + i];
        for(int i = 0; i < 7; i++)
            mapaResultado[row + 3 - i][col - 3] = sensores.terreno[9 + i];
        break;
    }
}

bool level2_Dijkstra_toFirstUnknown(const estado &origen, list<Action> &plan, char myMap[][200]) {
    //Borro la lista
	cout << "Calculando plan Dijkstra Explore\n";
	plan.clear();
	unordered_map<estado, int, StateHasher> closed;             // Lista de Cerrados
	NodeHeap discoveredHeap;			                        // Piramide de Abiertos

    nodo currentNode;
	currentNode.st = origen;
	currentNode.secuencia.empty();
    int inWhile = 0;
	int currentPrice = 0;
	discoveredHeap.insert(currentNode, currentPrice);

    while (!discoveredHeap.empty() and (myMap[currentNode.st.fila][currentNode.st.columna] != 0)){
        discoveredHeap.pop();
		closed.insert({currentNode.st, currentPrice});
		// Generar descendiente de girar a la derecha
		int hijoPrice;
		nodo hijoTurnR = currentNode;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		hijoPrice = discoveredHeap.getPrice(hijoTurnR);
		if (hijoPrice == -1){
            if (closed.find(hijoTurnR.st) == closed.end()) {
                hijoTurnR.secuencia.push_back(actTURN_R);
                discoveredHeap.insert(hijoTurnR, currentPrice + 1);
            }
		} else if (currentPrice + 1 < hijoPrice) {
            hijoTurnR.secuencia.push_back(actTURN_R);
			discoveredHeap.decreasePriority(hijoTurnR, currentPrice + 1);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = currentNode;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		hijoPrice = discoveredHeap.getPrice(hijoTurnL);
		if (hijoPrice == -1){
            if (closed.find(hijoTurnL.st) == closed.end()) {
                hijoTurnL.secuencia.push_back(actTURN_L);
                discoveredHeap.insert(hijoTurnL, currentPrice + 1);
            }
		} else if (currentPrice + 1 < hijoPrice) {
            hijoTurnL.secuencia.push_back(actTURN_L);
			discoveredHeap.decreasePriority(hijoTurnL, currentPrice + 1);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = currentNode;
		int newPrice = currentPrice;
		switch(myMap[hijoForward.st.fila][hijoForward.st.columna]) {
            case 'T': newPrice += 2; break;
            case 'B': newPrice += 5; break;
            case 'A': newPrice += 10; break;
            case 0: newPrice += 5; break;
            default: newPrice += 1;
		}
		if (!isForwardOccupied(hijoForward.st, myMap)) {
            hijoPrice = discoveredHeap.getPrice(hijoForward);
			if (hijoPrice == -1){
                if (closed.find(hijoForward.st) == closed.end()) {
                    hijoForward.secuencia.push_back(actFORWARD);
                    discoveredHeap.insert(hijoForward, newPrice);
                }
            } else if (newPrice < hijoPrice) {
                hijoForward.secuencia.push_back(actFORWARD);
                discoveredHeap.decreasePriority(hijoForward, newPrice);
            }
		}

		// Tomo el siguiente valor de la cola
		if (!discoveredHeap.empty()){
			currentNode = discoveredHeap.top().first;
			currentPrice = discoveredHeap.top().second;
		}
	}

	if (myMap[currentNode.st.fila][currentNode.st.columna] == 0){
		plan = currentNode.secuencia;
		return true;
	}
	else {
		cout << "No hay mas que explorar\n";
	}

	return false;
}
