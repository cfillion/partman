#include "vendor/catch.hpp"

#include <sstream>

#include "../src/tokens.hpp"
#include "tokens_mock.hpp"

using namespace std;

const char *M = "[tokens]";

TEST_CASE("Default token", M) {
  auto tk = make_shared<Token>();

  SECTION("Converts to an empty string") {
    REQUIRE(tk->code() == "");
  }

  SECTION("Stream operator") {
    stringstream ss = stringstream();
    ss << tk;
    REQUIRE(ss.str() == tk->code());
  }

  SECTION("empty() ignores the sub-tokens count") {
    REQUIRE(tk->empty() == false);
  }

  SECTION("Sub-tokens are separated by two new line") {
    *tk << make_shared<Token>();
    *tk << make_shared<Token>();
    REQUIRE(tk->code() == "\n\n\n");
  }

  SECTION("Empty sub-tokens are skipped") {
    *tk << make_shared<EmptyToken>();
    REQUIRE(tk->code() == "");
  }
}

TEST_CASE("Commands", M) {
  auto tk = make_shared<Command>("foo");

  SECTION("Lilypond code") {
    REQUIRE(tk->code() == "\\foo");
  }

  SECTION("Sub-tokens are appended") {
    *tk << make_shared<Command>("bar");
    REQUIRE(tk->code() == "\\foo \\bar");
  }

  SECTION("Empty sub-tokens are ignored") {
    *tk << make_shared<EmptyToken>();
    REQUIRE(tk->code() == "\\foo");
  }

  SECTION("Get command name") {
    REQUIRE(tk->name() == "foo");
  }

  SECTION("Set command name (method)") {
    tk->changeName("bar");
    REQUIRE(tk->name() == "bar");
  }

  SECTION("Set command name (operator)") {
    *tk = "bar";
    REQUIRE(tk->name() == "bar");
  }
}

TEST_CASE("Brace Blocks", M) {
  auto tk = make_shared<Block>(Block::BraceStyle);

  SECTION("Lilypond code") {
    REQUIRE(tk->code() == "{}");
  }

  SECTION("Sub-tokens formatting") {
    *tk << make_shared<TestToken>();
    REQUIRE(tk->code() == "{\n  test\n}");
  }

  SECTION("Sub-tokens indentation") {
    auto sub = make_shared<Block>(Block::BraceStyle);
    *tk << sub;
    *sub << make_shared<TestToken>();
    REQUIRE(tk->code() == "{\n  {\n    test\n  }\n}");
  }

  SECTION("Empty sub-tokens are ignored") {
    *tk << make_shared<EmptyToken>();
    REQUIRE(tk->code() == "{}");
  }
}

TEST_CASE("Bracket Blocks", M) {
  auto tk = make_shared<Block>(Block::BracketStyle);
  REQUIRE(tk->code() == "<<>>");
}

TEST_CASE("Variables", M) {
  auto val = make_shared<TestToken>();
  auto tk = make_shared<Variable>("foo", val);

  CHECK_FALSE(tk->empty());

  SECTION("Lilypond code") {
    REQUIRE(tk->code() == "foo = test");
  }

  SECTION("Get value") {
    REQUIRE(tk->value() == val);
  }

  SECTION("Set value (method)") {
    auto new_val = make_shared<EmptyToken>();
    tk->changeValue(new_val);

    CHECK(tk->empty());
    REQUIRE(tk->value() == new_val);
  }

  SECTION("Set value (operator)") {
    auto new_val = make_shared<EmptyToken>();
    *tk = new_val;
    REQUIRE(tk->value() == new_val);
  }
}

TEST_CASE("Booleans", M) {
  SECTION("true") {
    auto tk = make_shared<Boolean>(true);
    REQUIRE(tk->code() == "##t");
  }

  SECTION("false") {
    auto tk = make_shared<Boolean>(false);
    REQUIRE(tk->code() == "##f");
  }
}

TEST_CASE("Strings", M) {
  SECTION("Empty value") {
    auto tk = make_shared<String>("");
    CHECK(tk->empty());
    REQUIRE(tk->code() == "\"\"");
  }

  SECTION("Lilypond code") {
    auto tk = make_shared<String>("foo");
    CHECK_FALSE(tk->empty());
    REQUIRE(tk->code() == "\"foo\"");
  }

  SECTION("Quotes are escaped") {
    auto tk = make_shared<String>("a\"b");
    REQUIRE(tk->code() == "\"a\\\"b\"");
  }

  SECTION("Get value") {
    auto tk = make_shared<String>("foo");
    REQUIRE(tk->get() == "foo");
  }

  SECTION("Set value (method)") {
    auto tk = make_shared<String>("");
    tk->set("foo");
    REQUIRE(tk->get() == "foo");
  }

  SECTION("Set value (operator)") {
    auto tk = make_shared<String>("");
    *tk = "foo";
    REQUIRE(tk->get() == "foo");
  }
}

TEST_CASE("Literals", M) {
  SECTION("Lilypond code") {
    auto tk = make_shared<Literal>("whatever");
    REQUIRE(tk->code() == "whatever");
  }
}

TEST_CASE("Functions", M) {
  SECTION("Lilypond code") {
    auto tk = make_shared<Function>("fname");
    *tk << make_shared<TestToken>();
    REQUIRE(tk->code() == "#(fname test)");
  }
}

TEST_CASE("Comments", M) {
  SECTION("Undecorated") {
    auto tk = make_shared<Comment>("hello\nworld");
    REQUIRE(tk->code() == "% hello\n% world");
  }

  SECTION("Decorated") {
    auto tk = make_shared<Comment>("hello\nworld", true);
    REQUIRE(tk->code() == "% hello %\n% world %");
  }
}
