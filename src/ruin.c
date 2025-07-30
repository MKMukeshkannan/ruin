#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifndef RUIN_HEADER
#define RUIN_HEADER

#include <ft2build.h>
#include FT_FREETYPE_H

#include "base.h"
#include <stdio.h>
#include <stdlib.h>


#define WIDGET_ARRAY    40
#define WINDOW_ARRAY    40
#define DRAW_QUEUE_SIZE 40

#define ruin_SameLine(ctx, label)   DeferLoop(ruin_RowBegin(ctx, label), ruin_RowEnd(ctx))

// LAYOUT
#define ROW_SPACING      5

// HEADER MESS
typedef U32    ruin_Id; 
typedef enum   ruin_SizeKind    { RUIN_SIZEKIND_PIXEL, RUIN_SIZEKIND_TEXTCONTENT, RUIN_SIZEKIND_PARENTPERCENTAGE, RUIN_SIZEKIND_CHILDRENSUM, RUIN_SIZEKIND_GROW } ruin_SizeKind;
#define ruin_WidgetOptions U32
typedef enum   ruin_WidgetFlags { 
    RUIN_WIDGETFLAGS_NO_FLAGS         =  (1<<0),
    RUIN_WIDGETFLAGS_DRAW_BACKGROUND  =  (1<<1),
    RUIN_WIDGETFLAGS_DRAW_BORDER      =  (1<<2),
    RUIN_WIDGETFLAGS_DRAW_TEXT        =  (1<<3),
    RUIN_WIDGETFLAGS_CLICKABLE        =  (1<<4),
    RUIN_WIDGETFLAGS_HOVERABLE        =  (1<<5),
} ruin_WidgetFlags;
typedef enum   ruin_WindowFlags { RUIN_WINDOWFLAGS_DRAGABLE = (1<<0), RUIN_WINDOWFLAGS_NOMENU = (1<<1), RUIN_WINDOWFLAGS_NOTITLE = (1<<2) }  ruin_WindowFlags;
typedef enum   ruin_Axis      { RUIN_AXISX, RUIN_AXISY, RUIN_AXISCOUNT, }                                                                    ruin_Axis;
typedef enum   ruin_DrawType    { RUIN_DRAWTYPE_RECT, RUIN_DRAWTYPE_CLIP, RUIN_DRAWTYPE_TEXT }                                               ruin_DrawType;

typedef struct ruin_Size        { ruin_SizeKind kind; F32 value; F32 strictness; }                                                           ruin_Size;
typedef struct ruin_Vec2        { F32 x, y; }                                                                                                ruin_Vec2;
typedef struct ruin_Rect        { F32 x, y, h, w; }                                                                                          ruin_Rect;
typedef struct ruin_Color       { U8 r, g, b, a; }                                                                                           ruin_Color;
typedef struct ruin_Direction       { U8 left, right, top, bottom; }                                                                         ruin_Direction;

typedef struct ruin_CharInfo { U32 width; U32 rows; S32 bearingX; S32 bearingY; S64 advance; U8* buffer; } ruin_CharInfo;
ruin_Color make_color_hex(U32 color);

typedef struct ruin_DrawCall {
    ruin_DrawType type;
    union {
        struct ruin_DrawRect { ruin_Rect rect; ruin_Color color; U8 border_width; } draw_rect;
        struct ruin_DrawClip { ruin_Rect rect; } draw_clip;
        struct ruin_DrawText { const char* text; ruin_Vec2 pos; } draw_text;
    } draw_info_union;
} ruin_DrawCall;

typedef struct ruin_Widget                                                   ruin_Widget;
struct ruin_Widget {
    ruin_Size size[RUIN_AXISCOUNT];
    ruin_Vec2 fixed_size;
    ruin_Vec2 partially_offset;
    struct ruin_DrawCoords {
        ruin_Rect bbox;
        ruin_Vec2 text_pos;
    } draw_coords;

