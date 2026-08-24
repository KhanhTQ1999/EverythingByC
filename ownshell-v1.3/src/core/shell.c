#include "../../inc/types.h"
#include "../../inc/core/shell.h"

#define USR_INPUT_MAX 1024

typedef void (*shell_state_func_t)(struct shell_ctx *ctx);

static void shell_state_init(struct shell_ctx *ctx);
static void shell_state_interactive(struct shell_ctx *ctx);
static void shell_state_lexing(struct shell_ctx *ctx);
static void shell_state_parsing(struct shell_ctx *ctx);
static void shell_state_executing(struct shell_ctx *ctx);
static void shell_state_cleanup(struct shell_ctx *ctx);
static void shell_state_error(struct shell_ctx *ctx);
static void shell_state_exit(struct shell_ctx *ctx);

static const shell_state_func_t state_tbl[SHELL_STATE_COUNT] = {
    [SHELL_STATE_INIT] = shell_state_init,
    [SHELL_STATE_INTERACTIVE] = shell_state_interactive,
    [SHELL_STATE_LEXING] = shell_state_lexing,
    [SHELL_STATE_PARSING] = shell_state_parsing,
    [SHELL_STATE_EXECUTING] = shell_state_executing,
    [SHELL_STATE_CLEANUP] = shell_state_cleanup,
    [SHELL_STATE_ERROR] = shell_state_error,
    [SHELL_STATE_EXIT] = shell_state_exit
};

static void shell_state_transition(struct shell_ctx *ctx, enum shell_state next_state) {
   if(ctx == NULL || next_state >= SHELL_STATE_COUNT) return;

    ctx->prev_state = ctx->current_state;
    ctx->current_state = next_state;
}

static void shell_state_init(struct shell_ctx *ctx) {
    shell_state_transition(ctx, SHELL_STATE_INTERACTIVE);
}

static void shell_state_interactive(struct shell_ctx *ctx) {
    if(ctx == NULL) { return; }
    if(!ctx->is_running) { return; }

    printf("> ");
    fflush(stdout);

    char input[USR_INPUT_MAX];
    if(fgets(input, sizeof(input), stdin) == NULL) {
        shell_state_transition(ctx, SHELL_STATE_EXIT);
        return;
    }

    if(strlen(input) >= USR_INPUT_MAX - 1) {
        printf("Input too long. Maximum length is %d characters.\n", USR_INPUT_MAX - 1);
        return;
    }

    cstr_append(&ctx->input_buf, input);

    // Remove newline character
    size_t len = strlen(ctx->input_buf.data);
    if(len > 0 && ctx->input_buf.data[len - 1] == '\n') {
        ctx->input_buf.data[len - 1] = '\0';
        ctx->input_buf.len--;
    }

    shell_state_transition(ctx, SHELL_STATE_LEXING);
}

static void shell_state_lexing(struct shell_ctx *ctx) {
    if(ctx == NULL) { return; }

    if(ctx->input_buf.len == 0) {
        shell_state_transition(ctx, SHELL_STATE_INTERACTIVE);
        return;
    }

    i32 ret = lex_tokanize(&ctx->input_buf, &ctx->tok_list);
    if(ret != 0) {
        shell_state_transition(ctx, SHELL_STATE_ERROR);
        return;
    }
    shell_state_transition(ctx, SHELL_STATE_PARSING);
}

static void shell_state_parsing(struct shell_ctx *ctx) {
    if(ctx == NULL) { return; }

    i32 ret = parse_tokens(&ctx->tok_list, &ctx->ast);
    if(ret != 0) {
        shell_state_transition(ctx, SHELL_STATE_ERROR);
        return;
    }
    shell_state_transition(ctx, SHELL_STATE_EXECUTING);
}

static void shell_state_executing(struct shell_ctx *ctx) {
    if(ctx == NULL) { return; }

    i32 ret = execute_ast(&ctx->ast);
    if(ret != 0) {
        shell_state_transition(ctx, SHELL_STATE_ERROR);
        return;
    }
    shell_state_transition(ctx, SHELL_STATE_CLEANUP);
}

static void shell_state_cleanup(struct shell_ctx *ctx) {
    if(ctx == NULL) { return; }
    
    vec_clear(&ctx->tok_list);
    cstr_clear(&ctx->input_buf);
    ast_clear(&ctx->ast);

    shell_state_transition(ctx, SHELL_STATE_INTERACTIVE);
}

static void shell_state_error(struct shell_ctx *ctx) {
    printf("An error occurred in state: %d\n", ctx->current_state);
    shell_state_transition(ctx, SHELL_STATE_INTERACTIVE);
}

static void shell_state_exit(struct shell_ctx *ctx) {
    // Placeholder for exit state logic
    ctx->is_running = 0;
}

void shell_context_init(struct shell_ctx *self) {
    self->current_state = SHELL_STATE_INIT;
    self->prev_state = SHELL_STATE_INIT;
    self->tok_list = (struct Vec){0};
    self->input_buf = (struct Cstr){0};
    self->ast = (Ast){0};
    self->is_running = 1;
    self->exit_code = 0;
}

void shell_context_free(struct shell_ctx *self) {
    vec_free(&self->tok_list);
    cstr_free(&self->input_buf);
    ast_free(&self->ast);
}

i32 shell_run(struct shell_ctx *ctx) {
    while (ctx->is_running && ctx->current_state < SHELL_STATE_COUNT) {
        shell_state_func_t func = state_tbl[ctx->current_state];
        if(func == NULL) {
            ctx->is_running = 0;
            ctx->exit_code = 1;
            break;
        }
        func(ctx);
    }
    return ctx->exit_code;
}

