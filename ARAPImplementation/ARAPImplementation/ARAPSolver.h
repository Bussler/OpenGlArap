#pragma once
#include "MeshLoader.h"
//#include "VertexDragging.h"
#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <Eigen/SparseCholesky>
#include <iostream>
#include "eigen_containers.hpp"

using namespace Eigen;

namespace ARAP {

	struct SystemMatrix {
		Eigen::SparseMatrix<float> L_orig; // the original system matrix
		Eigen::SparseMatrix<float> L; // the system matrix with constraints applied
		Eigen::SimplicialLLT<Eigen::SparseMatrix<float>> solver; // solver, stores a reference to L.
	};

	struct FanWeight {
		OpenMesh::VertexHandle vertex; // The vertex index of the vertex in the Mesh.
		float weight; // The corresponding weight.
	};

	struct FanWeights {
		// maps from 'vertex index in mesh' to index into 'weights'.
		// (The number of neighbors is implicitly given by 'offsets[idx+1] - offsets[idx]')
		std::vector<size_t> offsets;

		// The edge weights.
		// If a vertex has multiple neighbors, they are stored consecutively.
		std::vector<FanWeight> weights;
	};

	class ARAPSolver
	{
	public:
		//Data
		Model * ModelDataPointer;
		TriMesh OrigMesh;

		//constructor
		ARAPSolver(Model* parsedModel, TriMesh& origMesh);
		~ARAPSolver();
		
		//interface to give constraints and movement in the ModelPointer and to calculate new pos of the Mesh into the ModelPointer
		void ArapStep(int iterations);

		void toggleConstraint(int idx);
		void untoggleConstraint(int i);
		void UpdateConstraint(int idx, glm::vec3 pos);

	private:
		SystemMatrix sysMatrix;

		std::vector<std::pair<int, Vector3f>> constraints; //constraint list
		bool changedConstraints = false;
		FanWeights edgeWeights; // calculate weights of mesh

		Vector3f vector3f_from_point(const TriMesh::Point& p);
		float compute_weight(TriMesh::Point v, TriMesh::Point u, TriMesh::Point other);
		FanWeights computeFanWeights();
		void solveRotations(const TriMesh &mesh, vector_Matrix3f& solvedRotations, const vector_Vector3f& targetPos); //orig points in OrigMesh, new deformed points in ModelPointer
		Eigen::Matrix3f procrustes(const vector_Vector3f& sourcePoints, const vector_Vector3f& targetPoints, const std::vector<float>& weights);
		
		void computeSystemMatrix(SystemMatrix& mat);
		void setSystemMatrixConstraints(const std::vector<std::pair<int, Vector3f>>& constraints);
		void solvePositions(const std::vector<std::pair<int, Vector3f>>& constraints, const vector_Matrix3f& rotations, vector_Vector3f& solvedPos);

	};

}