    ruin_Widget *first_child;
    ruin_Widget *last_child;
    ruin_Widget *next_sibling;
    ruin_Widget *prev_sibling;
    ruin_Widget *parent;

    const char* text;
    ruin_WidgetOptions flags;
    ruin_Id id;
    ruin_Direction padding;
    ruin_Direction border_width;

    ruin_Axis child_layout_axis;

    ruin_Color background;
    ruin_Color foreground;
    ruin_Color border_color;
    U32 child_count;
};

typedef struct {
    S16 top;
    ruin_Widget* items[100];
} ruin_WidgetStack;

internal ruin_WidgetStack* create_stack(Temp_Arena_Memory temp) {
    ruin_WidgetStack* stack = (ruin_WidgetStack*)arena_alloc(temp.arena, sizeof(ruin_WidgetStack));
    MEM_ZERO(stack, sizeof(ruin_WidgetStack));
    stack->top = -1;
    return stack; 
};

internal ruin_Widget* get_top(ruin_WidgetStack* stack) {
    if (stack->top == -1) { return NULL; };
    return stack->items[stack->top];
};

internal ruin_Widget* pop(ruin_WidgetStack* stack) {
    if (stack->top == -1) return NULL;
    return stack->items[stack->top--];
};

internal void push(ruin_WidgetStack* stack, ruin_Widget* widget) {
    stack->items[++stack->top] = widget;
};

internal bool is_stack_empty(ruin_WidgetStack* stack) {
    return (stack->top == -1);
};

typedef struct ruin_Window                     ruin_Window;
struct ruin_Window {

    ruin_Rect window_rect;
    ruin_Widget* root_widget;  // window tree, donot persist across frames
    const char* title;

    ruin_Window* next;
    ruin_Window* prev;
    char window_flags;
    ruin_Id id;

};
typedef struct ruin_WindowList { ruin_Window* first; ruin_Window* last; } ruin_WindowList;

typedef struct {
    struct {
        size_t index;
        ruin_Widget* items[WIDGET_ARRAY];      // caches -> list of all widgets in that current window, persisted across frames
    } widgets;
    struct {
        size_t index;
        ruin_Window* items[WINDOW_ARRAY];      // caches -> list of all widgets in that current window, persisted across frames
    } windows;
    struct {
        size_t index;
        ruin_DrawCall items[DRAW_QUEUE_SIZE]; // emptied every frame
    } draw_queue;

    Arena arena; // persisted arena, across frames
    ruin_Window* current_window;

    float font_size;
    float highest_bearing_y; // lenght of the tallest character from base line, use to make top left as orgin
    ruin_CharInfo font[128];

    ruin_Vec2 mouse_position;
    ruin_Id hot;
    ruin_Id active;
    U64 frame;


    ruin_WidgetStack parent_stack;
} ruin_Context;


