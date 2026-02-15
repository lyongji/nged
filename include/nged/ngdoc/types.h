#pragma once

#include "../gmath.h"
#include "../utils.h"
#include "../event.h"
#include <nlohmann/json_fwd.hpp>
#include <fmt/format.h> // TODO: maybe use std::format
#include <uuid.h>
#include <phmap.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace nged { // {{{

using sint  = intptr_t;
using uint  = uintptr_t;
using Json  = nlohmann::json;
using Vec2  = gmath::Vec2;
using Mat3  = gmath::Mat3;
using AABB  = gmath::AABB;
using Color = gmath::sRGBColor;

template<class K, class V>
using HashMap = phmap::flat_hash_map<K, V>;
template<class K>
using HashSet = phmap::flat_hash_set<K>;
template<class T>
using Vector     = std::vector<T>;
using String     = std::string;
using StringView = std::string_view;
template<class T>
using Optional   = std::optional<T>;

// ItemID & Connection {{{
class ItemID
{
  union
  {
    uint64_t id_;
    struct
    {
      uint32_t random_;
      uint32_t index_;
    };
  };

public:
  constexpr ItemID(uint64_t id = -2) : id_{id} {}
  constexpr ItemID(uint32_t random, uint32_t index) : random_(random), index_(index) {}
  constexpr ItemID(ItemID const& that)            = default;
  constexpr ItemID& operator=(ItemID const& that) = default;

  // make it able to be map key
  bool operator<(ItemID const& that) const { return id_ < that.id_; }
  // make it able to be hash map key
  bool operator==(ItemID const& that) const { return id_ == that.id_; }
  bool operator!=(ItemID const& that) const { return id_ != that.id_; }

  size_t   hash() const { return std::hash<uint64_t>()(id_); }
  uint64_t value() const { return id_; }
  uint32_t index() const { return index_; }
};

static constexpr ItemID ID_None = {-1u, -1u};

/// Connection to an output port of source node
struct InputConnection
{
  ItemID sourceItem{ID_None};
  sint   sourcePort{-1};

  bool operator==(InputConnection const& that) const
  {
    return sourceItem == that.sourceItem && sourcePort == that.sourcePort;
  }
};

/// Connection to an input port of destiny node
struct OutputConnection
{
  ItemID destItem{ID_None};
  sint   destPort{-1};

  bool operator==(OutputConnection const& that) const
  {
    return destItem == that.destItem && destPort == that.destPort;
  }
};

struct NodePin
{
  ItemID node  = ID_None;
  sint   index = -1;
  enum class Type
  {
    None,
    In,
    Out
  } type = Type::None;
  bool operator==(NodePin const& that) const
  {
    return node == that.node && index == that.index && type == that.type;
  }
  bool operator!=(NodePin const& that) const { return !operator==(that); }
};

static constexpr NodePin PIN_None = {ID_None, -1, NodePin::Type::None};
// }}}

} // }}} namespace nged

// std::hash {{{
template<>
struct std::hash<nged::ItemID>
{
  size_t operator()(nged::ItemID id) const { return id.hash(); }
};

template<>
struct std::hash<nged::OutputConnection>
{
  size_t operator()(nged::OutputConnection const& c) const
  {
    return c.destItem.hash() ^ std::hash<nged::sint>()(c.destPort * 2021);
  }
};
// }}}

namespace nged {

class Graph;
class GraphItem;
class Node;
class Link;
class Router;
class ResizableBox;
class GroupBox;
class GraphItemFactory;
class NodeGraphDoc;
class Canvas;
class NodeFactory;

using GraphItemPtr        = std::shared_ptr<GraphItem>;
using NodePtr             = std::shared_ptr<Node>;
using LinkPtr             = std::shared_ptr<Link>;
using RouterPtr           = std::shared_ptr<Router>;
using GraphPtr            = std::shared_ptr<Graph>;
using WeakGraphPtr        = std::weak_ptr<Graph>;
using NodeGraphDocPtr     = std::shared_ptr<NodeGraphDoc>;
using NodeFactoryPtr      = std::shared_ptr<NodeFactory>;
using GraphItemFactoryPtr = std::shared_ptr<GraphItemFactory>;
using UID                 = uuids::uuid;

// UID Related {{{
UID generateUID();

inline UID uidFromString(StringView str)
{
  return uuids::uuid::from_string(str).value();
}

inline String uidToString(UID uid)
{
  return uuids::to_string(uid);
}
// }}}

} // namespace nged
