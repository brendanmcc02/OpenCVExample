# TODO

## General

- [x] draw ground truth
- [x] binary threshold canny
- [x] figure out best canny parameters for min/max threshold

## Shape Analysis

- [x] run CCA **region.cpp line ~60**
- [ ] figure out best `NUM_MEDIAN_BLUR_ITERATIONS`
- [ ] investigate why some crossings have small noise around them
- [ ] investigate why some crossings are filled in and others have just the boundary
- [x] get shape analysis **recognition.cpp line 501**
- [x] filter by size: `contours[contour_number].size()`

## Other

- [ ] try gaussian smoothing instead of median?
