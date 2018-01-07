#pragma once
//------------------------------------------------------------------------------
/**
	Base class for all model nodes, which is the resource level instantiation
	of hierarchical model. Each node represents some level in the hierarchy,
	and the different type of node contain their specific data.

	The nodes have names for lookup, the name is setup during load. 

	A node is a part of an N-tree, meaning there are N children [0-infinity]
	for each node. The model itself keeps track of the root node, and a dictionary
	of all nodes and their corresponding names.
	
	(C) 2017 Individual contributors, see AUTHORS file
*/
//------------------------------------------------------------------------------
#include "core/refcounted.h"
#include "util/stringatom.h"
#include "io/binaryreader.h"
#include "math/bbox.h"
#include "ids/id.h"

namespace Models
{

ID_32_TYPE(ModelNodeId);

class StreamModelPool;
class ModelServer;
class Model;
class ModelNode
{
public:
	/// constructor
	ModelNode();
	/// destructor
	virtual ~ModelNode();

	/// return constant reference to children
	const Util::Array<ModelNodeId>& GetChildren() const;
	/// create an instance of a node, override in the leaf classes
	virtual ModelNodeId CreateInstance() const;

protected:
	friend class StreamModelPool;
	friend class ModelContext;
	friend class ModelServer;

	/// load data
	virtual bool Load(const Util::FourCC& fourcc, const Util::StringAtom& tag, const Ptr<IO::BinaryReader>& reader);
	/// unload data
	virtual void Unload();
	/// call when model node data is finished loading (not accounting for secondary resources)
	void OnFinishedLoading();

	// base class for instances
	struct Instance
	{
		ModelNodeId parent;	// id of parent
		ModelNodeId node;		// id of resource-level node
	};

	/// setup node
	virtual void Setup();
	/// discard node
	virtual void Discard();

	Util::StringAtom name;
	ModelNodeId parent;
	ModelNodeId model;
	Util::Array<ModelNodeId> children;
	Math::bbox boundingBox;
};

//------------------------------------------------------------------------------
/**
*/
inline const Util::Array<ModelNodeId>&
ModelNode::GetChildren() const
{
	return this->children;
}

} // namespace Models