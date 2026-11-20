#include "Application.h"

int main(int argc, char** argv)
{
	Application app;
	if (app.Run(argc, argv) == -1) {
		return 1;
	}
	return 0;
}
