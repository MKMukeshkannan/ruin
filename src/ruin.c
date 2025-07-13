#ifndef RUIN_HEADER
#define RUIN_HEADER

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
typedef struct ruin_Vec2        { U32 x, y; }                                                                                                ruin_Vec2;
typedef struct ruin_Rect        { U32 x, y, h, w; }                                                                                          ruin_Rect;
typedef struct ruin_Color       { U8 r, g, b, a; }                                                                                           ruin_Color;

typedef union ruin_DrawCall {
    ruin_DrawType type;
    struct ruin_DrawRect { ruin_Rect rect; ruin_Color color; } draw_rect;
    struct ruin_DrawClip { ruin_Rect rect; } draw_clip;
    struct ruin_DrawText { char* text; } draw_text;
} ruin_DrawCall;

typedef struct ruin_Widget                                                   ruin_Widget;
struct ruin_Widget {
    ruin_Size size[RUIN_AXISCOUNT];
    ruin_Rect draw_coords;

    ruin_Widget *first_child;
    ruin_Widget *last_child;
    ruin_Widget *next_sibling;
    ruin_Widget *prev_sibling;
    ruin_Widget *parent;

    char* text;
    char flags;
    ruin_Id id;
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

    ruin_Vec2 mouse_position;
    ruin_Id hot;
    ruin_Id active;
} ruin_Context;


ruin_Context* create_ruin_context() {
    const U32 ARENA_SIZE = 4096;
    Arena arena = {0};
    unsigned char* buffer = (unsigned char*) malloc(ARENA_SIZE);
    arena_init(&arena, buffer, ARENA_SIZE);
    ruin_Context* ctx = arena_alloc(&arena, sizeof(ruin_Context));

    MEM_ZERO(ctx, sizeof(ruin_Context));
    ctx->arena = arena;
    ctx->widgets.index = 0;
    ctx->windows.index = 0;

    return ctx;
};


typedef struct {
    S16 top;
    ruin_Widget* items[100];
} ruin_WidgetStack;

internal ruin_WidgetStack* create_stack(Temp_Arena_Memory temp) {
    ruin_WidgetStack* stack = arena_alloc(temp.arena, sizeof(ruin_WidgetStack));
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
        window = arena_alloc(&ctx->arena, sizeof(ruin_Window));
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
        root = arena_alloc(&ctx->arena, sizeof(ruin_Widget));
        MEM_ZERO(root, sizeof(ruin_Widget));

        root->id = root_id;
        root->size[0] = (ruin_Size) { .kind = RUIN_SIZEKIND_PIXEL, .value = ctx->current_window->window_rect.h, .strictness=1 }; ;
        root->size[1] = (ruin_Size) { .kind = RUIN_SIZEKIND_PIXEL, .value = ctx->current_window->window_rect.w, .strictness=1 }; ;
        root->text = "root##default";

        root->draw_coords.x=ctx->current_window->window_rect.x;
        root->draw_coords.y=ctx->current_window->window_rect.y;
        root->draw_coords.h=root->size[RUIN_AXISY].value;
        root->draw_coords.w=root->size[RUIN_AXISX].value;

        ctx->widgets.items[ctx->widgets.index++] = root;
    };
    root->first_child = NULL;
    window->root_widget = root;
};

void ruin_EndWindow(ruin_Context* ctx) {
    ctx->current_window = NULL;
};



void calculate_independent_sizes(ruin_Widget* root_widget, ruin_WidgetStack* stack) {
    push(stack, root_widget);

    while (get_top(stack) != NULL) {
        ruin_Widget* current_widget = pop(stack);

        // printf("sizeof %s => %fx%f\n", current_widget->text, current_widget->size[RUIN_AXISX].value, current_widget->size[RUIN_AXISY].value);
        for (ruin_Widget* temp = current_widget->first_child; temp != NULL; temp = temp->next_sibling) {
            push(stack, temp);
        };
    };

    stack->top = 0;

};


