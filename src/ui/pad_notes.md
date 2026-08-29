Issue 2: The 4096-column pad overwrites the right-side border
The pad plane created in ui_surface_addpad is 4096 columns wide and is a child of the parent plane. Notcurses does not automatically clip child planes to their parent's visible area — child planes can extend outside the parent and render over anything beneath them, including the border box.

The correct solution is to use NCPLANE_OPTION_MARGINALIZED when the pad should be bounded, but that is for the standard plane only. For an ordinary child plane acting as a scrollable pad, you need to either:

Option A: Use ncplane_set_scrolling + bound the plane to the inner width

Don't create the pad at 4096 columns. Instead, create it at the interior width of the box, and use

ncplane_set_scrolling(plane, true)

for vertical scrolling.

For horizontal scrolling, manage an x-offset yourself and re-render the visible slice. This is the simpler, correct approach for a viewer.


Option B: Use a "viewport" plane + move the pad behind it

Create a narrow "viewport" plane of the exact inner width, and a wide "content" plane behind it. 
Use ncplane_move_above/ncplane_below so the viewport plane sits on top and clips the content.

But notcurses composites by z-order only, not by clipping regions — so this won't work like ncurses pads.

Option C: Use ncplane_resize with keepy/keepx/keeplen* to present only a column window
When you scroll horizontally, call 

ncplane_resize(pad, 0, col_offset, visible_rows, visible_cols, 0, 0, visible_rows, visible_cols)

to slide the "keep" region. This is the notcurses-idiomatic way to implement a scrollable pad — you resize the plane to anchor to a different portion of its content.

The most practical fix for a text viewer is Option A with a fixed-width plane:

In ui_surface_addpad, instead of passing 4096 as cols, pass the inner width of the containing box.

If horizontal scrolling is needed, track a col_offset and re-fill the plane's content starting at that offset each time the view scrolls horizontally.

The plane itself stays at the display width and never bleeds past the border.

If you do need the 4096-wide plane for some reason, you must create it as a child of the inner content plane (not the box plane), and position the box border as a sibling plane that is above the pad in z-order (

ncplane_move_above(border_plane, pad_plane))
so the border composites on top and hides the overflow columns.
