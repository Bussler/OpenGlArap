#pragma once
#include "ARAPSolver.h"



ARAP::ARAPSolver::ARAPSolver(Model* parsedModel, TriMesh& origMesh)
{
	ModelDataPointer = parsedModel;
	this->OrigMesh = origMesh;

	edgeWeights = computeFanWeights(); //construct weights

	computeSystemMatrix(sysMatrix); //construct initial system Matrix

}


ARAP::ARAPSolver::~ARAPSolver()
{
}


void ARAP::ARAPSolver::ArapStep(int iterations)
{
	if (constraints.size() == 0)
		return;

	if (changedConstraints)
		setSystemMatrixConstraints(constraints);
	changedConstraints = false;

	vector_Matrix3f rotations;
	vector_Vector3f pos;

	//init pos with vertex pos from last frame 
	for (int i = 0; i < ModelDataPointer->meshes[0].vertices.size();i++) { //TODO better init with strides!
		glm::vec3 gvp = ModelDataPointer->meshes[0].vertices[i].Position;
		Vector3f vp(gvp.x, gvp.y, gvp.z);
		pos.push_back(vp);
	}

	for (int ii = 0; ii < iterations; ii++) { //vertex iterations

		solveRotations(OrigMesh, rotations, pos);
		solvePositions(constraints, rotations, pos);
	}
	
	//update pos of vertices in ModelPointer TODO find faster solution
	for (int i = 0;i < pos.size();i++) {
		Vector3f curPos = pos.at(i);
		ModelDataPointer->meshes[0].vertices[i].Position = glm::vec3(curPos.x(), curPos.y(), curPos.z());
	}
	ModelDataPointer->meshes[0].UpdateMeshVertices();

}

void ARAP::ARAPSolver::toggleConstraint(int idx)
{
	glm::vec3 vertPos(ModelDataPointer->meshes[0].vertices[idx].Position);
	std::pair<int, Vector3f> constraint(idx, Vector3f(vertPos.x, vertPos.y, vertPos.z));
	constraints.push_back(constraint);

	changedConstraints = true;
}

void ARAP::ARAPSolver::untoggleConstraint(int i)
{
	constraints.erase(constraints.begin() + i);
	changedConstraints = true;
}

void ARAP::ARAPSolver::UpdateConstraint(int idx, glm::vec3 pos)
{
	for (int i = 0; i < constraints.size();i++) {
		if (constraints.at(i).first == idx)
			constraints.at(i).second = Vector3f(pos.x, pos.y, pos.z);
	}
	changedConstraints = true;
}

Vector3f ARAP::ARAPSolver::vector3f_from_point(const TriMesh::Point& p) {
	changedConstraints = true;
	return Vector3f(p[0], p[1], p[2]);
}

float ARAP::ARAPSolver::compute_weight(TriMesh::Point v, TriMesh::Point u, TriMesh::Point other) {
	Vector3f vec1 = (vector3f_from_point(u) - vector3f_from_point(other)).normalized();
	Vector3f vec2 = (vector3f_from_point(v) - vector3f_from_point(other)).normalized();
	double angle = acos(vec1.dot(vec2));
	return 1 / tan(angle);
}

ARAP::FanWeights ARAP::ARAPSolver::computeFanWeights()
{
	FanWeights all_weights;

	for (auto v_it = OrigMesh.vertices_begin(); v_it != OrigMesh.vertices_end(); ++v_it) {
		all_weights.offsets.push_back(all_weights.weights.size()); // mark offset for this vertex

		for (TriMesh::VertexOHalfedgeIter voh_it = OrigMesh.cvoh_iter(*v_it); voh_it.is_valid(); ++voh_it) {
			float weight = 0;
			TriMesh::VertexHandle u = voh_it->to();
			TriMesh::HalfedgeHandle oheh(OrigMesh.opposite_halfedge_handle(*voh_it));

			if (!OrigMesh.is_boundary(*voh_it)) {
				TriMesh::HalfedgeHandle nxt_heh = OrigMesh.next_halfedge_handle(*voh_it);
				TriMesh::VertexHandle other = OrigMesh.to_vertex_handle(nxt_heh);
				weight += compute_weight(OrigMesh.point(*v_it), OrigMesh.point(u), OrigMesh.point(other));
			}
			if (!OrigMesh.is_boundary(oheh)) {
				TriMesh::HalfedgeHandle prv_heh = OrigMesh.prev_halfedge_handle(oheh);
				TriMesh::VertexHandle other = OrigMesh.from_vertex_handle(prv_heh);
				weight += compute_weight(OrigMesh.point(*v_it), OrigMesh.point(u), OrigMesh.point(other));
			}
			if (!OrigMesh.is_boundary(*voh_it) && !OrigMesh.is_boundary(oheh)) {
				weight /= 2;
			}

			//weight = 1.f / (mesh.point(*v_it) - mesh.point(u)).norm();
			if (weight < 0)
				weight = 0;

			FanWeight fw{ u, weight };
			all_weights.weights.push_back(fw);
		}

		//fanWeights.push_back(1.0f);//TODO compute Weights
	}

	all_weights.offsets.push_back(all_weights.weights.size()); // end marker

	return all_weights;
}

