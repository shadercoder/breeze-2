/****************************************************/
/* breeze Engine Scene Module  (c) Tobias Zirr 2011 */
/****************************************************/

#include <beAssetsInternal/stdafx.h>
#include "beAssets/beWaterControllers.h"

#include <beEntitySystem/beEntities.h>

#include <beCore/beReflectionProperties.h>
#include <beCore/bePersistentIDs.h>

#include <beScene/beRenderingPipeline.h>
#include <beScene/bePipelinePerspective.h>
#include "beScene/bePerspectiveStatePool.h"
#include "beScene/beQueueStatePool.h"

#include <beScene/bePerspectivePool.h>
#include <beScene/bePipePool.h>

#include <beScene/beRenderableMaterial.h>
#include <beScene/beRenderableEffectData.h>
#include <beScene/beAbstractRenderableEffectDriver.h>

#include <beScene/beRenderContext.h>
#include <beGraphics/Any/beAPI.h>
#include <beGraphics/Any/beDevice.h>
#include <beGraphics/Any/beBuffer.h>
#include <beGraphics/Any/beTexture.h>
#include <beGraphics/Any/beStateManager.h>
#include <beGraphics/Any/beDeviceContext.h>
#include <beGraphics/Any/beTextureTargetPool.h>
#include <beGraphics/Any/beEffect.h>
#include <beGraphics/Any/beEffectsAPI.h>

#include <beMath/beUtility.h>
#include <beMath/beVector.h>
#include <beMath/beMatrix.h>
#include <beMath/beSphere.h>
#include <beMath/bePlane.h>
#include <beMath/beAAB.h>
#include <beMath/beIntersect.h>
#include <beMath/beProjection.h>
#include <beMath/beBits.h>

#include <lean/containers/multi_vector.h>
#include <lean/containers/simple_vector.h>
#include <lean/memory/chunk_pool.h>

#include <lean/functional/algorithm.h>
#include <lean/io/numeric.h>

#include <beGraphics/DX/beError.h>

namespace beAssets
{

BE_CORE_PUBLISH_COMPONENT(WaterController)
BE_CORE_PUBLISH_COMPONENT(WaterControllers)

const beCore::ReflectionProperty ControllerProperties[] =
{
	beCore::MakeReflectionProperty<bool>("reflection", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&WaterController::EnableReflection) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&WaterController::IsReflectionEnabled) ),
	beCore::MakeReflectionProperty<uint4>("reflection resolution", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&WaterController::SetReflectionResolution) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&WaterController::GetReflectionResolution) ),
	beCore::MakeReflectionProperty<float>("reflection tolerance", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&WaterController::SetReflectionTolerance) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&WaterController::GetReflectionTolerance) )
};
BE_CORE_ASSOCIATE_PROPERTIES(WaterController, ControllerProperties);

class WaterControllers::M : public WaterControllers
{
public:
	lean::com_ptr<beg::API::Device> device;
	lean::resource_ptr<beScene::PerspectivePool> perspectivePool;
	
	struct Topology
	{
		lean::com_ptr<beg::API::Buffer> indexBuffer;
		uint4 indexCount;
	};
	Topology triTopology;
	Topology quadTopology;
	lean::com_ptr<beg::API::Buffer> instanceBuffer;

	uint4 defaultReflectionResolution;
	
	enum record_tag { record };
	enum state_tag { state };
	enum bounds_tag { bounds };
	enum renderable_data_tag { renderableData };
	enum observers_tag { observers };
	
	struct Record
	{
		WaterController *Reflected;
		lean::resource_ptr<beScene::RenderableMaterial> Material;

		uint8 PersistentID;

		Record(WaterController *reflected)
			: Reflected(reflected),
			PersistentID(-1) { }
	};

	struct Configuration
	{
		bem::faab3 LocalBounds;
		uint4 ReflectionResolution;
		float ClipTolerance;
		bool Reflection : 1;
		bool Planar : 1;

		Configuration(const bem::faab3 &bounds, uint4 defaultReflectionRes)
			: LocalBounds(bounds),
			ReflectionResolution(defaultReflectionRes),
			ClipTolerance(0.3f),
			Reflection(false),
			Planar(true) { }
	};

	struct State
	{
		Configuration Config;
		bool Visible : 1;
		bool Attached : 1;

		State(const bem::faab3 &bounds, uint4 defaultReflectionRes)
			: Config(bounds, defaultReflectionRes),
			Visible(true),
			Attached(false) { }
	};
	
	typedef lean::chunk_pool<WaterControllerHandle, 128> handle_pool;
	handle_pool handles;

	typedef lean::multi_vector_t< lean::simple_vector_binder<lean::vector_policies::semipod> >::make<
			Record, record_tag,
			State, state_tag,
			bem::faab3, bounds_tag,
			besc::RenderableEffectData, renderable_data_tag,
			bec::ComponentObserverCollection, observers_tag
		>::type controllers_t;
	
	struct Data
	{
		struct Queue : public beScene::QueueStateBase
		{
			struct Pass
			{
				lean::resource_ptr<const besc::AbstractRenderableEffectDriver> effectDriver;
				const besc::QueuedPass *pass;
				const beg::MaterialTechnique *material;
				beg::api::EffectShaderResource *reflectionTexture;
			};

			typedef lean::simple_vector<uint4, lean::vector_policies::inipod> pass_offsets_t;
			typedef lean::simple_vector<Pass, lean::vector_policies::semipod> passes_t;

			pass_offsets_t materialsToPasses;
			passes_t passes;

			void Reset(besc::PipelineQueueID id)
			{
				base_type::Reset(id);
				this->materialsToPasses.clear();
				this->passes.clear();
			}
		};

		typedef lean::simple_vector<beScene::RenderableMaterial*, lean::vector_policies::inipod> materials_t;
		typedef lean::simple_vector<uint4, lean::vector_policies::inipod> offsets_t;
		typedef besc::QueueStatePool<Queue, lean::vector_policies::semipod> queues_t;

		uint4 structureRevision;

		controllers_t controllers;

		materials_t uniqueMaterials;
		uint4 activeControllerCount;
		uint4 reflectionControllerCount;
		offsets_t controllersToMaterials;

		queues_t queues;
		
		lean::com_ptr<beg::API::Buffer> lightConstantBuffer;
		lean::com_ptr<beg::API::ShaderResourceView> lightConstantSRV;

		Data()
			: structureRevision(0),
			activeControllerCount(0),
			reflectionControllerCount(0) { }
	};
	Data dataSets[2];
	Data *data, *dataAux;
	uint4 controllerRevision;

	struct PerspectiveState;
	mutable besc::PerspectiveStatePool<PerspectiveState> perspectiveState;

	lean::resource_ptr<beCore::ComponentMonitor> pComponentMonitor;

	M(beCore::PersistentIDs *persistentIDs, beScene::PerspectivePool *perspectivePool, const beScene::RenderingPipeline &pipeline, const beGraphics::Device &device)
		: device(ToImpl(device)),
		triTopology(CreateTriangleBuffer(32)),
		perspectivePool( LEAN_ASSERT_NOT_NULL(perspectivePool) ),
		defaultReflectionResolution(512),
		data(dataSets),
		dataAux(&dataSets[1]),
		controllerRevision(0)
	{
	}

