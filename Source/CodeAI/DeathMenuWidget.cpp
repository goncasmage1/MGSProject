// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "DeathMenuWidget.h"

void UDeathMenuWidget::Show()
{
	AddToViewport();
}

void UDeathMenuWidget::Hide()
{
	RemoveFromParent();
}
