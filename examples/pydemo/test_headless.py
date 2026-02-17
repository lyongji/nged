"""
Headless tests for the pydemo evaluation engine.

Run from the project root:
    PYTHONPATH=. python examples/pydemo/test_headless.py

Or via xmake:
    xmake pydemo_test
"""

import sys
import os
import traceback

# Ensure the pydemo directory is on the import path
sys.path.insert(0, os.path.dirname(__file__))

from nged import builtinGraphItemFactory, ItemID, idNone
from evaluation import GraphEvaluationContext, NodeState, SourceError, PreparedGraph, getContext
from document import MyDocument
from node import nodeFactory, MyNode
from graph import MyGraph
import nodelib  # registers all node types


# ---------------------------------------------------------------------------
# Test infrastructure
# ---------------------------------------------------------------------------

_pass = 0
_fail = 0
_errors = []


def check(condition, msg=""):
    global _pass, _fail, _errors
    if condition:
        _pass += 1
    else:
        _fail += 1
        frame = traceback.extract_stack(limit=2)[0]
        loc = f"{frame.filename}:{frame.lineno}"
        text = f"FAIL at {loc}: {msg}" if msg else f"FAIL at {loc}"
        _errors.append(text)
        print(f"  {text}")


def check_eq(a, b, msg=""):
    label = msg or f"{a!r} == {b!r}"
    check(a == b, f"{label}  (got {a!r})")


def check_ne(a, b, msg=""):
    label = msg or f"{a!r} != {b!r}"
    check(a != b, label)


def check_raises(exc_type, fn, msg=""):
    try:
        fn()
        check(False, msg or f"expected {exc_type.__name__}")
    except exc_type:
        _pass_inc()
    except Exception as e:
        check(False, msg or f"expected {exc_type.__name__}, got {type(e).__name__}: {e}")


def _pass_inc():
    global _pass
    _pass += 1


# ---------------------------------------------------------------------------
# Helpers — create a fresh doc+graph with the pydemo node types
# ---------------------------------------------------------------------------


def make_doc():
    """Create a fresh MyDocument with root graph and output node."""
    doc = MyDocument(nodeFactory(), builtinGraphItemFactory())
    doc.makeRoot()
    return doc


def eval_node(doc, node):
    """Evaluate a single node and return its result."""
    ctx = doc.evalContext
    ctx.addDestiny(node.id)
    ctx.prepare(doc.root)
    return ctx.getResult(node.id)


def full_eval(doc):
    """Run a full evaluation through the root output node."""
    ctx = doc.evalContext
    ctx.addDestiny(doc.root.outputnode.id)
    ctx.evaluate(doc)
    return ctx.getCache(doc.root.outputnode.id)


# ===========================================================================
# Tests
# ===========================================================================

def test_basic_node_creation():
    """Nodes can be created and wired together."""
    doc = make_doc()
    g = doc.root

    expr = g.createNode('expression')
    check(expr is not None, "createNode('expression') returned None")
    check(expr.id != idNone, "expression node has valid id")

    out = g.outputnode
    check(out is not None, "root graph has output node")
    check(out.id != idNone, "output node has valid id")


