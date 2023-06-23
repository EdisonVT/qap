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
    std::vector<int> swap;
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
            int dist = calDistancia(distancia, orden[i], orden[j]);
            int flj = calFlujo(flujo, i, j);
            int temp = dist * flj;
            costo += temp;
        }
    }
    return costo;
}

Solucion solucionInicial(int N, const vector<vector<int>>& distancia, const vector<vector<int>>& flujo) {
    //lista con lugares de los objetos
    std::vector<int> orden(N);
    std::iota(orden.begin(), orden.end(), 0);

    //randomizar lista
    std::random_shuffle(orden.begin(), orden.end());

    std::cout << "Initial Solution: ";
    for (int i = 0; i < N; i++) {
        std::cout << orden[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "Cost: " << costo(N, distancia, flujo, orden) << std::endl;
    std::cout << "\n";

    //costo orden inicial
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
            std::vector<int> swap = {i, j};
            neighborhood.push_back({nuevoOrden, cost, swap});
        }
    }
    return neighborhood;
}

Solucion mejorVecino(const std::vector<Solucion>& neighborhood, const std::vector<std::vector<int>>& tabuList) {
    Solucion mejorVecino = neighborhood[0];
    for (const Solucion& neighbor : neighborhood) {
        if (std::find(tabuList[neighbor.swap[0]].begin(), tabuList[neighbor.swap[0]].end(), neighbor.swap[1]) == tabuList[neighbor.swap[0]].end() &&
            std::find(tabuList[neighbor.swap[1]].begin(), tabuList[neighbor.swap[1]].end(), neighbor.swap[0]) == tabuList[neighbor.swap[1]].end() &&
            neighbor.costo < mejorVecino.costo) {
            mejorVecino=neighbor;
        }
    }
    return mejorVecino;
}


void actualizarTabuList(int largo, std::vector<std::vector<int>>& tabuList, const Solucion& solucion) {
    
    //se agregan en la lista tabu (lista de listas) los intercambios de lugares
    tabuList.push_back(solucion.swap);

    //si los objetos que se movieron tienen mas de x movimientos tabues, se eliminan los mas antiguos
    if (tabuList.size() > largo) {
        tabuList.erase(tabuList.begin());
    }
}

// Función de búsqueda tabú para resolver el problema de asignación cuadrática
Solucion tabuSearch(int N, const vector<vector<int>>& distancia, const vector<vector<int>>& flujo) {
    const int maxIterations = 10; // Número máximo de iteraciones
    
    //solucion inicial
    Solucion mejorSolucion = solucionInicial(N,distancia, flujo);
    Solucion actual = mejorSolucion;

    //lista tabu
    std::vector<std::vector<int>> tabuList;

    //busqueda greedy
    int iteration = 0;
    while (iteration < maxIterations) {

        //generar vecindad
        std::vector<Solucion> neighborhood = generarNeighborhood(N,distancia, flujo, actual);
        
        //obtener mejor vecino
        Solucion mejorvecino = mejorVecino(neighborhood, tabuList);

        //actualizar lista tabu
        actualizarTabuList(3, tabuList, mejorvecino);
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

    //abrir archivo
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

    //pasar matrices a tipo vector
    std::vector<std::vector<int>> distancia;

    for (int i = 0; i < n; i++) {
        std::vector<int> row;
        for (int j = 0; j < n; j++) {
            row.push_back(data2[i][j]);
        }
        distancia.push_back(row);
    }

    std::vector<std::vector<int>> flujo;

    for (int i = 0; i < n; i++) {
        std::vector<int> row;
        for (int j = 0; j < n; j++) {
            row.push_back(data1[i][j]);
        }
        flujo.push_back(row);
    }

    //iniciar medida de tiempo
    auto start = std::chrono::steady_clock::now();
	std::srand(std::time(nullptr));

    //tabu search
    Solucion solucion = tabuSearch(n, distancia, flujo);

    //terminar medida de tiempo
    auto end = std::chrono::steady_clock::now();

    //calculo tiempo ejecucion
	auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	double runtime = double(diff.count()) * 0.000001;

    //entregar mejor solucion por consola
    std::cout << "Best Solution: ";
    for (int i = 0; i < n; i++) {
        std::cout << solucion.orden[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "Cost: " << solucion.costo << std::endl;

    //crear y escribir archivo output
    ofstream file;
    file.open(archivo+".out");
    file << to_string(solucion.costo)+" "+ to_string(runtime)+"\n";
    file << n;
    file << "\n";
    for (int i = 0; i < solucion.orden.size(); i++){
            file << solucion.orden[i];
            file << " ";
    }
    file.close();

    return 0;
}
