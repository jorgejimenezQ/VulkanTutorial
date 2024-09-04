// Include the DrawingTrianglesExample.cpp file
#include "DrawingTrianglesExample.h"

int main() {	
	DrawingTrianglesExample app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}