#pragma once

#include "CoreMinimal.h"

struct FButtonStyle;
struct FSlateBrush;
struct FTextBlockStyle;
class ISlateStyle;

/**
 * 背包 Slate 的集中样式。
 *
 * 颜色、边框、按钮和字体统一从这里读取，避免业务控件在 SNew 链中散落硬编码。
 * 样式不包含任何背包数据，因此可以在未来替换主题而不影响运行时组件。
 */
class DRAGONOATH_API FDOInventoryStyle
{
public:
	static void Initialize();
	static void Shutdown();

	static const ISlateStyle& Get();
	static const FSlateBrush* GetBrush(FName BrushName);
	static const FButtonStyle& GetButtonStyle();
	static const FTextBlockStyle& GetTitleTextStyle();
	static const FTextBlockStyle& GetBodyTextStyle();

private:
	static TSharedRef<class FSlateStyleSet> CreateStyleSet();

	static TSharedPtr<class FSlateStyleSet> StyleSet;
};
