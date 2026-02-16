#include <doctest/doctest.h>
#include <nged/nged.h>
#include <nlohmann/json.hpp>

#include <ostream>

namespace gmath {
std::ostream& operator<<(std::ostream& os, nged::Color const& c)
{
  return os << "Color(sRGB, " << int(c.r) << "," << int(c.g) << "," << int(c.b) << "," << int(c.a) << ")";
}
}

class DummyNode : public nged::Node
{
  int numInput=1;
  int numOutput=1;

public:
  DummyNode(int numInput, int numOutput, nged::Graph* parent, std::string const& type, std::string const& name)
    : nged::Node(parent, type, name)
    , numInput(numInput)
    , numOutput(numOutput)
  {
  }
  nged::sint numMaxInputs() const override { return numInput; }
  nged::sint numOutputs() const override { return numOutput; }
};

class SubGraphNode : public DummyNode
{
  nged::GraphPtr subgraph_;

public:
  SubGraphNode(nged::Graph* parent):
    DummyNode(1,1,parent,"subgraph","subgraph")
  {
    subgraph_ = std::make_shared<nged::Graph>(parent->docRoot(), parent, "subgraph");
  }
  virtual nged::Graph* asGraph() override { return subgraph_.get(); }
  virtual nged::Graph const* asGraph() const override { return subgraph_.get(); }
};

struct DummyNodeDef
{
  std::string type;
  int numinput, numoutput;
};

static DummyNodeDef defs[] = {
  { "exec", 4, 1 },
  { "null", 1, 1 },
  { "merge", -1, 1 },
  { "split", 1, 2 },
  { "out", 1, 0 },
  { "in", 0, 1 }
};

class MyNodeFactory: public nged::NodeFactory 
{
  nged::GraphPtr createRootGraph(nged::NodeGraphDoc* root) const override
  {
    return std::make_shared<nged::Graph>(root, nullptr, "root");
  }
  nged::NodePtr  createNode(nged::Graph* parent, std::string_view type) const override
  {
    std::string typestr(type);
    if (type=="subgraph")
      return std::make_shared<SubGraphNode>(parent);
    for (auto const& d: defs)
      if (d.type == type)
        return std::make_shared<DummyNode>(d.numinput, d.numoutput, parent, typestr, typestr);
    return std::make_shared<DummyNode>(4, 1, parent, typestr, typestr);
  }
  void listNodeTypes(
      nged::Graph* graph,
      void* context,
      void(*ret)(
        void* context,
        nged::StringView category,
        nged::StringView type,
        nged::StringView name)) const override
  {
    ret(context, "subgraph", "subgraph", "subgraph");
    for (auto const& d: defs)
      ret(context, "demo", d.type, d.type);
  }
};

TEST_CASE("Graph Creation") {
  auto itemfactory = nged::defaultGraphItemFactory();
  nged::NodeGraphDoc doc(std::make_shared<MyNodeFactory>(), itemfactory);
  doc.makeRoot();
  auto graph = doc.root();
  auto id = graph->add(nged::GraphItemPtr(doc.nodeFactory()->createNode(graph.get(), "null")));
  CHECK(id != nged::ItemID::None);
  CHECK(graph->get(id)->asNode() != nullptr);

  auto nodeptr = graph->createNode("exec");
  CHECK(nodeptr->asNode() != nullptr);
  CHECK(nodeptr->asNode()->numMaxInputs() == 4);
  CHECK(nodeptr->asNode()->numOutputs() == 1);

  graph->setLink(id, 0, nodeptr->id(), 0);
  CHECK(doc.numItems() == 3); // null, exec, link

  auto subgraphnode = graph->createNode("subgraph");
  auto* subgraph = subgraphnode->asGraph();
  CHECK(subgraph != nullptr);
  subgraph->createNode("null");
  CHECK(doc.numItems() == 5); // null, exec, link, subgraph, null

  SUBCASE("Graph Traverse") {
    auto exec = subgraph->createNode("exec");
    auto in1 = subgraph->createNode("null");
    auto in2 = subgraph->createNode("null");
    subgraph->setLink(in1->id(), 0, exec->id(), 0);
    subgraph->setLink(in2->id(), 0, exec->id(), 2);
    nged::GraphTraverseResult tr;
    CHECK(subgraph->travelBottomUp(tr, exec->id()));
    CHECK(tr.size() == 3);
    CHECK(tr.node(0) == exec.get());
    CHECK(tr.inputCount(0) == 3);
    CHECK(tr.inputOf(0, 0) == in1.get());
    CHECK(tr.inputOf(0, 1) == nullptr);
    CHECK(tr.inputOf(0, 2) == in2.get());
  }

  graph->remove({subgraphnode->id()});
  subgraphnode.reset();
  CHECK(doc.numItems() == 3); // subgraph and its content should be gone.
}

