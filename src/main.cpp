#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <utility>
#include <vector>
#include <bitset>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <random>
#include <climits>
using namespace std;

class Arista;
int cities = 4;
class Nodo {
public:
  vector<Arista*> aristas;
  
  int id;
  float x;
  float y;
  float radius;
  float color[3];

  GLuint VAO, VBO;
  GLuint shaderProgram;

  Nodo(int id, float x, float y, float radius, float r, float g, float b) {
    this->id = id;
    this->x = x;
    this->y = y;
    this->radius = radius;
    color[0] = r;
    color[1] = g;
    color[2] = b;
  }
  float distance(Nodo* goal) {
    return sqrt(pow(x - goal->x, 2) + pow(y - goal->y, 2));
  }
};
class Arista {
public:
	Nodo* nodos[2];
	float peso;
  float color[3];

  GLuint VAOLine;
  GLuint VBOLine;
  GLuint shaderProgram;

	Arista(Nodo* origen, Nodo* destino, float peso, float r, float g, float b) {
		nodos[0] = origen; //Origen
		nodos[1] = destino; //Destino
		this->peso = peso;
		origen->aristas.push_back(this);
		destino->aristas.push_back(this);
    color[0] = r;
    color[1] = g;
    color[2] = b;
	}
};

void compileShadersNodo(Nodo& nodo) {
  const char* vertexShaderSource = R"glsl(
  #version 330 core
  layout (location = 0) in vec2 aPos;
  void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
  }
  )glsl";

  const char* fragmentShaderSource = R"glsl(
  #version 330 core
  out vec4 FragColor;
  uniform vec3 nodeColor; // Color del nodo
  void main() {
    FragColor = vec4(nodeColor, 1.0); // Usar el color del nodo
  }
  )glsl";

  GLuint vertexShader, fragmentShader;
  GLint success;
  GLchar infoLog[512];

  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
  }

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
  }

  nodo.shaderProgram = glCreateProgram();
  glAttachShader(nodo.shaderProgram, vertexShader);
  glAttachShader(nodo.shaderProgram, fragmentShader);
  glLinkProgram(nodo.shaderProgram);
  glGetProgramiv(nodo.shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(nodo.shaderProgram, 512, NULL, infoLog);
    cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

//compileShaders para aristas
void compileShadersArista(Arista& arista) {
  const char* vertexShaderSource = R"glsl(
  #version 330 core
  layout (location = 0) in vec2 aPos;
  void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
  }
  )glsl";

  const char* fragmentShaderSource = R"glsl(
  #version 330 core
  out vec4 FragColor;
  uniform vec3 edgeColor; // Color de la arista
  void main() {
    FragColor = vec4(edgeColor, 1.0); // Usar el color de la arista
  }
  )glsl";

  GLuint vertexShader, fragmentShader;
  GLint success;
  GLchar infoLog[512];

  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
  }

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
  }

  arista.shaderProgram = glCreateProgram();
  glAttachShader(arista.shaderProgram, vertexShader);
  glAttachShader(arista.shaderProgram, fragmentShader);
  glLinkProgram(arista.shaderProgram);
  glGetProgramiv(arista.shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(arista.shaderProgram, 512, NULL, infoLog);
    cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}
