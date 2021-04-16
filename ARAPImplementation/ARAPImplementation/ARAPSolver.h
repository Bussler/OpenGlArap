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

	//struct for the systemMatrix that is needed to solve for positions
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
		// maps from 'vertex index in mesh' to index into 'weights' vector. For each vertex, we can get the outgoing edges and their weights.
		// The number of neighbors for a vertex is implicitly given by 'offsets[idx+1] - offsets[idx]'
		std::vector<size_t> offsets;

		// The edge weights.
		// If a vertex has multiple neighbors, they are stored consecutively.
		std::vector<FanWeight> weights;
	};

	class ARAPSolver
	{
	public:
		//Data
		Model * ModelDataPointer; //Mesh to be rendered
		TriMesh OrigMesh; //Original Mesh in base position. Used as initial guess for rotation solving

		//constructor
		ARAPSolver(Model* parsedModel, TriMesh& origMesh);
		~ARAPSolver();
		
		//performs ARAP algorithm and calculations rigid deformation. Constraints have to be toggled beforehand and their positions (from dragging) updated.
		void ArapStep(int iterations);

		void toggleConstraint(int idx); //registers vertex with id idx as a constraint that is not moved by the algorithm
		void untoggleConstraint(int i); //remove constraint i from the constraint list
		void UpdateConstraint(int idx, glm::vec3 pos); //updates the position of a vertex with id idx that is a registered constraint with the new pos

	private:
		SystemMatrix sysMatrix;

		std::vector<std::pair<int, Vector3f>> constraints; //constraint list: idx of vertex, vertex pos
		bool changedConstraints = false; //if we change pos of membership of our constraint list, we have to update our SystemMatrix
		FanWeights edgeWeights; // calculate weights of mesh

		Vector3f vector3f_from_point(const TriMesh::Point& p); //converts a TriMeshPoint into Vector3f
		float compute_weight(TriMesh::Point v, TriMesh::Point u, TriMesh::Point other); //compute weight from two points
		FanWeights computeFanWeights(); //compute all weights

		//solve target rotations from original Mesh frame pose. Initial Guess: previous frame (targetPos), solved Rotations in solvedRotations
		void solveRotations(const TriMesh &mesh, vector_Matrix3f& solvedRotations, const vector_Vector3f& targetPos);
		
		//solve for rotation matrices from base mesh pose to target mesh pose with the procrusts algorithm
		Eigen::Matrix3f procrustes(const vector_Vector3f& sourcePoints, const vector_Vector3f& targetPoints, const std::vector<float>& weights);
		
		void computeSystemMatrix(SystemMatrix& mat); //compute system Matrix L for solving of the new Positions
		void setSystemMatrixConstraints(const std::vector<std::pair<int, Vector3f>>& constraints); //update system matrix if we changed out constraints (membership/ pos)

		//solve for new Positions (solvedPos) by updating the rhs of our equation system with the previously solved rotations and updating rhs with our constraints
		void solvePositions(const std::vector<std::pair<int, Vector3f>>& constraints, const vector_Matrix3f& rotations, vector_Vector3f& solvedPos);

	};

}


