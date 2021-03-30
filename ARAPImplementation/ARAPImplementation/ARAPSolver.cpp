#include "ARAPSolver.h"



ARAP::ARAPSolver::ARAPSolver(Model* parsedModel, TriMesh* origMesh)
{
	ModelPointer = parsedModel;
	this->OrigMesh = origMesh;

	std::vector<Matrix3f> rots;
	solveRotations(rots);
}


ARAP::ARAPSolver::~ARAPSolver()
{
}

Vector3f ARAP::ARAPSolver::vector3f_from_point(const TriMesh::Point& p) {
	return Vector3f(p[0], p[1], p[2]);
}

void ARAP::ARAPSolver::solveRotations(std::vector<Matrix3f>& solvedRotations)
{
	solvedRotations.clear();

	// Iterate over each and every vertex v, v is the center point of the regarded mesh fan
	for (auto v_it = OrigMesh->vertices_begin(); v_it != OrigMesh->vertices_end(); ++v_it) {
		std::vector<float> fan_weights;
		std::vector<Vector3f> sourcePointsFan;
		std::vector<Vector3f> deformedPointsFan;

		//Better way to get from glm to eigen?
		const Vector3f center = vector3f_from_point(OrigMesh->point(*v_it));
		glm::vec3 deformCenterVec = ModelPointer->meshes[0].vertices[v_it->idx()].Position;
		const Vector3f center_deformed = Vector3f(deformCenterVec.x, deformCenterVec.y, deformCenterVec.z);
		
		// loop through fan
		for (TriMesh::VertexVertexIter vv_it = OrigMesh->vv_iter(*v_it); vv_it.is_valid(); ++vv_it) {
			//fan_weights.push_back(weights.weights[ii].weight); //TODO Weights
			fan_weights.push_back(1.0f);

			const OpenMesh::VertexHandle h(vv_it->idx());
			sourcePointsFan.push_back(vector3f_from_point(OrigMesh->point(h)) - center);

			glm::vec3 deformedGlm = ModelPointer->meshes[0].vertices[h.idx()].Position;
			const Vector3f deformedEigen = Vector3f(deformCenterVec.x, deformCenterVec.y, deformCenterVec.z);
			deformedPointsFan.push_back(deformedEigen - center_deformed);
		}

		Matrix3f rotation = procrustes(sourcePointsFan, deformedPointsFan, fan_weights);
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