void createCircle(Nodo& nodo) {
  glGenVertexArrays(1, &nodo.VAO);
  glGenBuffers(1, &nodo.VBO);

  glBindVertexArray(nodo.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, nodo.VBO);

  float vertices[200];
  float radius = nodo.radius;
  for (int i = 0; i < 100; ++i) {
    float theta = 2.0f * 3.1415926f * float(i) / float(100);
    vertices[i * 2] = radius * cosf(theta) + nodo.x;
    vertices[i * 2 + 1] = radius * sinf(theta) + nodo.y;
  }

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void createEdge(Arista& arista) {
  glGenVertexArrays(1, &(arista.VAOLine));
  glGenBuffers(1, &(arista.VBOLine));

  glBindVertexArray(arista.VAOLine);
  glBindBuffer(GL_ARRAY_BUFFER, arista.VBOLine);

  float lineVertices[] = {
    arista.nodos[0]->x, arista.nodos[0]->y,
    arista.nodos[1]->x, arista.nodos[1]->y
  };

  glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void normalizeNodes(vector<Nodo*>& nodos) {
  for (int i = 0; i < nodos.size(); i++) {
    nodos[i]->y = - nodos[i]->y;
    nodos[i]->x = - nodos[i]->x;
  }
}


float generarFloatAleatorio() {
  int precision = 1000;
  float numeroAleatorio = rand() % precision / static_cast<float>(precision);
  return numeroAleatorio;
}
class Grafo
{
public:
	vector<Nodo*> nodos;
  vector<Arista*> aristas;
	Grafo() {
    srand(time(nullptr));

    for (int i = 0; i < cities; i++) {
      insertarNodo(i, generarFloatAleatorio(), generarFloatAleatorio(), 0.01f, 1.0f, 0.0f, 0.0f);
      
      // cout << nodos[i]->x << ' ' << nodos[i]->y << endl;
    }
    // cout << endl;
    normalizeNodes(nodos);
    // for (int i = 0; i < 10; i++) {
      // cout << nodos[i]->x << ' ' << nodos[i]->y << endl;
    // }
    for (int i = 0; i < cities; i++) {
      for (int j = i + 1; j < cities; j++) {
        Nodo* origen = encontrarNodo(i);
        Nodo* destino = encontrarNodo(j);
        insertarArista(i, j, origen->distance(destino));
      }
    }
	}
  ~Grafo() {
    for (Nodo* nodo : nodos)
      delete nodo;

    for (Arista* arista : aristas)
      delete arista;
  }

private:
  Nodo* encontrarNodo(int d) {
    for (Nodo* nodo : nodos) {
      if (nodo->id == d) {
        return nodo;
      }
    }
    return NULL;
  }

public:
	void insertarNodo(int id, float x, float y, float radius, float r, float g, float b) {
		nodos.push_back(new Nodo(id, x, y, radius, r, g, b));
	}

	void insertarArista(int idOrigen, int idDestino, float peso) {
    Nodo* origen = encontrarNodo(idOrigen);
    Nodo* destino = encontrarNodo(idDestino);

    if (!origen || !destino) {
      return;
    }

		Arista* x = new Arista(origen, destino, peso, 0.2f, 0.2f, 0.2f);
    aristas.push_back(x);
	}

  void imprimir() {
    for (int i = 0; i < nodos.size(); i++) {
      createCircle(*nodos[i]);
      compileShadersNodo(*nodos[i]);
    }

    for (int i = 0; i < aristas.size(); i++) {
      createEdge(*aristas[i]);
      compileShadersArista(*aristas[i]);
    }
  }

  void recolorEdges(vector<int> path) {
    clearEdges();
    for (int i = 0; i < path.size(); i++) {
      Nodo* a;
      Nodo* b;
      if (i == path.size() - 1) {
        a = encontrarNodo(path[i]);
        b = encontrarNodo(path[0]);
      }
      else {
        a = encontrarNodo(path[i]);
        b = encontrarNodo(path[i + 1]);
      }
      for (int i = 0; i < a->aristas.size(); i++) {
        if ((a->aristas[i]->nodos[1]->id == b->id) || (a->aristas[i]->nodos[0]->id == b->id)) {
          a->aristas[i]->color[0] = 1.0f;
          a->aristas[i]->color[1] = 1.0f;
          a->aristas[i]->color[2] = 1.0f;
          break;
        }
      }
    }

    // for (int i = 0; i < aristas.size(); i++) {
    //   cout << aristas[i]->nodos[0]->id << ' ' << aristas[i]->color[0] << ',' << aristas[i]->color[1] << ',' << aristas[i]->color[2] << ' ' << aristas[i]->nodos[1]->id << endl; 
    // }
    // cout << endl;

  }
  int calculateFitnessGraph(vector<int> path) {
    // for (int i = 0; i < path.size(); i++) {
    //   cout << path[i];
    // }
    // cout << endl;

    int fitness = 0;
    Nodo* a;
    Nodo* b;
    for (int i = 0; i < path.size(); i++) {
      if (i == path.size() - 1) {
        a = encontrarNodo(path[i]);
        b = encontrarNodo(path[0]);
      }
      else {
        a = encontrarNodo(path[i]);
        b = encontrarNodo(path[i + 1]);
      }
      // cout << a->id << ' ' << b->id << ' ' << (a->distance(b) * 1000);
      fitness = fitness + (a->distance(b) * 1000);
    }
    return fitness;
  }

  void clearEdges() {
    for (int i = 0; i < aristas.size(); i++) {
      aristas[i]->color[0] = 0.2f;
      aristas[i]->color[1] = 0.2f;
      aristas[i]->color[2] = 0.2f;
    }
  }
};

class Point {
public:
    float x;
    float y;
    float color[3]; // Componentes RGB del color

    // Constructor
    Point(float _x, float _y, float r, float g, float b) : x(_x), y(_y) {
        color[0] = r;
        color[1] = g;
        color[2] = b;
    }
};


void newMaxY(vector<Point>& points) {

    // Inicializar el máximo como el primer punto del vector
    Point* maxPoint = &(points[0]);

    // Iterar sobre los puntos restantes y actualizar el máximo si se encuentra un punto con un valor de "y" mayor
    for (size_t i = 1; i < points.size(); ++i) {
        if (points[i].y > maxPoint->y) {
            maxPoint = &(points[i]);
        }
    }

    maxPoint->color[1] = 0.0f;
    maxPoint->color[2] = 1.0f;
    // Devolver el punto con el mayor valor de "y"
}

void newMinY(vector<Point>& points) {

    // Inicializar el máximo como el primer punto del vector
    Point* minPoint = &(points[0]);

    // Iterar sobre los puntos restantes y actualizar el máximo si se encuentra un punto con un valor de "y" mayor
    for (size_t i = 1; i < points.size(); ++i) {
        if (points[i].y < minPoint->y) {
            minPoint = &(points[i]);
        }
    }

    minPoint->color[1] = 0.0f;
    minPoint->color[2] = 1.0f;
    // Devolver el punto con el mayor valor de "y"
}


class Rectangles {
  public:
  float x1,x2,x3,x4,y1,y2,y3,y4,z1,z2,z3,z4;

  unsigned int VAORect;
  unsigned int VBORect;
  float color[3];

  Rectangles(float x1, float x2,float x3,float x4,
  float y1, float y2,float y3,float y4,
  float z1, float z2,float z3,float z4,
  float r, float g, float b) :
    x1(x1), x2(x2), x3(x3),x4(x4),
    y1(y1), y2(y2), y3(y3),y4(y4),
    z1(y1), z2(z2), z3(y3),z4(z4),

    VAORect
(0), VBORect(0){
    color[0] = r / 255.0f;
    color[1] = g / 255.0f;
    color[2] = b / 255.0f;
  }
};
void createRectangles(Rectangles& rectangles) {
  float vertices[] = {
    rectangles.x3,rectangles.y3,rectangles.z3,
    rectangles.x4,rectangles.y4,rectangles.z4,
    rectangles.x2,rectangles.y2,rectangles.z2,
    rectangles.x1,rectangles.y1,rectangles.z1,
  };

  glGenVertexArrays(1, &rectangles.VAORect);
  glBindVertexArray(rectangles.VAORect);

  glGenBuffers(1, &rectangles.VBORect);
  glBindBuffer(GL_ARRAY_BUFFER, rectangles.VBORect);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

bool getRandomBoolean() {
  return rand() % 2 == 0;
}
void shuffleVector(vector<bitset<10>>& vec) {
    // Obtener un generador de números aleatorios
    random_device rd;
    default_random_engine rng(rd());

    // Mezclar el vector utilizando shuffle
    shuffle(vec.begin(), vec.end(), rng);
}
void shuffleVector(vector<int>& vec) {
    // Obtener un generador de números aleatorios
    random_device rd;
    default_random_engine rng(rd());

    // Mezclar el vector utilizando shuffle
    shuffle(vec.begin(), vec.end(), rng);
}

vector<bitset<10>> generatePopulation(int populationSize) {
  vector<bitset<10>> population(populationSize);
  for (auto& individual : population) {
    for (int i = 0; i < 10; ++i) {
      individual[i] = getRandomBoolean();
    }
    individual[4] = false;
  }

  // cout << "Poblacion inicial: " << endl;
  // for (int i = 0; i < population.size(); i++) {
  //   cout << population[i] << endl;
  // }
  // cout << endl;

  return population;
}

vector<vector<int>> generatePopulationGraph(int populationSize) {
  vector<vector<int>> population(populationSize);
  for (int i = 0; i < populationSize; i++) {
    for (int j = 0; j < cities; j++) {
      population[i].push_back(j);
    }
    shuffleVector(population[i]);
  }

  // for (int i = 0; i < population.size(); i++) {
  //   for (int j = 0; j < 10; j++) {
  //     cout << population[i][j];
  //   }
  //   cout << endl;
  // }
  // cout << endl;

  return population;
}

float calculateFitness(const bitset<10>& individual) {
  bitset<5> X;
  bitset<5> Y;
  
  int xx;
  int yy;

  for (int i = 0; i < 5; i++) {
    X[i] = individual[i + 5];
  }

  for (int i = 0; i < 5; i++) {
    Y[i] = individual[i];
  }

  xx = X.to_ulong();
  yy = Y.to_ulong();

  //cout << "x = " << xx << ", " << "y = " << yy << endl;

  return xx - yy + (2 * xx * yy);
}

void printFitness(const bitset<10>& individual) {
  bitset<5> X;
  bitset<5> Y;
  
  int xx;
  int yy;

  for (int i = 0; i < 5; i++) {
    X[i] = individual[i + 5];
  }

  for (int i = 0; i < 5; i++) {
    Y[i] = individual[i];
  }

  xx = X.to_ulong();
  yy = Y.to_ulong();

  cout << "x = " << xx << ", " << "y = " << yy << endl;
}

Grafo G;
vector<int> tournamentSelectionGraph (vector<vector<int>> population) {
  int participantsSize = rand() % 91 + 10;
  int sizeParticipants = population.size() * participantsSize / 100;
  vector<vector<int>> participants;
  
  for (int i = 0; i < sizeParticipants; i++) {
    int index = rand() % population.size();
    participants.push_back(population[index]);
  }

  // for (int i = 0; i < sizeParticipants; i++) {
  //   for (int j = 0; j < 10; j++) {
  //     cout << participants[i][j];
  //   }
  //    cout << ' ' << G.calculateFitnessGraph(participants[i]) << endl;
  // }
  // cout << endl;

  int currentFitness = INT_MAX;
  vector<int> winner;

  for (int i = 0; i < sizeParticipants; i++) {
    if (currentFitness > G.calculateFitnessGraph(participants[i])) {
      winner = participants[i];
      currentFitness = G.calculateFitnessGraph(participants[i]);
    }
  }

  // cout << "winner: ";
  // for (int i = 0; i < winner.size(); i++) {
  //   cout << winner[i];
  // }
  // cout << endl;

  return winner;
}
bitset<10> tournamentSelection (vector<bitset<10>> population) {
  int participantsSize = rand() % 91 + 10;
  int sizeParticipants = population.size() * participantsSize / 100;
  vector<bitset<10>> participants;
  
  for (int i = 0; i < sizeParticipants; i++) {
    int index = rand() % population.size();
    participants.push_back(population[index]);
  }

  // for (int i = 0; i < sizeParticipants; i++) {
  //   cout << participants[i] << ' ' <<calculateFitness(participants[i]) << endl;
  // }
  // cout << endl;

  int currentFitness = -15;
  bitset<10> winner;

  for (int i = 0; i < sizeParticipants; i++) {
    if (currentFitness < calculateFitness(participants[i])) {
      winner = participants[i];
      currentFitness = calculateFitness(participants[i]);
    }
  }

  //cout << winner << endl;
  return winner;
}

vector<bitset<10>> rankingSelection (vector<bitset<10>> population){
  vector<float> fi;
  int Media = 0;
  for (int i = 0; i < population.size(); i++) {
    fi.push_back(calculateFitness(population[i]));
    Media = Media + calculateFitness(population[i]);
  }
  Media = Media / population.size();
  
  vector<int> ve;
  for (int i = 0; i < population.size(); i++) {
    ve.push_back(round(calculateFitness(population[i]) / Media));
    //cout << calculateFitness(population[i]) / Media << endl;
  }

  vector<bitset<10>> newPopulation;
  for (int i = 0; i < ve.size(); i++) {
    for (int j = 0; j < ve[i]; j++) {
      newPopulation.push_back(population[i]);
    }
  }

  shuffleVector(newPopulation);

  //cout << "CANDIDATOS DE LA NUEVA POBLACION: " << newPopulation.size() <<endl;
  //for (int i = 0; i < newPopulation.size(); i++) {
  //  cout << newPopulation[i] << endl;
  //}
  //cout << endl;
  return newPopulation;

}

pair<bitset<10>, bitset<10>> crossover(bitset<10> parent1, bitset<10> parent2) {
  int point = rand() % 10;
  //cout << endl << "point: " << point << endl;
  bitset<10> child1 = parent1;
  bitset<10> child2 = parent2;

  //cout << child1 << ", " << child2 << " -> "; 
  for (int i = 0; i < point; ++i) {
    child1[i] = parent2[i];
    child2[i] = parent1[i];
  }
  //cout << child1 << ", " << child2 << endl; 
  //cout << child1 << endl;
  //cout << child2 << endl;

  return make_pair(child1, child2);
}
pair<vector<int>, vector<int>> crossoverGraph(vector<int> parent1, vector<int> parent2) {
  int point = rand() % cities;
  //cout << endl << "point: " << point << endl;
  vector<int> child1 = parent1;
  vector<int> child2 = parent2;
  for (int i = 0; i < point; i++) {
    if (child1[i] != parent2[i]) {
      for (int j = 0; j < child1.size(); j++) {
        if (child1[j] == parent2[i]) {
          iter_swap(child1.begin() + i, child1.begin() + j);
        }
      }
    }
    if (child2[i] != parent1[i]) {
      for (int j = 0; j < child2.size(); j++) {
        if (child2[j] == parent1[i]) {
          iter_swap(child2.begin() + i, child2.begin() + j);
        }
      }
    }
  }

  //cout << child1 << endl;
  //cout << child2 << endl;

  return make_pair(child1, child2);
}

void mutation(pair<bitset<10>, bitset<10>>& children) {
  int point1 = 4;
  while(point1 == 4){
    point1 = rand() % 10;
  }
  int point2 = 4;
  while(point2 == 4){
    point2 = rand() % 10;
  }

  //cout << "Cambio en: " << point1 << ", " << point2 << endl;
  
  //cout<< children.first << ' ' << children.second << " -> " ;

  //if (children.first[point1] == 0) {
    children.first.flip(point1);
  //}
  //if (children.second[point2] == 0) {
    children.second.flip(point2);
  //}
  
  //cout << children.first << ' ' << children.second << endl;

  //cout<<calculateFitness(children.first) << ' ' << calculateFitness(children.second) << endl;
}
void mutationGraph(pair<vector<int>, vector<int>>& children) {
  int point11;
  point11 = rand() % cities;

  int point12 = point11;
  while(point12 == point11){
    point12 = rand() % cities;
  }

  int point21;
  point21 = rand() % cities;

  int point22 = point21;
  while(point22 == point21){
    point22 = rand() % cities;
  }

  iter_swap(children.first.begin() + point11, children.first.begin() + point12);
  iter_swap(children.second.begin() + point21, children.second.begin() + point22);
}

bitset<10> betterIndividual("0000001111");
void generateNextGeneration(vector<bitset<10>> &population) {
  vector<bitset<10>> nextGeneration;
  
  //Con seleccion por Torneo
  int elitism = 0;

  for (int i = 0; i < population.size(); i++) {
    if (calculateFitness(population[i]) >= calculateFitness(betterIndividual)) {
      
      nextGeneration.push_back(population[i]);
      betterIndividual = population[i];
      elitism++;
      //cout << population[i] << endl;
    }
    
  }
  for (int i = elitism; i < population.size(); i++) {
    bitset<10> parent1 = tournamentSelection(population);
    bitset<10> parent2 = tournamentSelection(population);

    pair<bitset<10>, bitset<10>> children = crossover(parent1, parent2);
    mutation(children);

    if (calculateFitness(children.first) > calculateFitness(children.second)) {
      nextGeneration.push_back(children.first);
    }
    else {
      nextGeneration.push_back(children.second);
    }
  }

  //Con seleccion por Ranking
  // vector<bitset<10>> rankingGeneration = rankingSelection(population);
  // for (int i = 0; i < population.size(); i = i + 2){
  //   pair<bitset<10>, bitset<10>> children = crossover(rankingGeneration[i], rankingGeneration[i + 1]);
  //   mutation(children);
  //   nextGeneration.push_back(children.first);
  //   nextGeneration.push_back(children.second);
  // }
  population = nextGeneration;

  // cout << "Siguiente generacion: " << endl;
  // for (int i = 0; i < population.size(); i++) {
  //   cout << population[i] << endl;
  // }
  // cout << endl;
}

vector<int> betterIndividualGraph;
void generateNextGenerationGraph(vector<vector<int>> &populationGraph) {
  vector<vector<int>> nextGeneration;
  
  //Con seleccion por Torneo
  float elitism = 0;
  

  for (int i = 0; i < populationGraph.size(); i++) {
    if (G.calculateFitnessGraph(populationGraph[i]) <= G.calculateFitnessGraph(betterIndividualGraph)) {
      
      nextGeneration.push_back(populationGraph[i]);
      betterIndividualGraph = populationGraph[i];
      elitism++;
      //cout << population[i] << endl;
    }
    
  }
  for (int i = elitism; i < populationGraph.size(); i++) {
    vector<int> parent1 = tournamentSelectionGraph(populationGraph);
    vector<int> parent2 = tournamentSelectionGraph(populationGraph);

    pair<vector<int>, vector<int>> children = crossoverGraph(parent1, parent2);
    mutationGraph(children);

    if (G.calculateFitnessGraph(children.first) < G.calculateFitnessGraph(children.second)) {
      nextGeneration.push_back(children.first);
    }
    else {
      nextGeneration.push_back(children.second);
    }
  }

  //Con seleccion por Ranking
  // vector<bitset<10>> rankingGeneration = rankingSelection(population);
  // for (int i = 0; i < population.size(); i = i + 2){
  //   pair<bitset<10>, bitset<10>> children = crossover(rankingGeneration[i], rankingGeneration[i + 1]);
  //   mutation(children);
  //   nextGeneration.push_back(children.first);
  //   nextGeneration.push_back(children.second);
  // }
  populationGraph = nextGeneration;

  // cout << "Siguiente generacion: " << endl;
  // for (int i = 0; i < population.size(); i++) {
  //   cout << population[i] << endl;
  // }
  // cout << endl;
}

pair<int, int> getMediaAndMax(vector<bitset<10>> population){
  int Media = 0;
  int Max = -15;
  
  for (int i = 0; i < population.size(); i++) {
    Media = Media + calculateFitness(population[i]);
    if (calculateFitness(population[i]) > Max) {
      Max = calculateFitness(population[i]);
    }
  }
  Media = Media / population.size();

  return make_pair(Media, Max);
}

pair<int, int> getMediaAndMin(vector<vector<int>> population){
  int Media = 0;
  int Min = INT_MAX;
  
  for (int i = 0; i < population.size(); i++) {
    Media = Media + G.calculateFitnessGraph(population[i]);

    //cout << G.calculateFitnessGraph(population[i]) <<"<"<< Min << endl;

    if (G.calculateFitnessGraph(population[i]) < Min) {
      Min = G.calculateFitnessGraph(population[i]);
    }
  }
  Media = Media / population.size();

  return make_pair(Media, Min);
}

//11111
//01111
vector<Point> Points;
vector<Point> PointsGraph;

float scaleToRange(float value) {
  return (value - -15.0f) / (946.0f - -15.0f) * (0.9f - 0.0f) + 0.0f;
}

void NormalizePoints(vector<Point>& points) {
  if (points.size() <= 1) return; // No es necesario normalizar si hay 0 o 1 punto

  float offset = 3.0f / points.size(); // Ajustar el offset para evitar dividir por 0
  float current = -1.0f; // Iniciar desde -1.0
  for (int i = 0; i < points.size(); i = i + 2)
  {
    points[i].x = current;
    points[i + 1].x = current;
    if (points[i].y < 0.0f || points[i].y > 1.0f) {
      points[i].y = scaleToRange(points[i].y);
    }
    if (points[i + 1].y < 0.0f || points[i + 1].y > 1.0f) {
      points[i + 1].y = scaleToRange(points[i + 1].y);
    }
    current += offset;
  }
}

float scaleToRangeGraph(float value) {
    float initialMin = 0.0f;   // Límite inferior del rango inicial
    float initialMax = 1000 * sqrt(2) * cities; // Límite superior del rango inicial
    float desiredMin = -0.9f;  // Límite inferior del rango deseado
    float desiredMax = 0.0f;   // Límite superior del rango deseado
    
    // Escala linealmente el valor desde el rango inicial al rango deseado
    return (value - initialMin) / (initialMax - initialMin) * (desiredMax - desiredMin) + desiredMin;
}


void NormalizePointsGraph(vector<Point>& points) {
  if (points.size() <= 1) return; // No es necesario normalizar si hay 0 o 1 punto

  float offset = 3.0f / points.size(); // Ajustar el offset para evitar dividir por 0
  float current = 0.0f; // Iniciar desde -1.0
  for (int i = 0; i < points.size(); i = i + 2)
  {
    points[i].x = current;
    points[i + 1].x = current;
    if (points[i].y < -1.0f || points[i].y > 0.0f) {
      points[i].y = scaleToRangeGraph(points[i].y);
    }
    if (points[i + 1].y < -1.0f || points[i + 1].y > 0.0f) {
      points[i + 1].y = scaleToRangeGraph(points[i + 1].y);
    }
    current += offset;
  }
}

//----------------------DIBUJAR PUNTOS------------------------
const char* vertexShaderSource = R"(
  #version 330 core
  layout (location = 0) in vec2 aPos;
  layout (location = 1) in vec3 aColor;

  out vec3 ourColor;

  void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    ourColor = aColor;
  }
)";
const char* fragmentShaderSource = R"(
  #version 330 core
  out vec4 FragColor;

  in vec3 ourColor;

  void main() {
    FragColor = vec4(ourColor, 1.0);
  }
)";
void drawPoints() {
  // Verificar si hay puntos para dibujar
  if (Points.empty() && PointsGraph.empty()) return;

  // Crear y configurar el Vertex Array Object (VAO)
  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // Crear el Vertex Buffer Object (VBO) para almacenar los datos de los puntos
  GLuint VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  // Unir los datos de los puntos de Points y PointsGraph en un solo vector
  std::vector<Point> combinedPoints;
  combinedPoints.insert(combinedPoints.end(), Points.begin(), Points.end());
  combinedPoints.insert(combinedPoints.end(), PointsGraph.begin(), PointsGraph.end());

  // Transferir los datos de los puntos combinados al VBO
  glBufferData(GL_ARRAY_BUFFER, combinedPoints.size() * sizeof(Point), combinedPoints.data(), GL_STATIC_DRAW);

  // Especificar la configuración de los atributos de los puntos
  // Coordenadas de posición
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)offsetof(Point, x));
  glEnableVertexAttribArray(0);
  // Color
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)offsetof(Point, color));
  glEnableVertexAttribArray(1);

  // Dibujar los puntos
  glPointSize(5.0f); // Establecer el tamaño de los puntos
  glDrawArrays(GL_POINTS, 0, combinedPoints.size());

  // Limpiar estado
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
}

