#ifndef RUIN_RAYLIB_RENDERER
#define RUIN_RAYLIB_RENDERER

#include "base.h"
#include "ruin_core.h"
#include <raylib.h>
#include <stdio.h>
#include "stdlib.h"


typedef struct {
    String8 font_name;
    Texture2D textures[128];
} FontTextures;
DECLARE_ARRAY(fonts_textures, FontTextures);
DEFINE_ARRAY(fonts_textures, FontTextures);

static FontTexturesArray g_raylib_font_textures;

void ruin_RaylibInit(ruin_Context* ctx) {
    ruin_FontInfoArray* fonts = ctx->fonts;
    g_raylib_font_textures.capacity = fonts->capacity;
    g_raylib_font_textures.index = 0;
    g_raylib_font_textures.items = (FontTextures*)arena_alloc(&ctx->arena, sizeof(FontTextures));

   for (int i = 0; i < fonts->index; ++i) {
       Image img = {0};
       for (int c = 0; c < 128; ++c) {
            ruin_Bitmap* bmp = &fonts->items[i].bitmap[c];

            img.width = bmp->width;
            img.height = bmp->rows;
            img.format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
            img.mipmaps = 1;

            img.data = bmp->buffer;
    
            Texture2D tex = LoadTextureFromImage(img);
            UnloadImage(img); 

           g_raylib_font_textures.items[i].font_name = fonts->items[i].font_name;
           g_raylib_font_textures.items[i].textures[c] = tex;
       };
   };
};

#ifdef USE_FREETYPE
void ruin_RaylibDrawText(ruin_Context* ctx, const char* string, float x, float y, float scale, ruin_Color color) {
    ruin_FontInfoArray* fonts = ctx->fonts;
    ruin_Bitmap current_char;
    size_t i = 0;
    
    // TODO: ::font:: make it variable for all font sizes
    y += fonts->items[0].font_size;
    while (string[i] != '\0') {
        current_char = fonts->items[0].bitmap[string[i]];
        float xpos = x + current_char.bearingX * scale;
        float ypos = y - current_char.bearingY * scale;
        float w    = current_char.width * scale;
        float h    = current_char.rows * scale;

        DrawTexturePro(g_raylib_font_textures.items[0].textures[string[i]], 
                       (Rectangle) {.x=0, .y=0, .width=(float)current_char.width, .height=(float)current_char.rows}, 
                       (Rectangle) {.x=xpos, .y=ypos, .width=w, .height=h}, 
                       (Vector2){}, 
                       0.0f, 
                       BLACK);

        x += (current_char.advance >> 6) * scale;
        ++i;
    };
};
#endif

void ruin_RaylibRender(ruin_Context* ctx) {

   BeginDrawing();
   // ruin_CharInfo* char_info = ctx->font;

   for (int i = 0; i < ctx->draw_queue.index; ++i) {
       switch (ctx->draw_queue.items[i].type) {
           case RUIN_DRAWTYPE_RECT: { 
               ruin_Rect rect = ctx->draw_queue.items[i].draw_info_union.draw_rect.rect;
               U8 thick = ctx->draw_queue.items[i].draw_info_union.draw_rect.border_width;
               ruin_Color color = ctx->draw_queue.items[i].draw_info_union.draw_rect.color;
               DrawRectangle(rect.x, rect.y, rect.w - rect.x, rect.h - rect.y, (Color) { .r=color.r, .g=color.g, .b=color.b, .a=color.a });
               DrawRectangleLinesEx((Rectangle) { .x=rect.x, .y=rect.y, .width=rect.w - rect.x, .height=rect.h - rect.y, }, thick, BLACK);
           } break;
           case RUIN_DRAWTYPE_TEXT: { 
               ruin_Vec2 pos = ctx->draw_queue.items[i].draw_info_union.draw_text.pos;
             #ifdef USE_FREETYPE
               ruin_RaylibDrawText(ctx, ctx->draw_queue.items[i].draw_info_union.draw_text.text, pos.x, pos.y, 1, (ruin_Color) {});
             #else
               DrawText(ctx->draw_queue.items[i].draw_info_union.draw_text.text, pos.x, pos.y, 16, BLACK);
             #endif
           } break;
           case RUIN_DRAWTYPE_CLIP: {  } break;
       };
   };

   // ruin_RaylibDrawText(ctx, "Hello", 0, 0, 1, (ruin_Color) {});

   ctx->draw_queue.index = 0;

   EndDrawing();
};


#endif
