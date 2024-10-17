# TODO

## General

- [x] draw ground truth
- [x] binary threshold canny
- [x] figure out best canny parameters for min/max threshold

## Shape Analysis

- [x] run CCA **region.cpp line ~60**
- [x] get shape analysis **recognition.cpp line 501**
- [x] do hulls instead of min bound rect
- [x] filter by hull area
- [ ] filter by hull edge size (or ratio of width-height)
- [ ] filter by rectangularity of hulls (`min_bound_rect_area/object_area`)

## Other

- [ ] try gaussian smoothing instead of median?
- [ ] figure out best `NUM_MEDIAN_BLUR_ITERATIONS`
- [ ] investigate why some crossings have small noise around them
- [ ] ~~investigate why some crossings are filled in and others have just the boundary~~ shouldn't matter too much
