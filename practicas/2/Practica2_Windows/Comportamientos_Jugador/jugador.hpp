#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <iostream>
#include <algorithm>
#include <vector>
#include <list>
#include <map>

struct estado {
  int fila;
  int columna;
  int orientacion;

  bool operator ==(const estado& other) const {
    return fila == other.fila && columna == other.columna && orientacion == other.orientacion;
  }
};

struct nodo{
	estado st;
	list<Action> secuencia;

	bool operator ==(const nodo& other) const {
        return st == other.st;
  }
};

inline void nullMap(char table[200][200], int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            table[i][j] = 0;
        }
    }
}

inline bool isMapEmpty(char table[200][200], int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (table[i][j] != 0)
                return false;
        }
    }
    return true;
}

class ComportamientoJugador : public Comportamiento {
  public:
    //level 2 vars
    char myMap[200][200];
    estado myMapState;
    estado pState;
    int offsetRow;
    int offsetCol;

    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
        // Inicializar Variables de Estado
        fil = col = 99;
        brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
        actual.orientacion = -1;
        destino.fila = -1;
        destino.columna = -1;
        destino.orientacion = -1;

        nullMap(myMap, 200);
        myMapState.columna = 100;
        myMapState.fila = 100;
        myMapState.orientacion = 0;
        pState.fila = -1;
        pState.columna = -1;
        offsetRow = 0; //myRow = row + offset (offset of 0 is imposible)
        offsetCol = 0;
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
        // Inicializar Variables de Estado
        fil = col = 99;
        brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
        actual.orientacion = -1;
        destino.fila = -1;
        destino.columna = -1;
        destino.orientacion = -1;

        nullMap(myMap, 200);
        myMapState.columna = 100;
        myMapState.fila = 100;
        myMapState.orientacion = 0;
        pState.fila = -1;
        pState.columna = -1;
        offsetRow = 0;
        offsetCol = 0;
    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void VisualizaPlan(const estado &st, const list<Action> &plan);
    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}
    void makePlan(Sensores);
  private:
    // Declarar Variables de Estado
    int fil, col, brujula;
    estado actual, destino;
    list<Action> plan;

    // Métodos privados de la clase
    bool pathFinding(int level, const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_BFS(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Dijkstra(const estado &origen, const estado &destino, list<Action> &plan);
    void PintaPlan(list<Action> plan);
    bool HayObstaculoDelante(estado &st);
    void refillResultMap(Sensores);
    void updateResultMap();
};

class NodeHeap {
    std::vector< std::pair< nodo, int> > heap;

    void sift_up(int start) {
        int child = start;
        int parent = child / 2;
        while (parent > 0 && heap[parent].second > heap[child].second) {
            std::swap(heap[child], heap[parent]);
            child = parent;
            parent /= 2;
        }
    }
    void sift_down() {
        int size = heap.size();
        int currInd = 1;
        int minChildInd = 0;
        bool foundPlace = false;
        while (2*currInd + 1 < size && !foundPlace) {
            minChildInd = (heap[2*currInd].second < heap[2*currInd +1].second) ?
                2*currInd : 2*currInd + 1;
            if (heap[minChildInd].second < heap[currInd].second) {
                std::swap(heap[currInd], heap[minChildInd]);
                currInd = minChildInd;
            }
            else foundPlace = true;
        }
        if (2*currInd + 1 == size) {
            if (heap[2*currInd].second < heap[currInd].second);
                std::swap(heap[currInd], heap[2*currInd]);
        }
    }
public:
    NodeHeap() {
        heap.reserve(40000);
    }
    void insert(nodo node, int price) {
        if (heap.size() == 0)
            heap.push_back(std::make_pair(node, price));
        heap.push_back(std::make_pair(node, price));
        sift_up(heap.size() - 1);
    }
    void decreasePriority(nodo node, int newPrice) {
        int size = heap.size();
        for (int i = 1; i < size; i++) {
            if (heap[i].first.st == node.st) {
                heap[i].second = newPrice;
                heap[i].first.secuencia = node.secuencia;
                sift_up(i);
                return;
            }
        }
        std::cout << "DECREASE PRIORITY ERROR: No such node\n";
    }
    void pop() {
        heap[1] = heap.back();
        heap.pop_back();
        sift_down();
    }
    std::pair<nodo, int> top() const {
        return heap[1];
    }
    bool empty() const {
        return heap.size() <= 1;
    }
    int getPrice(const nodo& node) const {
        int size = heap.size();
        for (int i = 1; i < size; i++) {
            if (heap[i].first.st == node.st)
                return heap[i].second;
        }
        return -1;
    }
    void print() const {
        int count = 0;
        int pow = 0;
        for (int ind = 1; ind < heap.size(); ind++) {
            std::cout << heap[ind].second << '(' << heap[ind].first.st.fila << ',' << heap[ind].first.st.columna<<','<<heap[ind].first.st.orientacion << ')' << ' ';
            count++;
            if (count == std::pow(2,pow)) {
                std::cout << std::endl;
                pow++;
                count = 0;
            }
        }
        if (count != 0)
            std::cout << std::endl;
    }
};

struct StateHasher {
  std::size_t operator()(const estado& st) const {
    return (st.orientacion + 1)*1000000 +
        st.fila*1000 +
        st.columna;
  }
};

struct NodeHasher {
  std::size_t operator()(const nodo& n) const {
    return (n.st.orientacion + 1)*1000000 +
        n.st.fila*1000 +
        n.st.columna;
  }
};

#endif