	/// Creates an index buffer full of triangle strips.
	Topology CreateTriangleBuffer(uint4 vertexRowCount)
	{
		Topology result;

		uint4 edgeRowCount = (vertexRowCount) ? vertexRowCount - 1 : 0;
		result.indexCount = edgeRowCount * edgeRowCount * 6;

		std::vector<uint2> indices(result.indexCount);
		uint2 *itIdx = indices.data();

		for (uint4 v = 0; v < edgeRowCount; ++v)
		{
			for (uint4 u = 0; u < edgeRowCount; ++u)
			{
				uint4 baseIndex = bem::bitzip( bem::vec(u, v) );
				uint4 topIndex = bem::bitzip( bem::vec(u, v + 1) );
				uint4 rightIndex = bem::bitzip( bem::vec(u + 1, v) );
				uint4 topRightIndex = bem::bitzip( bem::vec(u + 1, v + 1) );

				*itIdx++ = (uint2) baseIndex;
				*itIdx++ = (uint2) topIndex;
				*itIdx++ = (uint2) rightIndex;
				*itIdx++ = (uint2) rightIndex;
				*itIdx++ = (uint2) topIndex;
				*itIdx++ = (uint2) topRightIndex;
			}
		}

		result.indexBuffer = beg::Any::CreateStructuredBuffer(device, D3D11_BIND_INDEX_BUFFER, sizeof(uint2), result.indexCount, 0, indices.data());

		return result;
	}


	/// Verifies the given handle.
	friend LEAN_INLINE bool VerifyHandle(const M &m, const WaterControllerHandle handle) { return handle.Index < m.data->controllers.size(); }

	/// Fixes all controller handles to match the layout of the given controller vector.
	static void FixControllerHandles(controllers_t &controllers, uint4 internalIdx = 0)
	{
		// Fix subsequent handles
		for (; internalIdx < controllers.size(); ++internalIdx)
			controllers(record)[internalIdx].Reflected->Handle().SetIndex(internalIdx);
	}

	/// Gets the number of child components.
	uint4 GetComponentCount() const { return static_cast<uint4>(data->controllers.size()); }
	/// Gets the name of the n-th child component.
	beCore::Exchange::utf8_string GetComponentName(uint4 idx) const { return WaterController::GetComponentType()->Name; }
	/// Gets the n-th reflected child component, nullptr if not reflected.
	lean::com_ptr<const ReflectedComponent, lean::critical_ref> GetReflectedComponent(uint4 idx) const { return data->controllers[idx].Reflected; }
};

/// Creates a collection of mesh controllers.
lean::scoped_ptr<WaterControllers, lean::critical_ref> CreateWaterControllers(beCore::PersistentIDs *persistentIDs,
	beScene::PerspectivePool *perspectivePool, const beScene::RenderingPipeline &pipeline, const beGraphics::Device &device)
{
	return new_scoped WaterControllers::M(persistentIDs, perspectivePool, pipeline, device);
}

namespace
{

void CommitExternalChanges(WaterControllers::M &m, beCore::ComponentMonitor &monitor)
{
	LEAN_FREE_PIMPL(WaterControllers);

	M::Data &data = *m.data;

	if (monitor.Replacement.HasChanged(besc::RenderableMaterial::GetComponentType()))
	{
		uint4 controllerCount = (uint4) data.controllers.size();

		for (uint4 internalIdx = 0; internalIdx < controllerCount; ++internalIdx)
		{
			M::Record &record = data.controllers[internalIdx];
			besc::RenderableMaterial *newMaterial = record.Material;
			
			while (besc::RenderableMaterial *successor = newMaterial->GetSuccessor())
				newMaterial = successor;

			if (newMaterial != record.Material)
				record.Reflected->SetMaterial(newMaterial);
		}
	}
}

struct MaterialSorter
{
	const WaterControllers::M::controllers_t &v;

	MaterialSorter(const WaterControllers::M::controllers_t &v)
		: v(v) { }

	LEAN_INLINE bool operator ()(uint4 l, uint4 r) const
	{
		LEAN_FREE_PIMPL(WaterControllers);

		const besc::RenderableMaterial *left = v[l].Material;
		bool leftAttached = left && v(M::state)[l].Attached;
		bool leftReflection = v(M::state)[l].Config.Reflection;
		const besc::RenderableMaterial *right = v[r].Material;
		bool rightAttached = right && v(M::state)[r].Attached;
		bool rightReflection = v(M::state)[r].Config.Reflection;

		// Move null meshes outwards
		if (!leftAttached)
			return false;
		else if (!rightAttached)
			return true;
		// Group by state
		else if (leftReflection && !rightReflection)
			return true;
		else if (leftReflection == rightReflection)
		{
			// Sort by material
			if (left < right)
				return true;
			else if (left == right)
			{
				const besc::AbstractRenderableEffectDriver *leftDriver = left->GetTechniques().Begin[0].TypedDriver();
				const besc::AbstractRenderableEffectDriver *rightDriver = right->GetTechniques().Begin[0].TypedDriver();

				// Sort by effect
				return leftDriver < rightDriver;
			}
		}
		return false;
	}
};

/// Sort controllers by material and shader (moving null materials outwards).
void SortControllers(WaterControllers::M::controllers_t &destControllers, const WaterControllers::M::controllers_t &srcControllers)
{
	LEAN_FREE_PIMPL(WaterControllers);
	
	uint4 controllerCount = (uint4) srcControllers.size();
	
	lean::scoped_ptr<uint4[]> sortIndices( new uint4[controllerCount] );
	std::generate_n(&sortIndices[0], controllerCount, lean::increment_gen<uint4>(0));
	std::sort(&sortIndices[0], &sortIndices[controllerCount], MaterialSorter(srcControllers));
	
	destControllers.clear();
	lean::append_swizzled(srcControllers, &sortIndices[0], &sortIndices[controllerCount], destControllers);
}

void LinkControllersToUniqueMaterials(WaterControllers::M::Data &data)
{
	LEAN_FREE_PIMPL(WaterControllers);

	const uint4 controllerCount = (uint4) data.controllers.size();
	data.controllersToMaterials.resize(controllerCount);
	data.uniqueMaterials.clear();
	data.activeControllerCount = 0;
	data.reflectionControllerCount = 0;

	const besc::RenderableMaterial *prevMaterial = nullptr;
	uint4 materialIdx = 0;

	for (uint4 internalIdx = 0; internalIdx < controllerCount; ++internalIdx)
	{
		M::Record &controller = data.controllers[internalIdx];
		M::State &controllerState = data.controllers(M::state)[internalIdx];

		// Ignore null & detached meshes at the back
		if (!controller.Material || !controllerState.Attached)
			break;

		data.reflectionControllerCount += controllerState.Config.Reflection;
		++data.activeControllerCount;

		// Add new unique material
		if (prevMaterial != controller.Material)
		{
			materialIdx = (uint4) data.uniqueMaterials.size();
			data.uniqueMaterials.push_back(controller.Material);
			prevMaterial = controller.Material;
		}

		// Let controllers reference unique materials
		data.controllersToMaterials[internalIdx] = materialIdx;
	}
}

WaterControllers::M::Data::Queue::Pass ConstructPass(const beGraphics::MaterialTechnique *material,
	const besc::AbstractRenderableEffectDriver *effectDriver, const besc::QueuedPass *driverPass)
{
	WaterControllers::M::Data::Queue::Pass pass;

	pass.material = material;
	pass.pass = driverPass;
	pass.effectDriver = effectDriver;
	pass.reflectionTexture = ToImpl(effectDriver->GetEffect())->GetVariableBySemantic("ReflectionTexture")->AsShaderResource();

	return pass;
}

void AddTechniquePasses(WaterControllers::M::Data &data, uint4 materialIdx, besc::RenderableMaterial::Technique technique)
{
	LEAN_FREE_PIMPL(WaterControllers);

	besc::AbstractRenderableEffectDriver::PassRange passes = technique.TypedDriver()->GetPasses();

	for (uint4 passIdx = 0, passCount = Size4(passes); passIdx < passCount; ++passIdx)
	{
		const besc::QueuedPass *pass = &passes.Begin[passIdx];
		besc::PipelineQueueID queueID(pass->GetStageID(), pass->GetQueueID());
		M::Data::Queue &queue = data.queues.GetQueue(queueID);

		// Link material to beginning of pass range & insert pass
		queue.materialsToPasses.resize( materialIdx + 1, (uint4) queue.passes.size() );
		queue.passes.push_back( ConstructPass(technique.Technique, technique.TypedDriver(), pass) );
	}
}

void BuildQueues(WaterControllers::M::Data &data)
{
	LEAN_FREE_PIMPL(WaterControllers);

	data.queues.Clear();

	const uint4 materialCount = (uint4) data.uniqueMaterials.size();

	// Build queues from meshes
	for (uint4 materialIdx = 0; materialIdx < materialCount; ++materialIdx)
	{
		const besc::RenderableMaterial *material = data.uniqueMaterials[materialIdx];
		besc::RenderableMaterial::TechniqueRange techniques = material->GetTechniques();

		for (uint4 techniqueIdx = 0, techniqueCount = Size4(techniques); techniqueIdx < techniqueCount; ++techniqueIdx)
			AddTechniquePasses(data, materialIdx, techniques[techniqueIdx]);
	}
	
	// Discard unused queues
	data.queues.Shrink();

	// IMPORTANT: Finish implicit materials to pass offset ranges
	for (M::Data::Queue *it = data.queues.begin(), *itEnd = data.queues.end(); it < itEnd; ++it)
		it->materialsToPasses.resize(materialCount + 1, (uint4) it->passes.size());
}

} // namespace

