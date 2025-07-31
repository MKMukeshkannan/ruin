#ifndef RUIN_WIDGETS_H
#define RUIN_WIDGETS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ruin_core.h"

#define ruin_SameLine(ctx, label)   DeferLoop(ruin_RowBegin(ctx, label), ruin_RowEnd(ctx))

void ruin_RowBegin(ruin_Context* ctx, const char* label) ;
void ruin_RowEnd(ruin_Context* ctx);

B8 ruin_Label(ruin_Context* ctx, const char* label);
B8 ruin_SpacerX(ruin_Context* ctx);
B8 ruin_SpacerFixedX(ruin_Context* ctx, F32 space);
B8 ruin_SpacerY(ruin_Context* ctx);
B8 ruin_SpacerFixedY(ruin_Context* ctx, F32 space);
B8 ruin_Button(ruin_Context* ctx, const char* label);


#ifdef __cplusplus
}
#endif

#endif // RUIN_WIDGETS

