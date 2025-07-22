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


#define WIDGET_ARRAY    20
#define WINDOW_ARRAY    20
#define DRAW_QUEUE_SIZE 20


// HEADER MESS
typedef U32    ruin_Id; 
typedef enum   ruin_SizeKind    { RUIN_SIZEKIND_PIXEL, RUIN_SIZEKIND_TEXTCONTENT, RUIN_SIZEKIND_PARENTPERCENTAGE, RUIN_SIZEKIND_CHILDRENSUM } ruin_SizeKind;
typedef enum   ruin_WidgetFlags { RUIN_WIDGETFLAGS_CLICKABLE = (1<<0), RUIN_WIDGETFLAGS_HOVERABLE = (1<<1) }                                 ruin_WidgetFlags;
typedef enum   ruin_WindowFlags { RUIN_WINDOWFLAGS_DRAGABLE = (1<<0), RUIN_WINDOWFLAGS_NOMENU = (1<<1), RUIN_WINDOWFLAGS_NOTITLE = (1<<2) }  ruin_WindowFlags;
typedef enum   ruin_Axises      { RUIN_AXISX, RUIN_AXISY, RUIN_AXISCOUNT, }                                                                  ruin_Axises;
typedef enum   ruin_DrawType    { RUIN_DRAWTYPE_RECT, RUIN_DRAWTYPE_CLIP, RUIN_DRAWTYPE_TEXT }                                               ruin_DrawType;

typedef struct ruin_Size        { ruin_SizeKind kind; F32 value; F32 strictness; }                                                           ruin_Size;
typedef struct ruin_Vec2        { F32 x, y; }                                                                                                ruin_Vec2;
typedef struct ruin_Rect        { F32 x, y, h, w; }                                                                                          ruin_Rect;
typedef struct ruin_Color       { U8 r, g, b, a; }                                                                                           ruin_Color;

typedef struct ruin_CharInfo { U32 width; U32 rows; S32 bearingX; S32 bearingY; S64 advance; U8* buffer; } ruin_CharInfo;
ruin_Color make_color_hex(U32 color);

typedef struct ruin_DrawCall {
    ruin_DrawType type;
    union {
        struct ruin_DrawRect { ruin_Rect rect; ruin_Color color; } draw_rect;
        struct ruin_DrawClip { ruin_Rect rect; } draw_clip;
        struct ruin_DrawText { const char* text; ruin_Vec2 pos; } draw_text;
    } draw_info_union;
} ruin_DrawCall;

typedef struct ruin_Widget                                                   ruin_Widget;
struct ruin_Widget {
    ruin_Size size[RUIN_AXISCOUNT];
    ruin_Vec2 fixed_size;
    ruin_Vec2 partially_offset;
    ruin_Rect draw_coords;

    ruin_Widget *first_child;
    ruin_Widget *last_child;
    ruin_Widget *next_sibling;
    ruin_Widget *prev_sibling;
    ruin_Widget *parent;

    const char* text;
    char flags;
    ruin_Id id;

    ruin_Color background;
    ruin_Color foreground;
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

    ruin_CharInfo font[128];