// Helper: collect node pointers in traversal order
static std::vector<nged::Node*> traverseNodes(nged::GraphTraverseResult const& tr)
{
  std::vector<nged::Node*> out;
  for (size_t i = 0; i < tr.size(); ++i)
    out.push_back(tr.node(i));
  return out;
}

TEST_CASE("Graph Traversal") {
  auto itemfactory = nged::defaultGraphItemFactory();
  nged::NodeGraphDoc doc(std::make_shared<MyNodeFactory>(), itemfactory);
  doc.makeRoot();
  auto graph = doc.root();

  SUBCASE("Single node top-down") {
    auto a = graph->createNode("null");
    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, a->id()));
    CHECK(tr.size() == 1);
    CHECK(tr.node(0) == a.get());
    CHECK(tr.inputCount(0) == 0);
    CHECK(tr.outputCount(0) == 0);
  }

  SUBCASE("Single node bottom-up") {
    auto a = graph->createNode("null");
    nged::GraphTraverseResult tr;
    CHECK(graph->travelBottomUp(tr, a->id()));
    CHECK(tr.size() == 1);
    CHECK(tr.node(0) == a.get());
  }

  SUBCASE("Linear chain top-down: A -> B -> C") {
    auto a = graph->createNode("in");   // 0 in, 1 out
    auto b = graph->createNode("null"); // 1 in, 1 out
    auto c = graph->createNode("out");  // 1 in, 0 out
    graph->setLink(a->id(), 0, b->id(), 0);
    graph->setLink(b->id(), 0, c->id(), 0);

    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, a->id()));
    CHECK(tr.size() == 3);
    // topological order: A before B before C
    auto nodes = traverseNodes(tr);
    CHECK(nodes[0] == a.get());
    CHECK(nodes[1] == b.get());
    CHECK(nodes[2] == c.get());

    // Check closures for B
    auto bIdx = tr.find(b->id());
    REQUIRE(bIdx.valid());
    CHECK(bIdx.inputCount() == 1);
    CHECK(bIdx.input(0) == a.get());
    CHECK(bIdx.outputCount() == 1);
    CHECK(bIdx.output(0) == c.get());
  }

  SUBCASE("Linear chain bottom-up from C") {
    auto a = graph->createNode("in");
    auto b = graph->createNode("null");
    auto c = graph->createNode("out");
    graph->setLink(a->id(), 0, b->id(), 0);
    graph->setLink(b->id(), 0, c->id(), 0);

    nged::GraphTraverseResult tr;
    CHECK(graph->travelBottomUp(tr, c->id()));
    CHECK(tr.size() == 3);
    // bottom-up: C before B before A
    auto nodes = traverseNodes(tr);
    CHECK(nodes[0] == c.get());
    CHECK(nodes[1] == b.get());
    CHECK(nodes[2] == a.get());
  }

  SUBCASE("Diamond graph: A -> B, A -> C, B -> D, C -> D") {
    auto a = graph->createNode("in");
    auto b = graph->createNode("null");
    auto c = graph->createNode("null");
    auto d = graph->createNode("exec"); // 4 in, 1 out
    graph->setLink(a->id(), 0, b->id(), 0);
    graph->setLink(a->id(), 0, c->id(), 0);
    graph->setLink(b->id(), 0, d->id(), 0);
    graph->setLink(c->id(), 0, d->id(), 1);

    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, a->id()));
    CHECK(tr.size() == 4);
    // A must come before B and C; B and C must come before D
    auto nodes = traverseNodes(tr);
    CHECK(nodes[0] == a.get());
    CHECK(nodes[3] == d.get());
    // B and C can be in either order, but both at positions 1-2
    CHECK(((nodes[1] == b.get() && nodes[2] == c.get()) ||
           (nodes[1] == c.get() && nodes[2] == b.get())));

    // Check D's inputs
    auto dIdx = tr.find(d->id());
    REQUIRE(dIdx.valid());
    CHECK(dIdx.inputCount() == 2);
    CHECK(dIdx.input(0) == b.get());
    CHECK(dIdx.input(1) == c.get());
  }

  SUBCASE("Diamond bottom-up from D") {
    auto a = graph->createNode("in");
    auto b = graph->createNode("null");
    auto c = graph->createNode("null");
    auto d = graph->createNode("exec");
    graph->setLink(a->id(), 0, b->id(), 0);
    graph->setLink(a->id(), 0, c->id(), 0);
    graph->setLink(b->id(), 0, d->id(), 0);
    graph->setLink(c->id(), 0, d->id(), 1);

    nged::GraphTraverseResult tr;
    CHECK(graph->travelBottomUp(tr, d->id()));
    CHECK(tr.size() == 4);
    auto nodes = traverseNodes(tr);
    // D first, A last
    CHECK(nodes[0] == d.get());
    CHECK(nodes[3] == a.get());
  }

  SUBCASE("Partial traversal: only reachable nodes") {
    // A -> B -> C, D (disconnected)
    auto a = graph->createNode("in");
    auto b = graph->createNode("null");
    auto c = graph->createNode("out");
    auto d = graph->createNode("null"); // disconnected
    graph->setLink(a->id(), 0, b->id(), 0);
    graph->setLink(b->id(), 0, c->id(), 0);

    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, a->id()));
    CHECK(tr.size() == 3); // D not included
    CHECK(!tr.find(d->id()).valid());
  }

  SUBCASE("Multiple start points top-down") {
    // A -> C, B -> C
    auto a = graph->createNode("in");
    auto b = graph->createNode("in");
    auto c = graph->createNode("exec");
    graph->setLink(a->id(), 0, c->id(), 0);
    graph->setLink(b->id(), 0, c->id(), 1);

    nged::Vector<nged::ItemID> starts = {a->id(), b->id()};
    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, starts));
    CHECK(tr.size() == 3);
    // C must be last
    auto nodes = traverseNodes(tr);
    CHECK(nodes[2] == c.get());
  }

  SUBCASE("Multiple start points bottom-up") {
    // A -> C, A -> D
    auto a = graph->createNode("in");
    auto c = graph->createNode("out");
    auto d = graph->createNode("out");
    graph->setLink(a->id(), 0, c->id(), 0);
    graph->setLink(a->id(), 0, d->id(), 0);

    nged::Vector<nged::ItemID> starts = {c->id(), d->id()};
    nged::GraphTraverseResult tr;
    CHECK(graph->travelBottomUp(tr, starts));
    CHECK(tr.size() == 3);
    // A must be last (it's the source)
    auto nodes = traverseNodes(tr);
    CHECK(nodes[2] == a.get());
  }

  SUBCASE("Variable input (merge) node") {
    auto a = graph->createNode("in");
    auto b = graph->createNode("in");
    auto c = graph->createNode("in");
    auto m = graph->createNode("merge"); // -1 inputs
    graph->setLink(a->id(), 0, m->id(), -1);
    graph->setLink(b->id(), 0, m->id(), -1);
    graph->setLink(c->id(), 0, m->id(), -1);

    nged::GraphTraverseResult tr;
    CHECK(graph->travelBottomUp(tr, m->id()));
    CHECK(tr.size() == 4);
    // merge is first
    CHECK(tr.node(0) == m.get());
    CHECK(tr.inputCount(0) == 3);
  }

  SUBCASE("Sparse input ports") {
    auto a = graph->createNode("in");
    auto b = graph->createNode("in");
    auto e = graph->createNode("exec"); // 4 inputs
    // Connect to ports 0 and 3, leaving 1 and 2 empty
    graph->setLink(a->id(), 0, e->id(), 0);
    graph->setLink(b->id(), 0, e->id(), 3);

    nged::GraphTraverseResult tr;
    CHECK(graph->travelBottomUp(tr, e->id()));
    CHECK(tr.size() == 3);
    CHECK(tr.node(0) == e.get());
    CHECK(tr.inputCount(0) == 4);
    CHECK(tr.inputOf(0, 0) == a.get());
    CHECK(tr.inputOf(0, 1) == nullptr);
    CHECK(tr.inputOf(0, 2) == nullptr);
    CHECK(tr.inputOf(0, 3) == b.get());
  }

  SUBCASE("Node outputs in closure") {
    auto a = graph->createNode("in");
    auto b = graph->createNode("out");
    auto c = graph->createNode("out");
    graph->setLink(a->id(), 0, b->id(), 0);
    graph->setLink(a->id(), 0, c->id(), 0);

    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, a->id()));
    CHECK(tr.size() == 3);
    auto aIdx = tr.find(a->id());
    REQUIRE(aIdx.valid());
    CHECK(aIdx.outputCount() == 2);
    // Both b and c should be in outputs
    std::set<nged::Node*> outputNodes;
    for (int i = 0; i < aIdx.outputCount(); ++i)
      outputNodes.insert(aIdx.output(i));
    CHECK(outputNodes.count(b.get()));
    CHECK(outputNodes.count(c.get()));
  }

  SUBCASE("Router traversal: A -> Router -> B") {
    auto a = graph->createNode("in");
    auto b = graph->createNode("out");

    auto routerItem = doc.itemFactory()->make(graph.get(), "router");
    REQUIRE(routerItem != nullptr);
    auto routerID = graph->add(routerItem);
    REQUIRE(routerID != nged::ItemID::None);

    // A -> Router, Router -> B
    graph->setLink(a->id(), 0, routerID, 0);
    graph->setLink(routerID, 0, b->id(), 0);

    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, a->id()));
    CHECK(tr.size() == 2); // only nodes, not routers
    auto nodes = traverseNodes(tr);
    CHECK(nodes[0] == a.get());
    CHECK(nodes[1] == b.get());

    // Check B's input resolves through router to A
    auto bIdx = tr.find(b->id());
    REQUIRE(bIdx.valid());
    CHECK(bIdx.inputCount() == 1);
    CHECK(bIdx.input(0) == a.get());
  }

  SUBCASE("Chained routers: A -> R1 -> R2 -> B") {
    auto a = graph->createNode("in");
    auto b = graph->createNode("out");

    auto r1Item = doc.itemFactory()->make(graph.get(), "router");
    auto r2Item = doc.itemFactory()->make(graph.get(), "router");
    auto r1ID = graph->add(r1Item);
    auto r2ID = graph->add(r2Item);

    graph->setLink(a->id(), 0, r1ID, 0);
    graph->setLink(r1ID, 0, r2ID, 0);
    graph->setLink(r2ID, 0, b->id(), 0);

    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, a->id()));
    CHECK(tr.size() == 2);
    CHECK(tr.node(0) == a.get());
    CHECK(tr.node(1) == b.get());

    // Bottom-up should also work
    nged::GraphTraverseResult tr2;
    CHECK(graph->travelBottomUp(tr2, b->id()));
    CHECK(tr2.size() == 2);
    CHECK(tr2.node(0) == b.get());
    CHECK(tr2.node(1) == a.get());
  }

  SUBCASE("Router fan-out: A -> R -> B, R -> C") {
    auto a = graph->createNode("in");
    auto b = graph->createNode("out");
    auto c = graph->createNode("out");

    auto rItem = doc.itemFactory()->make(graph.get(), "router");
    auto rID = graph->add(rItem);

    graph->setLink(a->id(), 0, rID, 0);
    graph->setLink(rID, 0, b->id(), 0);
    graph->setLink(rID, 0, c->id(), 0);

    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, a->id()));
    CHECK(tr.size() == 3);
    CHECK(tr.node(0) == a.get());
    // B and C should both appear
    std::set<nged::Node*> nodeSet;
    for (size_t i = 0; i < tr.size(); ++i)
      nodeSet.insert(tr.node(i));
    CHECK(nodeSet.count(b.get()));
    CHECK(nodeSet.count(c.get()));
  }

  SUBCASE("Topology cache invalidation on link add/remove") {
    auto a = graph->createNode("in");
    auto b = graph->createNode("out");

    // Initially disconnected
    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, a->id()));
    CHECK(tr.size() == 1);

    // Add link
    graph->setLink(a->id(), 0, b->id(), 0);
    CHECK(graph->travelTopDown(tr, a->id()));
    CHECK(tr.size() == 2);
    CHECK(tr.node(0) == a.get());
    CHECK(tr.node(1) == b.get());

    // Remove link
    graph->removeLink(b->id(), 0);
    CHECK(graph->travelTopDown(tr, a->id()));
    CHECK(tr.size() == 1);
    CHECK(tr.node(0) == a.get());
  }

  SUBCASE("Topology cache invalidation on node remove") {
    auto a = graph->createNode("in");
    auto b = graph->createNode("null");
    auto c = graph->createNode("out");
    graph->setLink(a->id(), 0, b->id(), 0);
    graph->setLink(b->id(), 0, c->id(), 0);

    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, a->id()));
    CHECK(tr.size() == 3);

    // Remove middle node
    graph->remove({b->id()});
    CHECK(graph->travelTopDown(tr, a->id()));
    CHECK(tr.size() == 1); // only A remains reachable
  }

  SUBCASE("Accessor and Iterator interface") {
    auto a = graph->createNode("in");
    auto b = graph->createNode("null");
    auto c = graph->createNode("out");
    graph->setLink(a->id(), 0, b->id(), 0);
    graph->setLink(b->id(), 0, c->id(), 0);

    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, a->id()));

    // Test iterator
    int count = 0;
    for (auto const& acc : tr) {
      CHECK(acc.valid());
      CHECK(acc.node() != nullptr);
      ++count;
    }
    CHECK(count == 3);

    // Test find
    auto found = tr.find(b->id());
    CHECK(found.valid());
    CHECK(found.node() == b.get());

    // Test find with non-existent id
    auto notFound = tr.find(nged::ItemID::None);
    CHECK(!notFound.valid());
  }

  SUBCASE("checkLoopBottomUp detects cycle") {
    auto a = graph->createNode("null");
    auto b = graph->createNode("null");
    graph->setLink(a->id(), 0, b->id(), 0);
    graph->setLink(b->id(), 0, a->id(), 0);

    nged::Vector<nged::ItemID> loopPath;
    CHECK(graph->checkLoopBottomUp(a->id(), loopPath));
    CHECK(loopPath.size() >= 2);
  }

  SUBCASE("checkLoopBottomUp no false positive") {
    auto a = graph->createNode("in");
    auto b = graph->createNode("null");
    auto c = graph->createNode("out");
    graph->setLink(a->id(), 0, b->id(), 0);
    graph->setLink(b->id(), 0, c->id(), 0);

    nged::Vector<nged::ItemID> loopPath;
    CHECK(!graph->checkLoopBottomUp(c->id(), loopPath));
  }

  SUBCASE("traverse with allowLoop=true on cyclic graph") {
    auto a = graph->createNode("null");
    auto b = graph->createNode("null");
    graph->setLink(a->id(), 0, b->id(), 0);
    graph->setLink(b->id(), 0, a->id(), 0);

    nged::GraphTraverseResult tr;
    CHECK(graph->traverse(tr, {a->id()}, true, true));
    CHECK(tr.size() == 2);
  }

  SUBCASE("traverse with allowLoop=false rejects cycle") {
    auto a = graph->createNode("null");
    auto b = graph->createNode("null");
    graph->setLink(a->id(), 0, b->id(), 0);
    graph->setLink(b->id(), 0, a->id(), 0);

    nged::GraphTraverseResult tr;
    CHECK(!graph->traverse(tr, {a->id()}, true, false));
  }

  SUBCASE("Wider graph: multiple independent paths") {
    //  A1 -> B -> D
    //  A2 -> C -> D
    auto a1 = graph->createNode("in");
    auto a2 = graph->createNode("in");
    auto b  = graph->createNode("null");
    auto c  = graph->createNode("null");
    auto d  = graph->createNode("exec"); // 4 in

    graph->setLink(a1->id(), 0, b->id(), 0);
    graph->setLink(a2->id(), 0, c->id(), 0);
    graph->setLink(b->id(), 0, d->id(), 0);
    graph->setLink(c->id(), 0, d->id(), 1);

    nged::Vector<nged::ItemID> starts = {a1->id(), a2->id()};
    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, starts));
    CHECK(tr.size() == 5);

    // D must be last
    CHECK(tr.node(tr.size() - 1) == d.get());

    // D has 2 inputs at ports 0 and 1
    auto dIdx = tr.find(d->id());
    REQUIRE(dIdx.valid());
    CHECK(dIdx.inputCount() == 2);
    CHECK(dIdx.input(0) == b.get());
    CHECK(dIdx.input(1) == c.get());
  }

  SUBCASE("idmap lookup") {
    auto a = graph->createNode("in");
    auto b = graph->createNode("null");
    auto c = graph->createNode("out");
    graph->setLink(a->id(), 0, b->id(), 0);
    graph->setLink(b->id(), 0, c->id(), 0);

    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, a->id()));

    // find() uses idmap internally
    for (size_t i = 0; i < tr.size(); ++i) {
      auto acc = tr.find(tr.node(i)->id());
      CHECK(acc.valid());
      CHECK(acc.index() == i);
      CHECK(acc.node() == tr.node(i));
    }
  }

  SUBCASE("Empty start points") {
    nged::Vector<nged::ItemID> starts;
    nged::GraphTraverseResult tr;
    CHECK(graph->travelTopDown(tr, starts));
    CHECK(tr.size() == 0);
  }
}

