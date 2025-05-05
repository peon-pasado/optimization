#include <bits/stdc++.h>
using namespace std;


#include <iostream>
#include <list>
#include <vector>

struct Node;

// Estructura que representa una arista en el grafo
struct Edge {
    Node* to;                      // Puntero al nodo destino
    std::list<Edge>::iterator back; // Iterador a la arista opuesta
};

// Nodo del grafo
struct Node {
    int id;                      // Identificador del nodo
    std::list<Edge> adj;         // Lista de adyacencia

    Node(int id) : id(id) {}

    // Agregar una arista no dirigida a otro nodo
    void add_edge(Node* x, Node* y) {
        x->adj[0].emplace_back(Edge{y});                // Agrega la arista en 'this'
        y->adj[1].emplace_back(Edge{x});          // Agrega la arista en 'other'

        auto it_this = --x->adj[0].end();                   // Iterador a la arista agregada en 'this'
        auto it_other = --y->adj[1].end();           // Iterador a la arista agregada en 'other'

        it_this->back = it_other;                    // Referencia cruzada
        it_other->back = it_this;                    // Referencia cruzada
    }

    // Eliminar una arista no dirigida hacia otro nodo
    void remove_edge(Node* other) {
        for (auto it = adj.begin(); it != adj.end(); ++it) {
            if (it->to == other) {
                // Eliminar arista opuesta usando iterador back
                it->to->adj.erase(it->back);
                // Eliminar arista actual
                adj.erase(it);
                break;
            }
        }
    }

    // Imprimir la lista de adyacencia
    void print() {
        std::cout << "Node " << id << ": ";
        for (const auto& edge : adj) {
            std::cout << edge.to->id << " ";
        }
        std::cout << "\n";
    }
};

int main() {
    // Crear nodos
    Node a(1), b(2), c(3);

    // Agregar aristas
    a.add_edge(&b);
    b.add_edge(&c);
    a.add_edge(&c);

    // Imprimir listas de adyacencia
    std::cout << "Antes de eliminar aristas:\n";
    a.print();
    b.print();
    c.print();

    // Eliminar arista entre a y c
    a.remove_edge(&c);

    std::cout << "\nDespués de eliminar arista entre a y c:\n";
    a.print();
    b.print();
    c.print();

    return 0;
}
