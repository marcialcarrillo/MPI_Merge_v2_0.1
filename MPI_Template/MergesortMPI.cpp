#include <mpi.h> 
#include <vector>
#include <stdlib.h>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <random>
#include <iterator>
#include <functional>
using namespace std;

int input_number;
vector<int> vector_randoms;
int *array_randoms;
int *array_local_randoms;
int *array_local_randoms_input;
int *array_local_randoms_result;
vector<int> vector_local_randoms;
vector<int> vector_local_randoms_input;
vector<int> vector_local_randoms_result;
int range, process_quantity;

double local_start, local_finish, local_elapsed, elapsed;

//start of foos

void print1(vector<int>);
void print2(vector<int>);
void get_args(char* argv[], int& input_number);

//end of foos

int main(int argc, char* argv[])
{
	int process_id;

	/* Arrancar ambiente MPI */
	MPI_Init(&argc, &argv);             		/* Arranca ambiente MPI */
	MPI_Comm_rank(MPI_COMM_WORLD, &process_id); 		/* El comunicador le da valor a id (rank del proceso) */
	MPI_Comm_size(MPI_COMM_WORLD, &process_quantity);  /* El comunicador le da valor a p (número de procesos) */

	if (process_id == 0)
	{
		cin.ignore();
	}
	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);
	local_start = MPI_Wtime();

	get_args(argv, input_number);

	//int input_number = 1000;
	range = input_number / process_quantity;
	// First create an instance of an engine.
	random_device rnd_device;
	// Specify the engine and distribution.
	mt19937 mersenne_engine{ rnd_device() };  // Generates random integers
	uniform_int_distribution<int> dist{ 1, input_number};

	auto gen = [&dist, &mersenne_engine]()
	{
		return dist(mersenne_engine);
	};


	vector_randoms.resize(input_number);
	vector_local_randoms.resize(range);
	vector_local_randoms_input.resize(range);
	vector_local_randoms_result.resize(range);
	generate(begin(vector_randoms), end(vector_randoms), gen);


	array_randoms = vector_randoms.data();
	array_local_randoms = vector_local_randoms.data();
	array_local_randoms_input = vector_local_randoms_input.data();

	MPI_Scatter(array_randoms, range, MPI_INT, array_local_randoms, range, MPI_INT, 0, MPI_COMM_WORLD);

	vector<int> a, b;
	vector_randoms.clear();
	vector_randoms.resize(input_number);

	sort(vector_local_randoms.begin(), vector_local_randoms.end());

	int process_layers = log2(process_quantity);

	for (int i = 0; i < process_layers; ++i)
	{
		int displacement = (int)pow(2, i);
		if (process_id % (displacement * 2) == displacement)
		{			
			MPI_Send(vector_local_randoms.data(), range * displacement, MPI_INT, process_id - displacement, 0, MPI_COMM_WORLD);					   
		}
		MPI_Barrier(MPI_COMM_WORLD);
		if (process_id % (displacement * 2) == 0)
		{
			if (i != 0)
			{
				vector_local_randoms_input.clear();
				vector_local_randoms_input.resize(range * displacement);				
			}

			
			MPI_Recv(vector_local_randoms_input.data(), range * displacement, MPI_INT, process_id + displacement, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			vector_local_randoms_result.clear();
			vector_local_randoms_result.resize(vector_local_randoms.size() + vector_local_randoms_input.size());
			
			merge(vector_local_randoms.begin(), vector_local_randoms.end(), vector_local_randoms_input.begin(), vector_local_randoms_input.end(), vector_local_randoms_result.begin());
			vector_local_randoms.swap(vector_local_randoms_result);

		}

	}

	local_finish = MPI_Wtime();
	local_elapsed = local_finish - local_start;
	MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);



	MPI_Barrier(MPI_COMM_WORLD);
	if (process_id == 0)
	{
		print2(vector_local_randoms);
		cout << endl << endl << "Total Time Elapsed: " << elapsed << endl;
		cin.ignore();
	}

	MPI_Finalize();
	return 0;
}


void get_args(char* argv[], int& input_number)
{
	input_number = strtol(argv[1], NULL, 10);
}

void print2(vector<int> vec)
{
	for (int i = 0; i < vec.size(); i++)
	{
		cout << vec[i] << " ";
	}
}




// Optional