struct DummyTypedDef
{
  nged::String type;
  nged::String name;
  nged::Vector<nged::String> inputTypes;
  nged::Vector<nged::String> outputTypes;
};

class DummyTypedNode: public nged::TypedNode
{
public:
  DummyTypedNode(
    nged::Graph* parent,
    DummyTypedDef const& def)
    : nged::TypedNode(parent, def.type, def.name, def.inputTypes, def.outputTypes)
  {
  }

  nged::sint numMaxInputs() const override { return inputTypes_.size(); }
  nged::sint numOutputs() const override { return outputTypes_.size(); }
};

static DummyTypedDef typedDefs[] = {
  { "makeint", "makeint", {}, {"int"} },
  { "makefloat", "makefloat", {}, {"float"} },
  { "sumint", "sumint", { "int", "int" }, { "int" } },
  { "sumfloat", "sumfloat", { "float", "float" }, { "float" } },
  { "makelist", "makelist", { "any", "any", "any" }, { "list" } },
  { "reduce", "reduce", {"func", "list" }, {"any"} }
};

class MyTypedNodeFactory: public nged::NodeFactory 
{
  nged::GraphPtr createRootGraph(nged::NodeGraphDoc* root) const override
  {
    return std::make_shared<nged::Graph>(root, nullptr, "root");
  }
  nged::NodePtr  createNode(nged::Graph* parent, std::string_view type) const override
  {
    std::string typestr(type);
    for (auto const& d: typedDefs)
      if (d.type == type)
        return std::make_shared<DummyTypedNode>(parent, d);
    return nullptr;
  }
  void listNodeTypes(
      nged::Graph* graph,
      void* context,
      void(*ret)(
        void* context,
        nged::StringView category,
        nged::StringView type,
        nged::StringView name)) const override
  {
    for (auto const& d: typedDefs)
      ret(context, "demo", d.type, d.name);
  }
};

