\function{abs}
\synopsis{Compute the absolute value of a number}
\usage{y = abs(x)}
\description
  The \ifun{abs} function returns the absolute value of an arithmetic
  type.  If its argument is a complex number (\dtype{Complex_Type}),
  then it returns the modulus.  If the argument is an array, a new
  array will be created whose elements are obtained from the original
  array by using the \ifun{abs} function.
\seealso{sign, sqr}
\done

\function{acos}
\synopsis{Compute the arc-cosine of a number}
\usage{y = acos (x)}
\description
  The \ifun{acos} function computes the arc-cosine of a number and
  returns the result.  If its argument is an array, the
  \ifun{acos} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{acosh}
\synopsis{Compute the inverse cosh of a number}
\usage{y = acosh (x)}
\description
  The \ifun{acosh} function computes the inverse hyperbolic cosine of a number and
  returns the result.  If its argument is an array, the
  \ifun{acosh} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{asin}
\synopsis{Compute the arc-sine of a number}
\usage{y = asin (x)}
\description
  The \ifun{asin} function computes the arc-sine of a number and
  returns the result.  If its argument is an array, the
  \ifun{asin} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{asinh}
\synopsis{Compute the inverse-sinh of a number}
\usage{y = asinh (x)}
\description
  The \ifun{asinh} function computes the inverse hyperbolic sine of a number and
  returns the result.  If its argument is an array, the
  \ifun{asinh} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{atan}
\synopsis{Compute the arc-tangent of a number}
\usage{y = atan (x)}
\description
  The \ifun{atan} function computes the arc-tangent of a number and
  returns the result.  If its argument is an array, the
  \ifun{atan} function will be applied to each element and the result returned
  as an array.
\seealso{atan2, cos, acosh, cosh}
\done

\function{atan2}
\synopsis{Compute the arc-tangent of the ratio of two variables}
\usage{z = atan2 (y, x)}
\description
  The \ifun{atan2} function computes the arc-tangent of the ratio
  \exmp{y/x} and returns the result as a value that has the
  proper sign for the quadrant where the point (x,y) is located.  The
  returned value \exmp{z} will satisfy (-PI < z <= PI).  If either of the
  arguments is an array, an array of the corresponding values will be returned.
\seealso{hypot, cos, atan, acosh, cosh}
\done

\function{atanh}
\synopsis{Compute the inverse-tanh of a number}
\usage{y = atanh (x)}
\description
  The \ifun{atanh} function computes the inverse hyperbolic tangent of a number and
  returns the result.  If its argument is an array, the
  \ifun{atanh} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{ceil}
\synopsis{Round x up to the nearest integral value}
\usage{y = ceil (x)}
\description
  This function rounds its numeric argument up to the nearest integral
  value. If the argument is an array, the corresponding array will be
  returned.
\seealso{floor, round}
\done

\function{Conj}
\synopsis{Compute the complex conjugate of a number}
\usage{z1 = Conj (z)}
\description
  The \ifun{Conj} function returns the complex conjugate of a number.
  If its argument is an array, the \ifun{Conj} function will be applied to each
  element and the result returned as an array.
\seealso{Real, Imag, abs}
\done

\function{cos}
\synopsis{Compute the cosine of a number}
\usage{y = cos (x)}
\description
  The \ifun{cos} function computes the cosine of a number and
  returns the result.  If its argument is an array, the
  \ifun{cos} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{cosh}
\synopsis{Compute the hyperbolic cosine of a number}
\usage{y = cosh (x)}
\description
  The \ifun{cosh} function computes the hyperbolic cosine of a number and
  returns the result.  If its argument is an array, the
  \ifun{cosh} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{_diff}
\synopsis{Compute the absolute difference of two values}
\usage{y = _diff (x, y)}
\description
  The \ifun{_diff} function returns a floating point number equal to
  the absolute value of the difference of its two arguments.
  If either argument is an array, an array of the corresponding values
  will be returned.
