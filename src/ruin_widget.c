#include "stdio.h"
#include "ruin_core.h"
#include "ruin_widget.h"

internal bool hovered(ruin_Rect rectangle, ruin_Vec2 mouse_position) {
    return mouse_position.x >= rectangle.x && mouse_position.x <= rectangle.w && mouse_position.y >= rectangle.y && mouse_position.y <= rectangle.h;
};

void ruin_RowBegin(ruin_Context* ctx, const char* label) {
    ruin_Id id = hash_string(ctx, label);
    ruin_Widget* row_warpper = get_widget_by_id(ctx, id);
    if (row_warpper == NULL) {
        row_warpper = ruin_create_widget_ex(ctx, label, id, RUIN_WIDGETFLAGS_NO_FLAGS);
        row_warpper->child_layout_axis = RUIN_AXISX;
        row_warpper->size[RUIN_AXISY] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_CHILDRENSUM,
            .value = 0,
            .strictness = 1,
        };
        row_warpper->size[RUIN_AXISX] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_PARENTPERCENTAGE,
            .value = 1,
            .strictness = 1,
        };
    };

    push_widget_narry(ruin_WidgetStack__GetTop(ctx->parent_stack), row_warpper);
    ruin_WidgetStack__Push(ctx->parent_stack, row_warpper);
};
void ruin_RowEnd(ruin_Context* ctx) { 
    ruin_WidgetStack__Pop(ctx->parent_stack);
};

B8 ruin_Label(ruin_Context* ctx, const char* label) {
    ruin_Id id = hash_string(ctx, label);
    ruin_Widget* label_widget = get_widget_by_id(ctx, id);
    if (label_widget == NULL) {
        // push_color_stack(&ctx->background_color_stack, (ruin_Color) {.r=255, .b=0, .g=0, .a=255});
        label_widget = ruin_create_widget_ex(ctx, label, id, RUIN_WIDGETFLAGS_DRAW_TEXT|RUIN_WIDGETFLAGS_DRAW_BACKGROUND);
        // pop_color_stack(&ctx->background_color_stack);
    };
    push_widget_narry(ruin_WidgetStack__GetTop(ctx->parent_stack), label_widget);

    return false;
};

void ruin_SpacerFillX(ruin_Context* ctx) {
    ruin_Id id = hash_string(ctx, "");
    ruin_Widget* spacer = get_widget_by_id(ctx, id);
    if (spacer == NULL) {
        spacer = ruin_create_widget_ex(ctx, "", id, RUIN_WIDGETFLAGS_NO_FLAGS);
        spacer->size[RUIN_AXISX] = (ruin_Size) { .kind=RUIN_SIZEKIND_GROW, .value = 0, .strictness = 1 };
        spacer->size[RUIN_AXISY] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = 0, .strictness = 1 };
    };
    push_widget_narry(ruin_WidgetStack__GetTop(ctx->parent_stack), spacer);
};

void ruin_SpacerFixedX(ruin_Context* ctx, F32 space) {
    ruin_Id id = hash_string(ctx, "");
    ruin_Widget* spacer = get_widget_by_id(ctx, id);
    if (spacer == NULL) {
        spacer = ruin_create_widget_ex(ctx, "", id, RUIN_WIDGETFLAGS_NO_FLAGS);
        spacer->size[RUIN_AXISX] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = space, .strictness = 1 };
        spacer->size[RUIN_AXISY] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = 1, .strictness = 1 };
    };
    push_widget_narry(ruin_WidgetStack__GetTop(ctx->parent_stack), spacer);
};

void ruin_SpacerFillY(ruin_Context* ctx) {
    ruin_Id id = hash_string(ctx, "");
    ruin_Widget* spacer = get_widget_by_id(ctx, id);
    if (spacer == NULL) {
        spacer = ruin_create_widget_ex(ctx, "", id, RUIN_WIDGETFLAGS_NO_FLAGS);
        spacer->size[RUIN_AXISX] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = 0, .strictness = 1 };
        spacer->size[RUIN_AXISY] = (ruin_Size) { .kind=RUIN_SIZEKIND_GROW, .value = 0, .strictness = 1 };
    };
    push_widget_narry(ruin_WidgetStack__GetTop(ctx->parent_stack), spacer);
};

void ruin_SpacerFixedY(ruin_Context* ctx, F32 space) {
    ruin_Id id = hash_string(ctx, "");
    ruin_Widget* spacer = get_widget_by_id(ctx, id);
    if (spacer == NULL) {
        spacer = ruin_create_widget_ex(ctx, "", id, RUIN_WIDGETFLAGS_NO_FLAGS);
        spacer->size[RUIN_AXISX] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = 1, .strictness = 1 };
        spacer->size[RUIN_AXISY] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = space, .strictness = 1 };
    };
    push_widget_narry(ruin_WidgetStack__GetTop(ctx->parent_stack), spacer);
};


B8 ruin_Button(ruin_Context* ctx, const char* label) {
    ruin_Id id = hash_string(ctx, label);
    ruin_Widget* button_widget = get_widget_by_id(ctx, id);
    if (button_widget == NULL) {
        push_rectsides_stack(&ctx->padding_stack, (ruin_RectSide) { .left = 16, .right = 16, .top = 8, .bottom = 8, });
        button_widget = ruin_create_widget_ex(ctx, label, id, RUIN_WIDGETFLAGS_DRAW_TEXT|RUIN_WIDGETFLAGS_DRAW_BACKGROUND|RUIN_WIDGETFLAGS_DRAW_BORDER);
        pop_rectsides_stack(&ctx->padding_stack);
    }

    ruin_Rect rect = button_widget->draw_coords.bbox;
    ruin_Vec2 mouse_position = ctx->mouse_position;
    if (hovered(rect, mouse_position)) {
        button_widget->background_color = *get_color_stack_top(&ctx->active_color_stack);
    } else {
        button_widget->background_color = *get_color_stack_top(&ctx->background_color_stack);
    };

    push_widget_narry(ruin_WidgetStack__GetTop(ctx->parent_stack), button_widget);
    return false;
};

