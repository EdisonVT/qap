#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <limits>
#include <algorithm>
#include <chrono>

using namespace std;


struct Solucion {
    std::vector<int> orden;
    int costo;
};


int calDistancia(const vector<vector<int>>& distancia, int ubicacion1, int ubicacion2) {
    return distancia[ubicacion1][ubicacion2];
}

int calFlujo(const vector<vector<int>>& flujo, int  instalacion1, int  instalacion2) {
    return flujo[instalacion1][instalacion2];
}

int costo(int N, const vector<vector<int>>& distancia,const vector<vector<int>>& flujo, const vector<int>& orden) {
    int costo = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int dist = calDistancia(distancia, i, j);
            int flj = calFlujo(flujo, orden[i], orden[j]);
            int temp = dist * flj;
            costo += temp;
        }
    }
    return costo;
}

Solucion solucionInicial(int N, const vector<vector<int>>& distancia, const vector<vector<int>>& flujo) {
    std::vector<int> orden(N);
    std::vector<bool> asignado(N, false);
    for (int i = 0; i < N; i++) {
        int mejorUbicacion = -1;
        int minCosto = std::numeric_limits<int>::max();
        for (int j = 0; j < N; j++) {
            if (!asignado[j]) {
                int cost = costo(N, distancia, flujo, orden);
                if (cost < minCosto) {
                    minCosto = cost;
                    mejorUbicacion = j;
                }
            }
        }
        orden[i] = mejorUbicacion;
        asignado[mejorUbicacion] = true;
    }
    int cost = costo(N, distancia, flujo, orden);
    return {orden, cost};
}

// Función para generar una vecindad de soluciones mediante el intercambio de ubicaciones
std::vector<Solucion> generarNeighborhood(int N, const vector<vector<int>>& distancia, const vector<vector<int>>& flujo, const Solucion& actual) {
    std::vector<Solucion> neighborhood;
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            std::vector<int> nuevoOrden = actual.orden;
            std::swap(nuevoOrden[i], nuevoOrden[j]);
            int cost = costo(N, distancia, flujo, nuevoOrden);
            neighborhood.push_back({nuevoOrden, cost});
        }
    }
    return neighborhood;
}

Solucion mejorVecino(const std::vector<Solucion>& neighborhood, const std::vector<std::vector<int>>& tabuList) {
    Solucion mejorVecino = neighborhood[0];
    for (const Solucion& neighbor : neighborhood) {
        if (std::find(tabuList[neighbor.orden[0]].begin(), tabuList[neighbor.orden[0]].end(), neighbor.orden[1]) == tabuList[neighbor.orden[0]].end() &&
            std::find(tabuList[neighbor.orden[1]].begin(), tabuList[neighbor.orden[1]].end(), neighbor.orden[0]) == tabuList[neighbor.orden[1]].end() &&
            neighbor.costo < mejorVecino.costo) {
            mejorVecino=neighbor;
        }
    }
    return mejorVecino;
}


void actualizarTabuList(int N, std::vector<std::vector<int>>& tabuList, const Solucion& solucion) {
    const int tenure = 3; // Tamaño de la lista tabú
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (std::find(tabuList[i].begin(), tabuList[i].end(), j) != tabuList[i].end()) {
                tabuList[i].erase(std::find(tabuList[i].begin(), tabuList[i].end(), j));
            }
        }
    }
    tabuList[solucion.orden[0]].push_back(solucion.orden[1]);
    tabuList[solucion.orden[1]].push_back(solucion.orden[0]);
    if (tabuList[solucion.orden[0]].size() > tenure) {
        tabuList[solucion.orden[0]].erase(tabuList[solucion.orden[0]].begin());
    }
    if (tabuList[solucion.orden[1]].size() > tenure) {
        tabuList[solucion.orden[1]].erase(tabuList[solucion.orden[1]].begin());
    }
}

// Función de búsqueda tabú para resolver el problema de asignación cuadrática
Solucion tabuSearch(int N, const vector<vector<int>>& distancia, const vector<vector<int>>& flujo) {
    const int maxIterations = 100; // Número máximo de iteraciones
    
    //solucion inicial
    Solucion mejorSolucion = solucionInicial(N,distancia, flujo);
    Solucion actual = mejorSolucion;

    //lista tabu
    std::vector<std::vector<int>> tabuList(1);

    //busqueda greedy
    int iteration = 0;
    while (iteration < maxIterations) {

        //generar vecindad
        std::vector<Solucion> neighborhood = generarNeighborhood(N,distancia, flujo, actual);
        
        //obtener mejor vecino
        Solucion mejorvecino = mejorVecino(neighborhood, tabuList);
        
        //actualizar lista tabu
        actualizarTabuList(N, tabuList, mejorvecino);
        if (mejorvecino.costo < mejorSolucion.costo) {
            mejorSolucion = mejorvecino;
        }
        actual = mejorvecino;
        iteration++;
    }
    return mejorSolucion;
}

int main(int argc, char *args[]){

    //nombre archivo
    string archivo= args[1];

    std::cout << archivo+".dat";
    std::cout << "\n";

    ifstream File;
    File.open(archivo+".dat");

    //tamano instancia
    unsigned int n;
    File >> n;
    
    //matrices
    int data1 [n][n];
    int data2 [n][n];

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            File >> data1[i][j];
        }
    }
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            File >> data2[i][j];
        }
    }
    File.close();

    return 0;
}
