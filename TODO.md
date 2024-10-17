# TODO

## General

- [x] draw ground truth
- [x] binary threshold canny
- [x] figure out best canny parameters for min/max threshold

## Shape Analysis

- [x] run CCA **region.cpp line ~60**
- [x] get shape analysis **recognition.cpp line 501**
- [x] do hulls instead of min bound rect
- [x] investigate why some crossings are filled in and others have just the boundary - too many median smoothing iterations
- [x] investigate why some crossings have small noise around them - general noise, hard to work around
- [x] closing after otsu
- [x] filter by hull area
- [ ] ~~filter by hull edge size (or ratio of width-height)~~ don't think it's important as we already filter by area
- [ ] filter by rectangularity of hulls (`min_bound_rect_area/object_area`)

## Other

- [ ] try gaussian smoothing instead of median?
- [x] figure out best `NUM_MEDIAN_BLUR_ITERATIONS` - 1 should be best
