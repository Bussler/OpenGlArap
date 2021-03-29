#pragma once
#include "MeshLoader.h"
#include <iostream>
#include <vector>

namespace ARAP {

	class ARAPSolver
	{
	public:
		ARAPSolver(Model* parsedModel);
		~ARAPSolver();
		

	private:
		Model* ModelPointer;
		

	};

}


