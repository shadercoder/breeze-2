/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#include "bePhysicsInternal/stdafx.h"
#include "bePhysics/PX3/beMaterial.h"
#include "bePhysics/PX3/beDevice.h"

#include "bePhysics/beMaterialSerialization.h"

#include <beCore/beReflectionProperties.h>

#include <beCore/bePropertySerialization.h>
#include <lean/xml/xml_file.h>
#include <lean/xml/utility.h>

#include <lean/logging/errors.h>

namespace bePhysics
{

namespace PX3
{

const beCore::ReflectionProperties MaterialProperties = beCore::ReflectionProperties::construct_inplace()
	<< beCore::MakeReflectionProperty<float>("restitution", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&Material::SetRestitution) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&Material::GetRestitution) )
	<< beCore::MakeReflectionProperty<float>("static friction", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&Material::SetStaticFriction) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&Material::GetStaticFriction) )
	<< beCore::MakeReflectionProperty<float>("dynamic friction", beCore::Widget::Raw)
		.set_setter( BE_CORE_PROPERTY_SETTER(&Material::SetDynamicFriction) )
		.set_getter( BE_CORE_PROPERTY_GETTER(&Material::GetDynamicFriction) );

// Creates a material.
physx::PxMaterial* CreateMaterial(physx::PxPhysics &physics, float staticFriction, float dynamicFriction, float restitution)
{
	physx::PxMaterial *pMaterial = physics.createMaterial(staticFriction, dynamicFriction, restitution);

	if (!pMaterial)
		LEAN_THROW_ERROR_MSG("physx::PxPhysics::createMaterial()");

	return pMaterial;
}

// Constructor.
Material::Material(bePhysics::Device &device, float staticFriction, float dynamicFriction, float restitution)
	: m_pMaterial( CreateMaterial(*ToImpl(device), staticFriction, dynamicFriction, restitution) )
{
}

// Constructor.
Material::Material(physx::PxMaterial *material)
	: m_pMaterial( LEAN_ASSERT_NOT_NULL(material) )
{
}

// Destructor.
Material::~Material()
{
}

// Gets the reflection properties.
Material::Properties Material::GetMaterialProperties() 
{
	return ToPropertyRange(MaterialProperties);
}

// Gets the reflection properties.
Material::Properties Material::GetReflectionProperties() const
{
	return ToPropertyRange(MaterialProperties);
}

} // namespace

// Creates a physics material.
lean::resource_ptr<Material, true> CreateMaterial(Device &device, float staticFriction, float dynamicFriction, float restitution)
{
	return new_resource PX3::Material(
			PX3::CreateMaterial(*ToImpl(device), staticFriction, dynamicFriction, restitution)
		);
}

// Load the given material from the given XML node.
void LoadMaterial(Material &material, const rapidxml::xml_node<lean::utf8_t> &node)
{
	LoadProperties(material, node);
}

// Saves the given material to the given XML node.
void SaveMaterial(const Material &material, rapidxml::xml_node<lean::utf8_t> &node)
{
	SaveProperties(material, node);
}

// Loads the given material from the given XML document.
void LoadMaterial(Material &material, const rapidxml::xml_document<lean::utf8_t> &document)
{
	const rapidxml::xml_node<lean::utf8_t> *root = document.first_node("material");

	if (root)
		return LoadMaterial(material, *root);
	else
		LEAN_THROW_ERROR_MSG("(Physics) Material root node missing");
}

// Loads the given material from the given XML file.
void LoadMaterial(Material &material, const utf8_ntri &file)
{
	LEAN_LOG("Attempting to load (physics) material \"" << file.c_str() << "\"");

	LoadMaterial(material, lean::xml_file<lean::utf8_t>(file).document());

	LEAN_LOG("Material \"" << file.c_str() << "\" created successfully");
}

/// Loads the given material from the given XML node.
lean::resource_ptr<Material, true> LoadMaterial(Device &device, const rapidxml::xml_node<lean::utf8_t> &node)
{
	lean::resource_ptr<Material, true> material = CreateMaterial(device, 0.9f, 0.8f, 0.1f);
	LoadMaterial(*material, node);
	return material;
}

// Loads the given material from the given XML file.
lean::resource_ptr<Material, true> LoadMaterial(Device &device, const utf8_ntri &file)
{
	lean::resource_ptr<Material, true> material = CreateMaterial(device, 0.9f, 0.8f, 0.1f);
	LoadMaterial(*material, file);
	return material;
}

// Saves the given material to the given XML file.
void SaveMaterial(const Material &material, const utf8_ntri &file)
{
	lean::xml_file<lean::utf8_t> xml;
	rapidxml::xml_node<lean::utf8_t> &root = *lean::allocate_node<utf8_t>(xml.document(), "material");

	// ORDER: Append FIRST, otherwise parent document == nullptr
	xml.document().append_node(&root);
	SaveMaterial(material, root);

	xml.save(file);
}

} // namespace