// Commits changes.
void WaterControllers::Commit()
{
	LEAN_STATIC_PIMPL();

	if (m.pComponentMonitor)
		CommitExternalChanges(m, *m.pComponentMonitor);

	M::Data &prevData = *m.data;
	M::Data &data = *m.dataAux;

	if (prevData.structureRevision != m.controllerRevision)
	{
		// Rebuild internal data structures in swap buffer
		SortControllers(data.controllers, prevData.controllers);
		LinkControllersToUniqueMaterials(data);
		BuildQueues(data);
		
		if (data.activeControllerCount)
		{
/*			// Reallocate GPU buffers
			const size_t FloatRowSize = sizeof(float) * 4;

			LEAN_STATIC_ASSERT_MSG(
					sizeof(typename M::Constants) % FloatRowSize == 0,
					"Size of light constant buffer required to be multiple of 4 floats"
				);

			data.lightConstantBuffer = beg::Any::CreateStructuredBuffer(m.device, D3D11_BIND_SHADER_RESOURCE,
				sizeof(typename M::Constants), data.activeControllerCount, 0);
			data.lightConstantSRV = beg::Any::CreateSRV(data.lightConstantBuffer, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
				0, sizeof(typename M::Constants) / FloatRowSize * data.activeControllerCount);
*/		}

		data.structureRevision = m.controllerRevision;

		// Swap current data with updated data
		std::swap(m.data, m.dataAux);
		M::FixControllerHandles(m.data->controllers);
		
		prevData.lightConstantBuffer = nullptr;
		prevData.lightConstantSRV = nullptr;
	}
}

struct WaterControllers::M::PerspectiveState : public beScene::PerspectiveStateBase<const WaterControllers::M, PerspectiveState>
{
	struct VisibleMaterial
	{
		uint4 controllerIdx;
		uint4 materialIdx;

		VisibleMaterial(uint4 controllerIdx, uint4 materialIdx)
			: controllerIdx(controllerIdx),
			materialIdx(materialIdx) { }
	};

	struct Queue : public besc::QueueStateBase
	{
		struct VisiblePass
		{
			uint4 controllerIdx;
			uint4 passIdx;

			VisiblePass(uint4 controllerIdx, uint4 passIdx)
				: controllerIdx(controllerIdx),
				passIdx(passIdx) { }
		};

		typedef lean::simple_vector<VisiblePass, lean::vector_policies::inipod> visible_passes_t;
		visible_passes_t visiblePasses;

		void Reset(besc::PipelineQueueID id)
		{
			base_type::Reset(id);
			this->visiblePasses.clear();
		}
	};
	
	struct ExternalPass
	{
		uint4 controllerIdx;
		const M::Data::Queue::Pass *pass;

		ExternalPass(uint4 controllerIdx, const M::Data::Queue::Pass *pass)
			: controllerIdx(controllerIdx),
			pass(pass) { }
	};

	struct ReflectionConstants
	{
		bem::fvec2 Resolution;	///< Reflection resolution.
		bem::fvec2 Pixel;		///< Reflection pixel (= 1 / resolution).

		bem::fmat4 Space;		///< Reflection-space transformation matrix.
	};

	struct ReflectionState
	{
		uint4 ControllerIdx;

		static const uint4 FaceCount = 3;
		lean::com_ptr<beScene::PipelinePerspective> Perspective[FaceCount];

		ReflectionState(uint4 controllerIdx)
			: ControllerIdx(controllerIdx) { }
	};
	
	typedef lean::simple_vector<VisibleMaterial, lean::vector_policies::inipod> visible_materials_t;
	typedef lean::simple_vector<float, lean::vector_policies::inipod> distances_t;
	typedef besc::QueueStatePool<Queue, lean::vector_policies::semipod> queues_t;
	typedef lean::simple_vector<ExternalPass, lean::vector_policies::inipod> external_passes_t;

	typedef lean::simple_vector<ReflectionConstants, lean::vector_policies::inipod> reflection_constants_t;
	typedef lean::simple_vector<beg::TextureViewHandle, lean::vector_policies::inipod> reflection_textures_t;
	typedef lean::simple_vector<uint1, lean::vector_policies::inipod> active_t;
	typedef lean::simple_vector<ReflectionState, lean::vector_policies::semipod> reflection_state_t;

	uint4 structureRevision;

	visible_materials_t visibleMaterials;
	distances_t distances;
	queues_t queues;
	external_passes_t externalPasses;

	reflection_constants_t reflectionConstants;
	reflection_textures_t reflectionTextures;

	active_t reflectionsActive;
	reflection_state_t reflectionState;

	lean::com_ptr<beg::API::Buffer> shadowConstantBuffer;
	lean::com_ptr<beg::API::ShaderResourceView> shadowConstantSRV;

	PerspectiveState(const WaterControllers::M *parent)
		: base_type(parent),
		structureRevision(0) { }
		
	void Reset(beScene::Perspective *perspective)
	{
		base_type::Reset(perspective);

		this->visibleMaterials.clear();
		this->distances.clear();
		this->queues.Reset();
		this->externalPasses.clear();
		
		std::fill(this->reflectionsActive.begin(), this->reflectionsActive.end(), 0);
		this->reflectionState.clear();
	}

	void Synchronize(const M &m)
	{
		const M::Data &data = *m.data;

		if (this->structureRevision != data.structureRevision)
		{
			this->queues.CopyFrom(data.queues);

			this->reflectionConstants.resize(data.reflectionControllerCount);
			this->reflectionTextures.resize(data.reflectionControllerCount);
			this->reflectionsActive.resize(data.activeControllerCount);
/*
			// Reallocate GPU buffers
			const size_t FloatRowSize = sizeof(float) * 4;
		
			LEAN_STATIC_ASSERT_MSG(
					sizeof(ReflectionConstants) % FloatRowSize == 0,
					"Size of light constant buffer required to be multiple of 4 floats"
				);

			if (data.shadowControllerCount)
			{
				this->shadowConstantBuffer = beg::Any::CreateStructuredBuffer(m.device, D3D11_BIND_SHADER_RESOURCE,
					sizeof(typename M::PerspectiveState::ReflectionConstants), data.shadowControllerCount, 0);
				this->shadowConstantSRV = beg::Any::CreateSRV(this->shadowConstantBuffer, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
					0, sizeof(typename M::PerspectiveState::ReflectionConstants) / FloatRowSize * data.shadowControllerCount);
			}
			else
			{
				this->shadowConstantBuffer = nullptr;
				this->shadowConstantSRV = nullptr;
			}
*/
			this->Reset(this->PerspectiveBinding);
			this->structureRevision = data.structureRevision;
		}
	}
};

