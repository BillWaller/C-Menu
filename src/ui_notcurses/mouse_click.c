struct ncplane *process_mouse_click(struct ncplane *plane,
                                    const struct ncinput *mouse_event,
                                    int *y, int *x) {
    if (!mouse_event || !plane)
        return NULL;
    int c_y = mouse_event->y;
    int c_x = mouse_event->x;
    struct ncplane *cur = ncpile_top(plane);

    while (cur != NULL) {
        int p_y, p_x;
        int dim_y, dim_x;
        unsigned int d_y, d_x;
        ncp_yx(cur, &p_y, &p_x);
        ncplane_dim_yx(cur, &d_y, &d_x);
        dim_y = (int)d_y;
        dim_x = (int)d_x;
        if (c_y >= p_y && c_y < (p_y + dim_y) &&
            c_x >= p_x && c_x < (p_x + dim_x)) {
            if (y)
                *y = c_y - p_y;
            if (x)
                *x = c_x - p_x;
            return cur;
        }
        cur = ncplane_below(cur);
    }

    return NULL;
}
