typedef struct {
    ncplane *plane;
    ncplane *parent;
    ncplane *child;
    ncplane *prev_sibling;
    ncplane *next_sibling;
} NCPlaneNode;

typedef struct {
    NCPlaneNode *root;
    NCPlaneNode *current;
} NCPlaneTree;