ruin_Context* create_ruin_context() {
    const U32 ARENA_SIZE = 12288;
    Arena arena = {0};
    unsigned char* buffer = (unsigned char*) malloc(ARENA_SIZE);
    arena_init(&arena, buffer, ARENA_SIZE);
    ruin_Context* ctx = (ruin_Context*)arena_alloc(&arena, sizeof(ruin_Context));

    MEM_ZERO(ctx, sizeof(ruin_Context));
    ctx->arena = arena;
    ctx->widgets.index = 0;
    ctx->windows.index = 0;

    ctx->font_size = (ctx->font_size == 0) ? 14 : ctx->font_size;

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Freetype Init Problem");
    };

    FT_Face face;
    if (FT_New_Face(ft, "jetbrains.ttf", 0, &face)) {
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


internal ruin_Id hash_string(const char *str) {
    ruin_Id hash = 5381; int c;
    while ((c = *str++))  hash = ((hash << 5) + hash) + c;
    return hash;
};

void ruin_BeginWindow(ruin_Context* ctx, const char* title, ruin_Rect rect, ruin_WindowFlags flags) {
    ruin_Id id = hash_string(title);
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


    ruin_Id root_id = hash_string("root##default");
    ruin_Widget* root = get_widget_by_id(ctx, root_id);
    if (root == NULL) {
        root = (ruin_Widget*)arena_alloc(&ctx->arena, sizeof(ruin_Widget));
        MEM_ZERO(root, sizeof(ruin_Widget));

        root->id = root_id;
        root->size[RUIN_AXISX] = (ruin_Size) { .kind = RUIN_SIZEKIND_PIXEL, .value = ctx->current_window->window_rect.w, .strictness=1 }; ;
        root->size[RUIN_AXISY] = (ruin_Size) { .kind = RUIN_SIZEKIND_PIXEL, .value = ctx->current_window->window_rect.h, .strictness=1 }; ;
        root->text = "root##default";
        root->background = make_color_hex(0xFFFFFFFF);
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

        root->padding = (ruin_Direction) {
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

F32 ruin_GetWidth(ruin_Context* ctx, const char* string) {
    int i = 0;
    F32 width = 0.0f;
    ruin_CharInfo* font = ctx->font;
    while (string[i] != '\0') {
        width += (font[string[i]].advance >> 6);
        ++i;
    };

    return width;
};

F32 ruin_GetHeight(ruin_Context* ctx, const char* string) {
    int i = 0;
    F32 rows = 0.0f;
    ruin_CharInfo* font = ctx->font;
    while (string[i] != '\0') {
        rows = MAX(font[string[i]].rows, rows);
        ++i;
    };

    return rows;
};

void ruin_ComputeLayout(ruin_Context* ctx) { 

    ruin_Window** window_list = ctx->windows.items;

    Temp_Arena_Memory temp_mem = temp_arena_memory_begin(&ctx->arena);
    ruin_WidgetStack* widget_stack = create_stack(temp_mem);
    ruin_WidgetStack* widget_stack2 = create_stack(temp_mem); // FOR POST ORDER ONLY


    for (size_t i = 0; i < ctx->windows.index; ++i) {

        ruin_Widget* root = ctx->windows.items[i]->root_widget;

        // COMPUTE RAW SIZE
        {
            push(widget_stack, root);
            while (!is_stack_empty(widget_stack)) {

                ruin_Widget* current_top = pop(widget_stack);

                if (current_top->parent != NULL) {
                    current_top->parent->partially_offset.x = 0;
                    current_top->parent->partially_offset.y = 0;
                };

                if (current_top->size[RUIN_AXISX].kind == RUIN_SIZEKIND_PIXEL) {
                    current_top->fixed_size.x = current_top->size[RUIN_AXISX].value;
                };

                if (current_top->size[RUIN_AXISY].kind == RUIN_SIZEKIND_PIXEL) {
                    current_top->fixed_size.y = current_top->size[RUIN_AXISY].value;
                };

                if (current_top->size[RUIN_AXISX].kind == RUIN_SIZEKIND_TEXTCONTENT) {
                    F32 length = ruin_GetWidth(ctx, current_top->text);
                    current_top->size[RUIN_AXISX].value = length;
                    current_top->fixed_size.x = length;
                };

                if (current_top->size[RUIN_AXISY].kind == RUIN_SIZEKIND_TEXTCONTENT) {
                    F32 length = ruin_GetHeight(ctx, current_top->text);
                    current_top->size[RUIN_AXISY].value = length;
                    current_top->fixed_size.y = length;
                };

                for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling) { 
                    push(widget_stack, widget);
                }
            };
            widget_stack->top = -1;
        }

        // COMPUTE PARENT DEPENDENT WIDGETS
        { 
            push(widget_stack, root);
            while (!is_stack_empty(widget_stack)) {
                ruin_Widget* current_top = pop(widget_stack);

                if (current_top->size[RUIN_AXISX].kind == RUIN_SIZEKIND_PARENTPERCENTAGE) {
                    current_top->fixed_size.x = current_top->size[RUIN_AXISX].value * current_top->parent->fixed_size.x - current_top->parent->padding.left - current_top->parent->padding.right;
                };
                if (current_top->size[RUIN_AXISY].kind == RUIN_SIZEKIND_PARENTPERCENTAGE) {
                    current_top->fixed_size.y = current_top->size[RUIN_AXISY].value * current_top->parent->fixed_size.x;
                };

                for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling)
                    push(widget_stack, widget);
            };

            widget_stack->top = -1;
        }

        // COMPUTE CHILD DEPENDENT
        {  
            push(widget_stack, root);
            while (!is_stack_empty(widget_stack)) {
                ruin_Widget* current_top = pop(widget_stack);
                push(widget_stack2, current_top);

                for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling)
                    push(widget_stack, widget);
            };

            while (!is_stack_empty(widget_stack2)) {
                ruin_Widget* curr = pop(widget_stack2);
                int childsum_x = 0, childsum_y = 0, childmax_x = -1, childmax_y = -1;
                for (ruin_Widget* widget = curr->last_child; widget != NULL; widget = widget->prev_sibling) {
                    childsum_x += widget->fixed_size.x + widget->padding.left + widget->padding.right;
                    childsum_y += widget->fixed_size.y + widget->padding.top  + widget->padding.bottom;
                    childmax_x = MAX(childmax_x, widget->fixed_size.x + widget->padding.left + widget->padding.right);
                    childmax_y = MAX(childmax_y, widget->fixed_size.y + widget->padding.top + widget->padding.bottom);
                };

                // TODO: ::layout:: if the flow direction is Y AXIS, then height of widget must be summed instead of width;
                if (curr->size[RUIN_AXISX].kind == RUIN_SIZEKIND_CHILDRENSUM) {
                    curr->fixed_size.x = childsum_x;
                };
                if (curr->size[RUIN_AXISY].kind == RUIN_SIZEKIND_CHILDRENSUM) {
                    curr->fixed_size.y = childmax_y;
                };
            }

            widget_stack->top = -1;
            widget_stack2->top = -1;
        }

        // GROW SIZING gwx
        {    
            printf("\nGROW DEMP\n");
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




                } else {
                };


                for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling)
                    push(widget_stack, widget);
            };

            widget_stack->top = -1;
        }


       // POSITION DRAW CO-ORDS: drawposition
       {
            push(widget_stack, root);
            while (!is_stack_empty(widget_stack)) {
                ruin_Widget* current_top = pop(widget_stack);
                // DO YOUR STUFF
                if (current_top->parent != NULL) {
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
                };

                ruin_Rect rect = current_top->draw_coords.bbox;
                for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling) 
                    push(widget_stack, widget);
            };

            widget_stack->top = -1;
       }



       // PRINT DRAW CO-ORDS
       {
            printf("\nFINAL PRINT\n");
            push(widget_stack, root);
            while (!is_stack_empty(widget_stack)) {
                ruin_Widget* current_top = pop(widget_stack);


                ruin_Rect rect = current_top->draw_coords.bbox;
                ruin_Vec2 text_pos = current_top->draw_coords.text_pos;
                // DO YOUR STUFF
                printf("ABOUT TO PUSH %s\t => x:%f, y:%f, w:%f, h:%f dir:%i fw:%f\n", current_top->text, rect.x, rect.y, rect.w, rect.h, current_top->child_layout_axis, current_top->fixed_size.x);
                ctx->draw_queue.items[ctx->draw_queue.index++] = (ruin_DrawCall) {
                    .type = RUIN_DRAWTYPE_RECT,
                    .draw_info_union = {
                        .draw_rect = {
                            .rect = rect,
                            .color = current_top->background,
                            .border_width = (current_top->flags & RUIN_WIDGETFLAGS_DRAW_BORDER) ? (U8)1 : (U8)0,
                        }
                    }
                };
                if (current_top->flags & RUIN_WIDGETFLAGS_DRAW_TEXT) {
                    ctx->draw_queue.items[ctx->draw_queue.index++] = (ruin_DrawCall) {
                        .type = RUIN_DRAWTYPE_TEXT,
                        .draw_info_union = {
                             .draw_text = {
                                 .text = current_top->text,
                                 .pos = {.x = text_pos.x, .y = text_pos.y}
                             }
                        }
                    };
                }

                for (ruin_Widget* widget = current_top->last_child; widget != NULL; widget = widget->prev_sibling) { 
                    push(widget_stack, widget);
                }
            };

            widget_stack->top = -1;
       }

    };

    temp_arena_memory_end(temp_mem);
};

