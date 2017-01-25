// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "PauseMenuWidget.h"

void UPauseMenuWidget::Show()
{
	AddToViewport();
}

void UPauseMenuWidget::Hide()
{
	RemoveFromParent();
}



