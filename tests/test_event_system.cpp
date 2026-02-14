#include <nged/event.h>
#include <doctest/doctest.h>
#include <string>

using namespace nged;

TEST_CASE("Signal connection and emission") {
    Signal<int> sig;
    int value = 0;
    
    auto handle = sig.connect([&value](int v) {
        value = v;
    });

    sig.emit(42);
    CHECK(value == 42);

    sig.disconnect(handle);
    sig.emit(100);
    CHECK(value == 42); // Should not change
}

TEST_CASE("Request with veto logic") {
    Request<int> req;
    
    req.connect([](int v) { return v > 0; });
    req.connect([](int v) { return v < 100; });

    CHECK(req.invoke(50) == true);
    CHECK(req.invoke(-1) == false);
    CHECK(req.invoke(200) == false);
}

TEST_CASE("Signal multiple listeners") {
    Signal<std::string> sig;
    std::string result;

    sig.connect([&](std::string s) { result += "A" + s; });
    sig.connect([&](std::string s) { result += "B" + s; });

    sig.emit("-");
    CHECK((result == "A-B-" || result == "B-A-")); // Order not guaranteed but likely insertion order
}

TEST_CASE("Disconnection safety") {
    Signal<> sig;
    int count = 0;
    
    auto h1 = sig.connect([&](){ count++; });
    auto h2 = sig.connect([&](){ count+=2; });

    sig.emit();
    CHECK(count == 3);

    sig.disconnect(h1);
    sig.emit();
    CHECK(count == 5); // 3 + 2

    sig.disconnect(h2);
    sig.emit();
    CHECK(count == 5); // No change
}
