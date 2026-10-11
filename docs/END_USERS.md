## Using a abysscursor theme

Download a abysscursor theme and extract it to a new directory in `~/.local/share/icons`.

Make sure the first directory contains a manifest, for example:

```s
~/.local/share/icons/myCursorTheme/manifest.hl
```

## Overriding a theme

Set the `ABYSSCURSOR_THEME` env to your theme directory,
so for example to get the above to always load, use `export ABYSSCURSOR_THEME = myCursorTheme`.