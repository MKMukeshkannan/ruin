#ifndef RUIN_RAYLIB_RENDERER
#define RUIN_RAYLIB_RENDERER

#include "base.h"
#include "ruin_core.h"
#include <raylib.h>
#include <stdio.h>
#include "stdlib.h"

typedef struct {
    ruin_FontID font_id;
    Texture2D textures[128];
} FontTextures;
DEFINE_ARRAY_CACHES(FontTextures);

static FontTexturesArray g_raylib_font_textures;

void ruin_RaylibUpdateIO(ruin_Context* ctx) {
   ClearBackground(WHITE);
   Vector2 m_pos = GetMousePosition();
   ctx->mouse_position = (ruin_Vec2) { .x = m_pos.x, .y = m_pos.y };

   if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { ctx->mouse_action |= RUIN_MOUSE_BUTTON_CLICK_LEFT; };
   if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) { ctx->mouse_action &= ~RUIN_MOUSE_BUTTON_CLICK_LEFT; };

   if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) { ctx->mouse_action |= RUIN_MOUSE_BUTTON_CLICK_RIGHT; };
   if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) { ctx->mouse_action &= ~RUIN_MOUSE_BUTTON_CLICK_RIGHT; };

   if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) { ctx->mouse_action |= RUIN_MOUSE_BUTTON_CLICK_MIDDLE; };
   if (IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE)) { ctx->mouse_action &= ~RUIN_MOUSE_BUTTON_CLICK_MIDDLE; };

   if (IsMouseButtonPressed(MOUSE_BUTTON_SIDE)) { ctx->mouse_action |= RUIN_MOUSE_BUTTON_CLICK_EXTRA_1; };
   if (IsMouseButtonReleased(MOUSE_BUTTON_SIDE)) { ctx->mouse_action &= ~RUIN_MOUSE_BUTTON_CLICK_EXTRA_1; };

   if (IsMouseButtonPressed(MOUSE_BUTTON_EXTRA)) { ctx->mouse_action |= RUIN_MOUSE_BUTTON_CLICK_EXTRA_2; };
   if (IsMouseButtonReleased(MOUSE_BUTTON_EXTRA)) { ctx->mouse_action &= ~RUIN_MOUSE_BUTTON_CLICK_EXTRA_2; };

   // printf("\n==IO EVENTS==\n");
   // printf("mouse position:\n\t%f, %f\n", m_pos.x, m_pos.y);
   // printf("mouse click:\n\txxxxxrml\n\t"); for (int i = 6; i >= 0; i--) putchar((ctx->mouse_action & (1 << i)) ? '1' : '0'); printf("\n");
   // printf("prev window:\n\t"); printf("%llu\n", ctx->prev_active_window);
   // printf("active window:\n\t"); printf("%llu\n", ctx->active_window);
   // printf("\n=============\n\n");
};

void ruin_RaylibInit(ruin_Context* ctx) {
    ruin_FontInfoArray* fonts = &ctx->fonts;
    g_raylib_font_textures = FontTexturesArray__Init(&ctx->arena, fonts->capacity);

    for (size_t font_idx = 0; font_idx < fonts->index; ++font_idx) {
        ruin_Bitmap *bitmap = ruin_FontInfoArray__Get(fonts, font_idx)->bitmap;
        Image img; 
        FontTextures raylib_single_font_textures;
        for (size_t c = 0; c < 128; ++c) {

            img.data = bitmap[c].buffer;
            img.width = bitmap[c].width;
            img.height = bitmap[c].rows;
            img.mipmaps = 1;
            img.format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;

            raylib_single_font_textures.textures[c] = LoadTextureFromImage(img);
        };
        raylib_single_font_textures.font_id = font_idx;
        FontTexturesArray__Push(&g_raylib_font_textures, raylib_single_font_textures);
        UnloadImage(img);
    };

};

#ifdef USE_FREETYPE
void ruin_RaylibDrawText(ruin_Context* ctx, const char* string, float x, float y, float scale, ruin_Color color, ruin_FontID font_id) {
    ruin_FontInfo* font = ruin_FontInfoArray__Get(&ctx->fonts, font_id);
    ruin_Bitmap current_char;
    size_t i = 0;
    
    y += font->font_size;
    while (string[i] != '\0') {
         current_char = font->bitmap[string[i]];
         float xpos = x + current_char.bearingX * scale;
         float ypos = y - current_char.bearingY * scale;
         float w    = current_char.width * scale;
         float h    = current_char.rows * scale;

           DrawTexturePro(FontTexturesArray__Get(&g_raylib_font_textures, font_id)->textures[string[i]], 
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
                if (ctx->draw_queue.items[i].draw_info_union.draw_text.text != NULL)
                    ruin_RaylibDrawText(ctx, ctx->draw_queue.items[i].draw_info_union.draw_text.text, pos.x, pos.y, 1, (ruin_Color) {}, ctx->draw_queue.items[i].draw_info_union.draw_text.font_id);
             #else
               DrawText(ctx->draw_queue.items[i].draw_info_union.draw_text.text, pos.x, pos.y, 16, BLACK);
             #endif
           } break;
           case RUIN_DRAWTYPE_CLIP: {  } break;
       };
   };


   ctx->draw_queue.index = 0;

   EndDrawing();
};


#endif
