#include "vendor/catch.hpp"

#include <sstream>

#include "../src/tokens.hpp"
#include "tokens_mock.hpp"

using namespace std;

const char *M = "[tokens]";

TEST_CASE("Default token", M) {
  auto tk = make_shared<Token>();

  SECTION("Converts to an empty string") {
    const string code = tk->code();
    REQUIRE(code == "");
  }

  SECTION("Stream operator") {
    stringstream ss = stringstream() << tk;
    REQUIRE(ss.str() == tk->code());
  }

  SECTION("empty() ignores the sub-tokens count") {
    REQUIRE(tk->empty() == false);
  }

  SECTION("Separates sub-tokens by a new line") {
    *tk << make_shared<Token>();
    REQUIRE(tk->code() == "\n");
  }

  SECTION("Skips empty sub-tokens") {
    *tk << make_shared<EmptyToken>();
    REQUIRE(tk->code() == "");
  }
}