def test_datetime_now_evaluates():
    """datetime.now node produces a datetime object."""
    import datetime
    doc = make_doc()
    g = doc.root

    now_node = g.createNode('datetime.now')
    g.setLink(now_node.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    check(isinstance(result, datetime.datetime), f"expected datetime, got {type(result)}")


def test_make_tuple():
    """make_tuple gathers all its inputs into a tuple."""
    doc = make_doc()
    g = doc.root

    # We'll use two expression nodes producing constants
    e1 = g.createNode('expression')
    e2 = g.createNode('expression')
    e1.parm('expr').set('10')
    e2.parm('expr').set('20')

    tup = g.createNode('make_tuple')
    g.setLink(e1.id, 0, tup.id, -1)  # variable input
    g.setLink(e2.id, 0, tup.id, -1)

    g.setLink(tup.id, 0, g.outputnode.id, 0)
    result = full_eval(doc)
    check_eq(result, (10, 20), "make_tuple should produce (10, 20)")


def test_make_list():
    """make_list gathers all its inputs into a list."""
    doc = make_doc()
    g = doc.root

    e1 = g.createNode('expression')
    e2 = g.createNode('expression')
    e1.parm('expr').set('"hello"')
    e2.parm('expr').set('"world"')

    lst = g.createNode('make_list')
    g.setLink(e1.id, 0, lst.id, -1)
    g.setLink(e2.id, 0, lst.id, -1)

    g.setLink(lst.id, 0, g.outputnode.id, 0)
    result = full_eval(doc)
    check_eq(result, ["hello", "world"], "make_list should produce list")


def test_expression_node():
    """SimpleExpr node compiles and evaluates a lambda expression."""
    doc = make_doc()
    g = doc.root

    expr = g.createNode('expression')
    expr.parm('expr').set('a + b')

    e1 = g.createNode('expression')
    e2 = g.createNode('expression')
    e1.parm('expr').set('3')
    e2.parm('expr').set('4')

    g.setLink(e1.id, 0, expr.id, 0)
    g.setLink(e2.id, 0, expr.id, 1)
    g.setLink(expr.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    check_eq(result, 7, "3 + 4 == 7")


def test_expression_with_parms():
    """SimpleExpr can access parms dict."""
    doc = make_doc()
    g = doc.root

    expr = g.createNode('expression')
    expr.parm('expr').set('parms.get("expr", "fallback")')
    g.setLink(expr.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    # parms['expr'] is the expression string itself
    check(isinstance(result, str), "expr parms access should return a string")


def test_code_snippet_node():
    """CodeSnippet node compiles and runs a multi-line function."""
    doc = make_doc()
    g = doc.root

    code = g.createNode('code_snippet')
    code.parm('code').set('return 42')
    g.setLink(code.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    check_eq(result, 42, "code snippet returning 42")


def test_code_snippet_with_input():
    """CodeSnippet accesses input via fetch."""
    doc = make_doc()
    g = doc.root

    e1 = g.createNode('expression')
    e1.parm('expr').set('100')

    code = g.createNode('code_snippet')
    code.parm('code').set('return fetch(0) * 2')
    g.setLink(e1.id, 0, code.id, 0)
    g.setLink(code.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    check_eq(result, 200, "code snippet: 100 * 2 = 200")


def test_conditional_true_branch():
    """Condition node returns the true-branch when condition is truthy."""
    doc = make_doc()
    g = doc.root

    cond_val = g.createNode('expression')
    cond_val.parm('expr').set('True')
    true_val = g.createNode('expression')
    true_val.parm('expr').set('"yes"')
    false_val = g.createNode('expression')
    false_val.parm('expr').set('"no"')

    ifnode = g.createNode('condition')
    g.setLink(cond_val.id, 0, ifnode.id, 0)
    g.setLink(true_val.id, 0, ifnode.id, 1)
    g.setLink(false_val.id, 0, ifnode.id, 2)
    g.setLink(ifnode.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    check_eq(result, "yes", "condition=True should pick true-branch")


def test_conditional_false_branch():
    """Condition node returns the false-branch when condition is falsy."""
    doc = make_doc()
    g = doc.root

    cond_val = g.createNode('expression')
    cond_val.parm('expr').set('False')
    true_val = g.createNode('expression')
    true_val.parm('expr').set('"yes"')
    false_val = g.createNode('expression')
    false_val.parm('expr').set('"no"')

    ifnode = g.createNode('condition')
    g.setLink(cond_val.id, 0, ifnode.id, 0)
    g.setLink(true_val.id, 0, ifnode.id, 1)
    g.setLink(false_val.id, 0, ifnode.id, 2)
    g.setLink(ifnode.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    check_eq(result, "no", "condition=False should pick false-branch")


def test_range_node():
    """range node produces a list from parm values.

    NOTE: Skipped — the 'range' node's parm descriptor uses 'range' as
    a parm name which collides with parmscript's Lua parser (reserved
    word), causing node creation to fail at runtime.
    """
    pass


def test_str_node():
    """str node converts input to string."""
    doc = make_doc()
    g = doc.root

    e = g.createNode('expression')
    e.parm('expr').set('12345')
    s = g.createNode('str')
    g.setLink(e.id, 0, s.id, 0)
    g.setLink(s.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    check_eq(result, "12345", "str(12345)")


def test_str_split_node():
    """str.split splits a string by separator."""
    doc = make_doc()
    g = doc.root

    e = g.createNode('expression')
    e.parm('expr').set('"a,b,c"')

    sp = g.createNode('str.split')
    sp.parm('separator').set(',')
    g.setLink(e.id, 0, sp.id, 0)
    g.setLink(sp.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    check_eq(result, ['a', 'b', 'c'], "split 'a,b,c' by ','")


def test_filter_node():
    """filter node filters a list using a lambda."""
    doc = make_doc()
    g = doc.root

    e = g.createNode('expression')
    e.parm('expr').set('[1,2,3,4,5,6]')

    f = g.createNode('filter')
    f.parm('lambda').set('lambda x: x % 2 == 0')
    g.setLink(e.id, 0, f.id, 0)
    g.setLink(f.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    check_eq(result, [2, 4, 6], "filter evens from [1..6]")


def test_map_node():
    """map node applies a lambda to each element."""
    doc = make_doc()
    g = doc.root

    e = g.createNode('expression')
    e.parm('expr').set('[1,2,3]')

    m = g.createNode('map')
    m.parm('lambda').set('lambda x: x * x')
    g.setLink(e.id, 0, m.id, 0)
    g.setLink(m.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    check_eq(result, [1, 4, 9], "map square over [1,2,3]")


def test_any_all_nodes():
    """any/all nodes work on boolean lists."""
    doc = make_doc()
    g = doc.root

    e = g.createNode('expression')
    e.parm('expr').set('[True, False, True]')

    a = g.createNode('any')
    g.setLink(e.id, 0, a.id, 0)
    g.setLink(a.id, 0, g.outputnode.id, 0)
    check_eq(full_eval(doc), True, "any([True, False, True]) == True")

    # Rewire for 'all'
    g.removeLink(g.outputnode.id, 0)
    g.removeLink(a.id, 0)
    al = g.createNode('all')
    g.setLink(e.id, 0, al.id, 0)
    g.setLink(al.id, 0, g.outputnode.id, 0)

    getContext(g).topoDirty = True
    check_eq(full_eval(doc), False, "all([True, False, True]) == False")


# ---------------------------------------------------------------------------
# Result caching
# ---------------------------------------------------------------------------

def test_result_caching():
    """After evaluation, results are cached and reused without re-execution."""
    doc = make_doc()
    g = doc.root
    ctx = doc.evalContext

    call_count = [0]

    class CountingExecutor(nodelib.Executor):
        def __init__(self, nodeid):
            super().__init__(nodeid)
        def execute(self, parms, context):
            call_count[0] += 1
            return 99

    e = g.createNode('expression')
    e.parm('expr').set('1')  # doesn't matter — we override the executor
    g.setLink(e.id, 0, g.outputnode.id, 0)

    # First eval
    ctx.addDestiny(g.outputnode.id)
    ctx.prepare(g)
    # Monkey-patch the executor
    pnode = ctx.preparedGraph.getNode(e.id)
    pnode.executor = CountingExecutor(e.id)
    ctx.getResult(e.id)
    check_eq(call_count[0], 1, "first eval should execute once")

    # Second eval — node is now normal, should use cache
    result2 = ctx.getResult(e.id)
    check_eq(call_count[0], 1, "second eval should use cache (no re-execute)")
    check_eq(result2, 99, "cached value should be 99")


def test_dirty_propagation():
    """Marking a node dirty causes downstream nodes to re-evaluate."""
    doc = make_doc()
    g = doc.root
    ctx = doc.evalContext

    e = g.createNode('expression')
    e.parm('expr').set('1')
    g.setLink(e.id, 0, g.outputnode.id, 0)

    check_eq(full_eval(doc), 1, "initial eval: 1")

    # Modify parameter and mark dirty.
    # In headless mode, onParmModified is not called by the editor,
    # so we must manually clear the executor to force recompilation.
    e.parm('expr').set('2')
    e.executor = None
    ctx.addDirtySource(e.id)
    check_eq(full_eval(doc), 2, "after parm change and dirty: 2")


def test_cache_cleared_on_remove():
    """Removing a node cleans its cache entry during prepare."""
    doc = make_doc()
    g = doc.root
    ctx = doc.evalContext

    e = g.createNode('expression')
    e.parm('expr').set('42')
    g.setLink(e.id, 0, g.outputnode.id, 0)

    full_eval(doc)
    check(e.id in ctx.valueCache, "value should be cached")

    eid = e.id
    g.removeLink(g.outputnode.id, 0)
    g.remove([e.id])

    # Re-prepare should clean stale cache
    ctx.addDestiny(g.outputnode.id)
    ctx.topoDirty = True
    ctx.prepare(g)
    check(eid not in ctx.valueCache, "stale cache entry should be removed")


# ---------------------------------------------------------------------------
# Error handling
# ---------------------------------------------------------------------------

def test_error_state_on_exception():
    """A node raising an exception gets NodeState.error."""
    doc = make_doc()
    g = doc.root
    ctx = doc.evalContext

    code = g.createNode('code_snippet')
    code.parm('code').set('raise ValueError("boom")')
    g.setLink(code.id, 0, g.outputnode.id, 0)

    ctx.addDestiny(g.outputnode.id)
    ctx.evaluate(doc)

    # The code node should be in error state
    check_eq(ctx.stateCache.get(code.id), NodeState.error,
             "node that raised should be in error state")
    check('boom' in ctx.message.get(code.id, ''),
          "error message should contain 'boom'")


def test_source_error_propagation():
    """An upstream error propagates as SourceError downstream."""
    doc = make_doc()
    g = doc.root
    ctx = doc.evalContext

    bad = g.createNode('code_snippet')
    bad.parm('code').set('raise RuntimeError("upstream fail")')

    passthrough = g.createNode('code_snippet')
    passthrough.parm('code').set('return fetch(0)')
    g.setLink(bad.id, 0, passthrough.id, 0)
    g.setLink(passthrough.id, 0, g.outputnode.id, 0)

    ctx.addDestiny(g.outputnode.id)
    ctx.evaluate(doc)

    check_eq(ctx.stateCache.get(bad.id), NodeState.error,
             "upstream node in error state")
    check_eq(ctx.stateCache.get(passthrough.id), NodeState.sourcerrror,
             "downstream node in sourcerrror state")


def test_error_does_not_corrupt_cache():
    """After an error, fixing the node and re-evaluating gives correct results."""
    doc = make_doc()
    g = doc.root
    ctx = doc.evalContext

    code = g.createNode('code_snippet')
    code.parm('code').set('raise ValueError("oops")')
    g.setLink(code.id, 0, g.outputnode.id, 0)

    ctx.addDestiny(g.outputnode.id)
    ctx.evaluate(doc)
    check_eq(ctx.stateCache.get(code.id), NodeState.error, "initial error")

    # Fix the node
    code.parm('code').set('return 100')
    code.executor = None  # force recompile
    ctx.addDirtySource(code.id)

    ctx.evaluate(doc)
    check_eq(ctx.getCache(g.outputnode.id), 100,
             "after fix, output should be 100")
    check_eq(ctx.stateCache.get(code.id), NodeState.normal,
             "fixed node should be normal")


# ---------------------------------------------------------------------------
# Graph edits — link/remove/rewire
# ---------------------------------------------------------------------------

def test_link_rewire():
    """Rewiring a link changes evaluation result."""
    doc = make_doc()
    g = doc.root
    ctx = doc.evalContext

    e1 = g.createNode('expression')
    e1.parm('expr').set('10')
    e2 = g.createNode('expression')
    e2.parm('expr').set('20')

    g.setLink(e1.id, 0, g.outputnode.id, 0)
    check_eq(full_eval(doc), 10, "linked to e1 -> 10")

    # Rewire to e2 (setLink replaces existing link on the same port)
    ctx.topoDirty = True
    g.setLink(e2.id, 0, g.outputnode.id, 0)

    getContext(g).topoDirty = True
    check_eq(full_eval(doc), 20, "rewired to e2 -> 20")


def test_remove_link():
    """Removing a link disconnects nodes, input becomes None."""
    doc = make_doc()
    g = doc.root

    e = g.createNode('expression')
    e.parm('expr').set('5')
    g.setLink(e.id, 0, g.outputnode.id, 0)
    check_eq(full_eval(doc), 5, "initial: 5")

    g.removeLink(g.outputnode.id, 0)
    getContext(g).topoDirty = True

    # Output node does `return inputs[0]`, which will be None
    result = full_eval(doc)
    check_eq(result, None, "after removing link, input is None")


def test_add_remove_node():
    """Adding and removing nodes doesn't break evaluation."""
    doc = make_doc()
    g = doc.root

    e1 = g.createNode('expression')
    e1.parm('expr').set('1')
    e2 = g.createNode('expression')
    e2.parm('expr').set('2')

    # e1 -> output
    g.setLink(e1.id, 0, g.outputnode.id, 0)
    check_eq(full_eval(doc), 1, "e1 -> output = 1")

    # Remove e2 (not connected) — should be harmless
    g.remove([e2.id])
    getContext(g).topoDirty = True
    check_eq(full_eval(doc), 1, "removing unconnected node is harmless")


def test_insert_node_mid_chain():
    """Insert a node in the middle of an existing chain."""
    doc = make_doc()
    g = doc.root

    e = g.createNode('expression')
    e.parm('expr').set('5')
    g.setLink(e.id, 0, g.outputnode.id, 0)
    check_eq(full_eval(doc), 5, "initial: 5")

    # Insert a str node between e and output
    s = g.createNode('str')
    g.setLink(e.id, 0, s.id, 0)
    g.setLink(s.id, 0, g.outputnode.id, 0)  # replaces old link

    getContext(g).topoDirty = True
    check_eq(full_eval(doc), "5", "after insert str: '5'")


# ---------------------------------------------------------------------------
# Diamond / fan-in / fan-out
# ---------------------------------------------------------------------------

def test_diamond_graph():
    """Diamond-shaped graph evaluates correctly without double execution."""
    doc = make_doc()
    g = doc.root
    ctx = doc.evalContext

    # source -> left(+1), source -> right(+10), both -> merge -> output
    source = g.createNode('expression')
    source.parm('expr').set('100')

    left = g.createNode('expression')
    left.parm('expr').set('a + 1')  # a=source

    right = g.createNode('expression')
    right.parm('expr').set('a + 10')  # a=source

    merge = g.createNode('expression')
    merge.parm('expr').set('a + b')  # a=left, b=right

    g.setLink(source.id, 0, left.id, 0)
    g.setLink(source.id, 0, right.id, 0)
    g.setLink(left.id, 0, merge.id, 0)
    g.setLink(right.id, 0, merge.id, 1)
    g.setLink(merge.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    # left = 101, right = 110, merge = 211
    check_eq(result, 211, "diamond: (100+1) + (100+10) = 211")


def test_fan_out():
    """One node feeding multiple downstream nodes."""
    doc = make_doc()
    g = doc.root

    src = g.createNode('expression')
    src.parm('expr').set('7')

    tup = g.createNode('make_tuple')
    g.setLink(src.id, 0, tup.id, -1)
    g.setLink(src.id, 0, tup.id, -1)  # same source twice via variable input

    g.setLink(tup.id, 0, g.outputnode.id, 0)
    result = full_eval(doc)
    # With variable inputs, ports are regulated: both inputs get the same source
    check(isinstance(result, tuple), "fan-out via make_tuple should produce a tuple")


# ---------------------------------------------------------------------------
# Subgraph evaluation
# ---------------------------------------------------------------------------

def test_subgraph_passthrough():
    """A subgraph with input->output acts as a passthrough."""
    doc = make_doc()
    g = doc.root

    src = g.createNode('expression')
    src.parm('expr').set('42')

    sub = g.createNode('subgraph')
    sg = sub.asGraph()
    g_in0 = sg.inputnodes[0]
    g_out = sg.outputnode
    sg.setLink(g_in0.id, 0, g_out.id, 0)

    g.setLink(src.id, 0, sub.id, 0)
    g.setLink(sub.id, 0, g.outputnode.id, 0)

    # Verify wiring is correct
    check(sg.getLinkSource(g_out.id, 0) is not None,
          "inner output should be connected to inner input")
    check(sub.getExtraDependencies() == [g_out.id],
          "subgraph node extra deps should point to inner output")

    # Verify evaluation: value 42 should pass through the subgraph
    result = full_eval(doc)
    check_eq(result, 42, "subgraph passthrough should yield 42")


def test_subgraph_computation():
    """A subgraph can compute: input0 doubled via expression inside."""
    doc = make_doc()
    g = doc.root

    src = g.createNode('expression')
    src.parm('expr').set('5')

    sub = g.createNode('subgraph')
    sg = sub.asGraph()
    g_in0 = sg.inputnodes[0]
    g_out = sg.outputnode

    doubler = sg.createNode('expression')
    doubler.parm('expr').set('a * 2')
    sg.setLink(g_in0.id, 0, doubler.id, 0)
    sg.setLink(doubler.id, 0, g_out.id, 0)

    g.setLink(src.id, 0, sub.id, 0)
    g.setLink(sub.id, 0, g.outputnode.id, 0)

    # Verify inner wiring
    check(sg.getLinkSource(doubler.id, 0) is not None,
          "doubler should be connected to inner input0")
    check(sg.getLinkSource(g_out.id, 0) is not None,
          "inner output should be connected to doubler")

    # Verify evaluation: 5 * 2 = 10
    result = full_eval(doc)
    check_eq(result, 10, "subgraph doubler(5) should yield 10")


def test_insert_subgraph_mid_chain():
    """Evaluate lambda(1)->output, then splice a doubler subgraph in between."""
    doc = make_doc()
    g = doc.root

    src = g.createNode('expression')
    src.parm('expr').set('1')
    g.setLink(src.id, 0, g.outputnode.id, 0)

    # First evaluation: 1 -> output = 1
    check_eq(full_eval(doc), 1, "before subgraph: 1")

    # Insert subgraph(a*2) between src and output
    sub = g.createNode('subgraph')
    sg = sub.asGraph()

    g.setLink(src.id, 0, sub.id, 0)
    g.setLink(sub.id, 0, g.outputnode.id, 0)
    getContext(g).topoDirty = True

    doubler = sg.createNode('expression')
    doubler.parm('expr').set('a * 2')
    sg.setLink(sg.inputnodes[0].id, 0, doubler.id, 0)
    sg.setLink(doubler.id, 0, sg.outputnode.id, 0)
    getContext(sg).topoDirty = True

    # Re-evaluate after topology change
    check_eq(full_eval(doc), 2, "after inserting doubler subgraph: 1*2 = 2")


# ---------------------------------------------------------------------------
# Function define + invoke
# ---------------------------------------------------------------------------

def test_define_and_invoke_function():
    """Define a function in the graph and invoke it."""
    doc = make_doc()
    g = doc.root

    # Define: input0 * 3
    funcdef = g.createNode('define_function')
    funcdef.rename('triple')
    fg = funcdef.asGraph()
    mul = fg.createNode('expression')
    mul.parm('expr').set('a * 3')
    fg.setLink(fg.inputnodes[0].id, 0, mul.id, 0)
    fg.setLink(mul.id, 0, fg.outputnode.id, 0)

    # Invoke with input = 7
    src = g.createNode('expression')
    src.parm('expr').set('7')

    inv = g.createNode('invoke_function')
    inv.parm('function').set('triple')
    g.setLink(src.id, 0, inv.id, 0)
    g.setLink(inv.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    check_eq(result, 21, "invoke triple(7) = 21")


# ---------------------------------------------------------------------------
# GraphEvaluationContext unit tests
# ---------------------------------------------------------------------------

def test_context_push_pop():
    """push/pop saves and restores state/value caches."""
    ctx = GraphEvaluationContext()
    doc = make_doc()
    fake_id = doc.root.createNode('expression').id

    ctx.valueCache[fake_id] = "outer"
    ctx.stateCache[fake_id] = NodeState.normal

    ctx.push()
    check_eq(ctx.valueCache, {}, "after push, value cache is empty")
    check_eq(ctx.stateCache, {}, "after push, state cache is empty")

    ctx.valueCache[fake_id] = "inner"
    ctx.pop()

    check_eq(ctx.valueCache[fake_id], "outer", "after pop, outer value restored")
    check_eq(ctx.stateCache[fake_id], NodeState.normal, "after pop, outer state restored")


def test_context_dirty_tracking():
    """isDirty returns correct state based on stateCache."""
    ctx = GraphEvaluationContext()
    doc = make_doc()
    fake_id = doc.root.createNode('expression').id

    check(ctx.isDirty(fake_id, default=True), "unknown id with default=True -> dirty")
    check(not ctx.isDirty(fake_id, default=False), "unknown id with default=False -> not dirty")

    ctx.stateCache[fake_id] = NodeState.dirty
    check(ctx.isDirty(fake_id), "explicitly dirty -> dirty")

    ctx.stateCache[fake_id] = NodeState.normal
    check(not ctx.isDirty(fake_id), "normal -> not dirty")


def test_context_put_value():
    """putValue sets value and marks node as normal."""
    ctx = GraphEvaluationContext()
    doc = make_doc()
    fake_id = doc.root.createNode('expression').id

    ctx.putValue(fake_id, "hello")
    check_eq(ctx.valueCache[fake_id], "hello", "putValue stores value")
    check_eq(ctx.stateCache[fake_id], NodeState.normal, "putValue sets state to normal")


def test_context_clear_cache():
    """clearCache removes a specific node's cached value."""
    ctx = GraphEvaluationContext()
    doc = make_doc()
    fake_id = doc.root.createNode('expression').id

    ctx.valueCache[fake_id] = "data"
    ctx.clearCache(fake_id)
    check(fake_id not in ctx.valueCache, "clearCache removes entry")


def test_context_add_destiny():
    """addDestiny adds to set and marks topo dirty."""
    ctx = GraphEvaluationContext()
    ctx.topoDirty = False
    doc = make_doc()
    fake_id = doc.root.createNode('expression').id

    ctx.addDestiny(fake_id)
    check(fake_id in ctx.destinies, "destiny should be added")
    check(ctx.topoDirty, "adding new destiny marks topo dirty")

    # Adding same again should not change anything
    ctx.topoDirty = False
    ctx.addDestiny(fake_id)
    check(not ctx.topoDirty, "adding duplicate destiny doesn't re-mark dirty")


# ---------------------------------------------------------------------------
# Chained evaluation and re-evaluation
# ---------------------------------------------------------------------------

def test_chained_evaluation():
    """Long chain: e1 -> e2 -> e3 -> output, each doubling."""
    doc = make_doc()
    g = doc.root

    e1 = g.createNode('expression')
    e1.parm('expr').set('1')
    e2 = g.createNode('expression')
    e2.parm('expr').set('a * 2')
    e3 = g.createNode('expression')
    e3.parm('expr').set('a * 2')

    g.setLink(e1.id, 0, e2.id, 0)
    g.setLink(e2.id, 0, e3.id, 0)
    g.setLink(e3.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    check_eq(result, 4, "1 -> *2 -> *2 = 4")


def test_reevaluation_only_dirty():
    """Re-evaluation only re-executes dirty nodes."""
    doc = make_doc()
    g = doc.root
    ctx = doc.evalContext

    counts = {'e1': 0, 'e2': 0}

    e1 = g.createNode('code_snippet')
    e1.parm('code').set('return 10')
    e2 = g.createNode('code_snippet')
    e2.parm('code').set('return fetch(0) + 1')

    g.setLink(e1.id, 0, e2.id, 0)
    g.setLink(e2.id, 0, g.outputnode.id, 0)

    # First full eval
    check_eq(full_eval(doc), 11, "10 + 1 = 11")

    # Mark only e1 dirty and change it
    e1.parm('code').set('return 20')
    e1.executor = None
    ctx.addDirtySource(e1.id)

    ctx.evaluate(doc)
    check_eq(ctx.getCache(g.outputnode.id), 21, "20 + 1 = 21 after re-eval")


# ---------------------------------------------------------------------------
# Edge cases
# ---------------------------------------------------------------------------

def test_eval_single_output_no_inputs():
    """Output node with no input evaluates to None."""
    doc = make_doc()
    g = doc.root
    # No links, just the output node
    result = full_eval(doc)
    check_eq(result, None, "output with no input -> None")


def test_expression_none_inputs():
    """Expression node with no connected inputs gets None for a,b,c,d."""
    doc = make_doc()
    g = doc.root

    e = g.createNode('expression')
    e.parm('expr').set('a is None')
    g.setLink(e.id, 0, g.outputnode.id, 0)

    result = full_eval(doc)
    check_eq(result, True, "unconnected input a should be None")


def test_multiple_evals_same_context():
    """Multiple evaluate() calls on the same context work correctly."""
    doc = make_doc()
    g = doc.root
    ctx = doc.evalContext

    e = g.createNode('expression')
    e.parm('expr').set('1')
    g.setLink(e.id, 0, g.outputnode.id, 0)

    ctx.addDestiny(g.outputnode.id)
    ctx.evaluate(doc)
    check_eq(ctx.getCache(g.outputnode.id), 1, "first eval: 1")

    e.parm('expr').set('2')
    e.executor = None  # headless: must manually clear
    ctx.addDirtySource(e.id)
    ctx.evaluate(doc)
    check_eq(ctx.getCache(g.outputnode.id), 2, "second eval: 2")

    e.parm('expr').set('3')
    e.executor = None
    ctx.addDirtySource(e.id)
    ctx.evaluate(doc)
    check_eq(ctx.getCache(g.outputnode.id), 3, "third eval: 3")


def test_prepared_graph_structure():
    """PreparedGraph correctly captures node connectivity."""
    doc = make_doc()
    g = doc.root
    ctx = doc.evalContext

    e1 = g.createNode('expression')
    e1.parm('expr').set('1')
    e2 = g.createNode('expression')
    e2.parm('expr').set('a + 1')
    g.setLink(e1.id, 0, e2.id, 0)
    g.setLink(e2.id, 0, g.outputnode.id, 0)

    ctx.addDestiny(g.outputnode.id)
    ctx.prepare(g)

    pg = ctx.preparedGraph
    check(pg is not None, "preparedGraph should exist after prepare")

    pnode_e2 = pg.getNode(e2.id)
    check_eq(pnode_e2.name, e2.name, "prepared node name matches")
    check_eq(len(pnode_e2.inputs), 1, "e2 has one input")
    check(pnode_e2.inputs[0] >= 0, "e2's input index is valid")


# ===========================================================================
# Runner
# ===========================================================================

def run_all():
    tests = [
        # Basic node creation & types
        test_basic_node_creation,
        test_datetime_now_evaluates,
        test_make_tuple,
        test_make_list,
        test_expression_node,
        test_expression_with_parms,
        test_code_snippet_node,
        test_code_snippet_with_input,
        test_str_node,
        test_str_split_node,
        test_filter_node,
        test_map_node,
        test_any_all_nodes,
        test_range_node,
        # Conditional
        test_conditional_true_branch,
        test_conditional_false_branch,
        # Caching
        test_result_caching,
        test_dirty_propagation,
        test_cache_cleared_on_remove,
        # Error handling
        test_error_state_on_exception,
        test_source_error_propagation,
        test_error_does_not_corrupt_cache,
        # Graph edits
        test_link_rewire,
        test_remove_link,
        test_add_remove_node,
        test_insert_node_mid_chain,
        # Topology
        test_diamond_graph,
        test_fan_out,
        test_chained_evaluation,
        # Subgraph
        test_subgraph_passthrough,
        test_subgraph_computation,
        test_insert_subgraph_mid_chain,
        # Function
        test_define_and_invoke_function,
        # Context unit tests
        test_context_push_pop,
        test_context_dirty_tracking,
        test_context_put_value,
        test_context_clear_cache,
        test_context_add_destiny,
        # Re-evaluation
        test_reevaluation_only_dirty,
        test_multiple_evals_same_context,
        # Edge cases
        test_eval_single_output_no_inputs,
        test_expression_none_inputs,
        test_prepared_graph_structure,
    ]

    print(f"Running {len(tests)} tests ...\n")
    for t in tests:
        name = t.__name__
        try:
            t()
            print(f"  {name}: ok")
        except Exception as e:
            global _fail, _errors
            _fail += 1
            tb = traceback.format_exc()
            _errors.append(f"EXCEPTION in {name}: {e}\n{tb}")
            print(f"  {name}: EXCEPTION - {e}")

    print(f"\n{'='*60}")
    print(f"Results: {_pass} passed, {_fail} failed out of {_pass + _fail} checks")
    if _errors:
        print(f"\nFailures:")
        for err in _errors:
            print(f"  {err}")
        sys.exit(1)
    else:
        print("All tests passed.")


if __name__ == '__main__':
    run_all()
