#include <stdio.h>


int main() {
	FILE *file;
	char filename[] = "Output.txt";
	char text[] = "--Sample Text--";


	file = fopen(filename, "w");

	if (file == NULL) {
		printf("Error, file has not opened.\n");

	return 1;
	}



	fprintf(file, "%s", text);
	fclose(file);

	printf("String succesfully written to %s\n", filename);



#ifdef _WIN32

	system("notepad Output.txt");


#elif _linux_

	system("xdg-open Output.txt");

#endif


return 0;
}
