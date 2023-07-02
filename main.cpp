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

int costo_inicial(int N, const vector<vector<int>>& distancia,const vector<vector<int>>& flujo, const vector<int>& orden, const vector<bool>& asignado) {
    int cost = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (asignado[orden[i]] && asignado[orden[j]]){
                int dist = calDistancia(distancia, orden[i], orden[j]);
                int flj = calFlujo(flujo, i, j);
                int temp = dist * flj;
                cost += temp;
            }
        }
    }
    if (std::count (asignado.begin(), asignado.end(), true)<2){
        cost = costo(N, distancia, flujo, orden);
    }
    return cost;
}

Solucion solucionInicial(int N, const vector<vector<int>>& distancia, const vector<vector<int>>& flujo) {
    std::vector<int> orden(N);
    std::vector<bool> asignado(N, false);

    //vectores con las sumas de flujos y distancias totales
    std::vector<int> flowSum;
	std::vector<int> distSum;

	for (unsigned int i = 0; i < flujo.size(); i++) {
		int total = 0;
		for (unsigned int j = 0; j < flujo[i].size(); j++) {
			total += flujo[i][j];
		}
		flowSum.push_back(total);
	}
	for (unsigned int i = 0; i < distancia.size(); i++) {
		int total = 0;
		for (unsigned int j = 0; j < distancia[i].size(); j++) {
			total += distancia[i][j];
		}
		distSum.push_back(total);
	}

    //calculo de minimo flujo y maxima distancia
	std::vector<int>::iterator fmin = std::min_element(flowSum.begin(), flowSum.end());
	std::vector<int>::iterator dmax = std::max_element(distSum.begin(), distSum.end());

    //como punto inicial se asigna a la posicion mas alejada la instalacion con menos flujos 
    int init_i= int(std::distance(distSum.begin(), dmax));
    int init_j= int(std::distance(flowSum.begin(), fmin));

    orden[init_i]=init_j;
    asignado[init_j]=true;
	
    for (int i = 0; i < N; i++) {
        if (i==init_i){
            continue;
        }
        int mejorUbicacion = -1;
        int minCosto = std::numeric_limits<int>::max();
        for (int j = 0; j < N; j++) {
            if (!asignado[j]) {
                orden[i]=j;
                asignado[j]=true;
                int costo = costo_inicial(N, distancia, flujo, orden, asignado);
                if (costo < minCosto) {
                    minCosto = costo;
                    mejorUbicacion = j;
                }
                asignado[j]=false;
            }
        }
        orden[i] = mejorUbicacion;
        asignado[mejorUbicacion] = true;
    }
    std::cout << "Initial Solution: ";
    for (int i = 0; i < N; i++) {
        std::cout << orden[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "Cost: " << costo(N, distancia, flujo, orden) << std::endl;
    std::cout << "\n";

    int cost = costo(N, distancia, flujo, orden);
    return {orden, cost};
}

// Función para generar una vecindad de soluciones mediante el intercambio de ubicaciones
std::vector<Solucion> generarNeighborhood(int N, const vector<vector<int>>& distancia, const vector<vector<int>>& flujo, const Solucion& actual) {
    std::vector<Solucion> neighborhood;
    //se generan todos los swap posibles
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

Solucion mejorVecino(const std::vector<Solucion>& neighborhood, std::vector<std::vector<int>>& tabuList, int mejorCosto) {

    //primer vecino se toma como primer mejor vecino
    Solucion mejorVecino = neighborhood[0];
    for (const Solucion& neighbor : neighborhood) {

        bool tabu= false;

        //revisar lista tabu
        for (int i=0; i< tabuList.size(); i++){
            if ((tabuList[i][0]==neighbor.swap[0] && tabuList[i][1]==neighbor.swap[1]) || (tabuList[i][0]==neighbor.swap[1] && tabuList[i][1]==neighbor.swap[0])){
                tabu= true;
            }else if (neighbor.costo < mejorCosto * 0.85){
                tabuList.erase(tabuList.begin()+i);
            }
        }

        //comparar candidato
        if (!tabu && neighbor.costo < mejorVecino.costo) {
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
Solucion tabuSearch(int N, const vector<vector<int>>& distancia, const vector<vector<int>>& flujo, int tabuLargo, int iteraciones) {
    const int maxIterations = iteraciones; // Número máximo de iteraciones
    
    //solucion inicial greedy
    Solucion mejorSolucion = solucionInicial(N,distancia, flujo);
    Solucion actual = mejorSolucion;

    //lista tabu
    std::vector<std::vector<int>> tabuList;

    //busqueda tabu
    int iteration = 0;
    int iter_sin_mejoras=0;
    while (iteration < maxIterations) {

        Solucion mejorvecino;

        if(iter_sin_mejoras<(maxIterations/2)+1){
            //generar vecindad
            std::vector<Solucion> neighborhood = generarNeighborhood(N,distancia, flujo, actual);
            
            //obtener mejor vecino
            mejorvecino = mejorVecino(neighborhood, tabuList, mejorSolucion.costo);

            //actualizar lista tabu
            actualizarTabuList(tabuLargo, tabuList, mejorvecino);
        }
        else{
            iter_sin_mejoras=0;
            std::vector<int> nuevoOrden = actual.orden;
            std::rotate(nuevoOrden.begin(), nuevoOrden.begin()+nuevoOrden.size()/2, nuevoOrden.end());
            int cost = costo(N, distancia, flujo, nuevoOrden);
            mejorvecino= {nuevoOrden, cost};
        }

        if (mejorvecino.costo < mejorSolucion.costo) {
            mejorSolucion = mejorvecino;
            iter_sin_mejoras=0;
        }
        else{
            iter_sin_mejoras++;
        }
        actual = mejorvecino;
        iteration++;
    }

    return mejorSolucion;
}

int main(int argc, char *args[]){

    //nombre archivo
    string archivo= args[2];

    int iteraciones= stoi(args[3]);
    int tabuLargo= stoi (args[1]);

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
    Solucion solucion = tabuSearch(n, distancia, flujo, tabuLargo, iteraciones);

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