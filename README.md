# libwine

The `libwine` C++ library provides utilities for working with Wine environments
and Wine version information.

## Usage

To start using `libwine` in your project, add the following `depends`
value to your `manifest`, adjusting the version constraint as appropriate:

```
depends: libwine ^0.1.0
```

Then import the library in your `buildfile`:

```
import libs = libwine%lib{wine}
```

## License

`libwine` is licensed under the `GNU General Public License v3.0 or later`.