//----------------------DIBUJAR PUNTOS------------------------
vector<bitset<10>> population;
vector<vector<int>> populationGraph;

void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    float x = (xpos / 1200) * 2 - 1;
    float y =  1 - ((ypos/800)*2);

    if (x > 0.7f && x <0.99f && y < 0.88f && y > 0.75f)
    {
      pair<float, float> ys;
      Point newPoint1(x, y, 1.0f, 0.0f, 0.0f);
      Point newPoint2(x, y, 0.0f, 1.0f, 0.0f);
      if(!Points.empty()) {
        generateNextGeneration(population);
      }
      ys = getMediaAndMax(population);
      newPoint1 = Point(x, ys.first, 1.0f, 0.0f, 0.0f);
      newPoint2 = Point(x, ys.second, 0.0f, 1.0f, 0.0f);
      cout << "Media: " << ys.first << " Max: " << ys.second << endl; 

      Points.push_back(newPoint1);
      Points.push_back(newPoint2);
      
      NormalizePoints(Points);
      if (Points.size() == 2) {
        Points[1].color[1] = 0.0f;
        Points[1].color[2] = 1.0f;
      }
      else {
        newMaxY(Points);
      }
      printFitness(betterIndividual);
    }
    if (x > 0.7f && x <0.99f && y < 0.68f && y > 0.55f)
    {           
      for (int i = 0; i < 10; i++) {
        pair<float, float> ys;
        Point newPoint1(x, y, 1.0f, 0.0f, 0.0f);
        Point newPoint2(x, y, 0.0f, 1.0f, 0.0f);
        if(! Points.empty()) {
          generateNextGeneration(population);
        }
        ys = getMediaAndMax(population);
        newPoint1 = Point(x, ys.first, 1.0f, 0.0f, 0.0f);
        newPoint2 = Point(x, ys.second, 0.0f, 1.0f, 0.0f);
        cout << "Media: " << ys.first << " Max: " << ys.second << endl; 

        Points.push_back(newPoint1);
        Points.push_back(newPoint2);
      }
      NormalizePoints(Points);
      newMaxY(Points);
    }
    if (x > 0.7f && x <0.99f  && y < 0.48f && y > 0.35f)
    {           
      pair<float, float> ys;
      Point newPoint1(x, y, 1.0f, 0.0f, 0.0f);
      Point newPoint2(x, y, 0.0f, 1.0f, 0.0f);
      if(!PointsGraph.empty()) {
        generateNextGenerationGraph(populationGraph);
      }
      ys = getMediaAndMin(populationGraph);
      newPoint1 = Point(x, ys.first, 1.0f, 0.0f, 0.0f);
      newPoint2 = Point(x, ys.second, 0.0f, 1.0f, 0.0f);
      cout << "Media: " << ys.first << " Min: " << ys.second << endl; 

      PointsGraph.push_back(newPoint1);
      PointsGraph.push_back(newPoint2);
      
      NormalizePointsGraph(PointsGraph);
      if (PointsGraph.size() == 2) {
        PointsGraph[1].color[1] = 0.0f;
        PointsGraph[1].color[2] = 1.0f;
      }
      else {
        newMinY(PointsGraph);
      }
      // for (int i = 0; i < betterIndividualGraph.size(); i++) {
      //   cout << betterIndividualGraph[i];
      // }
      // cout << endl;
      
      G.recolorEdges(betterIndividualGraph);
    }

    if (x > 0.7f && x <0.99f  && y < 0.28f && y > 0.15f)
    {
      for (int i = 0; i < 10; i++) {
        pair<float, float> ys;
        Point newPoint1(x, y, 1.0f, 0.0f, 0.0f);
        Point newPoint2(x, y, 0.0f, 1.0f, 0.0f);
        if(!PointsGraph.empty()) {
          generateNextGenerationGraph(populationGraph);
        }
        ys = getMediaAndMin(populationGraph);
        newPoint1 = Point(x, ys.first, 1.0f, 0.0f, 0.0f);
        newPoint2 = Point(x, ys.second, 0.0f, 1.0f, 0.0f);
        cout << "Media: " << ys.first << " Min: " << ys.second << endl; 

        PointsGraph.push_back(newPoint1);
        PointsGraph.push_back(newPoint2);
        
        // for (int i = 0; i < betterIndividualGraph.size(); i++) {
        //   cout << betterIndividualGraph[i];
        // }
        // cout << endl;
        
      }
      NormalizePointsGraph(PointsGraph);
      newMinY(PointsGraph); 
      G.recolorEdges(betterIndividualGraph);
    }

    if (x > 0.7f && x <0.99f  && y < 0.08f && y > -0.05f)
    {           
      int poblacion = population.size();
      population.clear();
      population = generatePopulation(poblacion);
      Points.clear();
      betterIndividual = bitset<10>("0000001111");

      int poblacionGraph = populationGraph.size();
      populationGraph.clear();
      populationGraph = generatePopulationGraph(poblacionGraph);
      PointsGraph.clear();
      G.clearEdges();
      betterIndividualGraph = populationGraph[0];

    }
  }
}

