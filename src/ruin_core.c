#include <ft2build.h>
#include <stdio.h>
#include FT_FREETYPE_H

#include "ruin_core.h"

internal F32 ruin_GetWidth(ruin_Context* ctx, String8 string, ruin_FontID font_id) {
    F32 width = 0;
    for (int i = 0; i < string.len; ++i)
        width += (ruin_FontInfoArray__Get(&ctx->fonts, font_id)->bitmap[string.data[i]].advance >> 6); 

    return width;
};

void ruin_SetFontCount(ruin_Context *ctx, size_t number_of_font_sizes) {
    const U32 ARENA_SIZE = 0;
    Arena arena = {0};
    unsigned char* buffer = (unsigned char*) malloc(ARENA_SIZE);
    arena_init(&arena, buffer, ARENA_SIZE);

    // USED TO STORE GLYPH / BITMAP DATA, WHICH IS CLEARED
    ctx->font_bitmap_arena = arena;
    ctx->fonts = ruin_FontInfoArray__Init(&ctx->arena, number_of_font_sizes);
};

ruin_FontID ruin_LoadFont(ruin_Context* ctx, const char *path, const char *name, U32 font_size) { 
    String8 str_name = str_from_cstr(name, &ctx->arena);

    ruin_FontInfo font;
    font.font_name = str_name;
    font.font_size = font_size;

    int error;
    FT_Library ft;
    FT_Face face;
    error = FT_Init_FreeType (&ft);
    if (error) {fprintf(stderr, "Its an Error");};

    error = FT_New_Face(ft, path, 0, &face);
    if (error) {fprintf(stderr, "Its an Error");};

    error = FT_Set_Pixel_Sizes(face,  0, font_size);
    if (error) {fprintf(stderr, "Its an Error");};

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "ERROR::FREETYTPE: Failed to load Glyph");
            continue;
        };

        size_t total_pixels = face->glyph->bitmap.width * face->glyph->bitmap.rows;
        // U8* gray_alpha_data = (U8*)arena_alloc(&ctx->font_bitmap_arena, total_pixels * 2); 
        U8* gray_alpha_data = (U8*)malloc(total_pixels * 2); 
        for (size_t i = 0; i < total_pixels; i++) {
            gray_alpha_data[i * 2 + 0] = face->glyph->bitmap.buffer[i];
            gray_alpha_data[i * 2 + 1] = (face->glyph->bitmap.buffer[i] < 0) ? 0 : face->glyph->bitmap.buffer[i];
        };

        font.bitmap[c].width = face->glyph->bitmap.width;
        font.bitmap[c].height = face->size->metrics.height;
        font.bitmap[c].rows = face->glyph->bitmap.rows;
        font.bitmap[c].bearingX = face->glyph->bitmap_left;
        font.bitmap[c].bearingY = face->glyph->bitmap_top;
        font.bitmap[c].advance = face->glyph->advance.x;
        font.bitmap[c].buffer = gray_alpha_data;
        font.bitmap[c].pitch = face->glyph->bitmap.pitch;
    };

    ruin_FontInfoArray__Push(&ctx->fonts, font);

    if (ruin_FontIDStack__IsEmpty(&ctx->font_stack)) {
        ruin_FontIDStack__Push(&ctx->font_stack, ctx->fonts.index - 1);
    };
 
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return (ctx->fonts.index - 1);
};

void ruin_PushWidgetNArray(ruin_Widget* root_widget, ruin_Widget* new_widget) {
    if (root_widget->first_child == NULL) {
        new_widget->first_child = NULL;
        new_widget->next_sibling = NULL;
        new_widget->prev_sibling = NULL;
        new_widget->parent = root_widget;

        root_widget->first_child = new_widget;
        root_widget->last_child = new_widget;
        root_widget->child_count = 1;
    } else {
        ruin_Widget* temp = root_widget->first_child;
        while (temp->next_sibling != NULL) temp = temp->next_sibling;

        temp->next_sibling = new_widget;
        new_widget->prev_sibling = temp;
        new_widget->next_sibling = NULL;
        new_widget->parent = root_widget;
        new_widget->first_child = NULL;
        new_widget->last_child = NULL;
        root_widget->last_child = new_widget;
        root_widget->child_count++;
    };
};


