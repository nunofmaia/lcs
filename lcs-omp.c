#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h>

void print_new_matrix(int* matrix, int size1, int size2) {
	int i, j;
	for (i = 0; i < size1 + 1; i++) {
		for (j = 0; j < size2 + 1; j++) {
			int c = matrix[i * (size2 + 1) + j];
			printf("%d", c);
		}

		printf("\n");
	}
}


short cost(int x) {
	int i, n_iter = 20;
	double dcost = 0;
	for (i = 0; i < n_iter; i++) {
		dcost += pow(sin((double) x), 2) + pow(cos((double) x), 2);
	}

	return (short) (dcost / n_iter + 0.1);
}


int main(int argc, char const *argv[]) {
    FILE *file;

	int seq1_size = 0;
	char *seq1, *seq2;
	int seq2_size = 0;

	int *matrix;

	file = fopen(argv[1], "r");
	if (file != NULL) {		
		fscanf(file, "%d %d", &seq1_size, &seq2_size);

		seq1 = (char *) calloc(seq1_size + 1, sizeof(char));
		seq2 = (char *) calloc(seq2_size + 1, sizeof(char));

		fscanf(file, "%s %s", seq1, seq2);

	} else {
		printf("%s\n", "Unable to open file");
		exit(-1);
	}

	fclose(file);

	matrix = (int*) calloc((seq1_size + 1) * (seq2_size + 1), sizeof(int));
	if (matrix == NULL) {
		printf("Not enough memory. Learning how to program in C might help...\n");
		exit(-1);
	}


	int i, j;
	int gap = seq2_size + 1;

	sem_t* semaphores = (sem_t*) calloc((seq1_size + 1), sizeof(sem_t));
	sem_init(&semaphores[0], 0, seq2_size);
	
	#pragma omp parallel
	{

		#pragma omp for private(j) schedule(static, 1)
		for (i = 1; i < seq1_size + 1; i++) {
			int index = i * gap;
			char xi = seq1[i - 1];
			for (j = 1; j < seq2_size + 1; j++) {
				index++;
				char yj = seq2[j - 1];

				if (xi != yj) {
					sem_wait(&semaphores[i - 1]);
					int top_cell = matrix[index - gap];
					int left_cell = matrix[index - 1];

					if (top_cell > left_cell) {
						matrix[index] = top_cell;
						sem_post(&semaphores[i]);
					} else {
						matrix[index] = left_cell;
						sem_post(&semaphores[i]);
					}

				} else {
					sem_wait(&semaphores[i - 1]);
					matrix[index] = matrix[index - gap - 1] + cost(i);
					sem_post(&semaphores[i]);
				}
			}
		}

		#pragma omp for nowait
		for (i = 0; i < seq1_size + 1; i++) {
			sem_destroy(&semaphores[i]);
		}

		#pragma omp single
		{
			i = seq1_size;
			j = seq2_size;
			char xi, yj;
			int last_cell = matrix[i * gap + j];
			int len = last_cell;
			char lcs[len + 1];
			lcs[len] = '\0';
			
			while(last_cell > 0) {
				int index = i * gap + j;
				xi = seq1[i - 1];
				yj = seq2[j - 1];
				if (xi == yj) {
					lcs[last_cell - 1] = xi;
					last_cell--;
					i--;
					j--;
				} else if (matrix[index - gap] > matrix[index - 1]) {
					i--;
				} else {
					j--;
				}
			}
			
			printf("%d\n%s\n", len, lcs);
		}
	}
	

	free(matrix);
	free(seq1);
	free(seq2);

	return 0;
}