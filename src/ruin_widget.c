#include "stdio.h"
#include "ruin_core.h"
#include "ruin_widget.h"

internal bool hovered(ruin_Rect rectangle, ruin_Vec2 mouse_position) {
    return mouse_position.x >= rectangle.x && mouse_position.x <= rectangle.w && mouse_position.y >= rectangle.y && mouse_position.y <= rectangle.h;
};

void ruin_RowBegin(ruin_Context* ctx, const char* label) {
    ruin_WidgetID id = hash_string(ctx, label);
    ruin_Widget* row_warpper = get_widget_by_id(ctx, id);
    if (row_warpper == NULL) {
        row_warpper = ruin_create_widget_ex(ctx, label, id, RUIN_WIDGETFLAGS_NO_FLAGS);
        // row_warpper->background_color = (ruin_Color) { .r = 255, .g = 255, .b = 0, .a = 255};
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

    ruin_PushWidgetNArray(ruin_WidgetStack__GetTop(ctx->parent_stack), row_warpper);
    ruin_WidgetStack__Push(ctx->parent_stack, row_warpper);
};
void ruin_RowEnd(ruin_Context* ctx) { 
    ruin_WidgetStack__Pop(ctx->parent_stack);
};

B8 ruin_Label(ruin_Context* ctx, const char* label) {
    ruin_WidgetID id = hash_string(ctx, label);
    ruin_Widget* label_widget = get_widget_by_id(ctx, id);
    if (label_widget == NULL) {
        label_widget = ruin_create_widget_ex(ctx, label, id, RUIN_WIDGETFLAGS_DRAW_TEXT|RUIN_WIDGETFLAGS_DRAW_BACKGROUND);
    };
    ruin_PushWidgetNArray(ruin_WidgetStack__GetTop(ctx->parent_stack), label_widget);

    return false;
};

void ruin_SpacerFillX(ruin_Context* ctx) {
    ruin_WidgetID id = hash_string(ctx, "");
    ruin_Widget* spacer = get_widget_by_id(ctx, id);
    if (spacer == NULL) {
        spacer = (ruin_Widget*)arena_alloc(&ctx->temp_arena, sizeof(ruin_Widget));
        spacer->size[RUIN_AXISX] = (ruin_Size) { .kind=RUIN_SIZEKIND_GROW, .value = 0, .strictness = 1 };
        spacer->size[RUIN_AXISY] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = 0, .strictness = 1 };
    };
    ruin_PushWidgetNArray(ruin_WidgetStack__GetTop(ctx->parent_stack), spacer);
};

void ruin_SpacerFixedX(ruin_Context* ctx, F32 space) {
    ruin_WidgetID id = hash_string(ctx, "");
    ruin_Widget* spacer = get_widget_by_id(ctx, id);
    if (spacer == NULL) {
        spacer = (ruin_Widget*)arena_alloc(&ctx->temp_arena, sizeof(ruin_Widget));
        spacer->size[RUIN_AXISX] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = space, .strictness = 1 };
        spacer->size[RUIN_AXISY] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = 1, .strictness = 1 };
    };
    ruin_PushWidgetNArray(ruin_WidgetStack__GetTop(ctx->parent_stack), spacer);
};

void ruin_SpacerFillY(ruin_Context* ctx) {
    ruin_WidgetID id = hash_string(ctx, "");
    ruin_Widget* spacer = get_widget_by_id(ctx, id);
    if (spacer == NULL) {
        spacer = (ruin_Widget*)arena_alloc(&ctx->temp_arena, sizeof(ruin_Widget));
        spacer->size[RUIN_AXISX] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = 0, .strictness = 1 };
        spacer->size[RUIN_AXISY] = (ruin_Size) { .kind=RUIN_SIZEKIND_GROW, .value = 0, .strictness = 1 };
    };
    ruin_PushWidgetNArray(ruin_WidgetStack__GetTop(ctx->parent_stack), spacer);
};

void ruin_SpacerFixedY(ruin_Context* ctx, F32 space) {
    ruin_WidgetID id = hash_string(ctx, "");
    ruin_Widget* spacer = get_widget_by_id(ctx, id);
    if (spacer == NULL) {
        spacer = (ruin_Widget*)arena_alloc(&ctx->temp_arena, sizeof(ruin_Widget));
        spacer->size[RUIN_AXISX] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = 1, .strictness = 1 };
        spacer->size[RUIN_AXISY] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = space, .strictness = 1 };
    };
    ruin_PushWidgetNArray(ruin_WidgetStack__GetTop(ctx->parent_stack), spacer);
};


B8 ruin_Button(ruin_Context* ctx, const char* label) {
    ruin_WidgetID id = hash_string(ctx, label);
    ruin_Widget* button_widget = get_widget_by_id(ctx, id);
    if (button_widget == NULL) {
        button_widget = ruin_create_widget_ex(ctx, label, id, RUIN_WIDGETFLAGS_DRAW_TEXT|RUIN_WIDGETFLAGS_DRAW_BACKGROUND|RUIN_WIDGETFLAGS_DRAW_BORDER);
        button_widget->background_color = (ruin_Color) {  .r = 255, .g = 0, .b = 0, .a = 255 };
        button_widget->padding = (ruin_RectSide) { .left = 16, .right = 16, .top = 4, .bottom = 4, };
    }
    ruin_PushWidgetNArray(ruin_WidgetStack__GetTop(ctx->parent_stack), button_widget);

    ruin_Rect rect = button_widget->draw_coords.bbox;
    ruin_Vec2 mouse_position = ctx->mouse_position;
    bool is_hovered = hovered(rect, mouse_position);
    if (!is_hovered) {
        button_widget->background_color = (ruin_Color) {  .r = 248, .g = 248, .b = 248, .a = 255 };
    } else {
        button_widget->background_color = (ruin_Color) {  .r = 238, .g = 238, .b = 238, .a = 255 };
    };

    return (is_hovered&ctx->mouse_action);
};

