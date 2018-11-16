#pragma once
//------------------------------------------------------------------------------
/**
	TransformComponent

	(C) 2018 Individual contributors, see AUTHORS file
*/
//------------------------------------------------------------------------------
#include "game/component/component.h"
#include "basegamefeature/messages/basegameprotocol.h"

namespace Game
{

class TransformComponent
{
	__DeclareComponent(TransformComponent)
public:
	/// Setup callbacks.
	static void SetupAcceptedMessages();

	/// Set the local transform of instance. Will update hierarchy
	static void SetLocalTransform(const uint32_t& instance, const Math::matrix44& val);
	static void SetLocalTransform(const Game::Entity& entity, const Math::matrix44& val);
	
	/// Set the world transform of an instance. This will update the hierarchy and also set the local transform.
	static void SetWorldTransform(const uint32_t& instance, const Math::matrix44& val);
	static void SetWorldTransform(const Game::Entity& entity, const Math::matrix44& val);

	/// Return the local transform of an instance.
	static Math::matrix44 GetLocalTransform(const uint32_t& instance);
	static Math::matrix44 GetLocalTransform(const Game::Entity& entity);

	/// Return the world transform of an instance.
	static Math::matrix44 GetWorldTransform(const uint32_t& instance);
	static Math::matrix44 GetWorldTransform(const Game::Entity& entity);

	/// Update relationships
	static void SetParents(const uint32_t& start, const uint32_t& end, const Util::Array<Entity>& entities, const Util::Array<uint32_t>& parentIndices);

	/// Set parent of specific entity
	static void SetParent(const Game::Entity& entity, const Game::Entity& parent);
	static void SetParent(const uint32_t& instance, const uint32_t& parentInstance);

	/// Get parent of instance
	static uint32_t GetParent(const uint32_t& instance);
	
	/// Get owner of instance
	static Game::Entity GetOwner(const uint32_t& instance);

	/// Get first child of instance
	static uint32_t GetFirstChild(const uint32_t& instance);

	/// Get the first sibling of this instance
	static uint32_t GetNextSibling(const uint32_t& instance);

	/// Get the previous sibling of this instance
	static uint32_t GetPreviousSibling(const uint32_t& instance);

private:
	/// Update the transform hierarchy of this instance and all its children
	static void UpdateHierarchy(uint32_t instance);
};

} // namespace Game