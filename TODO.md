# TODO

## General

- [x] draw ground truth
- [x] binary threshold canny
- [x] figure out best canny parameters for min/max threshold
- [x] run CCA **region.cpp line ~60**
- [x] figure out best `NUM_MEDIAN_BLUR_ITERATIONS` - 1 should be best
- [x] get shape analysis **recognition.cpp line 501**
- [x] do hulls instead of min bound rect
- [x] investigate why some crossings are filled in and others have just the boundary - too many median smoothing iterations
- [x] investigate why some crossings have small noise around them - general noise, hard to work around
- [x] closing after otsu
- [x] filter by hull area
- [x] filter by rectangularity of hulls (`min_bound_rect_area/object_area`)
- [x] overlay hulls on real image, check for white-ish color
- [x] fix hulls being different shape on filtered
- [x] filter isolated hulls
- [x] deal with hulls that overlap
- [x] longest linear sequence
- [x] for linear sequences that tie for max, pick the straightest one
- [x] bullet-proof against vertical crossings
- [x] draw predicted box

## Other

- [x] change c iter variable to i

- [ ] error handling for empty/not found linear subsequences, what if there's only 3 and they don't align
- [ ] instead of picking the previous hull when there's overlap, choose the region that is whiter
- [ ] hough transform **results aren't very convincing**
- [ ] change naming to lowerCamelCase
- [ ] make consts more forgiving/looser? don't want to overfit/over-rely on 10 training images
- [ ] ~~try gaussian smoothing instead of median? **don't think it would be good because median smoothing preserves edges~~
- [ ] ~~filter by hull edge size (or ratio of width-height)~~ don't think it's important as we already filter by area
- [ ] make the file ken/windows friendly
