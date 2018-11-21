﻿#include <mpi.h> 
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
	/* Código a evaluar */

	//local_start = MPI_Wtime();

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

	/*for (auto i : vec)
	{
	cout << i << " ";
	}*/


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
			 
			//MPI_Send(array_local_randoms, range * displacement, MPI_INT, process_id - displacement, 0, MPI_COMM_WORLD);
			MPI_Send(vector_local_randoms.data(), range * displacement, MPI_INT, process_id - displacement, 0, MPI_COMM_WORLD);


			//vector<int> temp(public_nested_vectors[process_id].size() + public_nested_vectors[process_id + displacement].size());
			//merge(public_nested_vectors[process_id].begin(), public_nested_vectors[process_id].end(), public_nested_vectors[process_id + displacement].begin(), public_nested_vectors[process_id + displacement].end(), temp.begin());
			//public_nested_vectors[process_id].swap(temp);
		}
		MPI_Barrier(MPI_COMM_WORLD);
		if (process_id % (displacement * 2) == 0)
		{
			if (i != 0)
			{
				vector_local_randoms_input.clear();
				vector_local_randoms_input.resize(range * displacement);				
			}

			//MPI_Recv(array_local_randoms_input, range * displacement, MPI_INT, process_id + displacement, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(vector_local_randoms_input.data(), range * displacement, MPI_INT, process_id + displacement, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			vector_local_randoms_result.clear();
			vector_local_randoms_result.resize(vector_local_randoms.size() + vector_local_randoms_input.size());

			//merge(vector_local_randoms.begin(), vector_local_randoms.end(), vector_local_randoms_input.begin(), vector_local_randoms_input.end(), vector_local_randoms_result.begin());

			merge(vector_local_randoms.begin(), vector_local_randoms.end(), vector_local_randoms_input.begin(), vector_local_randoms_input.end(), vector_local_randoms_result.begin());
			vector_local_randoms.swap(vector_local_randoms_result);

			//receive and merge

			//vector<int> temp(public_nested_vectors[process_id].size() + public_nested_vectors[process_id + displacement].size());
			//merge(public_nested_vectors[process_id].begin(), public_nested_vectors[process_id].end(), public_nested_vectors[process_id + displacement].begin(), public_nested_vectors[process_id + displacement].end(), temp.begin());
			//public_nested_vectors[process_id].swap(temp);
		}

	}

	local_finish = MPI_Wtime();
	local_elapsed = local_finish - local_start;
	MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);



	//MPI_Gather(array_local_randoms, range, MPI_INT, array_randoms, range, MPI_INT, 0, MPI_COMM_WORLD);

	//MPI_Barrier(MPI_COMM_WORLD);

	//if (process_id == 0)
	//{
	//	//cout << "Prosess " << process_id << endl;
	//	print1(vector_randoms);
	//	//cout << vector_randoms.size() << endl;
	//	for (int i = 1; i < process_quantity; i++)
	//	{
	//		auto it_middle = vector_randoms.begin() + (i * range);
	//		auto it_end = vector_randoms.begin() + ((i + 1) * range);
	//		inplace_merge(vector_randoms.begin(), it_middle, it_end);
	//	}
	//	cout << endl << endl << "COMPLETE VECTOR: " << endl << endl;
	//	print2(vector_randoms);
	//	cout << endl << endl << "Total Time Elapsed: " << elapsed << endl;
	//	
	//}
	MPI_Barrier(MPI_COMM_WORLD);
	if (process_id == 0)
	{

		cout << endl << endl << "COMPLETE VECTOR: " << endl << endl;
		//vector_randoms.swap(vector_local_randoms);
		//print2(vector_randoms);
		print2(vector_local_randoms);
		cout << endl << endl << "Total Time Elapsed: " << elapsed << endl;
		cout << "Vector size5: " << vector_local_randoms.size();

	}

	if (process_id == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;
}


void get_args(char* argv[], int& input_number)
{
	input_number = strtol(argv[1], NULL, 10);
}

void print1(vector<int> vec)
{
	int subvector = 0;
	for (int i = 0; i < vec.size(); i++)
	{
		if (i % range == 0) {
			cout << endl << endl << "Subvector Number: " << subvector;
			subvector++;
			cout << endl << endl << endl;
		}
		cout << vec[i] << " ";
	}
}

void print2(vector<int> vec)
{
	for (int i = 0; i < vec.size(); i++)
	{
		cout << vec[i] << " ";
	}
}




// Optional