ruin_Widget* ruin_create_widget_ex(ruin_Context* ctx, const char* label, ruin_WidgetOptions opt) {
    ruin_Widget* widget;
    widget = (ruin_Widget*)arena_alloc(&ctx->arena, sizeof(ruin_Widget));
    widget->id = hash_string(label);
    ctx->widgets.items[ctx->widgets.index++] = widget; // PUSHES INTO THE CACEH
    
    widget->text = label;
    widget->flags = opt;
    widget->child_layout_axis = RUIN_AXISY;

    if (opt & RUIN_WIDGETFLAGS_DRAW_TEXT) {
        widget->size[RUIN_AXISX] = (ruin_Size) { .kind = RUIN_SIZEKIND_TEXTCONTENT, .value = 0, .strictness = 1, };
        widget->size[RUIN_AXISY] = (ruin_Size) { .kind = RUIN_SIZEKIND_TEXTCONTENT, .value = 0, .strictness = 1, };
    };

    if (opt & RUIN_WIDGETFLAGS_DRAW_BACKGROUND) {
        widget->background = (ruin_Color) {.r=250, .g=0, .b=250, .a=255};
        widget->foreground = make_color_hex(0x00FFFFFF);
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

internal void push_widget_narry(ruin_Widget* root_widget, ruin_Widget* new_widget) {
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

// TODO: ::layout::  utility to render elements on same_line / row_wise
void ruin_RowBegin(ruin_Context* ctx, const char* label) {
    ruin_Id id = hash_string(label);
    ruin_Widget* row_warpper = get_widget_by_id(ctx, id);
    if (row_warpper == NULL) {
        row_warpper = ruin_create_widget_ex(ctx, label, RUIN_WIDGETFLAGS_NO_FLAGS);
        row_warpper->child_layout_axis = RUIN_AXISX;
    };
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
    push_widget_narry(ctx->parent_stack.items[ctx->parent_stack.top], row_warpper);

    push(&ctx->parent_stack, row_warpper);
};
void ruin_RowEnd(ruin_Context* ctx) { 
    pop(&ctx->parent_stack);
};

B8 ruin_Label(ruin_Context* ctx, const char* label) {
    ruin_Id id = hash_string(label);
    ruin_Widget* label_widget = get_widget_by_id(ctx, id);
    if (label_widget == NULL) label_widget = ruin_create_widget_ex(ctx, label, RUIN_WIDGETFLAGS_DRAW_TEXT);
    push_widget_narry(get_top(&ctx->parent_stack), label_widget);

    return false;
};

B8 ruin_SpacerX(ruin_Context* ctx, const char* label) {
    ruin_Id id = hash_string(label);
    ruin_Widget* spacer = get_widget_by_id(ctx, id);
    if (spacer == NULL) {
        spacer = ruin_create_widget_ex(ctx, label, RUIN_WIDGETFLAGS_NO_FLAGS);
        spacer->size[RUIN_AXISX] = (ruin_Size) { .kind=RUIN_SIZEKIND_GROW, .value = 0, .strictness = 1 };
        spacer->size[RUIN_AXISY] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = 0, .strictness = 1 };
    };
    push_widget_narry(get_top(&ctx->parent_stack), spacer);
    return false;
};

B8 ruin_SpacerFixedX(ruin_Context* ctx, const char* label, F32 space) {
    ruin_Id id = hash_string(label);
    ruin_Widget* spacer = get_widget_by_id(ctx, id);
    if (spacer == NULL) {
        spacer = ruin_create_widget_ex(ctx, label, RUIN_WIDGETFLAGS_NO_FLAGS);
        spacer->size[RUIN_AXISX] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = space, .strictness = 1 };
        spacer->size[RUIN_AXISY] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = 1, .strictness = 1 };
    };
    push_widget_narry(get_top(&ctx->parent_stack), spacer);
    return false;
};

