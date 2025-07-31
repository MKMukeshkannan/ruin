#include <ft2build.h>
#include FT_FREETYPE_H

#include "ruin_core.h"

internal F32 ruin_GetWidth(ruin_Context* ctx, String8 string) {
    int i = 0;
    F32 width = 0.0f;
    ruin_CharInfo* font = ctx->font;
    for (int i = 0; i < string.len; ++i) 
        width += (font[string.data[i]].advance >> 6);

    return width;
};

internal F32 ruin_GetHeight(ruin_Context* ctx, String8 string) {
    int i = 0;
    F32 rows = 0.0f;
    ruin_CharInfo* font = ctx->font;
    for (int i = 0; i < string.len; ++i) 
        rows = MAX(font[string.data[i]].rows, rows);

    return rows;
};

void push_widget_narry(ruin_Widget* root_widget, ruin_Widget* new_widget) {
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

    // if (!is_stack_empty(&ctx->parent_stack)) printf("%llu\n", get_top(&ctx->parent_stack)->id);

    char buf[64];
    ruin_Id parent_id = is_stack_empty(&ctx->parent_stack) ? RUIN_TRANSIENT_ID: get_top(&ctx->parent_stack)->id;
    snprintf(buf, sizeof(buf), "seed%llu_label%s", parent_id, str);

    ruin_Id hash = 5381; int c; int i = 0;
    while ((c = buf[i++]))  hash = ((hash << 5) + hash) + c;

    return hash;
};

ruin_Context* create_ruin_context() {
    const U32 ARENA_SIZE = 12288;
    Arena arena = {0};
    unsigned char* buffer = (unsigned char*) malloc(ARENA_SIZE);
    arena_init(&arena, buffer, ARENA_SIZE);


    const U32 TEMP_ARENA_SIZE = 2048;
    Arena temp_arena = {0};
    unsigned char* temp_buffer = (unsigned char*) malloc(TEMP_ARENA_SIZE);
    arena_init(&temp_arena, temp_buffer, TEMP_ARENA_SIZE);

    ruin_Context* ctx = (ruin_Context*)arena_alloc(&arena, sizeof(ruin_Context));

    MEM_ZERO(ctx, sizeof(ruin_Context));
    ctx->arena = arena;
    ctx->temp_arena = temp_arena;
    ctx->widgets.index = 0;
    ctx->windows.index = 0;
    ctx->parent_stack.top = -1;

    ctx->font_size = (ctx->font_size == 0) ? 14 : ctx->font_size;
    push_color_stack(&ctx->background_color_stack, (ruin_Color) {.r=50, .g=50, .b=50, .a=1});
    push_color_stack(&ctx->foreground_color_stack, (ruin_Color) {.r=50, .g=50, .b=50, .a=1});
    push_color_stack(&ctx->hover_color_stack, (ruin_Color) {.r=250, .g=50, .b=50, .a=1});
    push_color_stack(&ctx->active_color_stack, (ruin_Color) {.r=250, .g=50, .b=50, .a=1});

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Freetype Init Problem");
    };

    FT_Face face;
    if (FT_New_Face(ft, "resources/jetbrains.ttf", 0, &face)) {
        fprintf(stderr, "Freetype Face Creation Problem");
    };
    FT_Set_Pixel_Sizes(face, 0, ctx->font_size);

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "ERROR::FREETYTPE: Failed to load Glyph");
            continue;
        }

        size_t total_pixels = face->glyph->bitmap.width * face->glyph->bitmap.rows;
        U8* gray_alpha_data = (U8*)malloc(total_pixels * 2); 
        for (size_t i = 0; i < total_pixels; i++) {
            gray_alpha_data[i * 2 + 0] = face->glyph->bitmap.buffer[i];
            gray_alpha_data[i * 2 + 1] = (face->glyph->bitmap.buffer[i] < 0) ? 0 : face->glyph->bitmap.buffer[i];
        };

        ctx->font[c] = (ruin_CharInfo) {
            .width = face->glyph->bitmap.width,
            .rows = face->glyph->bitmap.rows,
            .bearingX = face->glyph->bitmap_left,
            .bearingY = face->glyph->bitmap_top,
            .advance = face->glyph->advance.x,
            .buffer = gray_alpha_data,
        };
    }

    ctx->highest_bearing_y = ctx->font[108].bearingY;

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return ctx;
};