ruin_Id hash_string(ruin_Context* ctx, const char* str) {
    if (str == NULL || str[0] == '\0') return RUIN_TRANSIENT_ID;

    char buf[64];
    ruin_Id parent_id = ruin_WidgetStack__IsEmpty(ctx->parent_stack) ? RUIN_TRANSIENT_ID: ruin_WidgetStack__GetTop(ctx->parent_stack)->id;
    snprintf(buf, sizeof(buf), "seed%llu_label%s", parent_id, str);

    ruin_Id hash = 5381; int c; int i = 0;
    while ((c = buf[i++]))  hash = ((hash << 5) + hash) + c;

    return hash;
};

ruin_Context* create_ruin_context() {
    const U32 ARENA_SIZE = 42000;
    Arena arena = {0};
    unsigned char* buffer = (unsigned char*) malloc(ARENA_SIZE);
    arena_init(&arena, buffer, ARENA_SIZE);


    const U32 TEMP_ARENA_SIZE = 2500;
    Arena temp_arena = {0};
    unsigned char* temp_buffer = (unsigned char*) malloc(TEMP_ARENA_SIZE);
    arena_init(&temp_arena, temp_buffer, TEMP_ARENA_SIZE);

    ruin_Context* ctx = (ruin_Context*)arena_alloc(&arena, sizeof(ruin_Context));

    MEM_ZERO(ctx, sizeof(ruin_Context));
    ctx->arena = arena;
    ctx->temp_arena = temp_arena;
    ctx->widgets = ruin_WidgetArray__Init(&ctx->arena, WIDGET_ARRAY);
    ctx->windows = ruin_WindowArray__Init(&ctx->arena, WINDOW_ARRAY);

    ctx->parent_stack = ruin_WidgetStack__Init(&ctx->arena);
    ctx->active_color_stack = ruin_ColorStack__Init(&ctx->arena, 4);
    ctx->background_color_stack = ruin_ColorStack__Init(&ctx->arena, 4);
    ctx->foreground_color_stack = ruin_ColorStack__Init(&ctx->arena, 4);
    ctx->hover_color_stack = ruin_ColorStack__Init(&ctx->arena, 4);

    ctx->font_stack = ruin_FontIDStack__Init(&ctx->arena, 4);
    ctx->child_direction_stack = ruin_AxisStack__Init(&ctx->arena, 4);
    ctx->padding_stack = ruin_RectSideStack__Init(&ctx->arena, 4);

    // SETTING UP DEFAULT STYLING
    ruin_ColorStack__Push(&ctx->background_color_stack, (ruin_Color) {.r=50, .g=50, .b=50, .a=0});
    ruin_ColorStack__Push(&ctx->foreground_color_stack, (ruin_Color) {.r=50, .g=50, .b=50, .a=0});
    ruin_ColorStack__Push(&ctx->hover_color_stack, (ruin_Color) {.r=250, .g=50, .b=50, .a=0});
    ruin_ColorStack__Push(&ctx->active_color_stack, (ruin_Color) {.r=250, .g=50, .b=50, .a=0});

    ruin_AxisStack__Push(&ctx->child_direction_stack, RUIN_AXISX);
    ruin_RectSideStack__Push(&ctx->padding_stack, (ruin_RectSide) { .top = 0, .bottom = 0, .left = 0, .right = 0} );

    return ctx;
};

ruin_Window* get_window_by_id(ruin_Context* ctx, ruin_Id id) {
    for (size_t i = 0; i < ctx->windows.index; ++i) {
        if (ruin_WindowArray__Get(&ctx->windows, i)->id == id)
            return ruin_WindowArray__Get(&ctx->windows, i);
    };

    return NULL;
};

ruin_Widget* get_widget_by_id(ruin_Context* ctx, ruin_Id id) {
    for (size_t i = 0; i < ctx->widgets.index; ++i) {
        if (ruin_WidgetArray__Get(&ctx->widgets, i)->id == id)
            return ruin_WidgetArray__Get(&ctx->widgets, i); 
    };

    return NULL;
};