void ARAP::ARAPSolver::solveRotations(const TriMesh &mesh, vector_Matrix3f& solvedRotations, const vector_Vector3f& targetPos)
{
	solvedRotations.clear();

	// Iterate over each and every vertex v, v is the center point of the regarded mesh fan
	for (auto v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it) {
		std::vector<float> localWeights;

		vector_Vector3f sourcePointsFan;
		vector_Vector3f deformedPointsFan;

		//Better way to get from glm to eigen?
		const Vector3f center = vector3f_from_point(mesh.point(*v_it));
		glm::vec3 deformCenterVec = ModelDataPointer->meshes[0].vertices[v_it->idx()].Position;
		const Vector3f center_deformed = Vector3f(deformCenterVec.x, deformCenterVec.y, deformCenterVec.z);

		const size_t weight_idx_start = edgeWeights.offsets[v_it->idx()];
		const size_t weight_idx_end = edgeWeights.offsets[v_it->idx() + 1];

		// loop through fan 
		for (size_t ii = weight_idx_start; ii < weight_idx_end; ii++) {
			//fan_weights.push_back(weights.weights[ii].weight); //TODO Weights
			localWeights.push_back(edgeWeights.weights[ii].weight);

			const OpenMesh::VertexHandle h= edgeWeights.weights[ii].vertex;
			sourcePointsFan.push_back(vector3f_from_point(mesh.point(h)) - center);

			deformedPointsFan.push_back(targetPos[h.idx()] - center_deformed);
		}

		Matrix3f rotation = procrustes(sourcePointsFan, deformedPointsFan, localWeights);
		solvedRotations.push_back(rotation);
	}
}

//solves for rigid rotations with procrustes algotithm
Eigen::Matrix3f ARAP::ARAPSolver::procrustes(const vector_Vector3f& sourcePoints, const vector_Vector3f& targetPoints, const std::vector<float>& weights)
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

void ARAP::ARAPSolver::computeSystemMatrix(ARAP::SystemMatrix& mat)
{
	SystemMatrix ret{};

	const size_t vertexCount = OrigMesh.n_vertices();

	SparseMatrix<float> L(vertexCount, vertexCount);
	L.setZero();

	for (auto v_it = OrigMesh.vertices_begin(); v_it != OrigMesh.vertices_end(); ++v_it) {
		const auto v_idx = v_it->idx();

		const size_t weight_idx_start = edgeWeights.offsets[v_it->idx()];
		const size_t weight_idx_end = edgeWeights.offsets[v_it->idx() + 1];

		// loop through fan
		for (size_t jj = weight_idx_start; jj < weight_idx_end; jj++) {
			const float weight = edgeWeights.weights[jj].weight;
			const auto u_handle = edgeWeights.weights[jj].vertex;
			const auto u_idx = u_handle.idx();

			L.coeffRef(v_idx, v_idx) += weight;
			L.coeffRef(v_idx, u_idx) -= weight;
		}
	}
	L.makeCompressed();

	mat.L_orig = L;
}

void ARAP::ARAPSolver::setSystemMatrixConstraints(const std::vector<std::pair<int, Vector3f>>& constraints)
{
	sysMatrix.L = sysMatrix.L_orig;
	auto& L = sysMatrix.L;
	for (const auto& con : constraints) {
		const auto idx = con.first;
		for (int i = 0; i < L.cols(); i++)
			L.coeffRef(idx, i) = 0;
		for (int i = 0; i < L.rows(); i++)
			L.coeffRef(i, idx) = 0;
		L.coeffRef(idx, idx) = 1;
	}
	sysMatrix.solver.compute(L);
}

void ARAP::ARAPSolver::solvePositions(const std::vector<std::pair<int, Vector3f>>& constraints, const vector_Matrix3f& rotations, vector_Vector3f& solvedPos)
{
	auto& L = sysMatrix.L;
	auto& L_orig = sysMatrix.L_orig;
	const size_t vertexCount = OrigMesh.n_vertices();

	Matrix<float, Dynamic, 3> b(vertexCount, 3);
	b.setZero();

	//calc rhs b
	for (auto v_it = OrigMesh.vertices_begin(); v_it != OrigMesh.vertices_end(); ++v_it) {
		const auto v_idx = v_it->idx();

		const size_t weight_idx_start = edgeWeights.offsets[v_it->idx()];
		const size_t weight_idx_end = edgeWeights.offsets[v_it->idx() + 1];

		// loop through fan
		for (size_t jj = weight_idx_start; jj < weight_idx_end; jj++) {
			const float weight = edgeWeights.weights[jj].weight;
			const auto u_handle = edgeWeights.weights[jj].vertex;
			const auto u_idx = u_handle.idx();

			Matrix3f m_rot = rotations[v_idx] + rotations[u_idx];
			Vector3f tmp = 0.5f * weight * m_rot * (vector3f_from_point(OrigMesh.point(*v_it)) - vector3f_from_point(OrigMesh.point(u_handle)));
			b.row(v_idx) += tmp;
		}
	}

	//apply constraints to system
	for (const auto& con : constraints) {
		const auto idx = con.first;
		
		const size_t weight_idx_start = edgeWeights.offsets[idx];
		const size_t weight_idx_end = edgeWeights.offsets[idx + 1];

		// loop through fan
		for (size_t jj = weight_idx_start; jj < weight_idx_end; jj++) {
			const float weight = -edgeWeights.weights[jj].weight;
			const auto u_handle = edgeWeights.weights[jj].vertex;
			const auto u_idx = u_handle.idx();

			b.row(u_idx) -= weight * con.second;
		}
	}
	for (const auto& con : constraints)
		b.row(con.first) = con.second;

	//solve
	Matrix<float, Dynamic, 3> x;
	x = sysMatrix.solver.solve(b);
	solvedPos.clear();
	for (size_t i = 0; i < vertexCount; i++)
		solvedPos.push_back(x.row(i));
}



