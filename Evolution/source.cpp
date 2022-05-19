/*
Anieerudh Gokulakrishnan
12/13/2020
Genetic Algorithm Final
*/

#include <iostream>
#include <vector>
#include <ctime>
#include <iomanip>
#include <string>

using namespace std;

const int MAPSIZE = 12;

//enum of types to distinguish type
enum Type {
	RobotT = 0,
	EmptyT = 1,
	WallT = 2,
	BatteryT = 3,
};

//abstract base class to construct map with
struct GameObject {
	char avatar = '\0';
	virtual Type GetType() = 0;
	virtual ~GameObject() {}
};

struct Empty : public GameObject {
	Empty() { avatar = ' '; }
	Type GetType() { return EmptyT; }
};

struct Wall : public GameObject {
	Wall() { avatar = '#'; }
	Type GetType() { return WallT; }
};

struct Battery : public GameObject {
	int power = 5;
	Battery() { avatar = 'B'; }
	Type GetType() { return BatteryT; }
};

struct Gene {
	vector<Type> senses = vector<Type>(4);
	char action = '\0';
};

//Robot class with a brain that can evolve
struct Robot : public GameObject {
	GameObject* map[MAPSIZE][MAPSIZE];
	vector<Gene> Genes = vector<Gene>(16);
	vector<Type> sensor = vector<Type>(4);
	int energy = 5;
	int energyHarvested = 0;
	int xpos = 0, ypos = 0;
	int turns = 0;

	void CreateMap() {
		//create empty map
		int batteries = 0;
		for (int row = 0; row < MAPSIZE; row++) {
			for (int col = 0; col < MAPSIZE; col++) {
				if (row == 0 || row == MAPSIZE - 1 || col == 0 || col == MAPSIZE - 1)
					map[row][col] = new Wall();
				else
					map[row][col] = new Empty();
			}
		}
		while (batteries <= 40) {
			int x = random(1, MAPSIZE - 2); int y = random(1, MAPSIZE - 2);
			if (map[x][y]->GetType() == EmptyT) {
				map[x][y] = new Battery(); batteries++;
			}
		}

		int yCoord = random(2, 10); int xCoord = random(2, 10);
		ypos = yCoord; xpos = xCoord;
		map[ypos][xpos] = this;
	}

	void SetGene() {
		Type ref[4] = { RobotT, EmptyT, BatteryT, WallT };
		char possibleActions[5] = { 'N', 'S', 'E', 'W', 'R' };

		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < 4; j++) {
				Genes[i].senses[j] = ref[rand() % 4];
			}
			Genes[i].action = possibleActions[rand() % 5];
		}
	}

	Robot() { avatar = 'R'; }

	int random(int lo, int hi) { return (rand() % (hi - lo + 1)) + lo; }

	void Sense() {
		UpdatePosition();
		sensor[2] = map[this->ypos][this->xpos + 1]->GetType(); //east
		sensor[3] = map[this->ypos][this->xpos - 1]->GetType(); //west
		sensor[0] = map[this->ypos - 1][this->xpos]->GetType(); //north
		sensor[1] = map[this->ypos + 1][this->xpos]->GetType(); //south
	}

	void UpdatePosition() {
		for (size_t i = 0; i < MAPSIZE; i++)
		{
			for (size_t j = 0; j < MAPSIZE; j++)
			{
				if (map[i][j] == this) {
					this->xpos = j; this->ypos = i;
				}
			}
		}
	}

	void Move(char dir) {
		bool wall = 0;
		int preY = this->ypos, preX = this->xpos;
		energy--;
		turns++;

		if (dir == 'N') {
			if (map[this->ypos - 1][this->xpos]->GetType() == BatteryT) {
				energy += 5;
				energyHarvested += 5;
			}

			if (map[this->ypos - 1][this->xpos]->GetType() != WallT) map[this->ypos - 1][this->xpos] = this;
			else wall = true;
		}
		else if (dir == 'S') {
			if (map[this->ypos + 1][this->xpos]->GetType() == BatteryT) {
				this->energy += 5;
				energyHarvested += 5;
			}

			if (map[this->ypos + 1][this->xpos]->GetType() != WallT)map[this->ypos + 1][this->xpos] = this;
			else wall = true;
		}
		else if (dir == 'E') {
			if (map[this->ypos][this->xpos + 1]->GetType() == BatteryT) {
				this->energy += 5;
				energyHarvested += 5;
			}

			if (map[this->ypos][this->xpos + 1]->GetType() != WallT)map[this->ypos][this->xpos + 1] = this;
			else wall = true;
		}
		else if (dir == 'W') {
			if (map[this->ypos][this->xpos - 1]->GetType() == BatteryT) {
				this->energy += 5;
				energyHarvested += 5;
			}

			if (map[this->ypos][this->xpos - 1]->GetType() != WallT)map[this->ypos][this->xpos - 1] = this;
			else wall = true;
		}
		else {
			char tempDirs[4] = { 'N', 'S', 'E', 'W' };
			Move(tempDirs[rand() % 4]);
		}

		if (!wall) map[preY][preX] = new Empty;

		UpdatePosition();
		Sense();
	}

	void Simulate() {

		CreateMap();
		char targetDir; bool match = 0;

		while (energy > 0) {
			UpdatePosition();
			Sense();
			for (int i = 0; i < 16; i++) {
				if (sensor == Genes[i].senses) {
					targetDir = Genes[i].action;
					match = true;
				}
			}

			if (match) {
				Move(targetDir);
			}
			else {
				Move(Genes[15].action);
			}
		}

	}

	Type GetType() { return RobotT; }
};

