#include "MediaPipeFaceMeshObserverComponent.h"
#include "MediaPipePipelineComponent.h"
#include "MediaPipeShared.h"
#include "DrawDebugHelpers.h"

#include "mediapipe/modules/face_geometry/protos/face_geometry.pb.h"

// Matrix.TransformFVector4(MP_TO_UE(V.pos));

UMediaPipeFaceMeshObserverComponent::UMediaPipeFaceMeshObserverComponent()
{
}

FTransform ConvertTransform(const UmpMat4& InPose, float HScale = 1)
{
	// The Metric 3D space established within the Face Geometry module is a right-handed orthonormal metric 3D coordinate space. 
	// Within the space, there is a virtual perspective camera located at the space origin and pointed in the negative direction of the Z-axis.

	//				MediaPipe	UE4
	// Forward:		-Z			+X
	// Right:		+X			+Y
	// Up:			+Y			+Z

	FVector in_P(InPose.arr[12], InPose.arr[13], InPose.arr[14]);

	FVector P;
	P.X = -in_P.Z;
	P.Y = in_P.X * HScale;
	P.Z = in_P.Y;

	// COLUMN_MAJOR -> ROW_MAJOR
	FMatrix M = FMatrix::Identity;
	for (int row = 0; row < 3; ++row)
		for (int col = 0; col < 3; ++col)
			M.M[row][col] = InPose.m[col][row];

	auto in_R = M.Rotator();

	// (X)roll (Y)pitch (Z)yaw
	FRotator R;
	R.Roll = in_R.Yaw * HScale;
	R.Pitch = in_R.Roll;
	R.Yaw = -in_R.Pitch * HScale;

	return FTransform(R, P);
}

// WARNING: executed in MediaPipe thread context!
void UMediaPipeFaceMeshObserverComponent::OnUmpPacket(IUmpObserver* Observer)
{
	using TMessage = std::vector<mediapipe::face_geometry::FaceGeometry>;

	if (!UmpCompareType<TMessage>(Observer))
	{
		PLUGIN_LOG(Error, TEXT("Invalid FaceGeometry message"));
		return;
	}

	const auto& Message = UmpCastPacket<TMessage>(Observer->GetData());
	const int InCount = Message.size();
	Grow(Meshes, InCount);

	for (int MeshId = 0; MeshId < InCount; ++MeshId)
	{
		const auto& InFaceGeo = Message[MeshId];
		auto& Mesh = Meshes[MeshId];

		// Pose

		const auto& InPose = InFaceGeo.pose_transform_matrix();

		UmpMat4 InPoseMatrix;
		for (int i = 0; i < 16; i++)
			InPoseMatrix.arr[i] = InPose.packed_data(i);

		// by default: COLUMN_MAJOR, XREF: Convert4x4MatrixDataToArrayFormat
		if (InPose.layout() == mediapipe::MatrixData::ROW_MAJOR)
		{
			auto& m = InPoseMatrix.arr;
			std::swap(m[1], m[4]);
			std::swap(m[2], m[8]);
			std::swap(m[3], m[12]);
			std::swap(m[6], m[9]);
			std::swap(m[7], m[13]);
			std::swap(m[11], m[14]);
		}

		Mesh.Pose = ConvertTransform(InPoseMatrix, bFlipHorizontal ? -1 : 1);
		Mesh.Pose.SetScale3D(FVector(1, 1, 1));

		// FaceGeometry

		const auto& InMesh = InFaceGeo.mesh();
		const auto VertexType = InMesh.vertex_type();
		const auto PrimitiveType = InMesh.primitive_type();

		if (VertexType == mediapipe::face_geometry::Mesh3d::VERTEX_PT
			&& PrimitiveType == mediapipe::face_geometry::Mesh3d::TRIANGLE)
		{
			// VERTEX_PT = 0; Defined by 5 coordinates: position (XYZ) + texture coordinate (UV)
			// TRIANGLE = 0; Defined by 3 indices: triangle vertex IDs

			const auto NumVertices = InMesh.vertex_buffer_size() / 5;
			const auto NumIndices = InMesh.index_buffer_size();

			Mesh.Vertices.SetNum(NumVertices);
			Mesh.Indices.SetNum(NumIndices);

			const auto* VB = InMesh.vertex_buffer().data();
			const auto* IB = InMesh.index_buffer().data();

			for (int i = 0; i < NumVertices; i++)
			{
				auto& Vertex = Mesh.Vertices[i];

				FVector Pos(VB[0], VB[1], VB[2]);
				Vertex.Pos = FVector(-Pos.Z, Pos.X, Pos.Y) * UniformScale;
				Vertex.TexCoord = FVector2D(VB[3], VB[4]);

				VB += 5;
			}

			if (NumIndices > 0)
				memcpy(&Mesh.Indices[0], IB, NumIndices * sizeof(IB[0]));
		}
		else
		{
			Mesh.Vertices.SetNum(0);
			Mesh.Indices.SetNum(0);
		}
	}

	NumDetections = InCount;
	UpdateTimestamp();
}

