char *readsoname(char *name, FILE *file, int expected_type, 
		 int *type, int elfclass);
char *readsoname32(char *name, FILE *file, int expected_type, int *type);
char *readsoname64(char *name, FILE *file, int expected_type, int *type);
