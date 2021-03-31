#pragma once
#include "ARAPSolver.h"



ARAP::ARAPSolver::ARAPSolver(Model* parsedModel, TriMesh* origMesh)
{
	ModelPointer = parsedModel;
	this->OrigMesh = origMesh;

	computeFanWeights(fanWeights); //construct weights

	sysMatrix = computeSystemMatrix(); //construct initial system Matrix
}


ARAP::ARAPSolver::~ARAPSolver()
{
}

//TODO get constraints
void ARAP::ARAPSolver::ArapStep(int iterations, std::vector<std::pair<int, Vector3f>>& constraints)
{

	/*if (vertexDragging::changedConstraints)
		setSystemMatrixConstraints(constraints);
	vertexDragging::changedConstraints = false;*/

	std::vector<Matrix3f> rotations;
	std::vector<Vector3f> pos;

	for (int ii = 0; ii < iterations; ii++) {

		solveRotations(rotations);
		solvePositions(constraints, rotations, pos);
	}
	//TODO update pos of vertices in ModelPointer

}

Vector3f ARAP::ARAPSolver::vector3f_from_point(const TriMesh::Point& p) {
	return Vector3f(p[0], p[1], p[2]);
}

void ARAP::ARAPSolver::computeFanWeights(std::vector<float>& fanWeights)
{
	for (auto v_it = OrigMesh->vertices_begin(); v_it != OrigMesh->vertices_end(); ++v_it) {
		fanWeights.push_back(1.0f);//TODO compute Weights
	}
}

void ARAP::ARAPSolver::solveRotations(std::vector<Matrix3f>& solvedRotations)
{
	solvedRotations.clear();

	// Iterate over each and every vertex v, v is the center point of the regarded mesh fan
	for (auto v_it = OrigMesh->vertices_begin(); v_it != OrigMesh->vertices_end(); ++v_it) {
		std::vector<float> localWeights;
		std::vector<Vector3f> sourcePointsFan;
		std::vector<Vector3f> deformedPointsFan;

		//Better way to get from glm to eigen?
		const Vector3f center = vector3f_from_point(OrigMesh->point(*v_it));
		glm::vec3 deformCenterVec = ModelPointer->meshes[0].vertices[v_it->idx()].Position;
		const Vector3f center_deformed = Vector3f(deformCenterVec.x, deformCenterVec.y, deformCenterVec.z);
		
		// loop through fan
		for (TriMesh::VertexVertexIter vv_it = OrigMesh->vv_iter(*v_it); vv_it.is_valid(); ++vv_it) {
			//fan_weights.push_back(weights.weights[ii].weight); //TODO Weights
			localWeights.push_back(fanWeights[vv_it->idx()]);

			const OpenMesh::VertexHandle h(vv_it->idx());
			sourcePointsFan.push_back(vector3f_from_point(OrigMesh->point(h)) - center);

			glm::vec3 deformedGlm = ModelPointer->meshes[0].vertices[h.idx()].Position;
			const Vector3f deformedEigen = Vector3f(deformCenterVec.x, deformCenterVec.y, deformCenterVec.z);
			deformedPointsFan.push_back(deformedEigen - center_deformed);
		}

		Matrix3f rotation = procrustes(sourcePointsFan, deformedPointsFan, localWeights);
		solvedRotations.push_back(rotation);
	}
}

//solves for rigid rotations with procrustes algotithm
Eigen::Matrix3f ARAP::ARAPSolver::procrustes(const std::vector<Eigen::Vector3f>& sourcePoints, const std::vector<Eigen::Vector3f>& targetPoints, const std::vector<float>& weights)
{
	//setup
	Matrix<float, Dynamic, 3> Source(sourcePoints.size(), 3);
	Matrix<float, Dynamic, 3> Target(targetPoints.size(), 3);
	Map<const VectorXf> diag(weights.data(), weights.size());

	for (int i = 0; i < sourcePoints.size(); i++) {
		Source.row(i) = sourcePoints[i];
	}
	for (int i = 0; i < targetPoints.size(); i++) {
		Target.row(i) = targetPoints[i];
	}

	JacobiSVD<MatrixXf> svd(Source.transpose() * diag.asDiagonal() * Target, ComputeFullU | ComputeFullV);//Compute SVD to calculate Cov i = SUM(wij * eij * e'ij)
	Matrix3f rotation = svd.matrixV() * svd.matrixU().transpose();

	return rotation;
}