internal String8 id_seperator = String8("##");
ruin_Widget* ruin_create_widget_ex(ruin_Context* ctx, const char* full_name, ruin_Id id, ruin_WidgetOptions opt) {
    ruin_Widget widget;

    widget.id = id;
    widget.flags = opt;
    widget.child_layout_axis = *ruin_AxisStack__GetTop(&ctx->child_direction_stack);
    widget.padding = *ruin_RectSideStack__GetTop(&ctx->padding_stack);

    if (opt & RUIN_WIDGETFLAGS_DRAW_TEXT) {
        widget.font = *ruin_FontIDStack__GetTop(&ctx->font_stack);
        widget.size[RUIN_AXISX] = (ruin_Size) { .kind = RUIN_SIZEKIND_TEXTCONTENT, .value = 0, .strictness = 1, };
        widget.size[RUIN_AXISY] = (ruin_Size) { .kind = RUIN_SIZEKIND_TEXTCONTENT, .value = 0, .strictness = 1, };
    };

    if (opt & RUIN_WIDGETFLAGS_DRAW_BACKGROUND) {
        widget.background_color = *ruin_ColorStack__GetTop(&ctx->background_color_stack);
        widget.foreground_color = *ruin_ColorStack__GetTop(&ctx->foreground_color_stack);
        widget.hover_color      = *ruin_ColorStack__GetTop(&ctx->hover_color_stack);
        widget.active_color     = *ruin_ColorStack__GetTop(&ctx->active_color_stack);
    } else {
        widget.background_color = (ruin_Color) {.r = 255, .g = 255, .b = 255, .a = 0 };
        widget.foreground_color = (ruin_Color) {.r = 255, .g = 255, .b = 255, .a = 0 };
        widget.hover_color      = (ruin_Color) {.r = 255, .g = 255, .b = 255, .a = 0 };
        widget.active_color     = (ruin_Color) {.r = 255, .g = 255, .b = 255, .a = 0 };
    };

    if (opt & RUIN_WIDGETFLAGS_DRAW_BORDER) {
        widget.border_color = (ruin_Color) {.r=255, .g=255, .b=255, .a=255};
    };

    // SPLITING => HASHABLE STRING and DISPLAYABLE STRING
    String8 str_name = str_from_cstr(full_name, &ctx->arena);
    String8 str_display = str_name;
    size_t ind;
    if ((ind = str_index_of(str_name, id_seperator)) != (size_t)-1) 
        str_display = str_substring(str_display, 0, ind, &ctx->arena);

    widget.display_text = str_display;
    widget.widget_name  = str_name;

    return ruin_WidgetArray__Push(&ctx->widgets, widget);
};



void ruin_BeginWindow(ruin_Context* ctx, const char* title, ruin_Rect rect, ruin_WindowFlags flags) {
    ruin_Id id = hash_string(ctx, title);
    ruin_Window* window = get_window_by_id(ctx, id);

    if (window == NULL) {
        ruin_Window temp;

        temp.id = id;
        temp.title = title;
        temp.window_rect = rect;
        temp.window_flags = flags;
        
        window = ruin_WindowArray__Push(&ctx->windows, temp);
    };
    ctx->current_window = window;
    // printf("id: %llu\n", window->id);

    char buffer[50];
    snprintf(buffer, sizeof(buffer), "root##default%s", title);

    ruin_Id root_id = hash_string(ctx, buffer);
    ruin_Widget* root = get_widget_by_id(ctx, root_id);
    if (root == NULL) {
        ruin_Widget temp;
        temp.id = root_id;
        temp.parent = NULL;
        temp.size[RUIN_AXISX] = (ruin_Size) { .kind = RUIN_SIZEKIND_PIXEL, .value = ctx->current_window->window_rect.w, .strictness=1 }; ;
        temp.size[RUIN_AXISY] = (ruin_Size) { .kind = RUIN_SIZEKIND_PIXEL, .value = ctx->current_window->window_rect.h, .strictness=1 }; ;
        temp.child_count = 0;

        temp.draw_coords.bbox.x = ctx->current_window->window_rect.x;
        temp.draw_coords.bbox.y = ctx->current_window->window_rect.y;
        temp.draw_coords.bbox.h = ctx->current_window->window_rect.y + temp.size[RUIN_AXISY].value;
        temp.draw_coords.bbox.w = ctx->current_window->window_rect.x + temp.size[RUIN_AXISX].value;

        temp.draw_coords.text_pos.x = temp.draw_coords.bbox.x;
        temp.draw_coords.text_pos.y = temp.draw_coords.bbox.y;

        temp.partially_offset.x = 0;
        temp.partially_offset.y = 0;

        temp.flags = RUIN_WIDGETFLAGS_DRAW_BORDER|RUIN_WIDGETFLAGS_DRAW_BACKGROUND;
        temp.background_color = (ruin_Color) {.r=250, .g=250, .b=250, .a=255};
        temp.child_layout_axis = RUIN_AXISY;

        temp.padding = (ruin_RectSide) { .left = 20, .right = 20, .top = 20, .bottom = 20 };
        root = ruin_WidgetArray__Push(&ctx->widgets, temp);
    };
    root->first_child = NULL;
    window->root_widget = root;
    ruin_WidgetStack__Push(ctx->parent_stack, root);
};