    ruin_Vec2 mouse_position;
    ruin_Id hot;
    ruin_Id active;
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

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Freetype Init Problem");
    };

    FT_Face face;
    if (FT_New_Face(ft, "inter.ttf", 0, &face)) {
        fprintf(stderr, "Freetype Face Creation Problem");
    };
    FT_Set_Pixel_Sizes(face, 0, 48);

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "ERROR::FREETYTPE: Failed to load Glyph");
            continue;
        }

        size_t total_pixels = face->glyph->bitmap.width * face->glyph->bitmap.rows;
        U8* gray_alpha_data = (U8*)malloc(total_pixels * 2); 
        for (size_t i = 0; i < total_pixels; i++) {
            gray_alpha_data[i * 2 + 0] = face->glyph->bitmap.buffer[i];
            gray_alpha_data[i * 2 + 1] =  (face->glyph->bitmap.buffer[i] < 0) ? 0 : face->glyph->bitmap.buffer[i];
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
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return ctx;
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
        root->size[0] = (ruin_Size) { .kind = RUIN_SIZEKIND_PIXEL, .value = ctx->current_window->window_rect.h, .strictness=1 }; ;
        root->size[1] = (ruin_Size) { .kind = RUIN_SIZEKIND_PIXEL, .value = ctx->current_window->window_rect.w, .strictness=1 }; ;
        root->text = "root##default";
        root->background = make_color_hex(0xFFFFFFFF);

        root->draw_coords.x=ctx->current_window->window_rect.x;
        root->draw_coords.y=ctx->current_window->window_rect.y;
        root->draw_coords.h=ctx->current_window->window_rect.y + root->size[RUIN_AXISY].value;
        root->draw_coords.w=ctx->current_window->window_rect.x + root->size[RUIN_AXISX].value;

        root->partially_offset.x = 0;
        root->partially_offset.y = 0;

        ctx->widgets.items[ctx->widgets.index++] = root;
    };
    root->first_child = NULL;
    window->root_widget = root;



};

void ruin_EndWindow(ruin_Context* ctx) {
    ctx->current_window = NULL;
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
    F32 min_y = 0.0f;
    F32 max_y = 0.0f;
    F32 rows = 0.0f;
    ruin_CharInfo* font = ctx->font;
    while (string[i] != '\0') {
        min_y = MIN(font[string[i]].bearingY, min_y);
        max_y = MAX(font[string[i]].bearingY, max_y);
        rows = MAX(font[string[i]].rows, rows);
        ++i;
    };

    return rows;
};