ARAP::SystemMatrix * ARAP::ARAPSolver::computeSystemMatrix()
{
	SystemMatrix ret{};

	const size_t vertexCount = OrigMesh->n_vertices();

	SparseMatrix<float> L(vertexCount, vertexCount);
	L.setZero();

	for (auto v_it = OrigMesh->vertices_begin(); v_it != OrigMesh->vertices_end(); ++v_it) {
		const auto v_idx = v_it->idx();

		// loop through fan
		for (TriMesh::VertexVertexIter vv_it = OrigMesh->vv_iter(*v_it); vv_it.is_valid(); ++vv_it) { //TODO do we consider our starting vertex then too?
			const float weight = fanWeights[vv_it->idx()];
			const auto u_idx = vv_it->idx();

			L.coeffRef(v_idx, v_idx) += weight;
			L.coeffRef(v_idx, u_idx) -= weight;
		}
	}
	L.makeCompressed();

	ret.L_orig = L;

	return &ret;
}

void ARAP::ARAPSolver::setSystemMatrixConstraints(const std::vector<std::pair<int, Vector3f>>& constraints)
{
	sysMatrix->L = sysMatrix->L_orig;
	auto& L = sysMatrix->L;
	for (const auto& con : constraints) {
		const auto idx = con.first;
		for (int i = 0; i < L.cols(); i++)
			L.coeffRef(idx, i) = 0;
		for (int i = 0; i < L.rows(); i++)
			L.coeffRef(i, idx) = 0;
		L.coeffRef(idx, idx) = 1;
	}
	sysMatrix->solver.compute(L);
}

void ARAP::ARAPSolver::solvePositions(const std::vector<std::pair<int, Vector3f>>& constraints, const std::vector<Matrix3f>& rotations, std::vector<Vector3f>& solvedPos)
{
	auto& L = sysMatrix->L;
	auto& L_orig = sysMatrix->L_orig;
	const size_t vertexCount = OrigMesh->n_vertices();

	Matrix<float, Dynamic, 3> b(vertexCount, 3);
	b.setZero();

	//calc rhs b
	for (auto v_it = OrigMesh->vertices_begin(); v_it != OrigMesh->vertices_end(); ++v_it) {
		const auto v_idx = v_it->idx();

		// loop through fan
		for (TriMesh::VertexVertexIter vv_it = OrigMesh->vv_iter(*v_it); vv_it.is_valid(); ++vv_it) {
			const float weight = fanWeights[vv_it->idx()];
			const OpenMesh::VertexHandle u_handle(vv_it->idx());
			const auto u_idx = vv_it->idx();

			Matrix3f m_rot = rotations[v_idx] + rotations[u_idx];
			Vector3f tmp = 0.5f * weight * m_rot * (vector3f_from_point(OrigMesh->point(*v_it)) - vector3f_from_point(OrigMesh->point(u_handle)));
			b.row(v_idx) += tmp;
		}
	}

	//apply constraints to system
	for (const auto& con : constraints) {
		const auto idx = con.first;
		OpenMesh::VertexHandle h(idx);

		// loop through fan
		for (TriMesh::VertexVertexIter vv_it = OrigMesh->vv_iter(h); vv_it.is_valid(); ++vv_it) {
			const float weight = -fanWeights[vv_it->idx()];
			const auto u_idx = vv_it->idx();

			b.row(u_idx) -= weight * con.second;
		}
	}
	for (const auto& con : constraints)
		b.row(con.first) = con.second;

	//solve
	Matrix<float, Dynamic, 3> x;
	x = sysMatrix->solver.solve(b);
	solvedPos.clear();
	for (size_t i = 0; i < vertexCount; i++)
		solvedPos.push_back(x.row(i));
}



