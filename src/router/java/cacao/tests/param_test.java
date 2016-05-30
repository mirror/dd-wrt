
class param_test {
	static public void main(String[]s) {

		System.out.println("int 91 =  "+atest_int(1,2,3,4,5,6,7,8,9,10,
							  11,12,13));
		System.out.println("long 91 = "+atest_long(1l,2l,3l,4l,5l,6l,7l,
					  8l,9l,10l,11l,12l,13l));
		System.out.println("float 561.0 = "+atest_float(1,2,3,4,5,6,7,8,
		    9,10,11,12,13,14,15,16, 17,18,19,20,21,22,23,24,25,26,27,28,
		    29,30,31,32,33));
		System.out.println("double 561.0 = "+atest_double(1.0,2.0,3.0,
	            4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0,13.0,14.0,15.0,16.0,
		    17.0,18.0,19.0,20.0,21.0,22.0,23.0,24.0,25.0,26.0,27.0,28.0,
                    29.0,30.0,31.0,32.0,33.0));
		System.out.println("mixed 91 = "+atest_mixed(1,2l,3l,4l,5l,6,7,8
							     , 9,10,11,12,13));
		System.out.println("mixed4 91 = "+atest_mixed4(1,2l,3l,4,5l,6,7,
							      8,9,10,11,12,13));
		System.out.println("mixed1 91.0 = "+atest_mixed1(1, 2, 3, 4, 5,
                    6, 7, 8, 9, 10, 11, 12, 13));
		System.out.println("mixed2 91.0 = "+atest_mixed2(1, 2, 3, 4, 5, 
                    6, 7, 8, 9, 10, 11, 12, 13));
		System.out.println("mixed3 190.0 = "+atest_mixed3(1, 2, 3, 4, 5,
                    6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19));
	}
	
	static int atest_int(int i, int j, int k, int l, int m, int n, int o, 
			     int p, int q, int r, int s, int t, int u) {
		int a;
	
		a=i+j+k+l+m+n+o+p+q+r+s+t+u;
		return a;
	}
	
	static long atest_long(long i, long j, long k, long l, long m, long n, 
	    long o, long p, long q, long r, long s, long t, long u) {
		long a;
		a=i+j+k+l+m+n+o+p+q+r+s+t+u;
		return a;
	}
	
	static float atest_float(float i, float j, float k, float l, float m, float n, float o, float p, float q, float r,float  s, float t, float u, float v, float w, float x, float y, float z, float b, float c, float d, float e, float f, float g, float h, float ii, float jj , float kk, float ll, float mm, float nn, float oo, float pp) {
		float a;
		a=i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+b+c+d+e+f+g+h+ii+jj+kk+ll+mm+nn+oo+pp;
		return a;
	}
	
	static double atest_double(double i, double j, double k, double l, double m, double n, double o, double p, double q, double r, double s,double  t, double u, double v, double w, double x, double y, double z, double b, double c, double d, double e, double f, double g, double h, double ii, double jj, double kk, double ll, double mm, double nn, double oo, double pp ) {
		double a;
		a=i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+b+c+d+e+f+g+h+ii+jj+kk+ll+mm+nn+oo+pp;
		return a;
	}

        static long atest_mixed(int i, long  j, long k,long  l, long m,int  n, int o,int  p, int q,int  r, int s, int t, int u) {
	    long a;
	    a=i+j+k+l+m+n+o+p+q+r+s+t+u;
	    return a;
	}

        static long atest_mixed4(int i, long  j, long k,int l, long m,int  n, int o,int  p, int q,int  r, int s, int t, int u) {
	    long a;
	    a=i+j+k+l+m+n+o+p+q+r+s+t+u;
	    return a;
	}

	static float atest_mixed1(int i,int  j, int k,float  l, int m,float  n, int o,float  p, int q,float  r, int s, float t, int u) {
		int a;
		float b;
		a=i+j+k+m+o+q+s+u;
		b=l+n+p+r+t+(float)a;
		return b;
	}
	
	static float atest_mixed2(float i,float  j, float k,float  l, int m,float  n, int o,float  p, int q,float  r, int s, float t, int u) {
		int a;
		float b;
		a=m+o+q+s+u;
		b=i+k+j+l+n+p+r+t+(float)a;
		return b;
	}
	
	static float atest_mixed3(int i,int  j, int k,int  l, int m,int  n, int o,float  p, float q,float  r, float s, float t, int u, float v, float w, float x, float y, float z, int zz) {
		int a;
		float b;
		a=i+j+k+l+m+n+o+u+zz;
		b=p+q+r+s+t+v+w+x+y+z+(float)a;
		return b;
	}
}