// Perform visiblity culling.
void WaterControllers::Cull(besc::PipelinePerspective &perspective) const
{
	LEAN_STATIC_PIMPL_CONST();
	M::PerspectiveState &state = m.perspectiveState.GetState(perspective, &m);
	const M::Data &data = *m.data;

	// Initialize frame & cull state, not utilizing frame coherency so far
	state.Synchronize(m);
	state.visibleMaterials.clear();
	state.distances.clear();

	// Make sure this is not one of our perspectives
	// TODO: Sth. more individual?
	for (const M::PerspectiveState*const* itState = m.perspectiveState.begin(), *const*itStateEnd = m.perspectiveState.end(); itState != itStateEnd; ++itState)
	{
		const M::PerspectiveState &otherState = **itState;

		for (M::PerspectiveState::reflection_state_t::const_iterator it = otherState.reflectionState.begin(), itEnd = otherState.reflectionState.end(); it != itEnd; ++it)
		{
			const M::PerspectiveState::ReflectionState &reflectionState = *it;

			for (uint4 i = 0; i < reflectionState.FaceCount; ++i)
				if (reflectionState.Perspective[i] == &perspective)
					return;
		}
	}

	const beMath::fplane3 *planes = perspective.GetDesc().Frustum;
	beMath::fvec3 center = perspective.GetDesc().CamPos;

	// Find visible controllers
	for (uint4 controllerIdx = 0; controllerIdx < data.activeControllerCount; ++controllerIdx)
	{
		const bem::faab3 &bounds = data.controllers(M::bounds)[controllerIdx];
		
		bool visible = true;

		// Cull bounding box against frustum
		for (int i = 0; i < 6; ++i)
			visible &= (sdist(planes[i], bounds) <= 0.0f);

		// Build list of visible controllers
		if (visible)
		{
			float distSquared = distSq( closest_point(bounds, center), center );

			state.visibleMaterials.push_back( M::PerspectiveState::VisibleMaterial(controllerIdx, data.controllersToMaterials[controllerIdx]) );
			state.distances.push_back( distSquared );
		}
	}

	// IMPORTANT: Clear external passes ONCE before preparation of individual queues
	state.externalPasses.clear();
}

namespace
{

const uint4 ReflectionFaceCount = 3;
const float ReflectionFaceFOV = bem::Constants::pi<float>::value * (2.0f / 3.0f);

/// Adds a reflection.
uint4 AddReflection(WaterControllers::M::PerspectiveState::reflection_state_t &reflections, uint4 controllerIdx,
					const WaterControllers::M::Configuration &state,
					besc::PipelinePerspective &perspective, besc::PerspectivePool &perspectivePool)
{
	LEAN_FREE_PIMPL(WaterControllers);

	LEAN_STATIC_ASSERT(ReflectionFaceCount <= M::PerspectiveState::ReflectionState::FaceCount);
	uint4 faceCount = state.Planar ? 1 : ReflectionFaceCount;

	const beg::TextureTargetDesc targetDesc(
			state.ReflectionResolution, state.ReflectionResolution,
			beg::TextureTargetFlags::AutoGenMipMaps,
			beGraphics::Format::R16G16B16F,
			beGraphics::SampleDesc(),
			faceCount
		);
	lean::resource_ptr<besc::Pipe> reflectionPipe = perspectivePool.GetPipePool()->GetPipe(targetDesc);

	uint4 reflectionOffset = (uint4) reflections.size();
	M::PerspectiveState::ReflectionState &reflectionState = new_emplace(reflections) M::PerspectiveState::ReflectionState(controllerIdx);

	for (uint4 i = 0; i < ReflectionFaceCount; ++i)
		// TODO: Specific stages, limited processing, shadow re-use?
		reflectionState.Perspective[i] = perspectivePool.GetPerspective(reflectionPipe, nullptr);

	return reflectionOffset;
}


} // namespace

// Prepares the given render queue for the given perspective.
bool WaterControllers::Prepare(besc::PipelinePerspective &perspective, besc::PipelineQueueID queueID,
							   const besc::PipelineStageDesc &stageDesc, const besc::RenderQueueDesc &queueDesc) const
{
	LEAN_STATIC_PIMPL_CONST();
	M::PerspectiveState &state = m.perspectiveState.GetExistingState(perspective, &m);
	const M::Data &data = *m.data;
	
	const M::Data::Queue *pDataQueue = data.queues.GetExistingQueue(queueID);
	if (!pDataQueue) return false;
	const M::Data::Queue &dataQueue = *pDataQueue;
	
	// Prepare shadows for _rendered_ light passes
	for (M::PerspectiveState::visible_materials_t::iterator it = state.visibleMaterials.begin(), itEnd = state.visibleMaterials.end();
		it != itEnd && it->controllerIdx < data.reflectionControllerCount; ++it)
	{
		const M::PerspectiveState::VisibleMaterial &visibleMaterial = *it;

		// Make sure *something* is rendered
		if (dataQueue.materialsToPasses[visibleMaterial.materialIdx] < dataQueue.materialsToPasses[visibleMaterial.materialIdx + 1])
		{
			uint1 &reflectionActive = state.reflectionsActive[visibleMaterial.controllerIdx];

			// Add missing shadow
			if (!reflectionActive)
				AddReflection(
						state.reflectionState, visibleMaterial.controllerIdx,
						data.controllers(M::state)[visibleMaterial.controllerIdx].Config,
						perspective, *m.perspectivePool
					);
			
			// Mark shadows of visible lights as active
			reflectionActive = true;
		}
	}

	if (!queueDesc.DepthSort)
	{
		// Not utilizing frame coherence so far
		M::PerspectiveState::Queue &stateQueue = state.queues.GetParallelQueue(data.queues, pDataQueue);
		stateQueue.visiblePasses.clear();

		for (M::PerspectiveState::visible_materials_t::iterator it = state.visibleMaterials.begin(), itEnd = state.visibleMaterials.end(); it < itEnd; ++it)
		{
			const M::PerspectiveState::VisibleMaterial &visibleMaterial = *it;

			uint4 passStartIdx = dataQueue.materialsToPasses[visibleMaterial.materialIdx];
			uint4 passEndIdx = dataQueue.materialsToPasses[visibleMaterial.materialIdx + 1];

			for (uint4 passIdx = passStartIdx; passIdx < passEndIdx; ++passIdx)
				stateQueue.visiblePasses.push_back( M::PerspectiveState::Queue::VisiblePass(visibleMaterial.controllerIdx, passIdx) );
		}

		return !stateQueue.visiblePasses.empty();
	}
	else
	{
		besc::PipelinePerspective::QueueHandle jobQueue = perspective.QueueRenderJobs(queueID);
		size_t prevExtPassCount = state.externalPasses.size();

		for (uint4 i = 0, count = (uint4) state.visibleMaterials.size(); i < count; ++i)
		{
			const M::PerspectiveState::VisibleMaterial &visibleMaterial = state.visibleMaterials[i];

			uint4 passStartIdx = dataQueue.materialsToPasses[visibleMaterial.materialIdx];
			uint4 passEndIdx = dataQueue.materialsToPasses[visibleMaterial.materialIdx + 1];

			float distance = state.distances[i];

			for (uint4 passIdx = passStartIdx; passIdx < passEndIdx; ++passIdx)
			{
				uint4 externalPassIdx = (uint4) state.externalPasses.size();
				state.externalPasses.push_back( M::PerspectiveState::ExternalPass(visibleMaterial.controllerIdx, &dataQueue.passes[passIdx]) );
				perspective.AddRenderJob( jobQueue, besc::OrderedRenderJob(this, externalPassIdx, *reinterpret_cast<const uint4*>(&distance)) );
			}
		}

		return state.externalPasses.size() > prevExtPassCount;
	}
}

