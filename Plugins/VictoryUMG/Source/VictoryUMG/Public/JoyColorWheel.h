/*
	JoyColorWheel by Rama
*/
#pragma once

//~~~~~~~~~~~~ UMG ~~~~~~~~~~~~~~~~
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//UE4 Color Picker
#include "SJoyColorPicker.h"

#include "JoyColorWheel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoyColorChangedEvent, const FLinearColor&, NewColor);

UCLASS()
class VICTORYUMG_API  UJoyColorWheel : public UWidget
{
	GENERATED_UCLASS_BODY()

//Color Picker Slate
protected:
	TSharedPtr<SJoyColorPicker> ColorPicker;

//~~~~~~~~~~~~~~
// BP Exposed Core
//~~~~~~~~~~~~~~
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Joy Color Wheel")
	FLinearColor JoyColor;

	/** Should the color picker jump instantly to the chosen JoyColor when it is first created? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Joy Color Wheel")
	bool bSkipAnimationOnConstruction = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Joy Color Wheel")
	bool bDisplayInlineVersion = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Joy Color Wheel")
	bool bExpandAdvancedSection = false;


	/** Called whenever the color is changed! Yay! */
	UPROPERTY(BlueprintAssignable, Category="Widget Event", meta=(DisplayName="OnColorChanged (Joy Color Wheel)"))
	FOnJoyColorChangedEvent OnColorChanged;

	/** Get Color! */
	UFUNCTION(BlueprintPure, Category="Joy Color Wheel")
	FLinearColor GetColor();

	/** Set Color Picker's Color! */
	UFUNCTION(BlueprintCallable, Category="Joy Color Wheel")
	void SetColor(FLinearColor NewColor, bool SkipAnimation = false );

//~~~~~~~~~~~~~~~~~~
// Color Picker Internal
//~~~~~~~~~~~~~~~~~~
public:
	void ColorUpdated(FLinearColor NewValue);
	void ColorPickCancelled(FLinearColor NewValue);

//~~~~~~~~~~~~~~
// UMG Component
//~~~~~~~~~~~~~~
public:

	// UVisual interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// End of UVisual interface

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

#if WITH_EDITOR
	// UWidget interface
	//virtual const FSlateBrush* GetEditorIcon() override;
	virtual const FText GetPaletteCategory() override;
	// End UWidget interface

	// UObject interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	// End of UObject interface

#endif

};
