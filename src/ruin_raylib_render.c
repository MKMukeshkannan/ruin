#ifndef RUIN_RAYLIB_RENDERER
#define RUIN_RAYLIB_RENDERER

#include "ruin.c"
#include <raylib.h>
#include <stdio.h>

static Texture2D font_texture[128];

void ruin_RaylibInit(ruin_Context* ctx) {

    ruin_CharInfo* char_info = ctx->font;

    Image img = {0};
    for (int i = 0; i < 128; ++i) {
        img.width = char_info[i].width;
        img.height = char_info[i].rows;
        img.format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
        img.mipmaps = 1;


        img.data = char_info[i].buffer;
        Texture2D texture = LoadTextureFromImage(img);

        font_texture[i] = texture;
        free(char_info[i].buffer);
    };
    img.data = NULL;
    UnloadImage(img);
};

void ruin_RaylibDrawText(ruin_Context* ctx, const char* string, float x, float y, float scale, ruin_Color color) {

    ruin_CharInfo* font = ctx->font;
    ruin_CharInfo current_char;
    size_t i = 0;

    y+=15;
    while (string[i] != '\0') {
        current_char = font[string[i]];
        float xpos = x + current_char.bearingX * scale;
        float ypos = y - current_char.bearingY * scale;
        float w = current_char.width * scale;
        float h = current_char.rows * scale;

        DrawTexturePro(font_texture[string[i]], 
                       (Rectangle) {.x=0, .y=0, .height=current_char.rows, .width=current_char.width}, 
                       (Rectangle) {.x=xpos, .y=ypos, .height=h, .width=w}, 
                       (Vector2){}, 
                       0.0f, 
                       BLACK);

        x += (current_char.advance >> 6) * scale;
        ++i;
    };
    printf("\n");
};

void ruin_RaylibRender(ruin_Context* ctx) {

    ruin_CharInfo* char_info = ctx->font;

   for (int i = 0; i < ctx->draw_queue.index; ++i) {
       switch (ctx->draw_queue.items[i].type) {
           case RUIN_DRAWTYPE_RECT: { 
               ruin_Rect rect = ctx->draw_queue.items[i].draw_rect.rect;
               ruin_Color color = ctx->draw_queue.items[i].draw_rect.color;

               DrawRectangle(rect.x, rect.y, rect.w, rect.h - rect.y, (Color) { .a=color.a, .r=color.r, .b=color.b, .g=color.g, });
               DrawRectangleLinesEx((Rectangle) { .x=rect.x, .y=rect.y, .width=rect.w, .height=rect.h - rect.y, }, 1, BLACK);

           } break;
           case RUIN_DRAWTYPE_TEXT: { 
               ruin_Vec2 pos = ctx->draw_queue.items[i].draw_text.pos;
               ruin_RaylibDrawText(ctx, ctx->draw_queue.items[i].draw_text.text, pos.x, pos.y, 1, (ruin_Color) {});
           } break;
           case RUIN_DRAWTYPE_CLIP: {  } break;
       };
   };
};


#endif