namespace
{

const float ReflectionFaceRotation = bem::Constants::pi<float>::value - ReflectionFaceFOV / 2.0f;
const float CosReflectionFaceRotation = cos(ReflectionFaceRotation);
const float SinReflectionFaceRotation = sin(ReflectionFaceRotation);

/// Prepares the reflection data for rendering.
void PrepareReflection(const WaterControllers::M::Configuration &state,
					   const besc::RenderableEffectData &constants,
					   WaterControllers::M::PerspectiveState::ReflectionConstants &reflectionConstants,
					   const WaterControllers::M::PerspectiveState::ReflectionState &reflectionState,
					   besc::PipelinePerspective &perspective)
{
	LEAN_FREE_PIMPL(WaterControllers);

	const besc::PerspectiveDesc &camDesc = perspective.GetDesc();
	const beMath::fvec3 waterPos(constants.Transform[3]);
	const beMath::fmat3 waterOrientation(normalize_rows(bem::fmat3(constants.Transform)));
	
	const beMath::fvec3 reflectionPos = waterPos + reflect(camDesc.CamPos - waterPos, waterOrientation[1]);

	besc::PerspectiveDesc reflectDesc(camDesc);
	reflectDesc.CamPos = reflectionPos;
	reflectDesc.Flipped = !camDesc.Flipped;

	bem::fplane3 clipPlane(-waterOrientation[1], waterPos - waterOrientation[1] * state.ClipTolerance);
	
	if (state.Planar)
	{
		// Compute planarly reflected perspective
		reflectDesc.CamLook = reflect(camDesc.CamLook, waterOrientation[1]);
		reflectDesc.CamUp = reflect(camDesc.CamUp, waterOrientation[1]);
		reflectDesc.CamRight = reflect(camDesc.CamRight, waterOrientation[1]);
		reflectDesc.ViewMat = bem::mat_view(reflectDesc.CamPos, reflectDesc.CamLook, reflectDesc.CamUp, reflectDesc.CamRight);
		reflectDesc.ProjMat = bem::replace_near_plane(
				reflectDesc.ProjMat,
				mul( mat_transform(reflectDesc.CamPos, reflectDesc.CamLook, reflectDesc.CamUp, reflectDesc.CamRight), clipPlane )
			);
		reflectDesc.ViewProjMat = mul(reflectDesc.ViewMat, reflectDesc.ProjMat);
		besc::PerspectiveDesc::ExtractFrustum(reflectDesc.Frustum, reflectDesc.ViewProjMat);
		reflectDesc.OutputIndex = 0;

		reflectionState.Perspective[0]->SetDesc(reflectDesc);
		perspective.AddPerspective(reflectionState.Perspective[0]);
	}
	else
	{
		// Compute object-aligned perspective, rotated to include the object's zenith
		reflectDesc.CamLook = CosReflectionFaceRotation * waterOrientation[2] + SinReflectionFaceRotation * waterOrientation[1];
		reflectDesc.CamUp = SinReflectionFaceRotation * waterOrientation[2] - CosReflectionFaceRotation * waterOrientation[1];
		reflectDesc.CamRight = waterOrientation[0];
		
		const bem::fmat3 faceDelta = bem::mat_rot<3>(waterOrientation[1], ReflectionFaceFOV);

		// Compute reflection faces for the entire hemisphere
		for (uint4 i = 0; i < ReflectionFaceCount; ++i)
		{
			reflectDesc.ViewMat = bem::mat_view(reflectDesc.CamPos, reflectDesc.CamLook, reflectDesc.CamUp, reflectDesc.CamRight);
			reflectDesc.ProjMat = bem::mat_proj(ReflectionFaceFOV, 1.0f, camDesc.NearPlane, camDesc.FarPlane);
			reflectDesc.ProjMat = bem::replace_near_plane(
				reflectDesc.ProjMat,
				mul( mat_transform(reflectDesc.CamPos, reflectDesc.CamLook, reflectDesc.CamUp, reflectDesc.CamRight), clipPlane )
			);
			reflectDesc.ViewProjMat = mul(reflectDesc.ViewMat, reflectDesc.ProjMat);
			besc::PerspectiveDesc::ExtractFrustum(reflectDesc.Frustum, reflectDesc.ViewProjMat);
			reflectDesc.OutputIndex = i;

			reflectionState.Perspective[i]->SetDesc(reflectDesc);
			perspective.AddPerspective(reflectionState.Perspective[i]);

			// Rotate to next face
			reflectDesc.CamLook = mul(reflectDesc.CamLook, faceDelta);
			reflectDesc.CamUp = mul(reflectDesc.CamUp, faceDelta);
			reflectDesc.CamRight = mul(reflectDesc.CamRight, faceDelta);
		}
	}
}

} // namespace

// Prepares the collected render queues for the given perspective.
void WaterControllers::Collect(besc::PipelinePerspective &perspective) const
{
	LEAN_STATIC_PIMPL_CONST();
	M::PerspectiveState &state = m.perspectiveState.GetExistingState(perspective, &m);
	const M::Data &data = *m.data;

	uint4 inactiveReflectionCount = 0;

	// Prepare shadows for _rendered_ light passes
	for (M::PerspectiveState::reflection_state_t::iterator it = state.reflectionState.begin(), itEnd = state.reflectionState.end(); it != itEnd; ++it)
	{
		M::PerspectiveState::ReflectionState &reflectionState = *it;

		if (state.reflectionsActive[reflectionState.ControllerIdx])
			// Update active shadows
			PrepareReflection(data.controllers(M::state)[reflectionState.ControllerIdx].Config,
				data.controllers(M::renderableData)[reflectionState.ControllerIdx],
				state.reflectionConstants[reflectionState.ControllerIdx],
				reflectionState,
				perspective);
		else
			++inactiveReflectionCount;
	}

	if (inactiveReflectionCount)
		for (M::PerspectiveState::reflection_state_t::iterator it = state.reflectionState.end(), itEnd = state.reflectionState.end(); it-- != itEnd; )
			if (!state.reflectionsActive[it->ControllerIdx])
				state.reflectionState.erase(it);
}

namespace
{

struct PassMaterialSorter
{
	const WaterControllers::M::Data::Queue::passes_t &passes;

	PassMaterialSorter(const WaterControllers::M::Data::Queue::passes_t &passes)
		: passes(passes) { }

	LEAN_INLINE bool operator ()(const WaterControllers::M::PerspectiveState::Queue::VisiblePass &l,
								 const WaterControllers::M::PerspectiveState::Queue::VisiblePass &r) const
	{
		const WaterControllers::M::Data::Queue::Pass &leftPass = passes[l.passIdx];
		const WaterControllers::M::Data::Queue::Pass &rightPass = passes[r.passIdx];

		// TODO: Input layout?

		if (leftPass.pass < rightPass.pass)
			return true;
		else if (leftPass.pass == rightPass.pass)
			return leftPass.material < rightPass.material;

		return false;
	}
};

} // namespace

// Performs optional optimization such as sorting.
void WaterControllers::Optimize(const besc::PipelinePerspective &perspective, besc::PipelineQueueID queueID) const
{
	LEAN_STATIC_PIMPL_CONST();
	M::PerspectiveState &state = m.perspectiveState.GetExistingState(perspective, &m);
	const M::Data &data = *m.data;
	
	const M::Data::Queue *pDataQueue = data.queues.GetExistingQueue(queueID);
	if (!pDataQueue) return;
	const M::Data::Queue &dataQueue = *pDataQueue;
	M::PerspectiveState::Queue &stateQueue = state.queues.GetParallelQueue(data.queues, pDataQueue);

	std::sort(stateQueue.visiblePasses.begin(), stateQueue.visiblePasses.end(), PassMaterialSorter(dataQueue.passes));
}