TEST_CASE("TypedNode Test") {
  SUBCASE("TypeSystem Test") {
    auto& typesys = nged::TypeSystem::instance();
    typesys.registerType("int", "", {255,255,0,255});
    typesys.registerType("float");
    typesys.registerType("vec2");
    typesys.registerType("vec3");
    typesys.registerType("vec4");
    typesys.registerType("mat2");
    typesys.registerType("mat3");
    typesys.registerType("mat4");
    typesys.registerType("string");
    typesys.registerType("bool");
    typesys.setConvertable("int", "float");
    typesys.setConvertable("float", "vec2");
    typesys.setConvertable("float", "vec3");
    typesys.setConvertable("float", "vec4");
    typesys.setConvertable("int", "string");
    typesys.setConvertable("float", "string");

    CHECK(typesys.isConvertable("int", "float"));
    CHECK(typesys.isConvertable("float", "vec2"));
    CHECK(typesys.isConvertable("float", "vec3"));
    CHECK(!typesys.isConvertable("float", "int"));
    CHECK(!typesys.isConvertable("vec2", "float"));
    CHECK(!typesys.isConvertable("float", "mat4"));

    CHECK(typesys.isConvertable("int", "any"));
    CHECK(!typesys.isConvertable("any", "int"));
  }

  auto itemfactory = nged::defaultGraphItemFactory();
  nged::NodeGraphDoc doc(std::make_shared<MyTypedNodeFactory>(), itemfactory);
  doc.makeRoot();
  auto graph = doc.root();
  auto sumint = graph->createNode("sumint");
  CHECK(sumint->id() != nged::ItemID::None);
  CHECK(graph->get(sumint->id())->asNode() == sumint.get());

  auto sumfloat = graph->createNode("sumfloat");
  CHECK(sumfloat->asNode() != nullptr);
  CHECK(sumfloat->numMaxInputs() == 2);
  CHECK(sumfloat->numOutputs() == 1);

  auto makeint = graph->createNode("makeint");
  auto makefloat = graph->createNode("makefloat");
  CHECK(sumint->acceptInput(0, makeint.get(), 0));
  CHECK(!sumint->acceptInput(1, makefloat.get(), 0));
  CHECK(sumfloat->acceptInput(0, makeint.get(), 0));
  CHECK(sumfloat->acceptInput(1, makefloat.get(), 0));

  CHECK(sumint->getPinForIncomingLink(makefloat->id(), 0) == -1);
  CHECK(sumfloat->getPinForIncomingLink(makeint->id(), 0) == 0);

  CHECK(sumint->outputPinColor(0) == nged::Color{255,255,0,255});
  CHECK(sumint->inputPinColor(0) == nged::Color{255,255,0,255});
}

