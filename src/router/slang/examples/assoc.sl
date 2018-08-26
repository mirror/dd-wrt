% This example illustrates the use of associative arrays.
% The function 'analyse_file' counts the number of occurrences of each word
% in a specified file.  Once the file has been read in, it writes out
% the list of words and number of occurrences to the file counts.log

define analyse_file (file)
{
   variable fp = fopen (file, "r");
   if (fp == NULL)
     throw OpenError, "Unable to open $file"$;

   % Create an Integer_Type assoc array with default value of 0.
   variable a = Assoc_Type[Integer_Type, 0];

   variable line, word;
   while (-1 != fgets (&line, fp))
     {
	foreach word (strtok (strlow(line), "^\\w"))
	  a[word] = a[word] + 1;    %  default value of 0 assumed!!
     }
   () = fclose (fp);

   variable keys = assoc_get_keys (a);
   variable values = assoc_get_values (a);

   % The default array_sort for Int_Type is an ascending sort.  We want the
   % opposite.
   variable i = array_sort (values; dir=-1);

   fp = fopen ("count.log", "w");
   () = array_map (Int_Type, &fprintf, fp, "%s:\t%d\n", keys[i], values[i]);
   () = fclose (fp);
}