ruin_Window* get_window_by_id(ruin_Context* ctx, ruin_Id id) {
    for (size_t i = 0; i < ctx->windows.index; ++i) {
        if (ctx->windows.items[i]->id == id)
            return ctx->windows.items[i];
    };

    return NULL;
};

ruin_Widget* get_widget_by_id(ruin_Context* ctx, ruin_Id id) {
    for (size_t i = 0; i < ctx->widgets.index; ++i) {
        if (ctx->widgets.items[i]->id == id) 
            return ctx->widgets.items[i]; 
    };

    return NULL;
};

internal String8 id_seperator = String8("##");
ruin_Widget* ruin_create_widget_ex(ruin_Context* ctx, const char* full_name, ruin_Id id, ruin_WidgetOptions opt) {



    ruin_Widget* widget = (id != RUIN_TRANSIENT_ID)
        ? (ruin_Widget*)arena_alloc(&ctx->arena, sizeof(ruin_Widget))
        : (ruin_Widget*)arena_alloc(&ctx->temp_arena, sizeof(ruin_Widget));

    if (id != RUIN_TRANSIENT_ID) ctx->widgets.items[ctx->widgets.index++] = widget; // PUSHES INTO THE CACEH


    if (id != RUIN_TRANSIENT_ID) {
        String8 str_name = str_from_cstr(full_name, &ctx->arena);
        String8 str_display = str_name;

        size_t ind;
        if ((ind = str_index_of(str_name, id_seperator)) != (size_t)-1) str_display = str_substring(str_display, 0, ind, &ctx->arena);

        widget->display_text = str_display;
        widget->widget_name  = str_name;
    }
    
    widget->id = id;
    widget->flags = opt;

    widget->child_layout_axis = *get_axis_stack_top(&ctx->child_direction_stack);
    widget->padding = *get_rectsides_stack_top(&ctx->padding_stack);


    if (opt & RUIN_WIDGETFLAGS_DRAW_TEXT) {
        widget->size[RUIN_AXISX] = (ruin_Size) { .kind = RUIN_SIZEKIND_TEXTCONTENT, .value = 0, .strictness = 1, };
        widget->size[RUIN_AXISY] = (ruin_Size) { .kind = RUIN_SIZEKIND_TEXTCONTENT, .value = 0, .strictness = 1, };
    };

    if (opt & RUIN_WIDGETFLAGS_DRAW_BACKGROUND) {
        widget->background_color = *get_color_stack_top(&ctx->background_color_stack);
        widget->foreground_color = *get_color_stack_top(&ctx->foreground_color_stack);
        widget->hover_color = *get_color_stack_top(&ctx->hover_color_stack);
        widget->active_color = *get_color_stack_top(&ctx->active_color_stack);
    };

    if (opt & RUIN_WIDGETFLAGS_DRAW_BORDER) {
        widget->border_color = (ruin_Color) {.r=255, .g=255, .b=255, .a=255};
    };

    if (opt & RUIN_WIDGETFLAGS_NO_FLAGS) {
        // widget->size[RUIN_AXISX] = (ruin_Size) { .kind = RUIN_SIZEKIND_CHILDRENSUM, .value = 0, .strictness = 1, };
        // widget->size[RUIN_AXISY] = (ruin_Size) { .kind = RUIN_SIZEKIND_CHILDRENSUM, .value = 0, .strictness = 1, };
    };

    return widget;
};