\seealso{abs}
\done

\function{exp}
\synopsis{Compute the exponential of a number}
\usage{y = exp (x)}
\description
  The \ifun{exp} function computes the exponential of a number and
  returns the result.  If its argument is an array, the
  \ifun{exp} function will be applied to each element and the result returned
  as an array.
\seealso{expm1, cos, atan, acosh, cosh}
\done

\function{expm1}
\synopsis{Compute exp(x)-1}
\usage{y = expm1(x)}
\description
  The \ifun{expm1} function computes \exmp{exp(x)-1} and returns the
  result.  If its argument is an array, the \ifun{expm1} function will
  be applied to each element and the results returned as an array.

  This function should be called whenever \exmp{x} is close to 0 to
  avoid the numerical error that would arise in a naive computation of
  \exmp{exp(x)-1}.
\seealso{expm1, log1p, cos, atan, acosh, cosh}
\done

\function{feqs}
\synopsis{Test the approximate equality of two numbers}
\usage{Char_Type feqs (a, b [,reldiff [,absdiff]]}
\description
 This function compares two floating point numbers \exmp{a} and
 \exmp{b}, and returns a non-zero value if they are equal to within a
 specified tolerance; otherwise 0 will be returned.  If either is an
 array, a corresponding boolean array will be returned.

 The tolerances are specified as relative and absolute differences via
 the optional third and fourth arguments.  If no optional arguments
 are present, the tolerances default to \exmp{reldiff=0.01} and
 \exmp{absdiff=1e-6}.  If only the relative difference has been
 specified, the absolute difference (\exmp{absdiff}) will be taken to
 be 0.0.

 For the case when \exmp{|b|>=|a|}, \exmp{a} and \exmp{b} are
 considered to be equal to within the specified tolerances if either
 \exmp{|b-a|<=absdiff} or \exmp{|b-a|/|b|<=reldiff} is true.
\seealso{fneqs, fgteqs, flteqs}
\done

\function{fgteqs}
\synopsis{Compare two numbers using specified tolerances}.
\usage{Char_Type feqs (a, b [,reldiff [,absdiff]]}
\description
  This function is functionally equivalent to:
#v+
     (a >= b) or feqs(a,b,...)
#v-
  See the documentation of \ifun{feqs} for more information.
\seealso{feqs, fneqs, flteqs}
\done

\function{floor}
\synopsis{Round x down to the nearest integer}
\usage{y = floor (x)}
\description
  This function rounds its numeric argument down to the nearest
  integral value. If the argument is an array, the corresponding array
  will be returned.
\seealso{ceil, round, nint}
\done

\function{flteqs}
\synopsis{Compare two numbers using specified tolerances}.
\usage{Char_Type feqs (a, b [,reldiff [,absdiff]]}
\description
  This function is functionally equivalent to:
#v+
     (a <= b) or feqs(a,b,...)
#v-
  See the documentation of \ifun{feqs} for more information.
\seealso{feqs, fneqs, fgteqs}
\done

\function{fneqs}
\synopsis{Test the approximate inequality of two numbers}
\usage{Char_Type feqs (a, b [,reldiff [,absdiff]]}
\description
  This function is functionally equivalent to:
#v+
    not fneqs(a,b,...)
#v-
  See the documentation of \ifun{feqs} for more information.
\seealso{feqs, fgteqs, flteqs}
\done

\function{get_float_format}
\synopsis{Get the format for printing floating point values.}
\usage{String_Type get_float_format ()}
\description
 The \ifun{get_float_format} retrieves the format string used for
 printing single and double precision floating point numbers.  See the
 documentation for the \ifun{set_float_format} function for more
 information about the format.
\seealso{set_float_format}
\done

\function{hypot}
\synopsis{Compute sqrt(x1^2+x2^2+...+xN^2)}
\usage{r = hypot (x1 [,x2,..,xN])}
\description
  If given two or more arguments, \exmp{x1,...,xN}, the \ifun{hypot}
  function computes the quantity \exmp{sqrt(x1^2+...+xN^2)} using an
  algorithm that tries to avoid arithmetic overflow.  If any of the
  arguments is an array, an array of the corresponding values will be
  returned.

  If given a single array argument \exmp{x}, the \ifun{hypot} function
  computes \exmp{sqrt(sumsq(x))}, where \exmp{sumsq(x)} computes
  the sum of the squares of the elements of \exmp{x}.
\example
  A vector in Euclidean 3 dimensional space may be represented by an
  array of three values representing the components of the vector in
  some orthogonal cartesian coordinate system.  Then the length of the
  vector may be computed using the \ifun{hypot} function, e.g.,
#v+
      A = [2,3,4];
      len_A = hypot (A);
#v-
  The dot-product or scalar-product between two such vectors \exmp{A}
  and \exmp{B} may be computed using the \exmp{sum(A*B)}.  It is well
  known that this is also equal to the product of the lengths of the
  two vectors and the cosine of the angle between them.  Hence, the
  angle between the vectors \exmp{A} and \exmp{B} may be computed using
#v+
      ahat = A/hypot(A);
      bhat = B/hypot(B);
      theta = acos(\sum(ahat*bhat));
#v-
  Here, \exmp{ahat} and \exmp{bhat} are the unit vectors associated
  with the vectors \exmp{A} and \exmp{B}, respectively.
  Unfortunately, the above method for computing the angle between the
  vectors is numerically unstable when \exmp{A} and \exmp{B} are
  nearly parallel.  An alternative method is to use:
#v+
      ahat = A/hypot(A);
      bhat = B/hypot(B);
      ab = sum(ahat*bhat);
      theta = atan2 (hypot(bhat - ab*ahat), ab);
#v-
\seealso{atan2, cos, atan, acosh, cosh, sum, sumsq}
\done

\function{Imag}
\synopsis{Compute the imaginary part of a number}
\usage{i = Imag (z)}
\description
  The \ifun{Imag} function returns the imaginary part of a number.
  If its argument is an array, the \ifun{Imag} function will be applied to each
  element and the result returned as an array.
\seealso{Real, Conj, abs}
\done

\function{isinf}
\synopsis{Test for infinity}
\usage{y = isinf (x)}
\description
  This function returns 1 if x corresponds to an IEEE infinity, or 0
  otherwise. If the argument is an array, an array of the
  corresponding values will be returned.
\seealso{isnan, _Inf}
\done

\function{isnan}
\synopsis{isnan}
\usage{y = isnan (x)}
\description
  This function returns 1 if x corresponds to an IEEE NaN (Not a Number),
  or 0 otherwise.  If the argument is an array, an array of
  the corresponding values will be returned.
\seealso{isinf, _NaN}
\done

\function{log}
\synopsis{Compute the logarithm of a number}
\usage{y = log (x)}
\description
  The \ifun{log} function computes the natural logarithm of a number and
  returns the result.  If its argument is an array, the
  \ifun{log} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh, log1p}
\done

\function{log10}
\synopsis{Compute the base-10 logarithm of a number}
\usage{y = log10 (x)}
\description
  The \ifun{log10} function computes the base-10 logarithm of a number and
  returns the result.  If its argument is an array, the
  \ifun{log10} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{log1p}
\synopsis{Compute the logarithm of 1 plus a number}
\usage{y = log1p (x)}
\description
  The \ifun{log1p} function computes the natural logarithm of 1.0 plus
  \exmp{x} returns the result.  If its argument is an array, the
  \ifun{log1p} function will be applied to each element and the results
  returned as an array.

  This function should be used instead of \exmp{log(1+x)} to avoid
  numerical errors whenever \exmp{x} is close to 0.
\seealso{log, expm1, cos, atan, acosh, cosh}
\done

\function{_max}
\synopsis{Compute the maximum of two or more numeric values}
\usage{z = _max (x1,...,xN)}
\description
  The \ifun{_max} function returns a floating point number equal to
  the maximum value of its arguments.  If any of the argiments are
  arrays (of equal length), an array of the corresponding values will
  be returned.
\notes
  This function returns a floating point result even when the
  arguments are integers.
\seealso{_min, min, max}
\done

\function{_min}
\synopsis{Compute the minimum of two or more numeric values}
\usage{z = _min (x1,...,xN)}
\description
  The \ifun{_min} function returns a floating point number equal to
  the minimum value of its arguments.  If any of the argiments are
  arrays (of equal length), an array of the corresponding values will
  be returned.
\notes
  This function returns a floating point result even when the
  arguments are integers.
\seealso{min, _max, max}
\done

\function{mul2}
\synopsis{Multiply a number by 2}
\usage{y = mul2(x)}
\description
  The \ifun{mul2} function multiplies an arithmetic type by two and
  returns the result.  If its argument is an array, a new array will
  be created whose elements are obtained from the original array by
  using the \ifun{mul2} function.
\seealso{sqr, abs}
\done

\function{nint}
\synopsis{Round to the nearest integer}
\usage{i = nint(x)}
\description
  The \ifun{nint} rounds its argument to the nearest integer and
  returns the result.  If its argument is an array, a new array will
  be created whose elements are obtained from the original array
  elements by using the \ifun{nint} function.
\seealso{round, floor, ceil}
\done

\function{polynom}
\synopsis{Evaluate a polynomial}
\usage{Double_Type polynom([a0,a1,...aN], x [,use_factorial])}
\description
 The \ifun{polynom} function returns the value of the polynomial expression
#v+
     a0 + a1*x + a2*x^2 + ... + aN*x^N
#v-
 where the coefficients are given by an array of values
 \exmp{[a0,...,aN]}.  If \exmp{x} is an array, the function will
 return a corresponding array.  If the value of the optional
 \exmp{use_factorial} parameter is non-zero, then each term in the sum
 will be normalized by the corresponding factorial, i.e.,
#v+
     a0/0! + a1*x/1! + a2*x^2/2! + ... + aN*x^N/N!
#v-
\notes
  Prior to version 2.2, this function had a different calling syntax
  and and was less useful.

  The \ifun{polynom} function does not yet support complex-valued
  coefficients.

  For the case of a scalar value of \exmp{x} and a small degree
  polynomial, it is more efficient to use an explicit expression.
\seealso{exp}
\done

\function{Real}
\synopsis{Compute the real part of a number}
\usage{r = Real (z)}
\description
  The \ifun{Real} function returns the real part of a number. If its
  argument is an array, the \ifun{Real} function will be applied to
  each element and the result returned as an array.
\seealso{Imag, Conj, abs}
\done

\function{round}
\synopsis{Round to the nearest integral value}
\usage{y = round (x)}
\description
  This function rounds its argument to the nearest integral value and
  returns it as a floating point result. If the argument is an array,
  an array of the corresponding values will be returned.
\seealso{floor, ceil, nint}
\done

\function{set_float_format}
\synopsis{Set the format for printing floating point values.}
\usage{set_float_format (String_Type fmt)}
\description
  The \ifun{set_float_format} function is used to set the floating
  point format to be used when floating point numbers are printed.
  The routines that use this are the traceback routines and the
  \ifun{string} function, any anything based upon the \ifun{string}
  function. The default value is \exmp{"%S"}, which causes the number
  to be displayed with enough significant digits such that
  \exmp{x==atof(string(x))}.
\example
#v+
     set_float_format ("%S");        % default
     s = string (PI);                %  --> s = "3.141592653589793"
     set_float_format ("%16.10f");
     s = string (PI);                %  --> s = "3.1415926536"
     set_float_format ("%10.6e");
     s = string (PI);                %  --> s = "3.141593e+00"
#v-
\seealso{get_float_format, string, sprintf, atof, double}
\done

\function{sign}
\synopsis{Compute the sign of a number}
\usage{y = sign(x)}
\description
  The \ifun{sign} function returns the sign of an arithmetic type.  If
  its argument is a complex number (\dtype{Complex_Type}), the
  \ifun{sign} will be applied to the imaginary part of the number.  If
  the argument is an array, a new array will be created whose elements
  are obtained from the original array by using the \ifun{sign}
  function.

  When applied to a real number or an integer, the \ifun{sign} function
  returns \-1, \0, or \exmp{+1} according to whether the number is
  less than zero, equal to zero, or greater than zero, respectively.
\seealso{abs}
\done

\function{sin}
\synopsis{Compute the sine of a number}
\usage{y = sin (x)}
\description
  The \ifun{sin} function computes the sine of a number and
  returns the result.  If its argument is an array, the
  \ifun{sin} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{sinh}
\synopsis{Compute the hyperbolic sine of a number}
\usage{y = sinh (x)}
\description
  The \ifun{sinh} function computes the hyperbolic sine of a number and
  returns the result.  If its argument is an array, the
  \ifun{sinh} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{sqr}
\synopsis{Compute the square of a number}
\usage{y = sqr(x)}
\description
  The \ifun{sqr} function returns the square of an arithmetic type.  If its
  argument is a complex number (\dtype{Complex_Type}), then it returns
  the square of the modulus.  If the argument is an array, a new array
  will be created whose elements are obtained from the original array
  by using the \ifun{sqr} function.
\notes
  For real scalar numbers, using \exmp{x*x} instead of \exmp{sqr(x)}
  will result in faster executing code.  However, if \exmp{x} is an
  array, then \exmp{sqr(x)} will execute faster.
\seealso{abs, mul2}
\done

\function{sqrt}
\synopsis{Compute the square root of a number}
\usage{y = sqrt (x)}
\description
  The \ifun{sqrt} function computes the square root of a number and
  returns the result.  If its argument is an array, the
  \ifun{sqrt} function will be applied to each element and the result returned
  as an array.
\seealso{sqr, cos, atan, acosh, cosh}
\done

\function{tan}
\synopsis{Compute the tangent of a number}
\usage{y = tan (x)}
\description
  The \ifun{tan} function computes the tangent of a number and
  returns the result.  If its argument is an array, the
  \ifun{tan} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{tanh}
\synopsis{Compute the hyperbolic tangent of a number}
\usage{y = tanh (x)}
\description
  The \ifun{tanh} function computes the hyperbolic tangent of a number and
  returns the result.  If its argument is an array, the
  \ifun{tanh} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{_ispos}
\synopsis{Test if a number is greater than 0}
\usage{Char_Type _ispos(x)}
\description
  This function returns 1 if a number is greater than 0, and zero
  otherwise.  If the argument is an array, then the corresponding
  array of boolean (\dtype{Char_Type}) values will be returned.
\seealso{_isneg, _isnonneg}
\done

\function{_isneg}
\synopsis{Test if a number is less than 0}
\usage{Char_Type _isneg(x)}
\description
  This function returns 1 if a number is less than 0, and zero
  otherwise.  If the argument is an array, then the corresponding
  array of boolean (\dtype{Char_Type}) values will be returned.
\seealso{_ispos, _isnonneg}
\done

\function{_isnonneg}
\synopsis{Test if a number is greater than or equal to 0}
\usage{Char_Type _isnonneg(x)}
\description
  This function returns 1 if a number is greater or equal to 0, and zero
  otherwise.  If the argument is an array, then the corresponding
  array of boolean (\dtype{Char_Type}) values will be returned.
\seealso{_isneg, _isnonneg}
\done