TEST_CASE("Serialization Round Trip") {
  auto itemfactory = nged::defaultGraphItemFactory();

  // --- Build a graph ---
  nged::NodeGraphDoc doc(std::make_shared<MyNodeFactory>(), itemfactory);
  doc.makeRoot();
  auto graph = doc.root();

  auto exec  = graph->createNode("exec");   // 4 in, 1 out
  auto null1 = graph->createNode("null");    // 1 in, 1 out
  auto null2 = graph->createNode("null");
  auto merge = graph->createNode("merge");   // -1 (unlimited) in, 1 out
  auto out   = graph->createNode("out");     // 1 in, 0 out

  // Position nodes so we can verify positions survive round-trip
  exec->moveTo({100, 200});
  null1->moveTo({0, 0});
  null2->moveTo({0, 100});

  // Create links: null1 -> exec:0, null2 -> exec:1, exec -> out
  auto link1 = graph->setLink(null1->id(), 0, exec->id(), 0);
  auto link2 = graph->setLink(null2->id(), 0, exec->id(), 1);
  auto link3 = graph->setLink(exec->id(),  0, out->id(),  0);
  CHECK(link1 != nullptr);
  CHECK(link2 != nullptr);
  CHECK(link3 != nullptr);

  size_t itemCountBefore = doc.numItems();
  CHECK(itemCountBefore == 8); // 5 nodes + 3 links

  // --- Serialize ---
  nlohmann::json serialized;
  CHECK(graph->serialize(serialized));

  // Sanity: JSON should have items and links sections
  CHECK(serialized.contains("items"));
  CHECK(serialized.contains("links"));
  CHECK(serialized["items"].size() == 5);  // 5 nodes (non-link items)
  CHECK(serialized["links"].size() == 3);  // 3 links

  // --- Deserialize into a fresh doc ---
  nged::NodeGraphDoc doc2(std::make_shared<MyNodeFactory>(), itemfactory);
  doc2.makeRoot();
  auto graph2 = doc2.root();

  CHECK(graph2->deserialize(serialized));

  // --- Verify structure ---
  // Count nodes (non-link items)
  int nodeCount = 0;
  int linkCount = 0;
  graph2->forEachItem([&](nged::GraphItemPtr item) {
    if (item->asLink())
      ++linkCount;
    else
      ++nodeCount;
  });
  CHECK(nodeCount == 5);
  CHECK(linkCount == 3);

  // Find the exec node by type and verify its position
  nged::Node* exec2 = nullptr;
  nged::Node* null1_2 = nullptr;
  nged::Node* out2 = nullptr;
  graph2->forEachItem([&](nged::GraphItemPtr item) {
    if (auto* n = item->asNode()) {
      if (n->type() == "exec") exec2 = n;
      if (n->type() == "out")  out2 = n;
      if (n->type() == "null" && n->pos().x < 1.f && n->pos().y < 1.f)
        null1_2 = n;
    }
  });
  REQUIRE(exec2 != nullptr);
  REQUIRE(out2 != nullptr);

  // Verify position survived
  CHECK(exec2->pos().x == doctest::Approx(100.f));
  CHECK(exec2->pos().y == doctest::Approx(200.f));

  // Verify connectivity: exec should have an output link to out
  nged::InputConnection inConn;
  CHECK(graph2->getLinkSource(exec2->id(), 0, inConn));
  REQUIRE(inConn.sourceItem != nged::ItemID::None);
  auto sourceNode = graph2->get(inConn.sourceItem)->asNode();
  REQUIRE(sourceNode != nullptr);
  CHECK(sourceNode->type() == "null");

  // Verify exec's output goes to out
  nged::Vector<nged::OutputConnection> outConns;
  CHECK(graph2->getLinkDestiny(exec2->id(), 0, outConns));
  CHECK(outConns.size() == 1);
  CHECK(outConns[0].destItem == out2->id());

  SUBCASE("Double round-trip") {
    // Serialize again and verify JSON is equivalent
    nlohmann::json serialized2;
    CHECK(graph2->serialize(serialized2));
    CHECK(serialized2["items"].size() == serialized["items"].size());
    CHECK(serialized2["links"].size() == serialized["links"].size());
  }
}
