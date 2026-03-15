
#pragma once

#include <compare>
#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>

#include <libwine/export.hxx>

namespace wine
{
  // Thrown when we fail to make sense of a Wine version string.
  //
  class LIBWINE_SYMEXPORT version_parse_error
    : public std::runtime_error
  {
  public:
    explicit
    version_parse_error (const std::string& w)
      : std::runtime_error (w) {}

    explicit
    version_parse_error (const char* w)
      : std::runtime_error (w) {}
  };

  // Wine version representation.
  //
  // We generally expect the format to look like this:
  // <major>.<minor>[.<patch>][-rc<N>] [<build-info>]
  //
  // A few examples we see in practice:
  //   9.0
  //   8.21
  //   9.0-rc1
  //   8.0.2
  //   wine-9.0 (Staging)
  //
  struct LIBWINE_SYMEXPORT version
  {
    std::uint64_t major;
    std::uint64_t minor;
    std::uint64_t patch;
    std::uint64_t rc;
    std::string build_info;

    version () : major (0), minor (0), patch (0), rc (0) {}

    version (std::uint64_t mj,
             std::uint64_t mi,
             std::uint64_t p = 0,
             std::uint64_t r = 0,
             std::string b = "")
      : major (mj), minor (mi), patch (p), rc (r), build_info (std::move (b)) {}

    // Try to parse the given version string.
    //
    // Throw version_parse_error if the format is completely
    // unrecognized.
    //
    explicit
    version (const std::string& s);

    // Serialize back to a string representation.
    //
    // Note that we omit the patch or RC components if they are 0, and
    // the build info if it's empty. We also drop the "wine-" prefix
    // entirely.
    //
    std::string
    to_string () const;

    // Compare two versions ignoring the build info.
    //
    std::strong_ordering
    operator <=> (const version& o) const
    {
      if (std::strong_ordering c (major <=> o.major); c != 0)
        return c;

      if (std::strong_ordering c (minor <=> o.minor); c != 0)
        return c;

      if (std::strong_ordering c (patch <=> o.patch); c != 0)
        return c;

      // Deal with the RC quirk. If one is an RC and the other is a
      // stable release, the stable one is considered newer.
      //
      if (rc == 0 && o.rc != 0)
        return std::strong_ordering::greater;

      if (rc != 0 && o.rc == 0)
        return std::strong_ordering::less;

      // Otherwise, they are either both stable or both RCs, so we
      // just compare the RC numbers.
      //
      return rc <=> o.rc;
    }

    // Check for equality.
    //
    // Consistent with the three-way comparison, we ignore build info here.
    //
    bool
    operator == (const version& o) const = default;
  };

  // Parse a Wine version string.
  //
  // Throw version_parse_error if the string is garbage and we
  // can't extract a valid version.
  //
  LIBWINE_SYMEXPORT version
  parse_version (const std::string& s);
}

// Specialize std::formatter so we can easily print version objects.
//
template <>
struct std::formatter<wine::version, char>
{
  constexpr auto
  parse (format_parse_context& ctx)
  {
    return ctx.begin ();
  }

  auto
  format (const wine::version& v, format_context& ctx) const
  {
    return format_to (ctx.out (), "{}", v.to_string ());
  }
};