void ruin_EndWindow(ruin_Context* ctx) {
    ctx->current_window = NULL;
    ruin_WidgetStack__Pop(ctx->parent_stack);
};


internal void compute__raw_sizes(ruin_Context* ctx, ruin_WidgetStack* widget_stack, ruin_Widget* root) {
    ruin_WidgetStack__Push(widget_stack, root); 
    while (!ruin_WidgetStack__IsEmpty(widget_stack)) {
        ruin_Widget* current_top = ruin_WidgetStack__Pop(widget_stack);

        if (current_top->parent != NULL) {
            current_top->parent->partially_offset.x = 0;
            current_top->parent->partially_offset.y = 0;
        };

        if (current_top->size[RUIN_AXISX].kind == RUIN_SIZEKIND_PIXEL) 
            current_top->fixed_size.x = current_top->size[RUIN_AXISX].value;

        if (current_top->size[RUIN_AXISY].kind == RUIN_SIZEKIND_PIXEL) 
            current_top->fixed_size.y = current_top->size[RUIN_AXISY].value;

        if (current_top->size[RUIN_AXISX].kind == RUIN_SIZEKIND_TEXTCONTENT) 
            current_top->fixed_size.x = ruin_GetWidth(ctx, current_top->display_text, current_top->font);

        if (current_top->size[RUIN_AXISY].kind == RUIN_SIZEKIND_TEXTCONTENT) 
            current_top->fixed_size.y = (F32)ruin_FontInfoArray__Get(&ctx->fonts, current_top->font)->bitmap[0].height / 64;

        for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling)
            ruin_WidgetStack__Push(widget_stack, widget);
    };
    ruin_WidgetStack__Clear(widget_stack);
};

internal void compute__parent_depend_sizes(ruin_Context* ctx, ruin_WidgetStack* widget_stack, ruin_Widget* root) {
    ruin_WidgetStack__Push(widget_stack, root);
    while (!ruin_WidgetStack__IsEmpty(widget_stack)) {
        ruin_Widget* current_top = ruin_WidgetStack__Pop(widget_stack);

        for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling)
            ruin_WidgetStack__Push(widget_stack, widget);

        if (current_top->parent == NULL) continue;

        float x_padding = current_top->parent->padding.left + current_top->parent->padding.right;
        float y_padding = current_top->parent->padding.top  + current_top->parent->padding.bottom;

        if (current_top->size[RUIN_AXISX].kind == RUIN_SIZEKIND_PARENTPERCENTAGE) 
            current_top->fixed_size.x = (current_top->size[RUIN_AXISX].value * current_top->parent->fixed_size.x) - x_padding;

        if (current_top->size[RUIN_AXISY].kind == RUIN_SIZEKIND_PARENTPERCENTAGE)
            current_top->fixed_size.y = (current_top->size[RUIN_AXISY].value * current_top->parent->fixed_size.y) - y_padding;

    };
    ruin_WidgetStack__Clear(widget_stack);
};

internal void compute__child_depend_sizes(ruin_Context* ctx, ruin_WidgetStack* stack_1, ruin_WidgetStack* stack_2, ruin_Widget* root) {
    ruin_WidgetStack__Push(stack_1, root);
    while (!ruin_WidgetStack__IsEmpty(stack_1)) {
        ruin_Widget* current_top = ruin_WidgetStack__Pop(stack_1);
        ruin_WidgetStack__Push(stack_2, current_top);

        for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling) ruin_WidgetStack__Push(stack_1, widget);
    };

    while (!ruin_WidgetStack__IsEmpty(stack_2)) {
        ruin_Widget* curr = ruin_WidgetStack__Pop(stack_2);
        int childsum_x = 0, childsum_y = 0, childmax_x = -1, childmax_y = -1;
        for (ruin_Widget* widget = curr->last_child; widget != NULL; widget = widget->prev_sibling) {

            float x_padding = widget->padding.left + widget->padding.right;
            float y_padding = widget->padding.top + widget->padding.bottom;

            childsum_x += widget->fixed_size.x + x_padding;
            childsum_y += widget->fixed_size.y + y_padding;
            childmax_x = MAX(childmax_x, widget->fixed_size.x + x_padding);
            childmax_y = MAX(childmax_y, widget->fixed_size.y + y_padding);
        };

        // printf("name:%u child:%i\n", curr->child_count, childmax_x);

        // TODO: ::layout:: if the flow direction is Y AXIS, then height of widget must be summed instead of width;
        if (curr->size[RUIN_AXISX].kind == RUIN_SIZEKIND_CHILDRENSUM) {
            curr->fixed_size.x = childsum_x;
        };
        if (curr->size[RUIN_AXISY].kind == RUIN_SIZEKIND_CHILDRENSUM) {
            curr->fixed_size.y = childmax_y;
        }
    }

    ruin_WidgetStack__Clear(stack_1);
    ruin_WidgetStack__Clear(stack_2);
};

