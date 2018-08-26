#d optgen_text  \__newline__ The optional parameter \exmp{g} may be used to \
 \__newline__ specify the underlying random number generator.  See the \
 \__newline__ documentation for the \sfun{rand_new} function for more information.
#d optparm_text  \optgen_text \
 \__newline__ The \exmp{num} parameter indicates that \exmp{num} random values are to \
 \__newline__ be generated and returned as an array.

\function{rand_exp}
\synopsis{Generate exponentially distributed random numbers}
\usage{X = rand_exp([Rand_Type g,] beta [,num])}
\description
 This function generates random numbers that are distributed according
 to an exponential distribution with parameter beta > 0.  The
 distribution's probability density is given by
#v+
   P(x,beta) = (1/beta) exp(-x/beta)
#v-
 \optparm_text
\notes
 The exponential generator is commonly used to simulate waiting times
 between events.
\seealso{rand_new, rand_uniform}
\done

\function{rand_int}
\synopsis{Generate random integers}
\usage{X = rand_int ([Rand_Type g,] imin, imax [,num])}
\description
 This function may be used to generate a random integer \exmp{X} such
 that \exmp{imin <= X <= imax}.
 \optparm_text
\seealso{rand_uniform, rand, rand_new}
\done

\function{rand_tdist}
\synopsis{Generate random numbers from the Student t distribution}
\usage{X = rand_tdist ([Rand_Type g,] nu [,num])}
\description
 This function generates random numbers that are distributed according
 to the Student-t distribution with nu>0.0 degrees of freedom.
 \optparm_text
\seealso{rand_uniform, rand_new, rand_chisq, rand_fdist}
\done

\function{rand_fdist}
\synopsis{Generate random numbers from the F distribution}
\usage{X = rand_fdist ([Rand_Type g,], nu1, nu2 [,num])}
\description
 This function generates random numbers that are distributed according
 to the F-distribution, which is the ratio of two chi-squared
 distributed variates whose degrees of freedom are given by
 \exmp{nu1} (numerator) and \exmp{nu2} (denominator).
 \optparm_text
\seealso{rand_uniform, rand_chisq, rand_tdist, rand_gauss, rand_new}
\done

\function{rand_chisq}
\synopsis{Generate Chi-Square distributed random numbers}
\usage{X = rand_fdist ([Rand_Type g,] nu, [,num])}
\description
 This function generates random numbers that are distributed according
 to the Chi-squared distribution with \exmp{nu > 0} degrees of freedom.
 \optparm_text
\seealso{rand_uniform, rand_fdist, rand_tdist, rand_gauss, rand_new}
\done

\function{rand_flat}
\synopsis{Generate uniformly distributed random numbers}
\usage{X = rand_fdist ([Rand_Type g,] xmin, xmax [,num])}
\description
 This function generates random double-precision floating point
 numbers that are uniformly distributed in the range
 \exmp{xmin<=X<xmax}.
 \optparm_text
\seealso{rand_uniform, rand_uniform_pos, rand_int, rand, rand_new}
\done

\function{rand_gamma}
\synopsis{Generate Gamma distributed random numbers}
\usage{X = rand_gamma ([Rand_Type g,], k, theta [,num])}
\description
 This function returns random deviates that are Gamma-distributed
 according to the probability density
#v+
   P(x; k,theta) = x^(k-1)*exp(-x/theta)/(theta^k * Gamma(k))
#v-
 where \exmp{k,theta>0.0}.
 \optparm_text
\seealso{rand_beta, rand_uniform, rand_binomial, rand_new}
\done

\function{rand_binomial}
\synopsis{Generate random numbers from the binomial distribution}
\usage{X = rand_binomial ([Rand_Type g,], p, n, [,num])}
\description
  This function generates binomial distributed random numbers
  according to the probability density
#v+
   P(x;p,n) = n!/(k!*(n-k)!) * p^k * (1-p)^(n-k)
#v-
  where \exmp{n} is a non-negative integer and \exmp{0<=p<=1}.
 \optparm_text
\seealso{rand_gamma, rand_poisson, rand_uniform, rand_new}
\done

\function{rand_poisson}
\synopsis{Generate Poisson distributed random numbers}
\usage{k = rand_poisson ([Rand_Type g,] mu [,num])}
\description
 This function generates random unsigned integers that are
 poisson-distributed according to the probability distribution
#v+
   P(k;mu) = mu^k/k! * exp(-mu)
#v-
  where \exmp{mu>0.0}.
 \optparm_text
\seealso{rand_gauss, rand_uniform, rand_binomial, rand_new}
\done

\function{rand_geometric}
\synopsis{Generate random numbers from the geometric distribution}
\usage{k = rand_geometric ([Rand_Type g,] p [,num])}
\description
 This function generates random numbers that are distributed
 according to a geometric distribution with a probability density
#v+
   P(k; p) = p*(1-p)^(k-1)
#v-
 where \exmp{0<=p<=1}
 \optparm_text
\seealso{rand_poisson, rand_exp, rand_gauss, rand_uniform, rand_new}
\done

\function{rand_cauchy}
\synopsis{Generate random numbers from the Cauchy distribution}
\usage{X = rand_cauchy ([Rand_Type g,] gamma [,num])}
\description
 This function generates random numbers that are distributed
 according to a cauchy-distribution with a probability density
#v+
   P(x; gamma) = 1/(PI*gamma)/(1+(x/gamma)^2)
#v-
 where \exmp{gamma>=0.0}.
 \optparm_text
\seealso{rand_gauss, rand_poisson, rand_exp, rand_new}
\done

\function{rand_beta}
\synopsis{Generate random numbers from the beta distribution}
\usage{X = rand_fdist ([Rand_Type g,] a, b [,num])}
\description
 This function generates random numbers that are distributed
 according to a Beta-distribution with a probability density