GLuint CompileShader(GLenum shaderType, const char* source) {
    // Crear shader
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Verificar errores de compilación
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cerr << "Error al compilar shader:\n" << infoLog << endl;
        return 0;
    }

    return shader;
}
GLuint CreateShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource) {
    // Compilar shaders
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Enlazar shaders en un programa
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Verificar errores de enlace
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cerr << "Error al enlazar el programa de shaders:\n" << infoLog << endl;
        return 0;
    }

    // Limpiar shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(1200, 800, "Grafo", NULL, NULL);
  if (window == NULL) {
    cout << "Failed to create GLFW Window" << endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
  mouse_callback(window, button, action, mods);
  });

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    cout << "Failed to initialize GLAD" << endl;
    return -1;
  }

  cout << endl;
  int poblacion;
  cout << "Ingrese la poblacion: ";
  cin >> poblacion;
  // cities = poblacion;
  //Grafo G;
  srand(time(nullptr));

  //Generar poblacion inicial
  population = generatePopulation(poblacion);
  populationGraph = generatePopulationGraph(poblacion);
  betterIndividualGraph = populationGraph[0];
  


  vector<Rectangles*> buttons;
  float x1,x2,x3,x4,y1,y2,y3,y4,z1,z2,z3,z4;
  x1 =0.7f;
  y1 = 0.88f;
  z1 = 0.0f;

  x2 = 0.99f;
  y2 = 0.88f;
  z2 = 0.0f;

  x3 = 0.7f;
  y3 = 0.75f;
  z3 = 0.0f;

  x4 =  0.99f;
  y4 = 0.75f;
  z4 = 0.0f;

  for(int i = 0; i < 5;i++)
  {
    buttons.push_back(new Rectangles(x1,x2,x3,x4,y1,y2,y3,y4,z1,z2,z3,z4,0,255,0));
    createRectangles(*buttons[i]);
    y1-=0.2f;
    y2-=0.2f;
    y3-=0.2f;
    y4-=0.2f;
  }

  GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

  while (!glfwWindowShouldClose(window)) {
    // Procesar eventos de la ventana
    glfwPollEvents();
    processInput(window);

    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);

    for (int i = 0; i < G.nodos.size(); i++) {
      //cout << G.nodos[i]->id << "\t";
      glUseProgram(G.nodos[i]->shaderProgram);
      // Pasa el color del nodo al shader fragment
      int nodeColorLocation = glGetUniformLocation(G.nodos[i]->shaderProgram, "nodeColor");
      glUniform3fv(nodeColorLocation, 1, G.nodos[i]->color);
      glBindVertexArray(G.nodos[i]->VAO);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 100);
    }
    for (int i = 0; i < G.aristas.size(); i++) {
      glUseProgram(G.aristas[i]->shaderProgram);
      int edgeColorLocation = glGetUniformLocation(G.aristas[i]->shaderProgram, "edgeColor");
      glUniform3fv(edgeColorLocation, 1, G.aristas[i]->color);
      glBindVertexArray(G.aristas[i]->VAOLine);
      glDrawArrays(GL_LINES, 0, 2);
    }

    glUseProgram(shaderProgram);

    drawPoints();
    G.imprimir();

    glUseProgram(0);

    for(int i = 0; i<buttons.size();i++)
    {
      glBindVertexArray(buttons[i]->VAORect);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    glfwSwapBuffers(window);
  }

  // for (const auto& individual : population) {
  //   cout << individual << ' ' << calculateFitness(individual) << endl;
  // }

  // cout << tournamentSelection(population) << endl << endl;
  // cout << tournamentSelection(population) << endl;

  // pair<bitset<10>, bitset<10>> c = crossover(tournamentSelection(population), tournamentSelection(population));
  // cout << endl;
  // cout << c.first << ' ' << c.second << endl;

  // cout << endl;
  // mutation(c);
  // cout << c.first << ' ' << c.second << endl;

  // cout << endl;
  // vector<bitset<10>> nextGen = generateNextGeneration(population);
  // for (const auto& individual : nextGen) {
  //   cout << individual << ' ' << calculateFitness(individual) << endl;
  // }


  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}