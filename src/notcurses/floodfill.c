// Flood fill your box view with your background properties
// Parameters: plane, fallback glyph, stylemask, channel mask
ncplane_set_base(view->box.plane, " ", 0, CC_BOX_CHANNELS);

// Flood fill your line number gutter
ncplane_set_base(view->lnno.plane, " ", 0, CC_LN_CHANNELS);

// Flood fill your main text viewport area
ncplane_set_base(view->content.plane, " ", 0, CC_NT_CHANNELS);