#v+
   P(x; a,b) = x^(a-1)*(1-x)^(b-1)/B(a,b)
#v-
 where \exmp{a, b > 0}.
\seealso{rand_gamma, rand_binomial, rand_chisq}
\done

\function{rand_gauss}
\synopsis{Generate gaussian-distributed random numbers}
\usage{X = rand_gauss ([Rand_Type g,] sigma [,num])}
\description
 This function generates gaussian random numbers with the specified
 sigma and mean of 0 according to the probability density
#v+
   P(x; sigma) = 1/sqrt(2*PI*sigma^2) * exp(-0.5*x^2/sigma^2)
#v-
 \optparm_text
\notes
  This implementation utilizes the Box-Muller algorithm.
\seealso{rand_uniform, rand_poisson, rand_chisq, rand_gauss, rand_new}
\done

\function{rand}
\synopsis{Generate random integers numbers}
\usage{X = rand ([Rand_Type g,] [,num])}
\description
 This function generates unsigned 32 bit randomly distributed
 integers on the closed interval 0<=X<=0xFFFFFFFFUL.
 \optparm_text
\seealso{rand_new, rand_int, rand_uniform, rand_flat}
\done

\function{rand_uniform_pos}
\synopsis{Generate uniform positive random numbers}
\usage{X = rand_uniform_pos ([Rand_Type] [num])}
\description
  This function generates uniformly distributed random numbers in open
  interval \exmp{0<X<1}.
 \optparm_text
\seealso{rand_uniform, rand_new}
\done

\function{rand_uniform}
\synopsis{Generate uniform random numbers}
\usage{X = rand_uniform ([Rand_Type g] [num])}
\description
  This function generates uniformly distributed double precision
  numbers on the semi-closed interval \exmp{0<=r<1}.
 \optparm_text
\seealso{rand_uniform_pos, rand_int, rand_flat, rand_new}
\done

\function{srand}
\synopsis{Seed the random number generator}
\usage{srand ([Rand_Type g,] Array_Type seeds)}
\description
 This function may be used to seed an instance of a rand number
 generator using the values of an array of an unsigned long integers.
 If a generator (created by \ifun{rand_new}) is specified as the first
 argument, then is will be seeded; otherwise, the seeds will
 get applied to the default generator.
 \optparm_text
\example
#v+
    gen = rand_new ();
    srand (gen, [_time(), _pid(), 0xFF80743]);
#v-
\seealso{rand_new, rand, rand_uniform}
\done

\function{rand_new}
\synopsis{Instantiate a new random number generator}
\usage{Rand_Type rand_new ([array-of-seeds])}
\description
 This function creates a new instance of the basic random number
 generator.  An optional array of 32 bit unsigned integers may be used
 to seed the generator.  By default, the generator is seeding using
 the current time and process id.  The \ifun{srand} function may also
 be used to seed the generator.  The generator created by the
 \ifun{rand_new} function may be passed as the first argument to most
 of the other functions in the module to indicate that this instance
 should be used as the basic generator.
\example
 The following example shows how to create an array of 512 uniform
 random numbers derived from the default instance of the basic
 generator:
#v+
   x = rand_uniform (512);
#v-
 A specific instance of the generator may be created using the
 \ifun{rand_new} function and used by \ifun{rand_uniform} as follows:
#v+
   g = rand_new ([0x1234, 0x5678912, 0xEFAB1234]);
   x = rand_uniform (g, 512);
#v-
\notes
 The generator is a hybrid one that sums the results of 3 separate
 generators: George Marsaglia's MZRAN13 generator, a multiply with
 carry generator (also by Marsaglia), and a product generator.  The
 combined generator has a 192 bit state and a period exceeding 10^46.

 The resulting random sequences were tested using version 2.24.4 of
 the dieharder random number testing program.  The tests showed that
 this generator performed better than the famous Marsenne
 Twister (\exmp{mt19937}) both in terms of randomness and speed.  More
 information about the test results may be found at
 \url{http://www.jedsoft.org/slang/modules/rand.html}.

 The \exmp{mt19937} generator is separately available via the GSL
 module.
\seealso{srand, rand_int, rand_uniform}
\done

\function{rand_sample}
\synopsis{Randomly sample from one or more arrays}
\usage{(b1 [,b2,...]) = rand_sample ([Rand_Type g,] a1 [,a2,...], num);}
\description
 This function may be used to randomly sample \exmp{num} elements from
 one or more arrays (\exmp{a1,...}).  The arrays must be consistent in
 the sense that they must have the same leading dimension, which is
 the one to be sampled.

 The optional first argument may be used to specify a different
 instance of a random number generator.  Otherwise, the default
 generator will be used.
\example
 Suppose A is a 1-d array with 20 elements, and B is a 2d array with
 dimensions [20,30].  Then
#v+
   (A1, B1) = rand_sample (A, B, 5);
#v-
 will produce a 1d array A1 with 5 elements and a 2d array B1 with
 dimensions [5,30].
\notes
 The indices used to sample the arrays are created using the first
 \exmp{num} elements of a random permutation of the integers
 \exmp{[0:dim0-1]} where \exmp{dim0} is the size of the leading
 dimension.  The same set of indices are used for all the arrays.
 Hence any correspondence between \exmp{a0[i,..]} and \exmp{a1[i,...}
 will be preserved in the samples.
\seealso{rand_permutation, rand_new}
\done

\function{rand_permutation}
\synopsis{Generate a random permutation of integers}
\usage{p = rand_permutation ([Rand_Type g,] Int_Type n)}
\description
  This function may be used to generate a random permutation of the
  sequence of integers \exmp{0,1,..,n-1}.
  \optgen_text
\seealso{rand_sample}
\done