void ruin_ComputeLayout(ruin_Context* ctx) { 

    ruin_Window** window_list = ctx->windows.items;

    Temp_Arena_Memory temp_mem = temp_arena_memory_begin(&ctx->arena);
    ruin_WidgetStack* widget_stack = create_stack(temp_mem);


    for (size_t i = 0; i < ctx->windows.index; ++i) {

        ruin_Widget* root = ctx->windows.items[i]->root_widget;

        // COMPUTE RAW SIZE
        {
            // printf("\nRAW SIZEZZ \n");

            widget_stack->items[++widget_stack->top] = root;
            while (widget_stack->top != -1) {

                ruin_Widget* current_top = widget_stack->items[widget_stack->top];
                widget_stack->top--;

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

                // printf("%s: h=>%f w=>%f\n", current_top->text, current_top->fixed_size.x, current_top->fixed_size.y);
                for (ruin_Widget* i = current_top->last_child; i != NULL; i = i->prev_sibling) { 
                    widget_stack->items[++widget_stack->top] = i; 
                }
            };
            // printf("\n");
            widget_stack->top = -1;
        }

        // COMPUTE CHILD DEPENDENT WIDGETS
        {
            // printf("\nCHILD DEMP\n");
            widget_stack->items[++widget_stack->top] = root;
            while (widget_stack->top != -1) {

                ruin_Widget* current_top = widget_stack->items[widget_stack->top];
                widget_stack->top--;


                if (current_top->size[RUIN_AXISX].kind == RUIN_SIZEKIND_PARENTPERCENTAGE) {
                    current_top->fixed_size.x = current_top->size[RUIN_AXISX].value * current_top->parent->fixed_size.x;
                };
                if (current_top->size[RUIN_AXISY].kind == RUIN_SIZEKIND_PARENTPERCENTAGE) {
                    current_top->fixed_size.y = current_top->size[RUIN_AXISY].value * current_top->parent->fixed_size.x;
                };

                // printf("%s: s=>%f\n", current_top->text, current_top->fixed_size.x);
                for (ruin_Widget* i = current_top->last_child; i != NULL; i = i->prev_sibling) { widget_stack->items[++widget_stack->top] = i; }
            };

            widget_stack->top = -1;
        }


       // POSITION DRAW CO-ORDS: drawposition
       {
            printf("\nPOSITIONS and DRAW CO-ORDS\n");
            widget_stack->items[++widget_stack->top] = root;
            while (widget_stack->top != -1) {
                ruin_Widget* current_top = widget_stack->items[widget_stack->top];
                widget_stack->top--;


                // DO YOUR STUFF
                if (current_top->parent != NULL) {
                    current_top->draw_coords.x = current_top->parent->draw_coords.x + current_top->parent->partially_offset.x;
                    current_top->draw_coords.y = current_top->parent->draw_coords.y + current_top->parent->partially_offset.y;
                    current_top->draw_coords.w = current_top->fixed_size.x + current_top->parent->draw_coords.x + current_top->parent->partially_offset.x;
                    current_top->draw_coords.h = current_top->fixed_size.y + current_top->parent->draw_coords.y + current_top->parent->partially_offset.y;
                    
                    current_top->parent->partially_offset.y += current_top->draw_coords.h - current_top->draw_coords.y;
                    printf("\t parent offset y: %f\n", current_top->parent->partially_offset.y);
                };

                ruin_Rect rect = current_top->draw_coords;
                printf("%s\t => x:%f, y:%f, w:%f, h:%f\n", current_top->text, rect.x, rect.y, rect.w, rect.h);
                for (ruin_Widget* i = current_top->last_child; i != NULL; i = i->prev_sibling) { 
                    widget_stack->items[++widget_stack->top] = i; 
                }
            };

            widget_stack->top = -1;
       }



       // PRINT DRAW CO-ORDS
       {
            // printf("\nDRAW CO-ORDS\n");
            widget_stack->items[++widget_stack->top] = root;
            while (widget_stack->top != -1) {
                ruin_Widget* current_top = widget_stack->items[widget_stack->top];
                widget_stack->top--;


                ruin_Rect rect = current_top->draw_coords;
                // DO YOUR STUFF
                // printf("ABOUT TO PUSH %s\t => x:%f, y:%f, w:%f, h:%f\n", current_top->text, rect.x, rect.y, rect.w, rect.h);
                ctx->draw_queue.items[ctx->draw_queue.index++] = (ruin_DrawCall) {
                    .type = RUIN_DRAWTYPE_RECT,
                    .draw_info_union = {
                        .draw_rect = {
                            .rect = rect,
                            .color = current_top->background,
                        }
                    }
                };
               ctx->draw_queue.items[ctx->draw_queue.index++] = (ruin_DrawCall) {
                   .type = RUIN_DRAWTYPE_TEXT,
                   .draw_info_union = {
                        .draw_text = {
                            .text = current_top->text,
                            .pos = {.x = rect.x, .y = rect.y}
                        }
                   }
               };

//             root##default    => x:100.000000, y:50.000000, w:400.000000, h:400.000000
//             Inspector        => x:100.000000, y:50.000000, w:160.000000, h:98.000000
//             Contact me by tonight 9pm        => x:100.000000, y:148.000000, w:264.000000, h:196.000000
//             slider   => x:100.000000, y:246.000000, w:200.000000, h:266.000000
//                     indicator        => x:100.000000, y:246.000000, w:120.000000, h:266.000000
//             Ok3      => x:100.000000, y:316.000000, w:128.000000, h:364.000000
//             Ok13     => x:100.000000, y:414.000000, w:133.000000, h:462.000000



                for (ruin_Widget* i = current_top->last_child; i != NULL; i = i->prev_sibling) { 
                    widget_stack->items[++widget_stack->top] = i; 
                }
            };

            widget_stack->top = -1;
       }



        // printf("\n");

    };

    temp_arena_memory_end(temp_mem);


};

void ruin_SameLine() { };