class Population {
	vector<Robot> population = vector<Robot>(200);
	int generation = 0; int maxGen = 0;
	vector<float> averageFitness;

private:
	void CalcFitness() {
		float sum = 0;
		for (size_t i = 0; i < population.size(); i++) {
			sum += population[i].energyHarvested;
		}
		averageFitness.push_back(sum / population.size());
	}

	void swapping(Robot& a, Robot& b) {
		Robot temp(a);
		a = b;
		b = temp;
	}

	void Sort() {
		for (size_t i = 0; i < population.size(); i++) {
			for (size_t j = 1; j < (population.size() - i); j++) {
				if (population[j - 1].energyHarvested < population[j].energyHarvested) {
					swapping(population[j - 1], population[j]);
				}
			}
		}
	}

	vector<Robot> Breed(Robot& father, Robot& mother) {
		vector<Robot> children = vector<Robot>(2);
		for (int i = 0; i < 16; i++) {
			if (i < 8) { //first half
				children[0].Genes[i] = father.Genes[i];
				children[1].Genes[i] = mother.Genes[i];
			}
			else { //second half
				children[0].Genes[i] = mother.Genes[i];
				children[1].Genes[i] = father.Genes[i];
			}
		}

		//Mutation - 4 genes make 5 percent of 80 possible gene codes
		for (int i = 0; i < 4; i++) {
			Type ref[4] = { RobotT, EmptyT, BatteryT, WallT };
			if (i < 2) children[0].Genes[rand() % 16].senses[rand() % 4] = ref[rand() % 4];
			else children[1].Genes[rand() % 16].senses[rand() % 4] = ref[rand() % 4];
		}

		return children;
	}

	void Reproduce() {
		Sort();
		for (size_t i = 0; i < population.size() / 2; i += 2) {
			for (size_t j = 0; j < 2; j++)
				population[100 + i + j] = Breed(population[i], population[i + 1])[j];
		}

		vector<Robot> temp = vector<Robot>(200);
		for (size_t i = 0; i < population.size(); i++) {
			temp[i].Genes = population[i].Genes;
		}

		population = temp;

	}

	void PrintTable() {
		cout << "-----------+-------------\n";
		cout << "Generation | Avg. Fitness\n";
		cout << "-----------+-------------\n";
		for (size_t i = 0; i < averageFitness.size(); i++) {
			cout << i << "\t   | " << averageFitness[i] << endl;
		}
		cout << endl << endl;
	}

public:
	void Evolve(int& _maxGen) {
		maxGen = _maxGen;
		for (generation = 0; generation < maxGen; generation++) {
			cout << "Simulating generation: " << generation << endl;
			for (size_t i = 0; i < population.size(); i++) {
				if (generation == 0) population[i].SetGene();
				population[i].Simulate();
			}
			CalcFitness();
			Reproduce();
		}
		PrintTable();
	}

	void ComponentTest() {
		cout << "Simulating test generation" << endl;
		for (size_t i = 0; i < population.size(); i++) {
			population[i].SetGene();
			population[i].Simulate();
		}
		CalcFitness();
		cout << "Average Fitness of test gen: " << averageFitness[averageFitness.size() - 1];
		cout << endl << endl;
	}
};

int Program();
void UnitTest();

int main() {
	srand(97);
	Population pop;
	int gens = Program();
	pop.Evolve(gens);
	//UnitTest();
}

int Program() {
	system("cls");
	cout << "Type the number of generations you want the robots to evolve during.\nFrom my testing, the fitness curve starts to flatten at around 1500 generations.\n";
	string input;
	cout << "\nEnter generations: ";
	std::getline(cin, input);
	return stoi(input);
}

void UnitTest() {
	cout << endl << "Press Enter to view diagnostic tests: \n";
	cin.get();
	Population pop;
	pop.ComponentTest();
}