internal void compute__growable_sizes(ruin_Context* ctx, ruin_WidgetStack* widget_stack, ruin_Widget* root) {
    ruin_WidgetStack__Push(widget_stack, root);
    while (!ruin_WidgetStack__IsEmpty(widget_stack)) {
        ruin_Widget* current_top = ruin_WidgetStack__Pop(widget_stack);

        if (current_top->child_layout_axis == RUIN_AXISX) {
            int rem_width = 
                current_top->fixed_size.x - 
                current_top->padding.left - current_top->padding.right - ROW_SPACING;

            U8 growables = 0;

            for (ruin_Widget* widget = current_top->first_child; widget != NULL; widget = widget->next_sibling) {
                if (widget->size[RUIN_AXISX].kind == RUIN_SIZEKIND_GROW) growables++;
                rem_width -= widget->fixed_size.x + widget->padding.right + widget->padding.left + ROW_SPACING;
            };

            while (rem_width > 0) {
                for (ruin_Widget* widget = current_top->first_child; widget != NULL; widget = widget->next_sibling) {
                    if (widget->size[RUIN_AXISX].kind == RUIN_SIZEKIND_GROW) {
                        widget->fixed_size.x = rem_width / (F32)growables;
                    };
                };
                rem_width = 0;
            };



            // TODO: GROWABLE DOESN"T WORK VERTICALLy RN.

        } else {
        };


        for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling)
            ruin_WidgetStack__Push(widget_stack, widget);
    };

    ruin_WidgetStack__Clear(widget_stack);
};

internal void compute__draw_coordinates(ruin_Context* ctx, ruin_WidgetStack* widget_stack, ruin_Widget* root) {

    ruin_WidgetStack__Push(widget_stack, root);
    while (!ruin_WidgetStack__IsEmpty(widget_stack)) {
        ruin_Widget* current_top = ruin_WidgetStack__Pop(widget_stack);

        for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling) 
            ruin_WidgetStack__Push(widget_stack, widget);

        if (current_top->parent == NULL) continue;

        current_top->draw_coords.bbox.x = 
            current_top->parent->padding.left
            + current_top->parent->draw_coords.bbox.x 
            + current_top->parent->partially_offset.x;

        current_top->draw_coords.bbox.y = 
            current_top->parent->padding.top
            + current_top->parent->draw_coords.bbox.y 
            + current_top->parent->partially_offset.y;

        current_top->draw_coords.bbox.w = 
            current_top->padding.left 
            + current_top->parent->padding.right
            + current_top->padding.right 
            + current_top->fixed_size.x 
            + current_top->parent->draw_coords.bbox.x 
            + current_top->parent->partially_offset.x;

        current_top->draw_coords.bbox.h = 
            current_top->padding.top 
            + current_top->parent->padding.bottom
            + current_top->padding.bottom
            + current_top->fixed_size.y 
            + current_top->parent->draw_coords.bbox.y 
            + current_top->parent->partially_offset.y;

        current_top->draw_coords.text_pos.x = current_top->draw_coords.bbox.x + current_top->padding.left;
        current_top->draw_coords.text_pos.y = current_top->draw_coords.bbox.y + current_top->padding.top;

        if (current_top->parent->child_layout_axis == RUIN_AXISX) {
            current_top->parent->partially_offset.x += current_top->draw_coords.bbox.w - current_top->draw_coords.bbox.x + ROW_SPACING;
        } else {
            current_top->parent->partially_offset.y += current_top->draw_coords.bbox.h - current_top->draw_coords.bbox.y + ROW_SPACING;
        };

        ruin_Rect rect = current_top->draw_coords.bbox;
    };

    ruin_WidgetStack__Clear(widget_stack);
};