B8 ruin_LabelDynamic(ruin_Context* ctx, const char* label, const char** display) {
    ruin_Id id = hash_string(label);
    ruin_Widget* button_widget = get_widget_by_id(ctx, id);

    // ALLOCATE and CREATE BUTTON FOR THE FIRST TIME
    if (button_widget == NULL) {
        button_widget = (ruin_Widget*)arena_alloc(&ctx->arena, sizeof(ruin_Widget));
        button_widget->id = id;
        button_widget->text = label;
        button_widget->size[RUIN_AXISX] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_TEXTCONTENT,
            .value = 0,
            .strictness = 1,
        };
        button_widget->size[RUIN_AXISY] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_TEXTCONTENT,
            .value = 0,
            .strictness = 1,
        };
        button_widget->background = make_color_hex(0xFFFFFFFF);
        button_widget->foreground = make_color_hex(0x000FFFFF);
        ctx->widgets.items[ctx->widgets.index++] = button_widget;
    };
    button_widget->text = *display;

    // PUSH WIDGET TO ROOT OF THE WINDOW
    ruin_Widget* root_widget = ctx->current_window->root_widget;
    if (root_widget->first_child == NULL) {
        button_widget->first_child = NULL;
        button_widget->next_sibling = NULL;
        button_widget->prev_sibling = NULL;
        button_widget->parent = root_widget;

        root_widget->first_child = button_widget;
        root_widget->last_child = button_widget;
    } else {
        ruin_Widget* temp = root_widget->first_child;
        while (temp->next_sibling != NULL) {
            temp = temp->next_sibling;
        };

        temp->next_sibling = button_widget;
        button_widget->prev_sibling = temp;
        button_widget->next_sibling = NULL;
        button_widget->parent = root_widget;
        button_widget->first_child = NULL;
        button_widget->last_child = NULL;
        root_widget->last_child = button_widget;
    };

    return false;
}

B8 ruin_Label(ruin_Context* ctx, const char* label) {
    ruin_Id id = hash_string(label);
    ruin_Widget* button_widget = get_widget_by_id(ctx, id);

    // ALLOCATE and CREATE BUTTON FOR THE FIRST TIME
    if (button_widget == NULL) {
        button_widget = (ruin_Widget*)arena_alloc(&ctx->arena, sizeof(ruin_Widget));
        button_widget->id = id;
        button_widget->text = label;
        button_widget->size[RUIN_AXISX] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_TEXTCONTENT,
            .value = 0,
            .strictness = 1,
        };
        button_widget->size[RUIN_AXISY] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_TEXTCONTENT,
            .value = 0,
            .strictness = 1,
        };
        button_widget->background = make_color_hex(0xFFFFFFFF);
        button_widget->foreground = make_color_hex(0x000FFFFF);
        ctx->widgets.items[ctx->widgets.index++] = button_widget;
    };
    button_widget->text = label;

    // PUSH WIDGET TO ROOT OF THE WINDOW
    ruin_Widget* root_widget = ctx->current_window->root_widget;
    if (root_widget->first_child == NULL) {
        button_widget->first_child = NULL;
        button_widget->next_sibling = NULL;
        button_widget->prev_sibling = NULL;
        button_widget->parent = root_widget;

        root_widget->first_child = button_widget;
        root_widget->last_child = button_widget;
    } else {
        ruin_Widget* temp = root_widget->first_child;
        while (temp->next_sibling != NULL) {
            temp = temp->next_sibling;
        };

        temp->next_sibling = button_widget;
        button_widget->prev_sibling = temp;
        button_widget->next_sibling = NULL;
        button_widget->parent = root_widget;
        button_widget->first_child = NULL;
        button_widget->last_child = NULL;
        root_widget->last_child = button_widget;
    };

    return false;
};