B8 ruin_SpacerFixedY(ruin_Context* ctx, const char* label, F32 space) {
    ruin_Id id = hash_string(label);
    ruin_Widget* spacer = get_widget_by_id(ctx, id);
    if (spacer == NULL) {
        spacer = ruin_create_widget_ex(ctx, label, RUIN_WIDGETFLAGS_NO_FLAGS);
        spacer->size[RUIN_AXISX] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = 1, .strictness = 1 };
        spacer->size[RUIN_AXISY] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = space, .strictness = 1 };
    };
    push_widget_narry(get_top(&ctx->parent_stack), spacer);
    return false;
    return false;
};
B8 ruin_SpacerY(ruin_Context* ctx, const char* label) {
    ruin_Id id = hash_string(label);
    ruin_Widget* spacer = get_widget_by_id(ctx, id);
    if (spacer == NULL) {
        spacer = ruin_create_widget_ex(ctx, label, RUIN_WIDGETFLAGS_NO_FLAGS);
        spacer->size[RUIN_AXISX] = (ruin_Size) { .kind=RUIN_SIZEKIND_PIXEL, .value = 0, .strictness = 1 };
        spacer->size[RUIN_AXISY] = (ruin_Size) { .kind=RUIN_SIZEKIND_GROW, .value = 0, .strictness = 1 };
    };
    push_widget_narry(get_top(&ctx->parent_stack), spacer);
    return false;
    return false;
};

