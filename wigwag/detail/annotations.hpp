#ifndef WIGWAG_DETAIL_ANNOTATIONS_HPP
#define WIGWAG_DETAIL_ANNOTATIONS_HPP

#if WIGWAG_USE_HELGRIND_ANNOTATIONS
#	include <valgrind/helgrind.h>
#	define WIGWAG_ANNOTATE_HAPPENS_BEFORE(Marker_) do { ANNOTATE_HAPPENS_BEFORE(Marker_); } while (0)
#	define WIGWAG_ANNOTATE_HAPPENS_AFTER(Marker_) do { ANNOTATE_HAPPENS_AFTER(Marker_); } while (0)
#	define WIGWAG_ANNOTATE_RELEASE(Marker_) do { ANNOTATE_HAPPENS_BEFORE_FORGET_ALL(Marker_); } while (0)
#else
#	define WIGWAG_ANNOTATE_HAPPENS_BEFORE(Marker_) do { } while (0)
#	define WIGWAG_ANNOTATE_HAPPENS_AFTER(Marker_) do { } while (0)
#	define WIGWAG_ANNOTATE_RELEASE(Marker_) do { } while (0)
#endif

#endif