const FMediaPipeFaceMesh& UMediaPipeFaceMeshObserverComponent::GetMesh(int MeshId)
{
	if (MeshId >= 0 && MeshId < NumDetections)
	{
		return Meshes[MeshId];
	}
	PLUGIN_LOG(Error, TEXT("MeshId out of range"));
	static FMediaPipeFaceMesh Dummy;
	return Dummy;
}

const FTransform& UMediaPipeFaceMeshObserverComponent::GetMeshPose(int MeshId)
{
	return GetMesh(MeshId).Pose;
}

const FMediaPipeVertex& UMediaPipeFaceMeshObserverComponent::GetMeshVertex(int MeshId, int VertexId)
{
	const auto& Mesh = GetMesh(MeshId);
	if (VertexId >= 0 && VertexId < Mesh.Vertices.Num())
	{
		return Mesh.Vertices[VertexId];
	}
	PLUGIN_LOG(Error, TEXT("VertexId out of range"));
	static FMediaPipeVertex Dummy;
	return Dummy;
}

bool UMediaPipeFaceMeshObserverComponent::TryGetMesh(int MeshId, FMediaPipeFaceMesh& Mesh)
{
	if (MeshId >= 0 && MeshId < NumDetections)
	{
		Mesh = Meshes[MeshId];
		return true;
	}
	return false;
}

bool UMediaPipeFaceMeshObserverComponent::TryGetMeshPose(int MeshId, FTransform& Pose)
{
	if (MeshId >= 0 && MeshId < NumDetections)
	{
		Pose = Meshes[MeshId].Pose;
		return true;
	}
	return false;
}

bool UMediaPipeFaceMeshObserverComponent::TryGetMeshVertex(int MeshId, int VertexId, FMediaPipeVertex& Vertex)
{
	if (MeshId >= 0 && MeshId < NumDetections)
	{
		const auto& Mesh = Meshes[MeshId];
		if (VertexId >= 0 && VertexId < Mesh.Vertices.Num())
		{
			Vertex = Mesh.Vertices[VertexId];
			return true;
		}
	}
	return false;
}

void UMediaPipeFaceMeshObserverComponent::DrawDebugMeshVertices(int MeshId, const FTransform& Transform, float PrimitiveScale, FLinearColor Color)
{
	#if ENABLE_DRAW_DEBUG

	if (MeshId >= 0 && MeshId < NumDetections)
	{
		auto* World = GetWorld();
		const auto& Mesh = Meshes[MeshId];
		const int Count = Mesh.Vertices.Num();
		const auto RawColor = Color.ToFColor(false);

		for (int i = 0; i < Count; i++)
		{
			const auto& V = Mesh.Vertices[i];
			auto Pos = Transform.TransformPosition(V.Pos);
			DrawDebugPoint(World, Pos, PrimitiveScale, RawColor);
			//DrawDebugCoordinateSystem(World, Pos, FRotator::ZeroRotator, PrimitiveScale);
		}
	}

	#endif
}
