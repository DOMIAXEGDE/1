/*
0123456789
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Program by Dominic Alexander Cooper

int main() {
    // Code adapted by DAC from lynn on https://stackoverflow.com

    // k is the exponent + number of cells
    // k+1 values must perfectly fill the size of the set of elements in question

    FILE *fp = fopen("SOLUTION_RENAME.txt", "w");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    //char a[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 \t\n";
    char a[] = 
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"
    " \t\n\r\f\v"
    ;

    int k = strlen(a) - 1;
    printf("k = %d;\n", k);
    
    int noc;
    printf("n = ");
    scanf("%d", &noc);
    printf("\nNumber Of FILE Cells = %d\n", noc);
    
    int n = noc;
    int row, col;
    int rdiv;
    int id = 0;
    int nbr_comb = pow(k + 1, n);
    
    for (row = 0; row < nbr_comb; row++) {
        id++;
        fprintf(fp, "\nF%d\n", id);
        
        for (col = n - 1; col >= 0; col--) {
            rdiv = pow(k + 1, col);
            int cell = (row / rdiv) % (k + 1);
            fprintf(fp, "%c", a[cell]);
        }
    }

    fprintf(fp, "\n\nEnd.(k+1)^n = (%d + 1)^%d = %d\n", k, n, id);
    fclose(fp);
    
    return 0;
}