B8 ruin_Slider(ruin_Context* ctx, const char* label, F32* from, F32* to, F32* current) {
    ruin_Id id = hash_string(label);
    ruin_Widget* slider_widget = get_widget_by_id(ctx, id);

    // ALLOCATE and CREATE BUTTON FOR THE FIRST TIME
    float value = (float)(*current - *from) / (*to - *from);
    if (slider_widget == NULL) {
        slider_widget = (ruin_Widget*)arena_alloc(&ctx->arena, sizeof(ruin_Widget));
        slider_widget->id = id;
        slider_widget->text = label;
        slider_widget->size[RUIN_AXISX] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_PARENTPERCENTAGE,
            .value = 0.75,
            .strictness = 1,
        };
        slider_widget->size[RUIN_AXISY] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_PIXEL,
            .value = 20,
            .strictness = 1,
        };
        slider_widget->background = make_color_hex(0x252525FF);
        slider_widget->foreground = make_color_hex(0x000000FF);
        ctx->widgets.items[ctx->widgets.index++] = slider_widget;

        ruin_Widget* slider_indicator = (ruin_Widget*)arena_alloc(&ctx->arena, sizeof(ruin_Widget));
        slider_indicator->id = 1234;
        slider_indicator->size[RUIN_AXISX] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_PARENTPERCENTAGE,
            .value = value,
            .strictness = 1,
        };
        slider_indicator->text = "xxx";
        slider_indicator->size[RUIN_AXISY] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_PIXEL,
            .value = 20,
            .strictness = 1,
        };

        slider_widget->last_child = slider_indicator;
        slider_widget->first_child = slider_indicator;
        slider_indicator->background = make_color_hex(0xFF00FFFF);

        slider_indicator->parent = slider_widget;
        slider_indicator->next_sibling = NULL;
        slider_indicator->prev_sibling = NULL;
    };

    slider_widget->first_child->size[RUIN_AXISX] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_PARENTPERCENTAGE,
            .value = value,
            .strictness = 1,
        };

    // PUSH WIDGET TO ROOT OF THE WINDOW
    ruin_Widget* root_widget = ctx->current_window->root_widget;
    if (root_widget->first_child == NULL) {
        slider_widget->next_sibling = NULL;
        slider_widget->prev_sibling = NULL;
        slider_widget->parent = root_widget;

        root_widget->first_child = slider_widget;
        root_widget->last_child = slider_widget;
    } else {
        ruin_Widget* temp = root_widget->first_child;
        while (temp->next_sibling != NULL) {
            temp = temp->next_sibling;
        };

        temp->next_sibling = slider_widget;
        slider_widget->prev_sibling = temp;
        slider_widget->next_sibling = NULL;
        slider_widget->parent = root_widget;
        root_widget->last_child = slider_widget;
    };

    return false;
};


B8 ruin_Button(ruin_Context* ctx, const char* label) {
    ruin_Id id = hash_string(label);
    ruin_Widget* button_widget = get_widget_by_id(ctx, id);

    // ALLOCATE and CREATE BUTTON FOR THE FIRST TIME
    if (button_widget == NULL) {
        button_widget = (ruin_Widget*)arena_alloc(&ctx->arena, sizeof(ruin_Widget));
        button_widget->id = id;
        button_widget->text = label;
        button_widget->size[RUIN_AXISX] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_TEXTCONTENT,
            .value = 0,
            .strictness = 1,
        };
        button_widget->size[RUIN_AXISY] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_TEXTCONTENT,
            .value = 0,
            .strictness = 1,
        };
        button_widget->background = make_color_hex(0x0089F7FF);
        button_widget->foreground = make_color_hex(0x000000FF);
        ctx->widgets.items[ctx->widgets.index++] = button_widget;
    };

    // PUSH WIDGET TO ROOT OF THE WINDOW
    ruin_Widget* root_widget = ctx->current_window->root_widget;
    if (root_widget->first_child == NULL) {
        button_widget->first_child = NULL;
        button_widget->next_sibling = NULL;
        button_widget->prev_sibling = NULL;
        button_widget->parent = root_widget;

        root_widget->first_child = button_widget;
        root_widget->last_child = button_widget;
    } else {
        ruin_Widget* temp = root_widget->first_child;
        while (temp->next_sibling != NULL) {
            temp = temp->next_sibling;
        };

        temp->next_sibling = button_widget;
        button_widget->prev_sibling = temp;
        button_widget->next_sibling = NULL;
        button_widget->parent = root_widget;
        button_widget->first_child = NULL;
        button_widget->last_child = NULL;
        root_widget->last_child = button_widget;
    };

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
