#include "Application.h"

int main()
{
	Application app;
	if (app.Run() == -1) {
		return 1;
	}
	return 0;
}