void ruin_BeginWindow(ruin_Context* ctx, const char* title, ruin_Rect rect, ruin_WindowFlags flags) {
    ruin_Id id = hash_string(ctx, title);
    ruin_Window* window = get_window_by_id(ctx, id);

    if (window == NULL) {
        window = (ruin_Window*)arena_alloc(&ctx->arena, sizeof(ruin_Window));
        MEM_ZERO(window, sizeof(ruin_Window));

        window->id = id;
        window->title = title;
        window->window_rect = rect;
        window->window_flags = flags;
        
        ctx->windows.items[ctx->windows.index++] = window;
    };
    ctx->current_window = window;


    ruin_Id root_id = hash_string(ctx, "root##default");
    ruin_Widget* root = get_widget_by_id(ctx, root_id);
    if (root == NULL) {
        root = (ruin_Widget*)arena_alloc(&ctx->arena, sizeof(ruin_Widget));
        MEM_ZERO(root, sizeof(ruin_Widget));

        root->id = root_id;
        root->size[RUIN_AXISX] = (ruin_Size) { .kind = RUIN_SIZEKIND_PIXEL, .value = ctx->current_window->window_rect.w, .strictness=1 }; ;
        root->size[RUIN_AXISY] = (ruin_Size) { .kind = RUIN_SIZEKIND_PIXEL, .value = ctx->current_window->window_rect.h, .strictness=1 }; ;
        root->display_text = String8("root");
        root->child_count = 0;

        root->draw_coords.bbox.x = ctx->current_window->window_rect.x;
        root->draw_coords.bbox.y = ctx->current_window->window_rect.y;
        root->draw_coords.bbox.h = ctx->current_window->window_rect.y + root->size[RUIN_AXISY].value;
        root->draw_coords.bbox.w = ctx->current_window->window_rect.x + root->size[RUIN_AXISX].value;

        root->draw_coords.text_pos.x = root->draw_coords.bbox.x;
        root->draw_coords.text_pos.y = root->draw_coords.bbox.y;

        root->partially_offset.x = 0;
        root->partially_offset.y = 0;

        root->flags |= RUIN_WIDGETFLAGS_DRAW_BORDER;
        root->child_layout_axis = RUIN_AXISY;

        root->padding = (ruin_RectSide) {
            .left = 20,
            .right = 20,
            .top = 20,
            .bottom = 20,
        };

        ctx->widgets.items[ctx->widgets.index++] = root;
    };
    root->first_child = NULL;
    window->root_widget = root;
    push(&ctx->parent_stack, root);
};

void ruin_EndWindow(ruin_Context* ctx) {
    ctx->current_window = NULL;
    pop(&ctx->parent_stack);
};


internal void compute__raw_sizes(ruin_Context* ctx, ruin_WidgetStack* widget_stack, ruin_Widget* root) {
    push(widget_stack, root);
    while (!is_stack_empty(widget_stack)) {

        ruin_Widget* current_top = pop(widget_stack);

        if (current_top->parent != NULL) {
            current_top->parent->partially_offset.x = 0;
            current_top->parent->partially_offset.y = 0;
        };

        if (current_top->size[RUIN_AXISX].kind == RUIN_SIZEKIND_PIXEL) 
            current_top->fixed_size.x = current_top->size[RUIN_AXISX].value;

        if (current_top->size[RUIN_AXISY].kind == RUIN_SIZEKIND_PIXEL) 
            current_top->fixed_size.y = current_top->size[RUIN_AXISY].value;

        if (current_top->size[RUIN_AXISX].kind == RUIN_SIZEKIND_TEXTCONTENT) 
            current_top->fixed_size.x = ruin_GetWidth(ctx, current_top->display_text);

        if (current_top->size[RUIN_AXISY].kind == RUIN_SIZEKIND_TEXTCONTENT) 
            current_top->fixed_size.y = ruin_GetHeight(ctx, current_top->display_text);

        for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling)
            push(widget_stack, widget);
    };
    clear_stack(widget_stack);
};

internal void compute__parent_depend_sizes(ruin_Context* ctx, ruin_WidgetStack* widget_stack, ruin_Widget* root) {
    push(widget_stack, root);
    while (!is_stack_empty(widget_stack)) {
        ruin_Widget* current_top = pop(widget_stack);
        for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling)
            push(widget_stack, widget);

        if (current_top->parent == NULL) continue;

        float x_padding = current_top->parent->padding.left + current_top->parent->padding.right;
        float y_padding = current_top->parent->padding.top  + current_top->parent->padding.bottom;

        if (current_top->size[RUIN_AXISX].kind == RUIN_SIZEKIND_PARENTPERCENTAGE) 
            current_top->fixed_size.x = (current_top->size[RUIN_AXISX].value * current_top->parent->fixed_size.x) - x_padding;

        if (current_top->size[RUIN_AXISY].kind == RUIN_SIZEKIND_PARENTPERCENTAGE)
            current_top->fixed_size.y = (current_top->size[RUIN_AXISY].value * current_top->parent->fixed_size.y) - y_padding;

    };

    clear_stack(widget_stack);
};

