#ifndef POW_ALGORITHMS_H
#define POW_ALGORITHMS_H

/**
 * @brief binary power
 *
 * @param b base
 * @param e exponent
 * @return double powered number
 **/
double binary_pow(double b, unsigned long long e);

/**
 * @brief fast calculation of an approximate value of a degree
 *
 * @param a_coeff
 * @param base
 * @return double
 **/
double fast_pow(double a_coeff, double base);

/**
 * @brief fastest float calculation of an approximate value of a degree
 *
 * @param a_coeff
 * @param base
 * @return double
 **/
float fastest_pow(float a, float b);

#endif
