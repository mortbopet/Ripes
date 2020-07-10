#include <math.h>
#include <stdio.h>

float getf() {
	float f;
	char c;
	while(!scanf("%f", &f)) {
		while ((c = getchar()) != '\n' && c != EOF) { } // Clear input buffer
		printf("Invalid number, please try again.\n");
	}
	return f;
}

/**
  * Program calculating the power of a number based on floating-point base
  * and exponent values input by the user.
  */
int main() {
    float base, exp, result;
    printf("Enter a base number: \n");
    base = getf();
    printf("Enter an exponent: \n");
    exp = getf();

    result = pow(base, exp);

    printf("Result: %.1lf^%.1lf = %.2lf\n", base, exp, result);
    return 0;
}