internal void generate__draw_calls(ruin_Context* ctx, ruin_WidgetStack* widget_stack, ruin_Widget* root) {
    ruin_WidgetStack__Push(widget_stack, root);
    while (!ruin_WidgetStack__IsEmpty(widget_stack)) {
        ruin_Widget* current_top = ruin_WidgetStack__Pop(widget_stack);


        ruin_Rect rect = current_top->draw_coords.bbox;
        ruin_Vec2 text_pos = current_top->draw_coords.text_pos;
        // DO YOUR STUFF
        // printf("ABOUT TO PUSH %s\t => x:%f, y:%f, w:%f, h:%f dir:%i fw:%f cr:%i\n", current_top->display_text.data, rect.x, rect.y, rect.w, rect.h, current_top->child_layout_axis, current_top->fixed_size.x, current_top->background_color.r);
        // printf("color for %s: r:%u g:%u b:%u a:%u\n", current_top->display_text.data, current_top->background_color.r, current_top->background_color.g, current_top->background_color.b, current_top->background_color.a);

        ctx->draw_queue.items[ctx->draw_queue.index++] = (ruin_DrawCall) {
            .type = RUIN_DRAWTYPE_RECT,
            .draw_info_union = {
                .draw_rect = {
                    .rect = rect,
                    .color = current_top->background_color,
                    .border_width = (current_top->flags & RUIN_WIDGETFLAGS_DRAW_BORDER) ? (U8)1 : (U8)0,
                }
            }
        };
        if (current_top->flags & RUIN_WIDGETFLAGS_DRAW_TEXT) {
            ctx->draw_queue.items[ctx->draw_queue.index++] = (ruin_DrawCall) {
                .type = RUIN_DRAWTYPE_TEXT,
                .draw_info_union = {
                    .draw_text = {
                        .text = current_top->display_text.data,
                        .pos = {.x = text_pos.x, .y = text_pos.y},
                        .font_id = current_top->font
                    }
                }
            };
        }

        for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling) { 
            ruin_WidgetStack__Push(widget_stack, widget);
        }
    };

    ruin_WidgetStack__Clear(widget_stack);
};

internal bool hovered_window(ruin_Rect rectangle, ruin_Vec2 mouse_position) {
    return mouse_position.x >= rectangle.x && mouse_position.x <= rectangle.w && mouse_position.y >= rectangle.y && mouse_position.y <= rectangle.h;
};

void ruin_ComputeLayout(ruin_Context* ctx) { 

    Temp_Arena_Memory temp_mem = temp_arena_memory_begin(&ctx->arena);
    ruin_WidgetStack* widget_stack = ruin_WidgetStack__Init(temp_mem.arena);
    ruin_WidgetStack* widget_stack2 = ruin_WidgetStack__Init(temp_mem.arena);


    ruin_Id window;
    for (size_t i = 0; i < ctx->windows.index; ++i) {
        ruin_Widget* root = ruin_WindowArray__Get(&ctx->windows, i)->root_widget;
        // printf("window %s %f\n", ruin_WindowArray__Get(&ctx->windows, i)->title, root->draw_coords.bbox.x);

        compute__raw_sizes(ctx, widget_stack, root);
        compute__parent_depend_sizes(ctx, widget_stack, root);
        compute__child_depend_sizes(ctx, widget_stack, widget_stack2, root);
        compute__growable_sizes(ctx, widget_stack, root);
        compute__draw_coordinates(ctx, widget_stack, root);

        generate__draw_calls(ctx, widget_stack, root);
        printf("%s => ", ruin_WindowArray__Get(&ctx->windows, i)->title);

        if (hovered_window(root->draw_coords.bbox, ctx->mouse_position)) {
            window =  ruin_WindowArray__Get(&ctx->windows, i)->id;
        };
    };
    printf("\n");
    for (size_t i = 0; i < ctx->windows.index; ++i) {
        if (ruin_WindowArray__Get(&ctx->windows, i)->id == window) {
            ruin_WindowArray__MoveElementToLast(&ctx->windows, window);
            // printf("hoverd: %s\n", ruin_WindowArray__Get(&ctx->windows, i)->title);
        };
    };

    temp_arena_memory_end(temp_mem);
    arena_free_all(&ctx->temp_arena);
};

