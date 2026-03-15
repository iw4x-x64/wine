#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <libwine/wine.hxx>

#undef NDEBUG
#include <cassert>

using namespace std;
using namespace wine;

static void
test_version ()
{
  // Parse basic stable versions.
  //
  {
    auto v (parse_version ("9.0"));
    assert (v.major == 9 && v.minor == 0 && v.patch == 0);
    assert (v.rc == 0);
    assert (v.build_info.empty ());
    assert (v.to_string () == "9.0");
  }

  {
    auto v (parse_version ("8.21"));
    assert (v.major == 8 && v.minor == 21 && v.patch == 0);
    assert (v.rc == 0);
    assert (v.build_info.empty ());
    assert (v.to_string () == "8.21");
  }

  // Parse versions with an explicit patch component.
  //
  {
    auto v (parse_version ("8.0.2"));
    assert (v.major == 8 && v.minor == 0 && v.patch == 2);
    assert (v.rc == 0);
    assert (v.build_info.empty ());
    assert (v.to_string () == "8.0.2");
  }

  // Parse release candidate versions. Note how the RC number is extracted
  // and stored independently of the patch version.
  //
  {
    auto v (parse_version ("9.0-rc1"));
    assert (v.major == 9 && v.minor == 0 && v.patch == 0);
    assert (v.rc == 1);
    assert (v.build_info.empty ());
    assert (v.to_string () == "9.0-rc1");
  }

  {
    auto v (parse_version ("9.0-rc10"));
    assert (v.major == 9 && v.minor == 0 && v.patch == 0);
    assert (v.rc == 10);
    assert (v.build_info.empty ());
    assert (v.to_string () == "9.0-rc10");
  }

  // Parse versions carrying trailing build information.
  //
  {
    auto v (parse_version ("wine-9.0 (Staging)"));
    assert (v.major == 9 && v.minor == 0 && v.patch == 0);
    assert (v.rc == 0);
    assert (v.build_info == "(Staging)");
    assert (v.to_string () == "9.0 (Staging)");
  }

  {
    auto v (parse_version ("9.0 (Staging)"));
    assert (v.major == 9 && v.minor == 0 && v.patch == 0);
    assert (v.rc == 0);
    assert (v.build_info == "(Staging)");
    assert (v.to_string () == "9.0 (Staging)");
  }

  {
    auto v (parse_version ("9.0-rc1 (Staging)"));
    assert (v.major == 9 && v.minor == 0 && v.patch == 0);
    assert (v.rc == 1);
    assert (v.build_info == "(Staging)");
    assert (v.to_string () == "9.0-rc1 (Staging)");
  }

  {
    auto v (parse_version ("8.0.2 (Ubuntu 22.04)"));
    assert (v.major == 8 && v.minor == 0 && v.patch == 2);
    assert (v.rc == 0);
    assert (v.build_info == "(Ubuntu 22.04)");
    assert (v.to_string () == "8.0.2 (Ubuntu 22.04)");
  }

  // Handle the sometimes-present 'wine-' prefix.
  //
  {
    auto v (parse_version ("wine-8.21"));
    assert (v.major == 8 && v.minor == 21 && v.patch == 0);
    assert (v.rc == 0);
    assert (v.build_info.empty ());
    assert (v.to_string () == "8.21");
  }

  // Test error handling for malformed versions.
  //
  {
    vector<pair<string, string>> bad ({
      {"", "invalid major version"},
      {"9", "missing or invalid minor version"},
      {"9.", "invalid minor version"},
      {".0", "invalid major version"},
      {"9.0.", "invalid patch version"},
      {"9.0-", "invalid RC format"},
      {"9.0-rc", "invalid RC version"},
      {"9.0-beta1", "invalid RC format"},
      {"a.b", "invalid major version"},
      {"9.0a", "invalid build info format"},
      {"wine", "invalid major version"},
      {"wine-", "invalid major version"},
      {"wine-a.b", "invalid major version"}
    });

    for (const auto& p : bad)
    {
      try
      {
        parse_version (p.first);
        assert (false);
      }
      catch (const version_parse_error& e)
      {
        assert (string (e.what ()) == p.second);
      }
    }
  }

  // Deal with irregular whitespace in build info. It should be captured
  // exactly as-is after the leading separator space.
  //
  {
    auto v (parse_version ("9.0  (Staging)"));
    assert (v.build_info == "(Staging)");
  }

  {
    auto v (parse_version ("9.0    Multiple   Spaces"));
    assert (v.build_info == "Multiple   Spaces");
  }
}

static void
test_version_comparison ()
{
  version v1 ("9.0");
  version v2 ("8.21");
  version v3 ("9.0");
  version v4 ("9.0.1");
  version v5 ("9.0-rc1");

  // Test major version dominance.
  //
  assert (v1 > v2);
  assert (v2 < v1);

  // Test equality.
  //
  assert (v1 == v3);

  // Test patch version significance.
  //
  assert (v4 > v1);
  assert (v1 < v4);

  // Release candidates must sort before their final stable release.
  //
  assert (v5 < v1);
  assert (v1 > v5);

  // Test minor version significance.
  //
  version v6 ("9.1");
  assert (v6 > v1);
  assert (v1 < v6);

  // Compare two release candidates to ensure numerical (not lexical) order.
  //
  version v7 ("9.0-rc2");
  assert (v7 > v5);
  assert (v5 < v7);
}

static void
test_version_constructor ()
{
  // Construct from a valid string representation.
  //
  {
    version v ("9.0");
    assert (v.major == 9 && v.minor == 0 && v.patch == 0);
  }

  // Construct from an invalid string, expecting our custom parse error.
  // The word "invalid" fails immediately at the major version check.
  //
  {
    try
    {
      version v ("invalid");
      assert (false);
    }
    catch (const version_parse_error& e)
    {
      assert (string (e.what ()) == "invalid major version");
    }
  }

  // Construct from strings containing build information.
  //
  {
    version v ("8.21.2 Ubuntu");
    assert (v.major == 8);
    assert (v.minor == 21);
    assert (v.patch == 2);
    assert (v.rc == 0);
    assert (v.build_info == "Ubuntu");
    assert (v.to_string () == "8.21.2 Ubuntu");
  }

  {
    version v ("9.0-rc1 Staging");
    assert (v.major == 9);
    assert (v.minor == 0);
    assert (v.patch == 0);
    assert (v.rc == 1);
    assert (v.build_info == "Staging");
    assert (v.to_string () == "9.0-rc1 Staging");
  }
}

static void
test_version_roundtrip ()
{
  // Test that parsing and string conversion are perfect inverse operations.
  //
  vector<string> vs ({
    "9.0",
    "8.21",
    "8.0.2",
    "9.0-rc1",
    "9.0-rc10",
    "9.0 (Staging)",
    "9.0-rc1 (Staging)",
    "8.0.2 (Ubuntu 22.04)"
  });

  for (const auto& s : vs)
  {
    auto v (parse_version (s));

    // If the 'wine-' prefix is present, it will be naturally stripped
    // in our normalized string representation.
    //
    string x (s);
    if (x.compare (0, 5, "wine-") == 0)
      x = x.substr (5);

    assert (v.to_string () == x);

    // Re-parse the generated string to ensure stable roundtripping.
    //
    auto v2 (parse_version (v.to_string ()));
    assert (v2 == v);
  }
}

int main ()
{
  test_version ();
  test_version_comparison ();
  test_version_constructor ();
  test_version_roundtrip ();
}
