#include <libwine/wine-parse-version.hxx>

#include <algorithm>

using namespace std;

namespace wine
{
  namespace
  {
    // Parse an unsigned integer from a string.
    //
    // We parse a sequence of digits starting at position p in string s,
    // storing the result in r and updating p to point just past the
    // parsed number. Bail out if no digits are found or if we overflow.
    //
    bool
    parse_uint64 (const string& input, size_t& position, uint64_t& result)
    {
      // Bail out early if we're already at or past the end of the string.
      //
      if (position >= input.size ())
        return false;

      const char* b (input.data () + position);
      const char* e (input.data () + input.size ());

      // Let the standard library do the heavy lifting. It handles base 10
      // parsing and overflow detection natively without allocating or
      // throwing exceptions.
      //
      auto r (from_chars (b, e, result));

      // If we didn't see any digits or if the number overflowed uint64_t,
      // report failure.
      //
      if (r.ec != errc ())
        return false;

      // Update the position to point just past the parsed number.
      //
      position = static_cast<size_t> (r.ptr - input.data ());
      return true;
    }
  }

  version::
  version (const string& s)
  {
    // parse_version will throw if things go south, so we just
    // assign the result directly.
    //
    *this = parse_version (s);
  }

  string version::
  to_string () const
  {
    string r;

    // The major and minor components are always present.
    //
    r += std::to_string (major);
    r += '.';
    r += std::to_string (minor);

    // Tack on the patch version if we have one.
    //
    if (patch != 0)
    {
      r += '.';
      r += std::to_string (patch);
    }

    // Same for the RC component.
    //
    if (rc != 0)
    {
      r += "-rc";
      r += std::to_string (rc);
    }

    // Finally, append any build information.
    //
    if (!build_info.empty ())
    {
      r += ' ';
      r += build_info;
    }

    return r;
  }

  version
  parse_version (const string& s)
  {
    version r;
    size_t p (0);

    // Skip the "wine-" prefix if it happens to be there.
    //
    if (s.compare (0, 5, "wine-") == 0)
      p = 5;

    // We must at least have a valid major version.
    //
    if (!parse_uint64 (s, p, r.major))
      throw version_parse_error ("invalid major version");

    // The minor version is strictly required and must be separated
    // by a dot.
    //
    if (p >= s.size () || s[p] != '.')
      throw version_parse_error ("missing or invalid minor version");

    ++p;

    if (!parse_uint64 (s, p, r.minor))
      throw version_parse_error ("invalid minor version");

    // The patch version is optional, but if there's a dot, we expect
    // a valid number to follow.
    //
    if (p < s.size () && s[p] == '.')
    {
      ++p;

      if (!parse_uint64 (s, p, r.patch))
        throw version_parse_error ("invalid patch version");
    }

    // Check if we are dealing with a release candidate.
    //
    if (p < s.size () && s[p] == '-')
    {
      ++p;

      // Bail out if it doesn't look like an "-rc" suffix.
      //
      if (s.compare (p, 2, "rc") != 0)
        throw version_parse_error ("invalid RC format");

      p += 2;

      if (!parse_uint64 (s, p, r.rc))
        throw version_parse_error ("invalid RC version");
    }

    // Whatever is left must be build info, and it needs to be set off
    // by whitespace.
    //
    if (p < s.size ())
    {
      if (!isspace (static_cast<unsigned char> (s[p])))
        throw version_parse_error ("invalid build info format");

      // Eat up any leading whitespace.
      //
      p = distance (s.begin (),
                    find_if_not (s.begin () + p,
                                 s.end (),
                                 [] (unsigned char c)
      {
        return isspace (c);
      }));

      if (p < s.size ())
        r.build_info = s.substr (p);
    }

    return r;
  }
}
