#pragma once

#include <Vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>

class DrawingTrianglesExample
{
public:
	DrawingTrianglesExample();
	~DrawingTrianglesExample();
	void run();
};
