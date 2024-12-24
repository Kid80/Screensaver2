#include <iostream>
#ifndef _DEBUG
#include <windows.h>
#endif

#include "screensaver.h"

// Handling for if started with /c
// TODO: Do if can be bothered
void show_settings() {

}

// Handling for if started with /p
// TODO: Do if can be bothered
void start_with_handle(int handle) {

}

/*int main() {
	FILE* pFile;
	pFile = fopen("example.txt", "w");
	if (pFile != nullptr) {
		fprintf(pFile, "Hello, %s!\n", "World");
		fclose(pFile);
	}
	return 0;
}*/	

#ifndef _DEBUG
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdline, int cmdshow) {
	int argc;
	wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	int argIndex = 1;
	while (argIndex < argc) {
		if (argv[argIndex][0] == '/') {
			if (argv[argIndex][1] == 's') {
				start_screensaver();
			}
			else if (argv[argIndex][1] == 'c') {
				show_settings();
			}
			else if (argv[argIndex][1] == 'p') {
				argIndex++;
				// Do stuff Here
			}
			argIndex++;
		}
	}
	return 0;
}
#endif

#ifdef _DEBUG
int main(int argc, char* argv[]) {
	int argIndex = 1;
	while (argIndex < argc) {
		if (argv[argIndex][0] == '/'){
			if (argv[argIndex][1] == 's') {
				start_screensaver();
			}
			else if (argv[argIndex][1] == 'c') {
				show_settings();
			}
			else if (argv[argIndex][1] == 'p') {
				argIndex++;
				// TODO: Do the stuff here
			}
			argIndex++;
		}
	}
	return 0;
}
#endif