void calculate_position(ruin_Widget* root_widget, ruin_WidgetStack* stack) { 


    for (ruin_Widget* temp = root_widget->first_child; temp != NULL; temp = temp->next_sibling) push(stack, temp);


    U32 x = root_widget->draw_coords.x, y = root_widget->draw_coords.y;
    while (get_top(stack) != NULL) {
        ruin_Widget* current_widget = pop(stack);

        current_widget->draw_coords.x = x;
        current_widget->draw_coords.y = y;
        current_widget->draw_coords.h = current_widget->size[RUIN_AXISY].value + y;
        current_widget->draw_coords.w = current_widget->size[RUIN_AXISX].value + x;

        y += current_widget->size[RUIN_AXISY].value + 1;

        //printf("coords %s =>  %d,%d,%d,%d\n", current_widget->text, current_widget->draw_coords.x, current_widget->draw_coords.y, current_widget->draw_coords.w, current_widget->draw_coords.h);

        for (ruin_Widget* temp = current_widget->first_child; temp != NULL; temp = temp->next_sibling) push(stack, temp);
    };

    stack->top = 0;
};

void generate_draw_calls(ruin_Context* ctx, ruin_Widget* root_widget, ruin_WidgetStack* stack) {
    push(stack, root_widget);
    ctx->draw_queue.index = 0;

    while (get_top(stack) != NULL) {
        ruin_Widget* current_widget = pop(stack);


        // DO MY STUFF HERE
        ctx->draw_queue.items[ctx->draw_queue.index].type = RUIN_DRAWTYPE_RECT;
        ctx->draw_queue.items[ctx->draw_queue.index].draw_rect.rect = current_widget->draw_coords;
        ctx->draw_queue.items[ctx->draw_queue.index].draw_rect.color = (ruin_Color) { .r=255, .g=255, .b=255, .a=255 };

        ctx->draw_queue.index++;


        for (ruin_Widget* temp = current_widget->first_child; temp != NULL; temp = temp->next_sibling)
            push(stack, temp);
    };

    stack->top = 0;

};

void ruin_ComputeLayout(ruin_Context* ctx) { 

    ruin_Window** window_list = ctx->windows.items;

    Temp_Arena_Memory temp_mem = temp_arena_memory_begin(&ctx->arena);
    ruin_WidgetStack* widget_stack = create_stack(temp_mem);


    for (size_t i = 0; i < ctx->windows.index; ++i) {



        calculate_independent_sizes(ctx->windows.items[i]->root_widget, widget_stack);
        calculate_position(ctx->windows.items[i]->root_widget, widget_stack);
        generate_draw_calls(ctx, ctx->windows.items[i]->root_widget, widget_stack);
        



    };

    temp_arena_memory_end(temp_mem);


};

void ruin_SameLine() { };

B8 ruin_Button(ruin_Context* ctx, char* label) {
    ruin_Id id = hash_string(label);
    ruin_Widget* button_widget = get_widget_by_id(ctx, id);

    if (button_widget == NULL) {
        button_widget = arena_alloc(&ctx->arena, sizeof(ruin_Widget));
        button_widget->id = id;
        button_widget->text = label;
        button_widget->size[RUIN_AXISX] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_PIXEL,
            .value = 100,
            .strictness = 1,
        };
        button_widget->size[RUIN_AXISY] = (ruin_Size) {
            .kind = RUIN_SIZEKIND_PIXEL,
            .value = 28,
            .strictness = 1,
        };

        ctx->widgets.items[ctx->widgets.index++] = button_widget;
    };

    ruin_Widget* root_widget = ctx->current_window->root_widget;
    if (root_widget->first_child == NULL) {
        root_widget->first_child = button_widget;
    } else {

        ruin_Widget* temp = root_widget->first_child;
        while (temp->next_sibling != NULL) {
            temp = temp->next_sibling;
        };

        button_widget->prev_sibling = temp;
        button_widget->next_sibling = NULL;
        button_widget->parent = root_widget;
        temp->next_sibling = button_widget;
    };

    return false;
};

static B8 is_point_over_rect(ruin_Rect rect, ruin_Vec2 point);
static ruin_Rect overlap_rect(ruin_Rect rect1, ruin_Rect rect2);
static ruin_Rect expand_rect(ruin_Rect rect, int n);

ruin_Rect make_rect(U32 x_pos, U32 y_pos, U32 height, U32 width) {
  ruin_Rect res;
  res.x = x_pos;
  res.y = y_pos;
  res.h = height;
  res.w = width;

  return res;
};

ruin_Vec2 make_vec2(U32 x_pos, U32 y_pos) {
  ruin_Vec2 res;
  res.x = x_pos;
  res.y = y_pos;

  return res;
};

ruin_Color make_color(U8 red, U8 green, U8 blue, U8 alpha) {
  ruin_Color res;
  res.r = red;
  res.b = blue;
  res.g = green;
  res.a = alpha;

  return res;
};

#endif
