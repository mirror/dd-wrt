
import java.io.*;

public class x {
	static public char[][] text;
	static public int letters;
	final static int maxwidth = 200;
	final static int maxheight = 200;

	static public void main(java.lang.String [] argv) throws 
	    FileNotFoundException, IOException {

		int l,c,width,height;
		char a;
		int i;
		FileInputStream input = new FileInputStream ("x.java");
		PrintStream output = System.out;

	output.println ("Ausgabe startet");
		
		text =  new char[maxheight][maxwidth];
	output.println ("Feld angelegt");
	
		for (l=0; l<maxheight; l++) for (c=0; c<maxwidth; c++) 
			text[l][c] = ' ';
		
		letters = 0;
		width = 0;
		height = 0;

		l=0; c=0;
		try {
		 while ( (i=input.read()) != -1) {
			if (i == '\n') { c=0; l++; }
			else {
				if (i == '\t') { c = ( (c/4 + 1) * 4); }
				else {
					if (c<maxwidth && l<maxheight) {
						a = (char) i;
						text[l][c] = a;
						if (c>=width)  width=c+1;
						if (l>=height) height=l+1;
						}
					c++;
					}
				}
			}
		} catch (Throwable e) { };
		
	    output.println ("------------------------------------------------------");

		for (c=width-1; c>=0; c--) {
			for (l=0; l<height; l++) {
				if ( (a=text[l][c]) != ' ' ) letters ++;
				i = a;
				output.write (i);
				}
			output.println ();
			}

		output.println ("------------------------------------------------------");
		output.println ("Abdruckbare Buchstaben im Text: " + Integer.toString(letters) );


		output.flush();

		}


	}

