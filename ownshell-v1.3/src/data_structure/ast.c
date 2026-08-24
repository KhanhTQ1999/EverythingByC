#include "../../inc/data_structure/ast.h"

#include <stddef.h>

void ast_init(struct ast *self) {
    if (self == NULL) {
        return;
    }

    self->left = NULL;
    self->right = NULL;
}

void ast_clear(struct ast *self) {
    if (self == NULL) {
        return;
    }

    if (self->left != NULL) {
        ast_free(self->left);
        self->left = NULL;
    }

    if (self->right != NULL) {
        ast_free(self->right);
        self->right = NULL;
    }
}

void ast_free(struct ast *self) {
    if (self == NULL) {
        return;
    }

    ast_clear(self);
}