namespace
{

void RenderPass(const WaterControllers::M &m, const WaterControllers::M::PerspectiveState &state,
				const besc::RenderableEffectData &data, const besc::PipelinePerspective &perspective, uint4 controllerIdx,
				const WaterControllers::M::Data::Queue::Pass &pass, const besc::RenderContext &renderContext)
{
	LEAN_FREE_PIMPL(WaterControllers);

	const beGraphics::Any::DeviceContext &deviceContext = ToImpl(renderContext.Context());
	beGraphics::Any::StateManager &stateManager = ToImpl(renderContext.StateManager());

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	deviceContext->IASetInputLayout(nullptr);

	deviceContext->IASetIndexBuffer(m.triTopology.indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	stateManager.Revert();
	pass.material->Apply(deviceContext);

	struct DrawJob : lean::vcallable_base<besc::AbstractRenderableEffectDriver::DrawJobSignature, DrawJob>
	{
		uint4 indexCount;

		void operator ()(uint4 passIdx, beGraphics::StateManager &stateManager, const beGraphics::DeviceContext &context)
		{
			ToImpl(stateManager).Reset();
			ToImpl(context)->DrawIndexed(indexCount, 0, 0);
		}
	};

	DrawJob drawJob;
	drawJob.indexCount = m.triTopology.indexCount;

	// TODO: Do something sensible
	besc::RenderableEffectDataEx rde;
	rde.Data = data;
	rde.ElementCount = drawJob.indexCount;

	if (state.reflectionsActive[controllerIdx])
		pass.reflectionTexture->SetResource(ToImpl(state.reflectionTextures[controllerIdx]));

	pass.effectDriver->Render(pass.pass, &rde.Data, perspective, drawJob, stateManager, deviceContext);
}

} // namespace

// Prepares rendering from the collected render queues for the given perspective.
void WaterControllers::PreRender(const besc::PipelinePerspective &perspective, const besc::RenderContext &renderContext) const
{
	LEAN_STATIC_PIMPL_CONST();
	M::PerspectiveState &state = m.perspectiveState.GetExistingState(perspective, &m);
	const M::Data &data = *m.data;
	beg::api::DeviceContext *deviceContext = ToImpl(renderContext.Context());

/*	// TODO: Update changed only
	if (data.lightConstantBuffer)
		deviceContext->UpdateSubresource(data.lightConstantBuffer, 0, nullptr, &data.controllers(M::constants)[0], 0, 0);
	if (state.shadowConstantBuffer)
		deviceContext->UpdateSubresource(state.shadowConstantBuffer, 0, nullptr, &state.shadowConstants[0], 0, 0);
*/
	// Gather reflections 
	for (M::PerspectiveState::reflection_state_t::iterator it = state.reflectionState.begin(), itEnd = state.reflectionState.end(); it != itEnd; ++it)
	{
		const beg::TextureTarget *pTarget = (it->Perspective[0]) ? it->Perspective[0]->GetPipe()->GetAnyTarget("SceneTarget") : nullptr;
		state.reflectionTextures[it->ControllerIdx] =  beg::Any::TextureViewHandle( (pTarget) ? pTarget->GetTexture() : nullptr );
	}
}

// Renders the given render queue for the given perspective.
void WaterControllers::Render(const besc::PipelinePerspective &perspective, besc::PipelineQueueID queueID, const besc::RenderContext &context) const
{
	LEAN_STATIC_PIMPL_CONST();
	M::PerspectiveState &state =  m.perspectiveState.GetExistingState(perspective, &m);
	const M::Data &data = *m.data;
	
	const M::Data::Queue *pDataQueue = data.queues.GetExistingQueue(queueID);
	if (!pDataQueue) return;
	const M::Data::Queue &dataQueue = *pDataQueue;
	M::PerspectiveState::Queue &stateQueue = state.queues.GetParallelQueue(data.queues, pDataQueue);

	const besc::RenderableEffectData *renderableData = &data.controllers(M::renderableData)[0];
	const M::Data::Queue::Pass *passes = &dataQueue.passes[0];

	for (M::PerspectiveState::Queue::visible_passes_t::iterator it = stateQueue.visiblePasses.begin(), itEnd = stateQueue.visiblePasses.end();
		it != itEnd; ++it)
	{
		const M::PerspectiveState::Queue::VisiblePass &visiblePass = *it;
		RenderPass(m, state,
			renderableData[visiblePass.controllerIdx], perspective,
			visiblePass.controllerIdx, passes[visiblePass.passIdx], context);
	}
}

// Renders the given single object for the given perspective.
void WaterControllers::Render(uint4 objectID, const besc::PipelinePerspective &perspective, besc::PipelineQueueID queueID, const besc::RenderContext &context) const
{
	LEAN_STATIC_PIMPL_CONST();
	M::PerspectiveState &state =  m.perspectiveState.GetExistingState(perspective, &m);
	const M::Data &data = *m.data;

	LEAN_ASSERT(objectID < state.externalPasses.size());
	const M::PerspectiveState::ExternalPass &job = state.externalPasses[objectID];

	RenderPass(m, state,
		data.controllers(M::renderableData)[job.controllerIdx], perspective,
		job.controllerIdx, *job.pass, context);
}

/*
namespace
{

void SetupRendering(const besc::RenderContext &renderContext)
{
	beg::api::DeviceContext *deviceContext = ToImpl(renderContext.Context());

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	deviceContext->IASetInputLayout(nullptr);

	beg::api::Buffer *noBuffer = nullptr;
	UINT vertexStride = 0;
	UINT vertexOffset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &noBuffer, &vertexStride, &vertexOffset);
	deviceContext->IASetIndexBuffer(noBuffer, DXGI_FORMAT_R16_UINT, 0);
}

void RenderPass(const besc::LightEffectData &data, const besc::PipelinePerspective &perspective, uint4 controllerIdx,
				const WaterControllers::M::Data::Queue::Pass &pass, const besc::RenderContext &renderContext)
{
	const beGraphics::Any::DeviceContext &deviceContext = ToImpl(renderContext.Context());
	beGraphics::Any::StateManager &stateManager = ToImpl(renderContext.StateManager());

	stateManager.Revert();
	pass.material->Apply(deviceContext);

	struct DrawJob : lean::vcallable_base<AbstractLightEffectDriver::DrawJobSignature, DrawJob>
	{
		uint4 controllerIdx;

		void operator ()(uint4 passIdx, beGraphics::StateManager &stateManager, const beGraphics::DeviceContext &context)
		{
			ToImpl(stateManager).Reset();
			ToImpl(context)->DrawInstanced(4, 1, 0, controllerIdx);
		}
	};

	DrawJob drawJob;
	drawJob.controllerIdx = controllerIdx;

	pass.effectDriver->Render(pass.pass, &data, perspective, drawJob, stateManager, deviceContext);
}

} // namespace

// Prepares rendering from the collected render queues for the given perspective.
void WaterControllers::PreRender(const PipelinePerspective &perspective, const RenderContext &renderContext) const
{
	LEAN_STATIC_PIMPL_CONST();
	typename M::PerspectiveState &state = m.perspectiveState.GetExistingState(perspective, &m);
	const typename M::Data &data = *m.data;
	beg::api::DeviceContext *deviceContext = ToImpl(renderContext.Context());

	// TODO: Update changed only
	if (data.lightConstantBuffer)
		deviceContext->UpdateSubresource(data.lightConstantBuffer, 0, nullptr, &data.controllers(M::constants)[0], 0, 0);
	if (state.shadowConstantBuffer)
		deviceContext->UpdateSubresource(state.shadowConstantBuffer, 0, nullptr, &state.shadowConstants[0], 0, 0);

	// Gather shadows 
	for (typename M::PerspectiveState::shadow_state_t::iterator it = state.shadowState.begin(), itEnd = state.shadowState.end(); it != itEnd; ++it)
		state.shadowTextures[it->ControllerIdx] = GatherReflection(*it);
}

// Renders the given render queue for the given perspective.
void WaterControllers::Render(const besc::PipelinePerspective &perspective, besc::PipelineQueueID queueID, const besc::RenderContext &context) const
{
	LEAN_STATIC_PIMPL_CONST();
	typename M::PerspectiveState &state = m.perspectiveState.GetExistingState(perspective, &m);
	const typename M::Data &data = *m.data;
	
	const typename M::Data::Queue *pDataQueue = data.queues.GetExistingQueue(queueID);
	if (!pDataQueue) return;
	const typename M::Data::Queue &dataQueue = *pDataQueue;
	typename M::PerspectiveState::Queue &stateQueue = state.queues.GetParallelQueue(data.queues, pDataQueue);

	LightEffectData lightData;
	lightData.TypeID = LightInternals<WaterController>::LightTypeID;
	lightData.Lights = beg::Any::TextureViewHandle(data.lightConstantSRV);
	lightData.Reflections = beg::Any::TextureViewHandle(state.shadowConstantSRV);
	
	SetupRendering(context);

	{
		typename M::PerspectiveState::Queue::visible_passes_t::iterator it = stateQueue.visiblePasses.begin(), itEnd = stateQueue.visiblePasses.end();

		lightData.ReflectionMapCount = 1;

		// Draw lights with shadow
		for (; it != itEnd && it->controllerIdx < data.shadowControllerCount; ++it)
		{
			lightData.ReflectionMaps = &state.shadowTextures[it->controllerIdx];
			RenderPass(lightData, perspective, it->controllerIdx, dataQueue.passes[it->passIdx], context);
		}

		lightData.ReflectionMaps = nullptr;
		lightData.ReflectionMapCount = 0;

		// Draw lights without shadow
		for (; it != itEnd; ++it)
		{
			// TODO: Allow multiple ...
			RenderPass(lightData, perspective, it->controllerIdx, dataQueue.passes[it->passIdx], context);
		}
	}
}

// Renders the given single object for the given perspective.
void WaterControllers::Render(uint4 objectID, const besc::PipelinePerspective &perspective, besc::PipelineQueueID queueID, const besc::RenderContext &context) const
{
	LEAN_STATIC_PIMPL_CONST();
	typename M::PerspectiveState &state = m.perspectiveState.GetExistingState(perspective, &m);
	const typename M::Data &data = *m.data;

	LEAN_ASSERT(objectID < state.externalPasses.size());
	const typename M::PerspectiveState::ExternalPass &job = state.externalPasses[objectID];

	bool bHasReflection = job.controllerIdx < data.shadowControllerCount;

	LightEffectData lightData;
	lightData.TypeID = LightInternals<WaterController>::LightTypeID;
	lightData.Lights = beg::Any::TextureViewHandle(data.lightConstantSRV);
	lightData.Reflections = beg::Any::TextureViewHandle(state.shadowConstantSRV);
	lightData.ReflectionMaps = (bHasReflection) ? &state.shadowTextures[job.controllerIdx] : nullptr;
	lightData.ReflectionMapCount = bHasReflection;
	
	SetupRendering(context);
	RenderPass(lightData, perspective, job.controllerIdx, *job.pass, context);
}
*/
namespace
{

void PropertyChanged(WaterControllers::M &m, uint4 internalIdx)
{
	LEAN_FREE_PIMPL(WaterControllers);
	M::Data &data = *m.data;
	const bec::ComponentObserverCollection &observers = data.controllers(M::observers)[internalIdx];

	if (observers.HasObservers())
		observers.EmitPropertyChanged(*data.controllers(M::record)[internalIdx].Reflected);
}

void ComponentChanged(WaterControllers::M &m, uint4 internalIdx)
{
	LEAN_FREE_PIMPL(WaterControllers);
	M::Data &data = *m.data;
	const bec::ComponentObserverCollection &observers = data.controllers(M::observers)[internalIdx];

	if (observers.HasObservers())
		observers.EmitChildChanged(*data.controllers(M::record)[internalIdx].Reflected);
}

// Updates the transformed bounding sphere of the given controller.
void UpdateBounds(WaterControllers::M &m, uint4 internalIdx, lean::tristate extVisible)
{
	LEAN_FREE_PIMPL(WaterControllers);
	M::Data &data = *m.data;

	const M::State &state = data.controllers(M::state)[internalIdx];
	const besc::RenderableEffectData &constants = data.controllers(M::renderableData)[internalIdx];
	bem::faab3 &bounds = data.controllers(M::bounds)[internalIdx];

	bool bEntityVisible = (extVisible != lean::dontcare)
		? (extVisible != lean::carefalse)
		: (bounds.min < bounds.max);
	
	if (state.Visible && bEntityVisible)
		bounds = mulh(state.Config.LocalBounds, constants.Transform);
	else
		bounds = bem::faab3::invalid;
}

} // namespace

// Sets the material.
void WaterControllers::SetMaterial(WaterControllerHandle controller, besc::RenderableMaterial *pMaterial)
{
	BE_STATIC_PIMPL_HANDLE(controller);
	
	M::Record &record = m.data->controllers(M::record)[controller.Index];

	if (record.Material != pMaterial)
	{
		record.Material = pMaterial;
		++m.controllerRevision;

		ComponentChanged(m, controller.Index);
	}
}

// Gets the material.
besc::RenderableMaterial* WaterControllers::GetMaterial(const WaterControllerHandle controller)
{
	BE_STATIC_PIMPL_HANDLE_CONST(controller);
	return m.data->controllers(M::record)[controller.Index].Material;
}

// Sets the visibility.
void WaterControllers::SetVisible(WaterControllerHandle controller, bool bVisible)
{
	BE_STATIC_PIMPL_HANDLE(controller);
	m.data->controllers(M::state)[controller.Index].Visible = bVisible;

	UpdateBounds(m, controller.Index, lean::dontcare);
	PropertyChanged(m, controller.Index);
}

// Gets the visibility.
bool WaterControllers::IsVisible(const WaterControllerHandle controller)
{
	BE_STATIC_PIMPL_HANDLE_CONST(controller);
	return m.data->controllers(M::state)[controller.Index].Visible;
}

// Sets the local bounding box.
void WaterControllers::SetLocalBounds(WaterControllerHandle controller, const beMath::faab3 &bounds)
{
	BE_STATIC_PIMPL_HANDLE(controller);
	m.data->controllers(M::state)[controller.Index].Config.LocalBounds = bounds;

	UpdateBounds(m, controller.Index, lean::dontcare);
}

// Gets the local bounding box.
const beMath::faab3& WaterControllers::GetLocalBounds(const WaterControllerHandle controller)
{
	BE_STATIC_PIMPL_HANDLE_CONST(controller);
	return m.data->controllers(M::state)[controller.Index].Config.LocalBounds;
}

void WaterControllers::EnableReflection(WaterControllerHandle controller, bool bEnable)
{
	BE_STATIC_PIMPL_HANDLE(controller);
	M::Configuration &state = m.data->controllers(M::state)[controller.Index].Config;

	if (state.Reflection != bEnable)
	{
		state.Reflection = bEnable;
		++m.controllerRevision;

		PropertyChanged(m, controller.Index);
	}
}

// Gets the local bounding sphere.
bool WaterControllers::IsReflectionEnabled(const WaterControllerHandle controller)
{
	BE_STATIC_PIMPL_HANDLE_CONST(controller);
	return m.data->controllers(M::state)[controller.Index].Config.Reflection;
}

// Sets the local bounding sphere.
void WaterControllers::SetReflectionResolution(WaterControllerHandle controller, uint4 resolution)
{
	BE_STATIC_PIMPL_HANDLE(controller);
	m.data->controllers(M::state)[controller.Index].Config.ReflectionResolution = resolution;
	PropertyChanged(m, controller.Index);
}

// Gets the local bounding sphere.
uint4 WaterControllers::GetReflectionResolution(const WaterControllerHandle controller)
{
	BE_STATIC_PIMPL_HANDLE_CONST(controller);
	return m.data->controllers(M::state)[controller.Index].Config.ReflectionResolution;
}

// Sets the reflection clip tolerance.
void WaterControllers::SetReflectionTolerance(WaterControllerHandle controller, float tolerance)
{
	BE_STATIC_PIMPL_HANDLE(controller);
	m.data->controllers(M::state)[controller.Index].Config.ClipTolerance = tolerance;
	PropertyChanged(m, controller.Index);
}

// Gets the reflection clip tolerance.
float WaterControllers::GetReflectionTolerance(const WaterControllerHandle controller)
{
	BE_STATIC_PIMPL_HANDLE_CONST(controller);
	return m.data->controllers(M::state)[controller.Index].Config.ClipTolerance;
}

// Adds a controller
WaterController* WaterControllers::AddController()
{
	LEAN_STATIC_PIMPL();
	M::Data &data = *m.data;

	uint4 internalIdx = static_cast<uint4>(data.controllers.size());
	WaterController *handle = new(m.handles.allocate()) WaterController( WaterControllerHandle(&m, internalIdx) );

	try
	{
		data.controllers.push_back(
				M::Record(handle),
				M::State(bem::faab3(bem::vec(-1.0f, -1.0f, -1.0f), bem::vec(1.0f, 0.0f, 1.0f)), m.defaultReflectionResolution)
			);
	}
	catch (...)
	{
		m.handles.free(handle);
		throw;
	}

	++m.controllerRevision;

	return handle;
}

// Clones the given controller.
WaterController* WaterControllers::CloneController(const WaterControllerHandle controller)
{
	BE_STATIC_PIMPL_HANDLE(const_cast<WaterControllerHandle&>(controller));
	M::Data &data = *m.data;

	lean::scoped_ptr<WaterController> clone( m.AddController() );
	WaterControllerHandle cloneHandle = clone->Handle();
	
	// IMPORTANT: Clone only "data" part of state
	data.controllers(M::state)[cloneHandle.Index].Config = data.controllers(M::state)[controller.Index].Config;
	data.controllers(M::renderableData)[cloneHandle.Index] = data.controllers(M::renderableData)[controller.Index];
	SetMaterial(clone->Handle(), data.controllers[controller.Index].Material);
	
	return clone.detach();
}

// Removes a controller.
void WaterControllers::RemoveController(WaterController *pController)
{
	if (!pController || !pController->Handle().Group)
		return;

	BE_STATIC_PIMPL_HANDLE(pController->Handle());
	M::Data &data = *m.data;

	uint4 internalIdx  = pController->Handle().Index;

	try
	{
		data.controllers.erase(internalIdx);
		m.handles.free(pController);
	}
	LEAN_ASSERT_NOEXCEPT

	// Fix subsequent handles
	M::FixControllerHandles(data.controllers, internalIdx);

	++m.controllerRevision;
}

// Attaches the controller to the given entity.
void WaterControllers::Attach(WaterControllerHandle controller, beEntitySystem::Entity *entity)
{
	BE_STATIC_PIMPL_HANDLE(controller);
	M::State &state = m.data->controllers(M::state)[controller.Index];

	if (!state.Attached)
	{
		state.Attached = true;
		++m.controllerRevision;
	}
}

// Detaches the controller from the given entity.
void WaterControllers::Detach(WaterControllerHandle controller, beEntitySystem::Entity *entity)
{
	BE_STATIC_PIMPL_HANDLE(controller);
	M::State &state = m.data->controllers(M::state)[controller.Index];

	if (state.Attached)
	{
		state.Attached = false;
		++m.controllerRevision;
	}
}
// Sets the component monitor.
void WaterControllers::SetComponentMonitor(beCore::ComponentMonitor *componentMonitor)
{
	LEAN_STATIC_PIMPL();
	m.pComponentMonitor = componentMonitor;
}

// Gets the component monitor.
beCore::ComponentMonitor* WaterControllers::GetComponentMonitor() const
{
	LEAN_STATIC_PIMPL_CONST();
	return m.pComponentMonitor;
}

// Adds a property listener.
void WaterController::AddObserver(beCore::ComponentObserver *listener)
{
	BE_FREE_STATIC_PIMPL_HANDLE(WaterControllers, m_handle);
	m.data->controllers(M::observers)[m_handle.Index].AddObserver(listener);
}

// Removes a property listener.
void WaterController::RemoveObserver(beCore::ComponentObserver *pListener)
{
	BE_FREE_STATIC_PIMPL_HANDLE(WaterControllers, m_handle);
	m.data->controllers(M::observers)[m_handle.Index].RemoveObserver(pListener);
}

// Synchronizes this controller with the given entity controlled.
void WaterController::Flush(const beEntitySystem::EntityHandle entity)
{
	BE_FREE_STATIC_PIMPL_HANDLE(WaterControllers, m_handle);
	M::Data &data = *m.data;

	using beEntitySystem::Entities;

	const Entities::Transformation& entityTrafo = Entities::GetTransformation(entity);
	const M::State &state = data.controllers(M::state)[m_handle.Index];
	besc::RenderableEffectData &renderableData = data.controllers(M::renderableData)[m_handle.Index];
	
	renderableData.ID = Entities::GetCurrentID(entity);

	renderableData.Transform = mat_transform(
			entityTrafo.Position,
			entityTrafo.Orientation[2] * entityTrafo.Scaling[2],
			entityTrafo.Orientation[1] * entityTrafo.Scaling[1],
			entityTrafo.Orientation[0] * entityTrafo.Scaling[0]
		);
	renderableData.TransformInv = mat_transform_inverse(
			entityTrafo.Position,
			entityTrafo.Orientation[2] / entityTrafo.Scaling[2],
			entityTrafo.Orientation[1] / entityTrafo.Scaling[1],
			entityTrafo.Orientation[0] / entityTrafo.Scaling[0]
		);

	UpdateBounds(m, m_handle.Index, Entities::IsVisible(entity));
}

// Gets the number of child components.
uint4 WaterController::GetComponentCount() const
{
	return 1;
}

// Gets the name of the n-th child component.
beCore::Exchange::utf8_string WaterController::GetComponentName(uint4 idx) const
{
	return "Material";
}

// Gets the n-th reflected child component, nullptr if not reflected.
lean::com_ptr<const beCore::ReflectedComponent, lean::critical_ref> WaterController::GetReflectedComponent(uint4 idx) const
{
	return Reflect(GetMaterial());
}


// Gets the type of the n-th child component.
const beCore::ComponentType* WaterController::GetComponentType(uint4 idx) const
{
	return besc::RenderableMaterial::GetComponentType();
}

// Gets the n-th component.
lean::cloneable_obj<lean::any, true> WaterController::GetComponent(uint4 idx) const
{
	return bec::any_resource_t<besc::RenderableMaterial>::t( GetMaterial() );
}

// Returns true, if the n-th component can be replaced.
bool WaterController::IsComponentReplaceable(uint4 idx) const
{
	return true;
}

// Sets the n-th component.
void WaterController::SetComponent(uint4 idx, const lean::any &pComponent)
{
	SetMaterial( any_cast<besc::RenderableMaterial*>(pComponent) );
}

} // namespace
