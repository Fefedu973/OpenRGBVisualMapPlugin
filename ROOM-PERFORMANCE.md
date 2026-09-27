# Nonblocking color routing and bounded diagnostic

Image routing uses the optional `RGBControllerColorFrameInterface` supplied by
the Room core. Every LED route targeting the same controller is assembled into
one immutable, indexed color frame before submission. This includes multiple
zones and all segments of a fan hub. Overlapping entries retain map order;
unlisted LEDs are preserved. Sending one replaceable frame per segment would
lose the earlier segments, so this grouping is part of the interface contract.

The core's existing device worker applies the latest accepted frame and then
performs its usual device I/O. Image routing does not wait for that I/O or take
the controller's color access mutex. An accepted frame means queued, not sent
to the physical device. The route caches its topology token when rebound, so a
resize rejects old indexes until routes are rebuilt. `Busy`, `Invalid` and
`Stale` never trigger blocking per-LED writes. Only an absent interface or
`Unsupported` uses the legacy route. The existing Plugin API 5 vtable and native
image sink path are unchanged. The source image's remaining lease is relayed
to the color queue (100–5000 ms); no hardware cadence is promised by this path.

The September 27 baseline measured a 145.6 ms mean complete route (249.7 ms
maximum; 68 frames per diagnostic window). Legacy `SetColor` waits accounted
for about 4.08 s on the GPU controller, 3.66 s on one fan controller and 1.46 s
on one memory controller per ten-second report. The Wallpaper output accounted
for only about 1.4 ms. This evidence justified isolating device I/O from the
shared route; it did not justify changing Wallpaper's resolution or transport.
These are measurements of the previous synchronous route, not measurements of
the new queue's physical output or proof that every device can sustain 60 Hz.

## Bounded diagnostic

This diagnostic build times `RouteImage` and the individual `SetColor` calls
made by its legacy LED route. It logs at INFO only if at least one route took
more than 20 ms during a ten-second window. No environment variable or live
configuration change is needed. Look for `[VisualMap Perf]` in the OpenRGB log.

Each report contains frame count/mean/maximum duration and, for at most 28
controllers, the display name and `SetColor` call count/total/mean/maximum.
Names are limited to 96 bytes and control characters are removed. No colors,
frames, serials or network addresses are logged. Each report resets counters;
reports are limited to one per ten seconds per active map. The implementation
uses two monotonic clock reads per legacy LED sample and bounded metadata.

`SetColor` timing includes the native call and any wait for its access mutex,
but excludes pixel sampling. A slow complete route with fast `SetColor` calls
therefore points elsewhere; it must not be attributed to a particular driver
without the per-controller evidence. Native image sinks keep their existing
nonblocking submit path. The timing diagnostic changes no colors or ordering.

`tests/room-image-routing` checks one color frame across two zones and six
segments, repeated index order, cached topology rejection/rebind, all refusal
statuses without direct fallback and the explicit legacy fallback. It also
checks interval gating, quiet fast frames, exact
aggregates, bounded storage and safe single-line names, alongside the existing
real wrapper/image routing tests. No physical device is used by these tests.
