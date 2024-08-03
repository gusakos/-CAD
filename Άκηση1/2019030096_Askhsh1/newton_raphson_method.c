#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_DEGREE 100

// finds the derivative of given polyonym
void find_derivative(float coeffs[], int degree, float derivative[])        // 
{
    int i;

    for (i = 0; i < degree; i++)
    {
        derivative[i] = (degree-i) * coeffs[degree-i];

    }

}

// gives the polynym 
float f(float coeffs[], float x)
{

    float res= coeffs[5]*x*x*x*x*x + coeffs[4]*x*x*x*x + coeffs[3]*x*x*x + coeffs[2]*x*x + coeffs[1]*x + coeffs[0];
    return res;
}


// gives the derivative of the  polyonym
float f_der(float derivative[],float x ){
    float res= derivative[0]*x*x*x*x + derivative[1]*x*x*x + derivative[2]*x*x + derivative[3]*x + derivative[4];
    return res;
}


// calculates the result with the derivative given from f_der , Not with the definiotion of derivative
float newton_raphson(float x0, float tolerance, int max_iterations, float coeffs[], float derivative[])
{
    float x = x0;
    int i;
    for (i = 0; i < max_iterations; i++) {
        float fx = f(coeffs,x);
        float fdr = f_der(derivative,x);
        float delta_x = fx / fdr;

        x = x - delta_x;
       
        if (fabs(delta_x) < tolerance) {    //tolerance is 10 ^-3
            printf("Root found after %d iterations , %d subtractions, %d divisions ,9 additions and 9 multiplications    \n", i+1, i+1, i+1);
            return x;
        }
    }
    printf("Failed to find root with der after %d iterations.\n", max_iterations);
    return 11111111;    
}


// calculates the result  with the definiotion of derivative
float newton_raphson_with_definition(float x0, float tolerance, int max_iterations, float coeffs[], float delta)
{
    float x = x0;
    int i;
    for (i = 0; i < max_iterations; i++) {
        float fx = f(coeffs,x);
        float fdr = (f(coeffs,(x+delta))- fx)/delta;

        float delta_x = fx / fdr;
        x = x - delta_x;
        if (fabs(delta_x) < tolerance) {    //tolerance is 10 ^-3
            printf("Root found after %d iterations ,%d subtractions,%d  divisions ,10 additions and 10 multiplications    \n", i+1,i+2,i+2);
            return x;
        }
    }
    printf("Failed to find root WITH definition after %d iterations.\n", max_iterations);
    return 11111111;                    // 111111 is returned in case of failure followed by a message root not found
}


int main()
{
    float coeffs[MAX_DEGREE+1]= {0};        //coefficients of the polyonym
    float derivative[MAX_DEGREE+1]={0};     // coefficients of the derivative of the polyonym

    float delta= 1e-3;                      // given delta to calulate the derivative with definition
    int degree, i;
    float x0 = 0;                           // Initial guess for the root
    float tolerance = 1e-3;                 // Tolerance for convergence
    int max_iterations = 25;                // Maximum number of iterations

    // Read the polynomial coefficients from user input
    printf("Enter the degree of the polynomial: ");
    scanf("%d", &degree);
    printf("Enter the coefficients of the polynomial from highest to lowest degree: ");
    for (i = degree; i >= 0; i--)
    {
        scanf("%f", &coeffs[i]);
    }

    // Find the derivative of the polynomial
    find_derivative(coeffs, degree, derivative);


    printf("\n");


    printf("Here is the result of the first method \n" );

    float root1 = newton_raphson(x0, tolerance, max_iterations, coeffs, derivative);
    if (root1 !=11111111 ){
        printf("The root is approximately %f\n \n", root1);
    }
    printf("Here is the result of the second method \n" );

    float root2 = newton_raphson_with_definition(x0, tolerance, max_iterations, coeffs, delta);
     if (root2 !=11111111 ){
        printf("The root is approximately %f\n", root2);
     }

    return 0;


}