bool clicked(ruin_Rect rectangle, ruin_Vec2 mouse_position) {
    return mouse_position.x >= rectangle.x && mouse_position.x <= rectangle.w && mouse_position.y >= rectangle.y && mouse_position.y <= rectangle.h;
};


B8 ruin_Button(ruin_Context* ctx, const char* label) {
    ruin_Id id = hash_string(label);
    ruin_Widget* label_widget = get_widget_by_id(ctx, id);
    if (label_widget == NULL) label_widget = ruin_create_widget_ex(ctx, label, RUIN_WIDGETFLAGS_DRAW_TEXT|RUIN_WIDGETFLAGS_DRAW_BACKGROUND|RUIN_WIDGETFLAGS_DRAW_BORDER);


    ruin_Rect rect = label_widget->draw_coords.bbox;
    ruin_Vec2 mouse_position = ctx->mouse_position;
    label_widget->padding = (ruin_Direction) { .left = 16, .right = 16, .top = 8, .bottom = 8, };

    if (clicked(rect, mouse_position)) {
        label_widget->background = (ruin_Color) { .r=220, .g=220, .b=220, .a=255 };
    } else {
        label_widget->background = (ruin_Color) { .r=250, .g=250, .b=250, .a=255 };
    };

    push_widget_narry(get_top(&ctx->parent_stack), label_widget);
    return false;
};

static B8 is_point_over_rect(ruin_Rect rect, ruin_Vec2 point);
static ruin_Rect overlap_rect(ruin_Rect rect1, ruin_Rect rect2);
static ruin_Rect expand_rect(ruin_Rect rect, int n);

ruin_Color make_color_hex(U32 color) {
  ruin_Color res = (ruin_Color) {
        (U8)((color >> (8 * 0)) & 0xFF),  // a
        (U8)((color >> (8 * 1)) & 0xFF),  // b
        (U8)((color >> (8 * 2)) & 0xFF),  // g
        (U8)((color >> (8 * 3)) & 0xFF)   // r
  };

  return res;
};

#endif


#ifdef __cplusplus
}
#endif
