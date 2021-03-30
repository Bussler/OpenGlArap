#pragma once
#include "MeshLoader.h"
#include <iostream>
#include <vector>
#include <Eigen/Dense>

using namespace Eigen;

namespace ARAP {

	class ARAPSolver
	{
	public:
		ARAPSolver(Model* parsedModel, TriMesh* origMesh);
		~ARAPSolver();
		

	private:
		Model* ModelPointer;
		TriMesh* OrigMesh;

		//TODO constraint list
		//TODO calculate weights of mesh

		Vector3f vector3f_from_point(const TriMesh::Point& p);
		void solveRotations(std::vector<Matrix3f>& solvedRotations); //orig points in OrigMesh, new deformed points in ModelPointer
		Eigen::Matrix3f procrustes(const std::vector<Vector3f>& sourcePoints, const std::vector<Vector3f>& targetPoints, const std::vector<float>& weights);
		

	};

}


