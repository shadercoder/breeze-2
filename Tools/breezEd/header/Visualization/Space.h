#ifndef SPACE_H
#define SPACE_H

#include "breezEd.h"

#include <beEntitySystem/beWorld.h>
#include <beEntitySystem/beSimulation.h>
#include <beEntitySystem/beEntities.h>

#include <beScene/beRenderContext.h>
#include <beScene/beRenderingController.h>
#include <beScene/beMeshControllers.h>

#include <beScene/beResourceManager.h>
#include <beScene/beEffectDrivenRenderer.h>

#include <beScene/beAssembledMesh.h>
#include <beScene/beRenderableMesh.h>
#include <beGraphics/beMaterial.h>

#include <lean/smart/resource_ptr.h>
#include <vector>

struct WidgetType
{
	enum T
	{
		Arrow,
		Circle,
		Plug,

		Count
	};
};


/*
bracket##mem

#define DEFAULT_MAGIC_BRACKET (
#define DEFAULT_MAGIC_BRACKET_END

#define DEFAULT_MEM_MOVE(mem) , mem(std::move(right.mem))

#define DEFAULT_MOVE_CTOR_MEMMOVE(mem, ...) , mem(right.mem) DEFAULT_MOVE_CTOR_MEMMOVE(__VA_ARGS__)
#define DEFAULT_MOVE_CTOR(type, mem, ...) type(const type &right) : mem(std::move(right.mem)) DEFAULT_MOVE_CTOR_MEMMOVE(DEFAULT_MAGIC_BRACKET, __VA_ARGS__, END) { }


bracket##mem

#define DEFAULT_MAGIC_BRACKET (
#define DEFAULT_MAGIC_BRACKET_END

#define DEFAULT_MEM_MOVE(mem) , mem(std::move(right.mem))

#define DEFAULT_MOVE_CTOR_MEMMOVE(bracket, mem, ...) , mem(right.mem) DEFAULT_MOVE_CTOR_MEMMOVE##__VA_ARGS__   (__VA_ARGS__)
#define DEFAULT_MOVE_CTOR(type, mem, ...) type(const type &right) : mem(std::move(right.mem)) DEFAULT_MOVE_CTOR_MEMMOVE(DEFAULT_MAGIC_BRACKET, __VA_ARGS__, END) { }
*/

/// Point, line, widget visualizer.
class SpaceVisualizer
{
public:
	struct WidgetData
	{
		lean::resource_ptr<beScene::AssembledMesh> mesh;
		lean::resource_ptr<beg::Material> material;
		uint4 colorID;
	};

	struct Widget
	{
		beEntitySystem::Entity *entity;
		beScene::MeshController *mesh;
	};
	typedef std::vector<Widget> widgets_t;

private:
	lean::resource_ptr<beEntitySystem::World> m_world;
	beScene::RenderingController *m_scene;
	beScene::MeshControllers *m_meshes;

	lean::resource_ptr<beEntitySystem::Simulation> m_simulation;

	WidgetData m_data[WidgetType::Count];
	widgets_t m_widgets[WidgetType::Count];

public:
	/// Constructor.
	SpaceVisualizer(beScene::ResourceManager &resources, beScene::EffectDrivenRenderer &renderer);
	/// Destructor.
	~SpaceVisualizer();
};

#endif
