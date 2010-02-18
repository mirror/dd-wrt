% This example illustrates the use of associative arrays.  
% The function 'analyse_file' counts the number of occurrences of each word
% in a specified file.  Once the file has been read in, it writes out
% the list of words and number of occurrences to the file counts.log

define analyse_file (file)
{
   variable fp;
   variable line;
   variable i, a, n, word;
   variable keys, values;

   fp = fopen (file, "r");
   if (fp == NULL)
     verror ("Unable to open %s", file);
   
   % Create an Integer_Type assoc array with default value of 0.
   a = Assoc_Type[Integer_Type, 0];
   
   while (-1 != fgets (&line, fp))
     {
	foreach word (strtok (strlow(line), "^a-zA-Z\d128-\d255"))
	  a[word] = a[word] + 1;    %  default value of 0 assumed!!
     }
   
   () = fclose (fp);
   keys = assoc_get_keys (a);
   values = assoc_get_values (a);

   i = array_sort (values);
   keys = keys[i];
   values = values[i];

   fp = fopen ("count.log", "w");
   % The default array_sort for Int_Type is an ascending sort.  We want the 
   % opposite.
   for (i = n-1; i >= 0; i--)
     {
	() = fputs (sprintf ("%s:\t%d\n", keys[i], values[i]), fp);
     }
   () = fclose (fp);
}