internal void compute__child_depend_sizes(ruin_Context* ctx, ruin_WidgetStack* stack_1, ruin_WidgetStack* stack_2, ruin_Widget* root) {
    push(stack_1, root);
    while (!is_stack_empty(stack_1)) {
        ruin_Widget* current_top = pop(stack_1);
        push(stack_2, current_top);

        for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling) push(stack_1, widget);
    };

    while (!is_stack_empty(stack_2)) {
        ruin_Widget* curr = pop(stack_2);
        int childsum_x = 0, childsum_y = 0, childmax_x = -1, childmax_y = -1;
        for (ruin_Widget* widget = curr->last_child; widget != NULL; widget = widget->prev_sibling) {

            float x_padding = widget->padding.left + widget->padding.right;
            float y_padding = widget->padding.top + widget->padding.bottom;


            childsum_x += widget->fixed_size.x + x_padding;
            childsum_y += widget->fixed_size.y + y_padding;
            childmax_x = MAX(childmax_x, widget->fixed_size.x + x_padding);
            childmax_y = MAX(childmax_y, widget->fixed_size.y + y_padding);
        };

        // TODO: ::layout:: if the flow direction is Y AXIS, then height of widget must be summed instead of width;
        if (curr->size[RUIN_AXISX].kind == RUIN_SIZEKIND_CHILDRENSUM) 
            curr->fixed_size.x = childsum_x;
        if (curr->size[RUIN_AXISY].kind == RUIN_SIZEKIND_CHILDRENSUM) 
            curr->fixed_size.y = childmax_y;
    }

    clear_stack(stack_1);
    clear_stack(stack_2);
};

internal void compute__growable_sizes(ruin_Context* ctx, ruin_WidgetStack* widget_stack, ruin_Widget* root) {
    push(widget_stack, root);
    while (!is_stack_empty(widget_stack)) {
        ruin_Widget* current_top = pop(widget_stack);

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
            push(widget_stack, widget);
    };

    clear_stack(widget_stack);
};

internal void compute__draw_coordinates(ruin_Context* ctx, ruin_WidgetStack* widget_stack, ruin_Widget* root) {
    push(widget_stack, root);

    while (!is_stack_empty(widget_stack)) {
        ruin_Widget* current_top = pop(widget_stack);
        for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling) push(widget_stack, widget);


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

    widget_stack->top = -1;
};

internal void generate__draw_calls(ruin_Context* ctx, ruin_WidgetStack* widget_stack, ruin_Widget* root) {
    push(widget_stack, root);
    while (!is_stack_empty(widget_stack)) {
        ruin_Widget* current_top = pop(widget_stack);


        ruin_Rect rect = current_top->draw_coords.bbox;
        ruin_Vec2 text_pos = current_top->draw_coords.text_pos;
        // DO YOUR STUFF
        // printf("ABOUT TO PUSH %s\t => x:%f, y:%f, w:%f, h:%f dir:%i fw:%f cr:%i\n", current_top->display_text.data, rect.x, rect.y, rect.w, rect.h, current_top->child_layout_axis, current_top->fixed_size.x, current_top->background_color.r);
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
                        .pos = {.x = text_pos.x, .y = text_pos.y}
                    }
                }
            };
        }

        for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling) { 
            push(widget_stack, widget);
        }
    };

    clear_stack(widget_stack);
};

void ruin_ComputeLayout(ruin_Context* ctx) { 

    Temp_Arena_Memory temp_mem = temp_arena_memory_begin(&ctx->arena);
    ruin_WidgetStack* widget_stack = create_stack(temp_mem);
    ruin_WidgetStack* widget_stack2 = create_stack(temp_mem); // FOR POST ORDER ONLY


    for (size_t i = 0; i < ctx->windows.index; ++i) {
        ruin_Widget* root = ctx->windows.items[i]->root_widget;

        compute__raw_sizes(ctx, widget_stack, root);
        compute__parent_depend_sizes(ctx, widget_stack, root);
        compute__child_depend_sizes(ctx, widget_stack, widget_stack2, root);
        compute__growable_sizes(ctx, widget_stack, root);
        compute__draw_coordinates(ctx, widget_stack, root);

        generate__draw_calls(ctx, widget_stack, root);
    };

    temp_arena_memory_end(temp_mem);
    arena_free_all(&ctx->temp_arena);
};

