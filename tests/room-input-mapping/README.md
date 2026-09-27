# Keyboard geometry without captured input

`Build-Tests.cmd` uses `QT_ROOT` and optionally `OPENRGB_CORE_DIR`. It builds the
production Visual Map router and core virtual wrapper, with synthetic devices
only, then runs Qt offscreen. It never registers Raw Input or opens hardware.

The optional `room_input::RGBControllerInputMappingInterface` exports a cached,
value-only inverse of the exact forward LED sample positions. Keyboard member
segments keep their global LED indexes, identities and names; affine transforms,
fractional points and duplicate physical positions are preserved. It does not
interpret names as scan codes. Out-of-canvas coordinates are preserved rather
than silently clamped. The input consumer chooses whether to accept those points.

Readers copy one mapping generation under a dedicated mutex and never call a
physical controller. Membership changes clear the cache; rebuild and disable /
re-enable advance its generation. Disabled maps return no geometry. More than
16,384 keyboard points makes the inverse map unavailable instead of returning
a misleading partial keyboard. The host must clear pending visual events when
the source map or generation changes.

Coverage: two segments, non-keyboard exclusion, explicit independent 90-degree
affine expectation, fractional duplicate cells, cached identity access,
disable/re-enable, move/rebind, removal, immutable old copies, simultaneous reads
and rebuilds, admission limit and a legacy host without the image attachment API.
The test does not prove physical keyboard-to-controller identity matching; that
belongs to the separate native input service.
