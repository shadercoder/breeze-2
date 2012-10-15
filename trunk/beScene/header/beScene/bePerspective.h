/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#ifndef BE_SCENE_PERSPECTIVE
#define BE_SCENE_PERSPECTIVE

#include "beScene.h"
#include <beCore/beShared.h>
#include <beMath/beVectorDef.h>
#include <beMath/beMatrixDef.h>
#include <beMath/bePlaneDef.h>
#include <memory>
#include <lean/memory/chunk_heap.h>
#include <lean/tags/noncopyable.h>

namespace beScene
{
	
// Prototypes
class Pipe;

/// Perspective flags.
namespace PerspectiveFlags
{
	// Enum.
	enum T
	{
		None = 0,

		Omnidirectional		/// Treats the perspective as omnidirectional, don't forget to provide custum frustum planes!
	};
}

/// Perspective description.
struct PerspectiveDesc
{
	beMath::fvec3 CamPos;		///< Camera position.
	beMath::fvec3 CamRight;		///< Camera orientation.
	beMath::fvec3 CamUp;		///< Camera orientation.
	beMath::fvec3 CamLook;		///< Camera orientation.
	beMath::fmat4 ViewMat;		///< View matrix.
	beMath::fmat4 ProjMat;		///< Projection matrix.
	beMath::fmat4 ViewProjMat;	///< View-projection matrix.
	beMath::fplane3 Frustum[6];	///< Frustum planes.
	float NearPlane;			///< Frustum near plane distance.
	float FarPlane;				///< Frustum far plane distance.
	bool Flipped;				///< Flipped flag.
	float Time;					///< Time.
	float TimeStep;				///< Time Step.
	uint4 OutputIndex;			///< Output index.
	uint4 Flags;				///< Perspective flags.

	/// Default constructor.
	LEAN_INLINE PerspectiveDesc() { }
	/// Constructor.
	LEAN_INLINE PerspectiveDesc(
		const beMath::fvec3 &camPos,
		const beMath::fvec3 &camRight,
		const beMath::fvec3 &camUp,
		const beMath::fvec3 &camLook,
		const beMath::fmat4 &viewMat,
		const beMath::fmat4 &projMat,
		const beMath::fmat4 &viewProjMat,
		const beMath::fplane3 *frustum,
		float nearPlane,
		float farPlane,
		bool flipped,
		float time,
		float timeStep,
		uint4 out = 0,
		uint4 flags = PerspectiveFlags::None)
			: CamPos(camPos),
			CamRight(camRight),
			CamUp(camUp),
			CamLook(camLook),
			ViewMat(viewMat),
			ProjMat(projMat),
			ViewProjMat(viewProjMat),
			NearPlane(nearPlane),
			FarPlane(farPlane),
			Flipped(flipped),
			Time(time),
			TimeStep(timeStep),
			OutputIndex(out),
			Flags(flags)
	{
		if (frustum)
			memcpy(Frustum, frustum, sizeof(Frustum));
	}
	/// Constructor.
	LEAN_INLINE PerspectiveDesc(
		const beMath::fvec3 &camPos,
		const beMath::fvec3 &camRight,
		const beMath::fvec3 &camUp,
		const beMath::fvec3 &camLook,
		const beMath::fmat4 &viewMat,
		const beMath::fmat4 &projMat,
		float nearPlane,
		float farPlane,
		bool flipped,
		float time,
		float timeStep,
		uint4 out = 0,
		uint4 flags = PerspectiveFlags::None)
			: CamPos(camPos),
			CamRight(camRight),
			CamUp(camUp),
			CamLook(camLook),
			ViewMat(viewMat),
			ProjMat(projMat),
			ViewProjMat(Multiply(viewMat, projMat)),
			NearPlane(nearPlane),
			FarPlane(farPlane),
			Flipped(flipped),
			Time(time),
			TimeStep(timeStep),
			OutputIndex(out),
			Flags(flags)
	{
		ExtractFrustum(Frustum, ViewProjMat);
	}

	/// Multiplies the given two matrices.
	static BE_SCENE_API beMath::fmat4 Multiply(const beMath::fmat4 &a, const beMath::fmat4 &b);
	/// Extracts a view frustum from the given view projection matrix.
	static BE_SCENE_API void ExtractFrustum(beMath::fplane3 frustum[6], const beMath::fmat4 &viewProj);
};

/// Rendering perspective.
class Perspective : public lean::noncopyable_chain<beCore::Shared>
{
private:
	typedef lean::chunk_heap<0, lean::default_heap, 0, 16> data_heap;
	data_heap m_dataHeap;

	size_t m_dataHeapWatermark;

protected:
	PerspectiveDesc m_desc;		///< Perspective description.

	/// Frees all data allocated from this perspective's internal heap.
	BE_SCENE_API void FreeData();

public:
	/// Constructor.
	BE_SCENE_API Perspective(const PerspectiveDesc &desc);
	/// Destructor.
	BE_SCENE_API virtual ~Perspective();

	/// Allocates data from this perspective's internal heap.
	BE_SCENE_API void* AllocateData(size_t size);

	/// Gets the perspective description.
	LEAN_INLINE const PerspectiveDesc& GetDesc() const { return m_desc; };

	/// Optionally gets a pipe.
	virtual Pipe* GetPipe() const { return nullptr; }
};

/// Allocates data from the given perspective's internal heap.
template <class Type>
LEAN_INLINE Type* AllocateData(Perspective &perspective)
{
	return static_cast<Type*>(perspective.AllocateData(sizeof(Type)));
}

} // namespace

/// Allocates data from the given perspective's internal heap.
LEAN_INLINE void* operator new(size_t size, beScene::Perspective &perspective) { return perspective.AllocateData(size); }
LEAN_INLINE void operator delete(void*, beScene::Perspective&) throw() { }

/// Allocates data from the given perspective's internal heap.
LEAN_INLINE void* operator new[](size_t size, beScene::Perspective &perspective) { return perspective.AllocateData(size); }
LEAN_INLINE void operator delete[](void*, beScene::Perspective&) throw() { }

#endif